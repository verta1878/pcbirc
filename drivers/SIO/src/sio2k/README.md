# SIO2K V2 — Clean-Room Serial I/O Driver Suite for OS/2

**Release:** V2.0 Alpha — August 8, 2026 02:00 UTC
**License:** GPLv3 (clean-room reimplementation)
**Status:** Core architecture built. Hexadecimal audit fixes applied (W-01..W-04). All modules compile clean.

## V2 Architecture (from DESIGN.TXT)

```
  OS/2 Applications
       │
  SIO2K.SYS  ← Logical layer (IOCtl, DCB, buffering, config file)
       │ IDC (Inter-Device Communication)
       ├── UART.SYS   ← Physical: 8250/16550/16650/16750/16850/16950
       ├── ESP.SYS    ← Physical: Hayes ESP card
       └── VMODEM.SYS ← Physical: virtual modem port
              │
         VMODEM.EXE   ← Telnet/VMP application
       │
  VSIO2K.SYS ← VDD (works with ANY OS/2 serial driver)
  VX00.SYS   ← FOSSIL (nearly unchanged from V1)
```

## V2 Features to Implement

1. SIO2K↔UART split via IDC (AltDriver=uart$,n)
2. Config file parser (SIO2K.CFG — Os2Device, BaseUart sections)
3. Auto IRQ detection (no user IRQ specification)
4. No boot-time UART touch (probe on first open only)
5. Auto FIFO sizing (probe actual depth)
6. Auto crystal frequency detection (non-standard oscillators)
7. PCI serial card support (PCI.INC database)
8. SuperIO chip support (SMC FDC37xxx, Winbond W83977)
9. 256 port limit (up from 16)
10. Custom device names (not limited to COMn)
11. Driver coexistence (run alongside COM.SYS)
12. VSIO2K works with any serial driver
13. Port swapping (MODES COM1=COM10)
14. Log file generation (all drivers)
15. VMODEM.SYS as physical layer driver
16. LOGGER.EXE, MODES.EXE, PCI.EXE utilities

## Source Directories

| Dir | Component | Status |
|-----|-----------|--------|
| sio2k/ | SIO2K.SYS logical driver | Empty |
| uart/ | UART.SYS physical driver | Empty |
| vsio2k/ | VSIO2K.SYS virtual driver | Empty |
| vx00/ | VX00.SYS FOSSIL driver | Empty (port from V1) |
| vmodem/ | VMODEM.SYS + VMODEM.EXE | Empty |
| esp/ | ESP.SYS Hayes ESP driver | Empty |
| logger/ | LOGGER.EXE | Empty |
| modes/ | MODES.EXE | Empty |
| pci/ | PCI.EXE | Empty |
| tools/ | D4TEST.EXE (copied from V1) | Ready |

## Documentation (from SIO2K v2.03 distribution)

| File | Contents |
|------|----------|
| doc/DESIGN.TXT | V2 architecture description |
| doc/TECHTALK.TXT | Block I/O, auto FIFO, auto crystal, no-boot-touch |
| doc/SAMPLE.CFG | Full config file format with all options |
| doc/PCI.INC | PCI device ID database format and entries |
| doc/HISTORY.TXT | V2.00a through V2.03 changelog |
| doc/FAQ.TXT | User FAQ with config examples |
| doc/MODES.TXT | MODES.EXE documentation |
| doc/LOGGER.TXT | LOGGER.EXE documentation |
| doc/PCI.TXT | PCI support documentation |
| doc/INSTALL.TXT | Installation guide |
| doc/VMODEM.TXT | Virtual modem documentation |
| doc/PMLM.TXT | Line monitor documentation |

## Next Session Plan

1. Side-by-side V1↔V2 comparison (architecture, IOCtl, features)
2. Code audit of V1 against V2 spec (what V1 does wrong for V2 compat)
3. Begin SIO2K.SYS (logical layer) + UART.SYS (physical layer) split
4. Config file parser (SIO2K.CFG)
5. IDC interface definition between SIO2K↔UART
