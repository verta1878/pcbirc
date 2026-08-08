# Changelog — SIO Clean-Room Rebuild

## [1.1.0] — 2026-08-07 23:50 UTC — Feature Complete

### Added
- M01: CONFIG.SYS INTERNET keyword support in parser
- M03: Extended UART detection — 16650, 16750 (with EFR probe, 64-byte FIFO)
- M06: Log file generation — writes \SIO.LOG during driver initialization
- M10: Proper VDM instance management (AllocVDMData/FreeVDMData with compaction)
- M11: FOSTEST.EXE — 20-test FOSSIL (INT 14h) conformance harness for DOS VDMs
- KickTxBatch — FIFO-aware batch transmit (fills FIFO in one ISR pass)

### Fixed
- W03: vmodem.h soclose macro — conditional for OS/2 vs POSIX
- W04: TTYPE telnet subnegotiation response added
- W05: su.c ShowSignalsH avoids double port open
- VSIO VDD linking — inline memset/memmove/memcpy (no C runtime in kernel)

---

## [1.0.0] — 2026-08-07 00:00 UTC — Audited Release

### All 16 audit bugs fixed
- See BUGFIXES.md for full details

### Components
- SIO.SYS — 2,151 lines WASM, 6,140 bytes LX binary
- VSIO.SYS — 596 lines C, 2,731 bytes LX binary
- VX00.SYS — 953 lines ASM, 1,072 bytes MZ binary
- VMODEM.EXE — 1,150 lines C, 32,449 bytes LX binary
- SU.EXE — 326 lines C, 15,923 bytes LX binary
- PMLM.EXE — 274 lines C, 19,809 bytes LX binary
- VIEWPMLM.EXE — 189 lines C, 19,073 bytes LX binary
- INSTALL.EXE — 183 lines C, 21,425 bytes LX binary
- D4TEST.EXE — 592 lines C, 19,235 bytes LX binary

### Totals
- 10,209 lines of source across 28 files
- 9 OS/2 binaries, 0 compile errors
- 16 bugs found and fixed, 10 warnings documented, 13 missing features noted

---

## [0.9.0] — 2026-08-06 23:30 UTC — V2 Docs Ingested

### Added
- SIO2K v2.03 documentation ingested (DESIGN.TXT, TECHTALK.TXT, SAMPLE.CFG,
  PCI.INC, HISTORY.TXT, FAQ.TXT, MODES.TXT, LOGGER.TXT, PCI.TXT)
- Full V2 architecture analysis documented
- D4 conformance test mapping to VX00 FOSSIL functions

---

## [0.8.0] — 2026-08-06 22:30 UTC — Full Assembly

### Changed
- SIO.SYS fully ported to WASM-compatible syntax (1,538→2,005 lines)
- All "short jump out of range" errors resolved
- Linked as LX format (matching original SIO.SYS)

---

## [0.7.0] — 2026-08-06 22:00 UTC — D4 Conformance + Build Environment

### Added
- D4TEST.EXE — 37-test conformance harness for ASYNC IOCtl validation
- OpenWatcom v2 cross-compilation toolchain configured
- All 8 modules compile to .obj with 0 errors
- Linker scripts for all components

---

## [0.6.0] — 2026-08-06 21:45 UTC — Phase 5 Utilities

### Added
- SU.EXE — port status, signal control, baud rate setting
- PMLM.EXE — line monitor with VIO display, column tracking, TX monitoring
- VIEWPMLM.EXE — trace file viewer with hex dump and keyboard navigation
- INSTALL.EXE — automated driver installation and CONFIG.SYS update

---

## [0.5.0] — 2026-08-06 21:20 UTC — Phase 4 Complete + Audit

### Added
- VMODEM.EXE — telnet/VMP virtual modem with AT command set, MD5 auth
- Full code audit: 15 bugs found, 12 fixed

### Fixed (from first audit)
- siobuf.asm: PhysToGDTSel mapping for ring buffer base addresses
- sioinit.asm: INTERNET keyword detection (was matching 'IRQ')
- sioinit.asm: PrintMsg calling convention (DevHlp_Save_Message → INT 21h)
- sioinit.asm: COMn header SDevNext linking
- sioio.asm: PhysToVirt mapping for Read/Write transfer buffers
- sioisr.asm: ISR service tracking (ESI accumulator across scan passes)
- sioisr.asm: DCB field access simplified with named EQU offsets
- vsio.c: VDMDATA stored in g_vdms array
- vsio.c: PDD function pointer assembly from seg:ofs
- vx00.asm: Fn04_Init stack imbalance (BP-based stack modification)
- vmodem.c: g_numPorts initialization
- vmodem.c: Shared secret parsing from original input
- vmodem.c: CmdQuerySReg pointer arithmetic

---

## [0.4.0] — 2026-08-06 21:00 UTC — Phase 3 VX00.SYS

### Added
- VX00.SYS — complete FOSSIL driver (FTS-0001 Rev 5)
- All 20 INT 14h functions implemented
- DOS device driver header, INT 14h hook/chain

---

## [0.3.0] — 2026-08-06 20:45 UTC — Phase 2 VSIO.SYS

### Added
- VSIO.SYS — Virtual Device Driver for DOS VDMs
- I/O port hooks for all 8 UART registers
- Virtual UART state per VDM
- PDD-VDD communication protocol (vsio.h)

---

## [0.2.0] — 2026-08-06 20:31 UTC — Phase 1 SIO.SYS

### Added
- SIO.SYS — complete OS/2 Physical Device Driver
- Device header, strategy entry, command dispatch (32 commands)
- UART detection (8250→16550A), init, baud rate, modem signals
- Ring buffer operations (put/get/peek/flush/count/free)
- ISR with shared-IRQ scanning, RX/TX/LSR/MSR handlers, XON/XOFF
- CONFIG.SYS parser, UART probing, buffer allocation, header chaining
- Open/Close with IRQ claim/release, DTR/RTS per DCB
- Read/Write with timeout modes, blocking via DevHlp_ProcBlock
- Input/Output status and flush
- All 20 IOCtl functions (41h–74h)

---

## [0.1.0] — 2026-08-06 19:47 UTC — Project Foundation

### Added
- Project structure with 8 component directories
- README.md with full API specification from documentation
- OS/2 Toolkit DDK headers extracted
- Original SIO documentation (SIOREF.TXT, VX00.TXT, VMODEM.TXT)
