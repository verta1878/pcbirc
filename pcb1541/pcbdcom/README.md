# pcbcomm — PCB COMM (unified serial layer)

> **Name:** `pcbcomm` = **PCB** **COMM**. Locked in. An
> earlier planning doc (`pcb1541/pcbcomm/README.md`) proposed
> `pcbcomm`; that doc is historical and preserved only as design
> record. Canonical name everywhere: **pcbcomm**. Binaries:
> `PCBCOMM.EXE`, `PCBCOMM.SYS`, `PCBCOMM.OBJ`, config: `PCBCOMM.CFG`.

> **Location:** `pcb1541/pcbcomm/` is the canonical home -- THIS tree.
> The 15.3 and 15.4 lines reference it via build-time include path; no
> source duplication.
>
> Corrected 2026-09-22. This paragraph previously said
> `pcb154/pcbcomm/`, and that folder has never existed. Anything written
> against the old claim -- notably the FOSSIL 5C status note in
> `pcbcomm-fossil5c.zip`, which cites `pcb154/pcbcomm/src/int14.c` -- was
> following the README, not making a mistake. Those paths become correct
> once they are re-pointed at `pcb1541/pcbcomm/`.

> **Why there is no merge (clarified 2026-09-25).** The two trees are
> not a fork to reconcile -- they are two *roles* of one codebase, and
> both are meant to exist:
>
> * `pcb1541/pcbcomm/` is the **source-code home**. This is where the
>   driver is developed: every backend `.c`, the `ref/linux/` study
>   material, the gap analysis, the research backends. Development happens
>   here.
> * `toolkit/pwa154/pcbcomm/` is the **redistributable SDK** -- the tree
>   that ships the linkable `.OBJ`. Its reason to exist is the link matrix
>   (`PCBCOMM_BL.OBJ`, `_7L`, `_WL`, ... one built object per compiler x
>   memory model) plus the drop-in recipe, headers for consumers, and
>   examples. A sysop or door author never compiles this tree; they link
>   its prebuilt `.OBJ` into `PCBOARD.MAK` in place of Clark's
>   `COMMDRV.OBJ`. See `toolkit/pwa154/pcbcomm/docs/SDK.md` and
>   `LINKOUT.md`.
>
> So the relationship is source -> built artifact, the same split Clark
> himself shipped (`COMMDRV.OBJ` + `FOSSIL.OBJ` as link-time objects, see
> "SDK -- two artifacts, not one" below). Consolidating them into one tree
> would defeat the point: the SDK tree is deliberately a lean, consumer-
> facing cut, not a second development copy.
>
> The only thing that genuinely *moves between* the two is the COMM-DRV
> shim and its header (`ser_rs232_shim.c` + the 6,422-byte `pcbcomm.h`,
> decided below) -- the source of that shim lands in `pcb1541`, and the
> SDK tree builds its `.OBJ` variants from it. That is a normal
> source->artifact hand-off, not a merge.
>
> One narrow open item remains, and it is a *check*, not a merge: confirm
> the files that differ between the trees (`int14.c`, `inc/pcbcomm.h`) are
> deliberate lean SDK cuts and not stale drift that fell behind `pcb1541`.
> If deliberate, nothing to do; if drift, re-cut the SDK copy from source.

> **Second tree, historical folding note:** `toolkit/pwa154/pcbcomm/`
> exists, 31 files against this tree's 53. Neither is a superset. The
> file-by-file comparison below (2026-09-22) predates the clarification
> above -- it still reads as a "merge audit," but per that note the right
> framing is: which unique files are SDK-only, which are source-only, and
> which single shim+header pair hands off from source to SDK. Compared
> 2026-09-22:
>
> * Only here (30 files): `SPEC.md`, `GAP-ANALYSIS.md`,
>   `BUILD-STATUS.md`, `PCBCOMM.MAK`, `PCBCOMM.CFG.sample`, the whole
>   `ref/linux/` reference set, and six backends the other tree lacks --
>   chase_iolan, digi_comxi, equinox_sst, gtek, hub6, stallion_brumby.
> * Only there (8 files): `docs/SDK.md`, `docs/LINKOUT.md`,
>   `examples/{simple,multiport,tsrless}.c`, `lib/README.md`,
>   `lib/NOPCBCOMM.README`, `src/nopcbcomm_stub.c`.
> * In both but different: `README.md`, `inc/backend.h`,
>   `inc/pcbcomm.h`, `src/pcbcomm.c`, `src/ser_rs232_shim.c`.
>
> **Decided:** the COMM-DRV shim from `toolkit/pwa154/pcbcomm/` is the
> one to keep. Its `src/ser_rs232_shim.c` uses the shared header instead
> of declaring a private `struct port_param`, wires `pp->opcb` and
> `pp->auxpcb` to the embedded `compat_opcb` / `compat_auxpcb` blocks,
> refreshes them through `update_opcb()`, and fills `lngth`, `cardtype`,
> `protocol`, `outbuf_len` and `block[]`. Its `inc/pcbcomm.h` (6,422
> bytes) declares `port_param`, `opcb_type`, `auxpcb_type` and the
> `ser_rs232_*` prototypes; the copy here (1,849 bytes) declares none of
> them. **Both files have to move together** -- the kept shim will not
> compile against this tree's smaller header.
>
> The draft shim that was here is retired to
> `attic/superseded-pcbcomm/`. Until the merge lands, this tree has no
> shim; the working one is in `toolkit/pwa154/pcbcomm/src/`.

> **The kept shim still needs four fixes**, all established against
> `MODEMDRV.C` and against the `MODEMDRV.OBJ` built from it on
> 2026-09-22:
>
> 1. **Signatures.** Every `unsigned int port` / `unsigned int n` /
>    `unsigned int which` must become `int`. PCBoard compiles with `-P`,
>    so TLINK matches mangled names: the object asks for
>    `@SER_RS232_GETPACKET$QIINUC` -- `(int,int,unsigned char *)`.
>    `unsigned int` mangles differently and the link fails outright.
>    Thirteen declarations, in the shim and in `inc/pcbcomm.h`.
> 2. **Port numbering is off by one.** `port_by_num()` rejects 0 and
>    indexes `g_ports[port-1]`. `MODEMDRV.C` line 421 passes
>    `Asy.ComPortNumber - 1`, so COM1 arrives as 0 and every call
>    returns `RS232ERR_PARAM`. Accept `0 .. g_n_ports-1` and index
>    directly.
> 3. **`putpacket(port, 0, NULL)` must kick the transmitter.**
>    `COMMDRV_turnonxmit()` calls it for that and nothing else; it
>    currently returns success without acting.
> 4. **DTR/RTS must go through the backend.** The shim writes the MCR
>    directly with `outp(p->base + 4, ...)`, which is right for a UART
>    and wrong for Digi FEP, Cyclom, Stallion and RocketPort -- the same
>    class of bug already fixed in `int14.c`'s `status_word()`.
>
> Two call conventions the shim already has right, recorded so they are
> not "tidied" later: `getpacket(Port, 0, buf)` exists only to make the
> card refresh `opcb` -- the buffer is scratch, not an output -- and
> `getpacket(Port, 32767, NULL)` must refresh the counters and leave the
> data alone, because `COMMDRV_inbytes()` reads
> `pcb.opcb->inbuf_count` on the very next line.

> **The interface header now exists.** `pcbcbase/COMMDRV/H/COMM.H`,
> written 2026-09-22, is the reconstructed COMM-DRV SDK header --
> 13 functions, `port_param` / `opcb_param` / `auxpcb_param`, and the
> constants. `MODEMDRV.C` compiles clean against it in both the PCBOARD
> and the door-SDK (`-DLIB`) flavours. It is where `PCBOARD.MAK` line 64
> looks, and it should become the single source of truth for the ABI:
> `inc/pcbcomm.h` should include it rather than redeclare those structs.
> See `APPLY.txt`, "COMM-DRV SDK HEADER RECOVERED".

> **Where the three pieces live** -- one header, shared source, one
> library per compiler:
>
> | Piece | Path | Shared? |
> |---|---|---|
> | interface header | `pcbcbase\COMMDRV\H\COMM.H` | yes, one copy for every branch |
> | library source | `toolkit\pwa154\pcbcomm\src\pcbcomm.c` + the backends beside it, folding into `pcb1541\pcbcomm\src\` | yes, one copy |
> | built library | `pcbcbase\commdrv\lib\COMMDRBL.LIB` (with `LIBSBL.LIB`) | **no -- one per compiler** |
>
> The header is shared because neither `pcb153\153\PCBOARD.MAK` (line
> 64) nor `pcb154\MAIN\153\PCBOARD.MAK` (line 43) defines `LIBSDIR`
> itself -- it comes from the environment, where `BLDDOS.BAT` sets
> `LIBSDIR=\PCBCBASE`. Both add `$(LIBSDIR)\COMMDRV\H` to the include
> path, and both link `$(LIBSDIR)\commdrv\lib\commdrbl.lib` at lines
> 820-821. `ZMODEM.MAK` uses the same two paths.

> **Delta / OpenWatcom.** The Delta 15.4 leg does not build the COMM-DRV
> backend at all today: `pcb154\MAIN\WATCOM\PCBOARD.MK` and
> `pcb154\MAIN\153\PCBWAT2.MK` are three lines each and name neither
> `COMMDRV` nor `MODEMDRV` nor `comm.h`, even though
> `pcb154\MAIN\SOURCE\MODEM\MODEMDRV.C` is present. So COMM-DRV is a
> Borland-only path right now. When the Watcom leg is fleshed out:
>
> 1. **`COMM.H` needs compiler guards.** It uses `far` and `LIBENTRY`,
>    which are Borland/MSC spellings; OpenWatcom wants `__far`. Guard
>    them the way `pcbcomm`'s own `compat.h` already guards the
>    interrupt keywords.
> 2. **The library cannot be shared, only the source.** `COMMDRBL.LIB`
>    is a Borland large-model OMF library; Watcom has different name
>    mangling and calling conventions. Each branch builds its own from
>    the same `ser_rs232_shim.c`.
> 3. **The name check is per compiler.** PCBoard compiles with `-P`, so
>    TLINK matches mangled names: the BC 3.1 `MODEMDRV.OBJ` asks for
>    `@SER_RS232_GETPACKET$QIINUC`. Watcom will emit its own spelling.
>    The source fix is the same either way -- port and count parameters
>    must be `int`, not `unsigned int` -- but the verification has to be
>    redone against that branch's own `MODEMDRV.OBJ`.

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
cd PCBCOMM
make -f PCBCOMM.MAK CC=BC31       # Borland C++ 3.1
make -f PCBCOMM.MAK CC=MSC70      # Microsoft C 7.0
wmake -f PCBCOMM.MAK CC=OWC       # OpenWatcom 1.9
```

Produces `PCBCOMM.EXE` (TSR) and `PCBCOMM.SYS` (device driver) from
the same source tree.

## Sample config

See `PCBCOMM.CFG.sample` for the config file format. Copy to
`PCBCOMM.CFG` and edit for your hardware.

## Status — v1 progress

Fully implemented:
- 8250/16550 UART (uart.c + uart_backend.c)
- Boca dumb multi-port (boca_backend.c)
- 8259 PIC + shared-IRQ dispatcher (irq.c)
- INT 14h FOSSIL handler (int14.c)
- PCBCOMM.CFG parser + dual-mode entry (pcbcomm.c)
- Build system (PCBCOMM.MAK)

Fully ported (v1, single-chip config):
- Cyclades Cyclom-Y (cyclom_backend.c) — CD1400 register access,
  channel init, cy_interrupt() SVRR walk with RX/TX/modem service
  dispatch. Ported from Linux cyclades.c. Multi-chip wiring in
  pcbcomm.c is v1.1 work — see TODO in cyclom_backend.c.

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

**pcbcomm v1.1 is code-complete.** All 7 backends fully ported AND
multi-port wired (parse_config now uses backend->card_get() hook +
per-port subport index). Every smart-card backend has a static card
pool (max 4 cards per backend type) with shared card_pool_get()
helper that groups config lines sharing a card_addr into one card
record. Each port's subport (0..N-1 within card) selects the correct
chip/channel.


Each skeleton captures the essential card-detection registers so
PCBCOMM.CFG validates hardware presence at load. Full ISRs get
filled in next pass, one card at a time, with the Linux source
open side-by-side.

**pcbcomm v1.2 SHIPPED 2026-09-01.** All 5 planned features landed:
Arnet backend (8th card), ser_rs232 shim (13-fn COMMDRV.OBJ
replacement), INT 14h AH>=0x10 extensions, `_dos_keep()` TSR install
fix, and full SDK packaging in `toolkit/pwa154/pcbcomm/`. Total:
3,829 lines GPLv3, 15 .c + 6 .h files, 8 backends. OpenWatcom
verified clean build; PCBDTSR.EXE = 37,800 bytes.

**pcbcomm v1.4 SHIPPED 2026-09-03 (refined).** Three post-WCSC
intelligent multiport backends, all `#if defined(PCB1541)`-gated —
they ship only in 15.41 builds. 15.4 stays lean at WCSC-parity.

  Stallion Brumby/ONboard  -> stallion_brumby_backend.c  (15.41 only)
  Chase Research IOLAN     -> chase_iolan_backend.c      (15.41 only)
  Equinox SST-8/16/32/64   -> equinox_sst_backend.c      (15.41 only)

**Build for 15.41 (extended):**  `wmake -f PCBCOMM.MAK CC=OWC TARGET=15.41`
**Build for 15.4 (default):**    `wmake -f PCBCOMM.MAK CC=OWC`

These backends are written from public documentation without hardware
in the pcbirc lab for validation. Sysops with matching hardware should
test and report. Header comments in each backend cite the public
references used. When PCB1541 is not defined, they compile to empty
translation units — no symbols leak into 15.4.

**pcbcomm v1.3 SHIPPED 2026-09-03.** Full WCSC-DOS card parity
reached. All 8 DOS card families in WCSC's COMMDRV.RED now have a
matching pcbcomm backend:

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

## Delta 15.4 — tightening pass (2026-09-25)

Cleanup toward a tight, buildable 15.4 Delta target. Ordered
cheapest/safest first; only item 1 is done.

1. **[done] Attic the superseded `src/int14-r1.c`.** A 211-line earlier
   revision of the INT 14h handler whose header still reads "int14.c".
   The live handler is the 701-line `src/int14.c` that `PCBCOMM.MAK`
   builds; nothing referenced `-r1`. Moved to
   `attic/superseded-pcbcomm/pcb1541/pcbcomm/src/` by `ATTIC-CLEANUP.BAT`
   (run from repo root; move-only, deletes nothing).
2. **[todo] Re-cut the SDK's `int14.c` from source.** The SDK copy in
   `toolkit/pwa154/pcbcomm/src/int14.c` is the *old* 211-line cut, not
   the current 701-line handler -- i.e. the SDK has drifted behind
   source. Re-generate it so source -> artifact stays honest.
3. **[todo] Land the shim in the source tree.** `pcb1541/pcbcomm/` has no
   shim today (its draft is already atticked); the kept shim lives only
   in `toolkit/pwa154/pcbcomm/src/ser_rs232_shim.c`. For the Delta target
   to build from source, that shim + the 6,422-byte `inc/pcbcomm.h`
   belong here in `src/`/`inc/`, and the SDK builds its `.OBJ` from them.
4. **[todo] Delta link proof.** The Watcom/Delta leg (`PCBOARD.MK`,
   `PCBWAT2.MK`) names neither COMMDRV nor MODEMDRV yet, so nothing
   proves pcbcomm links into Delta. Add `wmake -f PCBCOMM.MAK CC=OWC`
   producing the `.OBJ`, then a link test against Delta's `MODEMDRV.OBJ`.
5. **[todo] The four shim fixes** (see "The kept shim still needs four
   fixes" above): `unsigned int`->`int` signatures, off-by-one port
   numbering, `putpacket(port,0,NULL)` transmitter kick, and
   backend-routed DTR/RTS. These slot in after item 3.

Backend gating is already clean and needs no change: the three v1.4
backends (chase_iolan, equinox_sst, stallion_brumby) are correctly
`#if defined(PCB1541)`-gated (15.41-only); the other eleven are
always-on and compile into the 15.4 Delta target.
