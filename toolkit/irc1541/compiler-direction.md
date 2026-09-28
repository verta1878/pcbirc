# irc1541 Toolkit — Compiler Direction

## Decision

For the 15.41 IRC toolkit, the clean path is to port everything to
**ow2irc** (OpenWatcom2irc), NOT to maintain the old three-compiler
DOS matrix (Turbo C 2.0 / Borland C++ 3.1 / Microsoft C 7.0) that
Clark's original SDK used.

## Why

- ow2irc is under active development with OW1 features carried forward.
- Its assembler is **WASM / JASM / MASM-compatible**, so the assembly
  modules (ASYNC.ASM, the serial layer, etc.) can be assembled with one
  modern toolchain instead of period-correct TASM.
- One modern toolchain replaces three aging DOS compilers — simpler to
  build, maintain, and reproduce.

## Scope

- **pwa153 / pwa154** stay on the historical compilers (PCBKIT Turbo C
  2.0, PCBKBC Borland C++ 3.1, PCBKMS Microsoft C 7.0) — those are
  preservation targets: reproduce Clark's SDK exactly.
- **delta154** = the Watcom (OW1) bridge.
- **irc1541** = ow2irc going forward. New/added toolkit code targets
  ow2irc and its WASM/JASM/MASM-compatible assembler.

The historical matrix matters for the frozen versions (match Clark).
The living version (irc1541) moves to the modern toolchain.
