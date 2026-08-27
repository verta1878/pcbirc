# FOSSIL.VXD Build Status — RESOLVED (2026-08-21)

The VxD now builds. Full recovery is in `src/vxd-recovered/`
(self-contained: ported source, genuine DDK includes, build scripts,
verified binaries, and docs/RECOVERY.md).

## Summary
- **Was:** never assembled — bundled VMM.INC was corrupted (~290 garbled
  lines where BeginProc/EndProc + segment generators belong).
- **Now:** genuine Microsoft VMM.INC recovered from the freely-mirrored
  Win98 DDK; ported to JWasm (3 dialect fixes); 3 FOSSIL.ASM source bugs
  fixed. Assembles 0 warnings / 0 errors; links to a valid LE VxD.
- **Verified:** output is functionally equivalent to a genuine MASM
  6.11d build — _LDATA and _RCODE byte-identical, _LTEXT 99.68% identical
  (differences are equivalent jump encodings only).

## Remaining
Load/run validation on real Win98 hardware — the one gate left. Does not
affect Win98 users: the shipping driver is the ring-3 DLL (builds clean,
50/50 + 12/12 tests). The VxD is the optional ring-0 path.

See `src/vxd-recovered/docs/RECOVERY.md` for the complete report.
