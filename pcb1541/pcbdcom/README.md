# pcbdcom — PCB DOS COM (unified serial layer)

> **Name:** `pcbdcom` = **PCB** **D**OS **COM**. Locked in. An
> earlier planning doc (`pcb1541/pcbcomm/README.md`) proposed
> `pcbcomm`; that doc is historical and preserved only as design
> record. Canonical name everywhere: **pcbdcom**. Binaries:
> `PCBDCOM.EXE`, `PCBDCOM.SYS`, `PCBDCOM.OBJ`, config: `PCBDCOM.CFG`.

> **Location:** `pcb154/pcbdcom/` is the canonical home. 15.41 line
> references the same tree via build-time include path (no source
> duplication).

Drop-in replacement for WCSC COMM-DRV. Matches the `ser_rs232_*` API
that PCBoard's `MODEMDRV.C` calls into (see `GAP-ANALYSIS.md` for the
full interface derived from `INSTALL.DAT` and `MODEMDRV.C`), plus
standard FOSSIL INT 14h dispatch and COMM-DRV's own extensions at
AH ≥ 0x10.

**See `GAP-ANALYSIS.md`** — Phase 1 discovery from plain-text sources
(no reverse engineering). Confirms 8th card is Arnet SmartPort Plus,
lists all 9 COMMDRV .DRV modules WCSC shipped, and captures the
full `ser_rs232_*` API PCBoard calls.

## COMM-DRV was optional

INSTALL.DAT line 109: "The First Time Installation does *not* automatically
install COMM-DRV, PPL, or PCBMail. They must be selected manually if desired."

It is install group 'c'. Ships on disk 1 as COMMDRV.RED, installs to COMMDRV\.
A stock PCBoard install has no COMM-DRV at all. Never load-bearing — which is
why replacing it costs nothing in compatibility.

## What it did

Port multiplexer / hardware abstraction for multiport serial cards. A 16-node
board needs 16 ports; the BIOS knows four. Intelligent boards (Arnet SmartPort
Plus, DigiBoard COM/Xi — both named in PCBoard docs as /M version hardware)
have an onboard CPU and dual-ported RAM and NO UART chips. Hence DRVSETUP's
"Card Segment" column: memory-mapped, not I/O-mapped.

DRVSETUP screen, for interface compatibility:
  Port Number | Card Type | Sub-Port | Base Address | IRQ | Card Segment | FOSSIL
  16 ports. F1 help, F2 edit, Alt-I insert, Alt-D delete, Alt-R repeat,
  PgDn/PgUp paging, ESC exit.

## pcbcomm design

One abstraction, pluggable backends:
  UART 16550            have it (serial.c)
  FOSSIL INT 14h        have it (serial.c)
  Win32 / POSIX tty     have it (serial.c)
  Multiport dumb        new — Boca 16, banked 16550s
  Multiport intelligent new — Digi / Arnet shared-memory interface
  TCP socket            15.41 only, #ifdef-gated out of 15.4

Port table maps node -> backend + params, matching DRVSETUP's layout so
sysop-facing config stays familiar.

## TSR: 15.4 yes, 15.41 optional

Sysops expect a TSR. It is how the driver has always worked, it is what their
CONFIG.SYS and BOARD.BAT are built around, and one resident copy serves
several nodes.

  15.4   ships pcbcomm as a loadable TSR hooking INT 14h. Familiar shape,
         familiar config, drop-in for anyone who ran COMM-DRV. Serial only.
  15.41  additionally offers a linked-in build for sysops who want the
         conventional memory back and simpler failure modes. Adds TCP.

Same source, two link targets.

## References

  Digi ClassicBoard spec: ftp1.digi.com/support/utilities/9200282B.doc
    Implements BOTH DigiBoard and Arnet interrupt modes — IRQ Status Reg A/B
    (DigiBoard/StarGate), Reg C (Arnet), mode select at offset 03. One backend
    covers both vendors. UART clock 1.8432 -> 7.3728 MHz at offset 04 for
    460.8K baud.
  FreeBSD digi driver — BSD licence, PC/Xe and PC/Xi, polling-based.
  Linux epca / Digi drivers — GPL, same hardware families.

## Licensing

The free wcscnet.com download is COMM-DRV/Lib, the Windows library, and the
page states it does not include source. Free of charge, not open source, and
not the DOS driver Clark shipped. Writing our own against published hardware
specs is the correct route and is unencumbered.

## SDK — two artifacts, not one

The circularity ("an SDK to compile pcbcomm?") is a naming problem. Clark
already separated the layers, in Feb 1994:

  Toolkit3/PCBKIT_S.ZIP contains
    12726  1994-02-15 17:53   COMMDRV.OBJ
    11762  1994-02-15 17:53   FOSSIL.OBJ

Same timestamp — added as a pair, 18 months after the other stub objects.
The serial backend was ALREADY a link-time choice. A door links COMMDRV.OBJ
or FOSSIL.OBJ into the same slot; the API above is identical either way.

So:
  PCBCOMM.EXE / .SYS   the driver, resident TSR in 15.4   (was COMMDRV.EXE/COMMTSR.EXE)
  PCBCOMM.OBJ          link-time client stub              (was COMMDRV.OBJ)
  PCBCOMM.H            API header                         (was folded into PCBTOOLS.H)

The SDK does not compile pcbcomm. pcbcomm builds from its own sources; the
SDK consumes it by supplying the .OBJ that drops into the existing slot.
Existing doors relink with a one-line .PRJ change and no source edits.

### Link-out idiom

Appendix D of the toolkit docs: a hello-world door against the full library
is 49K, so Clark shipped stub objects (NOCHAT.OBJ, NOHELP.OBJ, NOSCREEN.OBJ,
NOSHELL.OBJ, NOTXT.OBJ, ...) that satisfy symbols with empty bodies. List the
ones you don't need before the .LIB and the linker takes the stubs.

pcbcomm backends follow this: link only the backend you use. Multiport code
costs nothing on a single-modem board.

### Constraints for a drop-in PCBCOMM.OBJ

  Memory models: S/M/C/L across three compilers (PCBKIT_*, PCBKBC_*,
  PCBKMS_*) = 12 variants. Our OpenWatcom flat build is a 13th target,
  not a replacement.

  Calling convention: Pascal, not C. Callee cleans the stack (chosen for
  code size). Names case-fold to uppercase. Must match or nothing links.

## Crew

  pcbcomm serial core, UART + FOSSIL backends    kiddo, wrench
  Multiport backends (Digi, Arnet, Boca)         evga
  PCBDraw TCP teleconference                     sysop/0
  SDK packaging, memory-model matrix, docs       hexadecimal


## Roadmap / status

**Next up: remake pcbcomm (right after the IC reconstruction).** This is
the unified serial/comm layer that everything else leans on — the point
where a caller's connection (UART, telnet/FOSSIL, or now SSH) is bridged
into PCBoard's input/output. Both the IC work and the new SSH front end
(pcb1541/dropbear/) hand their sessions through this layer, so remaking
pcbcomm cleanly unblocks both:

- UART 16550 — native serial (serial.c)
- FOSSIL — via netfosdl (drivers/)
- telnet — via netmodem2irc
- SSH — via Dropbear (pcb1541/dropbear/), terminating the encrypted
  session and bridging it in exactly like the telnet path

The remake should present one backend-agnostic session interface so
adding SSH beside telnet is a backend, not a special case.

After pcbcomm: **archivers / unarchivers** (see section 8 of
todo/PCB1541_DRAFT.md) — extend the 4 hardcoded slots (ZIP/ARJ/
ARC/LZH) to modern formats (RAR/7Z/TGZ/TAR), following Mystic's
ARCHIVE.DAT data-driven model. The built-in format sniffers already
live in pcb153/SOURCE/SUPPORT/DIZ.C (ZIP/ARJ/LZH/ARC magic-number
detection) — a natural starting point.

## Related: network client/server

The PCBoard network client/server + teleconference design lives with
pcbis (`pcb1541/pcbis/CLIENT-SERVER-DESIGN.md`). pcbcomm's role there is
the transport layer — TCP / serial / telnet / SSH backends feeding the
one session interface the server uses.

## Building v1

```
cd PCBDCOM
make -f PCBDCOM.MAK CC=BC31       # Borland C++ 3.1
make -f PCBDCOM.MAK CC=MSC70      # Microsoft C 7.0
wmake -f PCBDCOM.MAK CC=OWC       # OpenWatcom 1.9
```

Produces `PCBDCOM.EXE` (TSR) and `PCBDCOM.SYS` (device driver) from
the same source tree.

## Sample config

See `PCBDCOM.CFG.sample` for the config file format. Copy to
`PCBDCOM.CFG` and edit for your hardware.

## Status — v1 progress

Fully implemented:
- 8250/16550 UART (uart.c + uart_backend.c)
- Boca dumb multi-port (boca_backend.c)
- 8259 PIC + shared-IRQ dispatcher (irq.c)
- INT 14h FOSSIL handler (int14.c)
- PCBDCOM.CFG parser + dual-mode entry (pcbdcom.c)
- Build system (PCBDCOM.MAK)

Fully ported (v1, single-chip config):
- Cyclades Cyclom-Y (cyclom_backend.c) — CD1400 register access,
  channel init, cy_interrupt() SVRR walk with RX/TX/modem service
  dispatch. Ported from Linux cyclades.c. Multi-chip wiring in
  pcbdcom.c is v1.1 work — see TODO in cyclom_backend.c.

Fully ported (v1, single-card config):
- DigiBoard PC/Xe (digi_pcxe_backend.c) — thin probe/init over
  shared FEP layer.
- DigiBoard AccelePort (digi_accel_backend.c) — thin probe/init
  over shared FEP layer; card-type check (ACCELE_ID, PCXEM_ID,
  EISAXEM_ID, PCIXEM_ID, PCIXR_ID) + up to 64 ports.

Shared implementation (both Digi cards):
- digi_fep.c + digi_fep.h — FEP command queue (fepcmd), event queue
  drain, board_chan struct access, per-channel init, ISR, write path.
  Ported from Linux epca.c (2.6.32, GPLv2).

Fully ported (v1, single-card config):
- Comtrol RocketPort (rocket_backend.c) — MUDBAC controller + AIOP
  enum, per-channel init via 18-tuple indexed-register writes
  (rp_init_data[] from Linux RData[]), baud programming, IRQ-mode
  ISR that walks _INT_CHAN on each AIOP to service RX/TX FIFOs.
  Ported from Linux rocket.c (POLLED mode there → IRQ mode here).

Fully ported (v1, single-card config):
- Stallion EasyIO (easyio_backend.c) — surprise: uses CD1400 UARTs
  (same chip family as Cyclades) but I/O-mapped instead of memory-
  mapped. Board detect via EIO_IDBITMASK (4RS / 8DI / 8RS / 8M /
  MK3 revision). ISR polls board status EIO_INTRPEND, then walks
  CD1400 SVRR same as cyclom. Ported from Linux stallion.c.

**pcbdcom v1.1 is code-complete.** All 7 backends fully ported AND
multi-port wired (parse_config now uses backend->card_get() hook +
per-port subport index). Every smart-card backend has a static card
pool (max 4 cards per backend type) with shared card_pool_get()
helper that groups config lines sharing a card_addr into one card
record. Each port's subport (0..N-1 within card) selects the correct
chip/channel.


Each skeleton captures the essential card-detection registers so
PCBDCOM.CFG validates hardware presence at load. Full ISRs get
filled in next pass, one card at a time, with the Linux source
open side-by-side.

**pcbdcom v1.2 SHIPPED 2026-09-01.** All 5 planned features landed:
Arnet backend (8th card), ser_rs232 shim (13-fn COMMDRV.OBJ
replacement), INT 14h AH>=0x10 extensions, `_dos_keep()` TSR install
fix, and full SDK packaging in `toolkit/pwa154/pcbdcom/`. Total:
3,829 lines GPLv3, 15 .c + 6 .h files, 8 backends. OpenWatcom
verified clean build; PCBDTSR.EXE = 37,800 bytes.

**pcbdcom v1.4 SHIPPED 2026-09-03 (refined).** Three post-WCSC
intelligent multiport backends, all `#if defined(PCB1541)`-gated —
they ship only in 15.41 builds. 15.4 stays lean at WCSC-parity.

  Stallion Brumby/ONboard  -> stallion_brumby_backend.c  (15.41 only)
  Chase Research IOLAN     -> chase_iolan_backend.c      (15.41 only)
  Equinox SST-8/16/32/64   -> equinox_sst_backend.c      (15.41 only)

**Build for 15.41 (extended):**  `wmake -f PCBDCOM.MAK CC=OWC TARGET=15.41`
**Build for 15.4 (default):**    `wmake -f PCBDCOM.MAK CC=OWC`

These backends are written from public documentation without hardware
in the pcbirc lab for validation. Sysops with matching hardware should
test and report. Header comments in each backend cite the public
references used. When PCB1541 is not defined, they compile to empty
translation units — no symbols leak into 15.4.

**pcbdcom v1.3 SHIPPED 2026-09-03.** Full WCSC-DOS card parity
reached. All 8 DOS card families in WCSC's COMMDRV.RED now have a
matching pcbdcom backend:

  COMMDV00  GENERIC     -> uart_backend.c
  COMMDV01  INTEL HUB6  -> hub6_backend.c        (v1.3 session A)
  COMMDV02  DIGI-COMXI  -> digi_comxi_backend.c  (v1.3 session B)
  COMMDV03  ARNET-SPORT -> arnet_backend.c       (v1.2)
  COMMDV04  BOCA(1610)  -> boca_backend.c        (v1.1)
  COMMDV05  DIGI-PCX*   -> digi_pcxe_backend.c   (v1.1)
  COMMDV06  GTEK(8Fx)   -> gtek_backend.c        (v1.3 session B)
  COMMDV07  INT14H      -> int14.c               (v1.1)

Plus 4 backends for post-WCSC cards (Cyclades Cyclom-Y, Digi
AccelePort, Comtrol RocketPort, Comtrol EasyIO). Total: 10 card
backends, 4,275 lines GPLv3.
