# pcb1541/install/build

Build scripts for the install v1.11+ byte-exact INSTALL.EXE
reconstruction.

Scoped to `pcb1541/install/` — the standalone installer binary — NOT
to the pwa153 SDK library matrix that lives under `MAIN/build/`.

## Files

- `BLDINS.BAT` — DOSBox-X build wrapper. Compiles `install-1011.c`
  with Borland C++ 3.1 + TLINK 5.1, links against API.LIB from
  OS/2 SDK 1.03. Produces `INSTALL.EXE` (NE Family API, linker
  bytes 5.10). Run manually from DOSBox-X; does not integrate
  with `MAIN/build/scripts/BUILD.BAT`.

## Why not MAIN/build/scripts/

`MAIN/` is the "cross-version SDK model" — the pwa153 toolkit
library-matrix builds (PCBKBC / PCBKIT / PCBKMS). Its `BUILD.BAT`
dispatcher builds 119-module × 4-memory-model .LIB files across
three compiler families.

`pcb1541/install/` is a different beast entirely — one standalone
installer binary (INSTALL.EXE), one compiler (BC 3.1), one output
file. Doesn't fit the MAIN/ dispatcher model. Lives alongside its
own source at `pcb1541/install/src/` and `pcb1541/install/build/`.

## See also

- `../src/install-1011.c` — the v1.11 source (empty main() stub at
  v1.11.0; grows through v1.11.10 to full byte-exact reconstruction)
- `../src/install-1010.c` — the v1.10 portable-C reference impl
- `../src/README.md` — full install/ tree documentation
- `../reference/INSTALL.EXE` — Clark's original binary (byte-exact
  target)
- `docs/pcboard-internals/INSTALL-EXE-PARITY.md` — full v1.11 phase
  roadmap + toolchain determination
- `devtools/COMPILERS.md` — "install v1.11 toolchain — CONFIRMED"
  section with verification chain
