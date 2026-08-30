# pcbdcom source scaffold

Initial source tree for PCB DOS COM. Scaffold only — implementations
land phase by phase per `MAIN/build/PCBBLDBT-ROADMAP.md` phase 4.

## Files

| File | Purpose | Status |
|---|---|---|
| `pcbdcom.c` | Main entry, dual-mode loader (CONFIG.SYS device OR TSR), argument parsing | scaffold |
| `../inc/uart.h` | 8250/16450/16550 register + bit definitions | complete (reference constants) |
| `uart.c` | UART chip probe + register I/O | TODO |
| `fossil.c` | FOSSIL INT 14h dispatch | TODO |
| `irq.c` | IRQ handler install/uninstall, PIC programming | TODO |
| `ring.c` | TX/RX ring buffer implementation | TODO |
| `modem.c` | Modem control (DTR/RTS/DCD/CTS/DSR/RI) | TODO |

## Reference sources (crew-owned or free)

- **`drivers/netfosdl/`** (Free Pascal, DOS FOSSIL, verified 2026-08-19)
  — closest match to what we're building. Ports directly to C.
- **`drivers/SIO/v2/uart/`** (OS/2 SIO clean-room, GPLv3) —
  register-level UART code, algorithm reference.
- **Linux `drivers/tty/serial/8250/*.c`** (GPL) — chip probe
  idioms for UART_TYPE_8250 through UART_TYPE_16750 detection.
- **Linux `drivers/char/{pcxx,epca,rocket,istallion}.c`** (GPL) —
  smart multi-port card drivers, for later phase.

## Build

Not yet — waiting on TODO files. Once implementations land, build
under DOSBox-X + MSC 7.0 with:

```
CL /AL /Ox /Zp /Fepcbdcom.exe pcbdcom.c uart.c fossil.c irq.c ring.c modem.c
```

Model = LARGE (all pointers far) for the TSR path.
Also targets: Watcom C, Borland C++ 3.1 (via cross-compilation shim).

## Design constraints

- **Single-file `.DRV` output** (not the 18-file collection commdrv had).
- **DOS primary target**; Win98 later if wrench's 98fossil license clears.
- **GPL license** — matches netfosdl, SIO, cyclades in `drivers/`.
- **No firmware bundled** — smart-card firmware only if user supplies
  from vendor disk OR we find explicit free version.

See `MAIN/build/PCBBLDBT-ROADMAP.md` phase 4 for full dependencies.
