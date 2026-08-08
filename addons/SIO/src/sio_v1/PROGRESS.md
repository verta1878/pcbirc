# SIO Project — Progress Report

**Last Updated:** August 8, 2026 01:30 UTC

## Contributors

| Name | Role |
|------|------|
| evga | Lead developer (Claude) |
| wrench | D4 conformance test design, FOSSIL function mapping |
| hexadecimal | ISR & ring buffer audit (4 fixes) |

---

## Project 1: Mystic BBS Source Recovery

**Status:** COMPLETE

- Recovered from partial binary-reconstructed archive
- 64,103 lines compiled with FPC 3.2.2, zero errors
- Working ELF binary boots and looks for mystic.dat
- 4 bugs fixed in fork files
- 40 upstream units pulled from FIDOSOFT/mysticbbs GPL v3
- 5 fork-specific functions written from scratch
- 14 CP437 encoding issues fixed

**Caveats:** 40 upstream units are placeholders for fork versions (A39-A61 changes unknown). 5 invented functions may differ from original behavior.

---

## Project 2: SIO V1 Clean-Room Rebuild

**Status:** FEATURE COMPLETE — V1.2.0

### Components (9 binaries, all compile+link with OpenWatcom v2)

| Binary | Lines | Size | Format |
|--------|-------|------|--------|
| SIO.SYS | 2,449 | 6,944 B | LX (OS/2 PDD) |
| VSIO.SYS | 604 | 2,829 B | LX (OS/2 VDD) |
| VX00.SYS | 953 | 1,072 B | MZ (DOS driver) |
| VMODEM.EXE | 1,150 | 32,537 B | LX (OS/2 console) |
| SU.EXE | ~350 | 16,191 B | LX |
| PMLM.EXE | 274 | 19,809 B | LX |
| VIEWPMLM.EXE | 189 | 19,073 B | LX |
| INSTALL.EXE | 183 | 21,425 B | LX |
| D4TEST.EXE | 592 | 19,235 B | LX |

### Test Harnesses
- **D4TEST.EXE:** 37 tests covering all 20 ASYNC IOCtl functions (41h-74h)
- **FOSTEST:** 20 tests for FTS-0001 INT 14h functions (for DOS VDMs)

### Audit History

| Audit | By | Bugs Found | Fixed |
|-------|-----|-----------|-------|
| Audit 1 (Phase 4) | evga | 15 | 15/15 |
| Audit 2 (Full project) | evga | 16 | 16/16 |
| Audit 3 (ISR/buffers) | hexadecimal | 4 | 4/4 |
| **Total** | | **35** | **35/35** |

### Key Bug Fixes (highlights)
- ISR never wrote received bytes to RX buffer (A2-05)
- Read/Write never mapped caller's transfer buffer (A2-03/04)
- GetPort always returned COM1 (A2-01)
- VX00 Fn04_Init stack imbalance (A2-07)
- VMODEM never opened COM ports (A2-12/13)
- VSIO sent partial baud divisors (A2-10)
- RingBuf count not CLI/STI protected (hex W-01)
- FIFO drain loop could hang ISR (hex W-02)
- FIFO fill hardcoded to 16 bytes (hex W-03)

### Features Implemented
- All 20 ASYNC IOCtl functions (41h-74h)
- UART detection: 8250, 8250A, 16450, 16550, 16550A, 16650, 16750
- Full ISR with shared-IRQ, XON/XOFF, CTS/RTS handshake
- CONFIG.SYS parser with INTERNET keyword
- Ring buffers with CLI/STI-protected count fields
- FIFO-aware batch transmit (KickTxBatch)
- ISR drain loop with 256-byte safety limit
- Log file generation (\SIO.LOG)
- DCB with SIO forced bits (FIFO enable, trigger 8, TX load 16)
- FOSSIL driver (20 INT 14h functions, FTS-0001 Rev 5)
- Virtual modem with telnet, AT commands, MD5 auth, TTYPE subneg
- Line monitor (PMLM) with VIO display and trace output
- Trace viewer (VIEWPMLM) with hex dump
- Automated installer with CONFIG.SYS update

### Remaining V1 Items (deferred to V2)
- Hayes ESP / Tport hardware support
- SMP spinlocks (uses CLI/STI only)
- PCMCIA hot-plug
- VMP framing protocol
- VMODEM.SYS physical layer driver
- PCI serial card scanning
- SuperIO chip support

---

## Project 3: SIO2K V2 Clean-Room Rebuild

**Status:** ALPHA — Core architecture built

### Components Built

| Component | Lines | Status |
|-----------|-------|--------|
| sio2k_idc.h | 143 | IDC interface — 25 commands, 5 callbacks, complete |
| sio2k.c | 662 | Logical layer — all 20 IOCtls, Open/Close, DCB, callbacks |
| cfgparse.c + .h | 424 | Config parser — Os2Device, BaseUart, DosDevice sections |
| uart.c | 688 | Physical layer — detection, auto-FIFO, auto-crystal, block I/O |
| d4test.c | 592 | Test harness — copied from V1 (same IOCtl interface) |

**All 4 V2 modules compile clean with OpenWatcom v2.**

### V2 Architecture

```
  OS/2 Applications
       │
  SIO2K.SYS  ← Logical (IOCtl, DCB, config)
       │ IDC
       ├── UART.SYS   ← Physical (8250→16950)
       ├── ESP.SYS    ← Physical (Hayes ESP)
       └── VMODEM.SYS ← Physical (virtual modem)
              │
         VMODEM.EXE   ← Telnet/VMP app
       │
  VSIO2K.SYS ← VDD (universal — works with any serial driver)
  VX00.SYS   ← FOSSIL (nearly unchanged from V1)
```

### V2 Remaining Work
- VSIO2K.SYS (universal VDD)
- VMODEM.SYS (physical layer)
- ESP.SYS (Hayes ESP)
- MODES.EXE, LOGGER.EXE, PCI.EXE utilities
- PCI scanning with PCI.INC database
- SuperIO chip detection
- Block I/O (REP INSB/OUTSB) in real ASM
- Full code audit

---

## Build Environment

- **Compiler:** OpenWatcom v2 (2024-03-01 build)
- **Assembler:** WASM (x86, 16-bit and 32-bit)
- **C Compilers:** WCC (16-bit), WCC386 (32-bit)
- **Linker:** WLINK (NE, LX, MZ formats)
- **Target:** OS/2 2.0+ (LX format) and DOS (MZ format)
- **Host:** Linux x86_64 cross-compilation

---

## Release History

| Version | Date | Description |
|---------|------|-------------|
| V1.2.0 | 2026-08-08 01:15 | Hexadecimal ISR audit fixes |
| V1.1.0 | 2026-08-07 23:50 | Feature complete (M01,M03,M06,M10,M11,W03-W05) |
| V1.0.0 | 2026-08-07 00:00 | 16/16 bugs fixed, 9 binaries |
| V0.9.0 | 2026-08-06 23:30 | V2 docs ingested |
| V0.8.0 | 2026-08-06 22:30 | Full WASM assembly |
| V0.7.0 | 2026-08-06 22:00 | D4 + build environment |
| V0.6.0 | 2026-08-06 21:45 | Phase 5 utilities |
| V0.5.0 | 2026-08-06 21:20 | Phase 4 + first audit |
| V0.4.0 | 2026-08-06 21:00 | VX00.SYS FOSSIL |
| V0.3.0 | 2026-08-06 20:45 | VSIO.SYS VDD |
| V0.2.0 | 2026-08-06 20:31 | SIO.SYS PDD |
| V0.1.0 | 2026-08-06 19:47 | Project foundation |
