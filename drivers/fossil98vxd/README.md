# FOSSIL.VXD — Recovered Win98 Ring-0 Driver

The WinFOSSIL VxD (Windows 95/98/ME virtual FOSSIL driver), recovered
from a state where it had never assembled, now building clean and
verified against a genuine MASM 6.11d reference build.

## Contents
```
src/               JWasm-buildable source (ported VMM.INC etc.)
src/ddk-genuine/   Pristine unmodified Win98 DDK includes
build/             build_jwasm.sh (Linux) + BUILD_masm.bat (DDK)
verified/          FOSSIL.vxd + MASM reference VxD + OBJs + SHA256SUMS
docs/RECOVERY.md   Full recovery + verification report
```

## Quick build (Linux)
```
cd build
JWASM=/path/to/jwasm WLINK=/path/to/wlink ./build_jwasm.sh
```
Produces `FOSSIL.vxd` — a valid LE-format VxD.

## Status
Assembles clean (0/0), links to valid LE VxD, output verified
functionally equivalent to genuine MASM 6.11d. **Not yet load-tested on
real Win98 hardware** — that is the one remaining gate. Win98 users are
unaffected: the shipping driver is the ring-3 DLL, which passes all
tests. This VxD is the optional ring-0 path.

See `docs/RECOVERY.md` for the complete story.

GPLv3 — FPC264IRC Contributors, 2026.
