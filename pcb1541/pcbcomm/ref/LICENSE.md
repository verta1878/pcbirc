# Reference source licensing

All files under `linux/` and `userspace/` are unmodified copies of the
upstream Linux kernel and setserial sources, retained here for
reference during pcbdcom v1 development. Their original licenses
apply:

## Linux kernel serial drivers (GPLv2)

- `linux/8250_core.c`, `linux/8250_port.c` — Copyright Russell King,
  Alan Cox, and others. Linux v6.6, GPLv2.
- `linux/serial.h`, `linux/serial_reg.h` — Copyright Ted Ts'o. Linux
  v6.6, GPLv2.
- `linux/cyclades.c` — Copyright Cyclades Corporation, later Randolph
  Bentson, Ivan Passos, Marcio Saito. Linux v2.6.32, GPLv2.
- `linux/epca.c`, `linux/epca.h` — Copyright Digi International.
  Linux v2.6.32, GPLv2.
- `linux/rocket.c`, `linux/rocket.h`, `linux/rocket_int.h` —
  Copyright Comtrol Corporation, Theodore Ts'o. Linux v2.6.32, GPLv2.
- `linux/istallion.c`, `linux/stallion.c` — Copyright Stallion
  Technologies, Greg Ungerer. Linux v2.6.32, GPLv2.

## Userspace (GPLv2)

- `userspace/setserial.c` — Copyright Ted Ts'o and Rick Sladkey.
  setserial 2.17 (2000), GPLv2.

## Compatibility with pcbdcom (GPLv3)

GPLv2 code can be incorporated into GPLv3 works (one-way compatibility
per GNU FSF). pcbdcom ports and adapts these drivers under GPLv3;
each derived source file will credit the original authors and cite
the upstream file it was ported from.

Ports live in `../src/`. Files in `ref/` are for reading and cross-
reference only; they are NOT compiled into pcbdcom.
