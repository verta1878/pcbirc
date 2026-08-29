# pcbdcom — PCB DOS COM (unified serial layer)

Supersedes WCSC COMM-DRV. See BINARY-CATALOG.md section F.

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
