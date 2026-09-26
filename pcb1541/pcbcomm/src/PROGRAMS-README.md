# pcbcomm programs — wrench, 2026-09-26

Three new source files for the pcbcomm (COMM-DRV replacement) project.
All go in `pcb1541/pcbdcom/src/` alongside the existing backends and int14.c.

## pcbdtsr.c (409 lines) — PCBDTSR.EXE

Replaces Clark's COMMTSR.EXE. The resident TSR that loads PCBCOMM.CFG,
probes serial card backends, registers IRQs, hooks INT 14h, and goes
resident.

Built on hexadecimal's existing pcbdcom.c framework (config parser,
backend registry, install path, _dos_keep). Adds:
- `-i` install (default), `-d` deinstall, `-s` status switches
- Resident detection via PCBCOMM\x01 signature cookie in memory
- Deinstall via AH=FFh/AL=01h admin command through INT 14h

**NOTE FOR HEXADECIMAL:** int14.c needs ~10 lines added to recognize
AH=FFh/AL=01h as the admin unload command. It should call
pcbdcom_int14_uninstall() + pcbdcom_irq_shutdown() and return
AX=0x4F52 ("OR" = OK-Removed). Without this, the -d switch can't
tell the resident copy to shut down.

## drvsetup.c (457 lines) — DRVSETUP.EXE

Replaces Clark's DRVSETUP.EXE. Full-screen DOS text-mode editor for
PCBCOMM.CFG.

Displays a table: Port / Card Type / Sub-Port / Base Address / IRQ /
Card Segment / FOSSIL. Keys:
- Up/Down/Left/Right: navigate
- Enter: edit field
- PgDn/PgUp: scroll
- Alt-I: insert port, Alt-D: delete, Alt-R: repeat (copy+increment)
- ESC: exit with save prompt (Y=save, N=resume, A=abort)

Usage: `DRVSETUP [configfile]` (defaults to PCBCOMM.CFG)

## test.c (341 lines) — TEST.EXE

Replaces Clark's TEST.EXE. Port diagnostics utility.

Features:
- UART chip identification (8250, 8250A, 16450, 16550, 16550A, 16750)
- Port presence probe (detect hardware at I/O address)
- Internal loopback test (MCR bit 4, 6 test patterns)
- Auto-detect COM1-COM4 (`TEST -a`)
- Reads PCBCOMM.CFG for multiport card base addresses
- Modem signal display (CTS/DSR/DCD/RI)
- Line status display (DR/OE/PE/FE/BI/THRE/TEMT)

Usage: `TEST <port> [-l] [-v]` or `TEST -a`

## Build

Same MAK targets as the existing pcbcomm code (BC31 -ml / OW2 -ml / MSC7 /AL).
PCBDTSR links against int14.obj + uart.obj + irq.obj + all backends.
DRVSETUP and TEST are standalone (no backend linkage needed).
