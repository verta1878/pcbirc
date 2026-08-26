# PCBoard 15.3 PWA — Crew Changes

What the crew added/changed/fixed on top of Clark's 15.3 PWA source.
Clark's 15.3 docs (PCB153.NEW, README.1ST) describe the original.

## Compiler

15.3 PWA builds with **Borland C++ 3.1** (Clark's original compiler),
under DOSBox-X headless. Flag: `-c -P -ml -DPCB152 -DLIB -DCOMM`.
TASM (in BC31/BIN) handles inline asm.

## Toolkit — Compiles 100% on DOS

270/270 DOS objects compile under Borland BC31.

### Borland-specific fixes applied
- TYPES.HPP: bool typedef fix (BC31 is pre-C++98, no built-in bool)
- TYPES.HPP: _FARDATA_ defined (far for Borland, empty for Watcom)
- PCBOARD.H: AUTO=-32768 in Borland path (BC31 16-bit enum overflow;
  same 0x8000 bit pattern, used as bitmask so safe)
- CONFFUNC.C: #pragma inline moved after comment block (asm keywords
  inside /* */ confused the BCC parser)
- VIRTUAL1.C: explicit (char*)/(VirType*)malloc casts for C++ mode
- CNAMES.C: getconfrecord/putconfrecord return types aligned,
  closecnames duplicate declaration removed (atexit needs C convention)

## Version

15.3 PWA reports v15.3 (changed from the shared 15.4 source:
INIT.C version string and BetaVersion, DEFINES.H VERSION_MINOR = "3").

## Binaries

15 EXEs in OUT/pwa153/ (Borland, no suffix).
MAKEIDX and USERNET built this phase by linking against the toolkit
library. Verified executing under DOSBox-X.

## Gap Binaries (in progress)

Still to build for PWA: PCBCP, PCBIS (port from Watcom).
