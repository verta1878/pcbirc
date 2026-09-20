# pcbdcom build status

## v1.1 verification — 2026-08-31

**PASS** — all 13 source files compile clean on both primary compilers.

## Toolchain results

| Compiler | Version | PCBDTSR.EXE size | Files | Warnings |
|---|---|---|---|---|
| OpenWatcom | 2.0 beta (Aug 28 2026) | 35,306 bytes | 13/13 | 0 |
| Borland C++ | 3.1 (1992) | 28,892 bytes | 13/13 | 0 |
| Microsoft C | 7.0 | compile 13/13, link blocked | 13/13 | 8 (C4761) |

Both compilers produce valid MS-DOS MZ executables.

BC31 build is 6414 bytes smaller — BC31's optimizer is more aggressive
on 16-bit code, and OpenWatcom includes more runtime library overhead
by default. Both are within acceptable size for a TSR.

## Portability work done today

Created `inc/compat.h` to abstract compiler-specific INT 14h handler
syntax:

- OpenWatcom: `void __interrupt __far handler(unsigned _es, _ds, ...)`
- Borland C++: `void interrupt handler(unsigned bp, di, si, ds, ...)`
- Microsoft C: `void _interrupt _far handler(unsigned _es, _ds, ...)`

The three compilers have:
1. Different keywords: `__interrupt __far` vs `interrupt` vs `_interrupt _far`
2. Different register argument orders on the stack frame
3. Different argument naming conventions (underscored vs plain)

`compat.h` provides:
- `PCBDCOM_INTERRUPT` — the interrupt keyword + return type combo
- `PCBDCOM_INT14_ARGS` — the register argument list in correct order
- `PCBDCOM_AX/BX/CX/DX` — access macros for register values
- `PCBDCOM_UNUSED_REGS` — `(void)` casts for unused regs
- `pcbdcom_isr_t` — typedef for interrupt-function pointer (for
  `_dos_setvect` call type matching)

Adding a new compiler = new `#elif` block in compat.h. No .c changes.

## Build commands

### OpenWatcom (Linux cross-compile)
```
export WATCOM=/path/to/openwatcom
export PATH=$WATCOM/binl64:$PATH
export INCLUDE=$WATCOM/h
wcc -Iinc -ml -bt=dos -zq src/<file>.c -fo=obj/<file>.obj
wcl -ml -bt=dos -k32768 -fe=PCBDTSR.EXE obj/*.obj
```

### Borland C++ 3.1 (DOSBox-X on Linux)
```
SET PATH=C:\BC31\BIN;%PATH%
BCC -c -ml -IINC -IC:\BC31\INCLUDE -w- -DPCBDCOM_V1 -n..\OBJ <file>.C
BCC -ml -LC:\BC31\LIB -ePCBDTSR.EXE *.OBJ
```

DOSBox-X handles COMMAND.COM + BCC + TLINK. BC31 8.3 filename
constraints handled by renaming source files (uart_backend.c →
UARTBACK.C, cyclom_backend.c → CYCLOM.C, etc.).

## Files compiled (13)

boca_backend, card_pool, cyclom_backend, digi_accel_backend,
digi_fep, digi_pcxe_backend, easyio_backend, int14, irq, pcbdcom,
rocket_backend, uart, uart_backend.

## MSC70 status

**Partial pass** — all 13 files compile clean under CL.EXE. Only warnings
are C4761 "integral size mismatch in argument : conversion supplied"
in cyclom.c and uart_backend.c (7 sites) — real warnings worth fixing
in a follow-up (explicit casts on the offending arg passes) but not
blocking.

**Link blocked** in this session. LINK.EXE runs under HDPMI32 -r
(HX Extender) but produces only a 32-byte header-only .MAP file and
no .EXE. Same LINK.EXE works fine outside pcbdcom (verified by other
BUILDROOT modules). Suspect: our `void interrupt` handler symbol
export or some OMF record LINK 5.15 doesn't like. Needs deeper
investigation.

Working set of MSC70 OBJs is 13 files including INT14.OBJ,
totaling ~34 KB. Once link cleared, expected PCBDTSR.EXE size ~30 KB
(between BC31 and OW).

Deferred to a follow-up MSC70 debugging session. Two working
compilers (OW + BC31) already give strong coverage. MSC70 is bundled in DOSBOXX.ZIP's BUILDROOT/MSC70/BIN/
(NMAKE.EXE etc. visible in the zip listing). Adding it to step 1 is a
matter of writing another dosbox conf that mounts C:\MSC70 and runs
the equivalent CL invocation. Deferred to a follow-up pass since two
compilers is already good coverage.

## Size analysis

|Compiler|Baseline (all 7 backends)|Est. modular (1 backend)|
|---|---|---|
|OpenWatcom|35,306 bytes|~5-6 KB skeleton|
|BC31|28,892 bytes|~4-5 KB skeleton|

v2 modular refactor (later) drops resident size dramatically for
sysops using only one card family.


## v1.2 verification — 2026-09-01

**PASS** on OpenWatcom cross-compile: 15/15 files clean, PCBDTSR.EXE
37,800 bytes (up from v1.1's 35,306 — added Arnet backend, ser_rs232
shim, INT 14h AH>=0x10 extensions).

New in v1.2:
- src/arnet_backend.c (231 lines) — Arnet SmartPort / SmartPort Plus,
  8th card family. Auto-detects Plus firmware via mailbox probe.
- src/ser_rs232_shim.c (216 lines) — 13-function COMMDRV.OBJ-compatible
  API surface (ser_rs232_init/setup/getport/getbyte/putbyte/getpacket/
  putpacket/viewpacket/flush/dtr_on/dtr_off/rts_on/rts_off).
- src/int14.c extended with COMM-DRV AH=0x10 (commgo), AH=0x11 (port
  count query), AH=0x12 (commstop), AH=0x13 (backend name query),
  AH=0x14 (baud rate get/set).
- src/pcbdcom.c main() uses _dos_keep() / keep() per compiler for
  proper TSR install (was leaving driver un-resident). device_entry()
  and .SYS device driver path removed for WCSC parity — original
  COMM-DRV only shipped as an EXE TSR.
- inc/pcbdcom.h extended: parity, data_bits, stop_bits, flow, card_state
  fields on pcbdcom_port_t; PCBDCOM_MAX_CARDS = 8; PCBDCOM_BUF_SIZE.

New in toolkit/pwa154/pcbdcom/ (SDK packaging):
- inc/PCBDCOM.H — public API header for SDK consumers
- docs/SDK.md, docs/LINKOUT.md — SDK usage + PCBoard integration guide
- examples/simple.c, multiport.c, tsrless.c
- src/nopcbdcom_stub.c — empty ser_rs232_* symbols for #ifdef-out builds
- lib/README.md, lib/NOPCBDCOM.README — .OBJ variant naming matrix
