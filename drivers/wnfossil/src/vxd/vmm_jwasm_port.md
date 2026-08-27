# VMM.INC → JWasm Port Notes

Two verified fixes to make the VxD assemble under JWasm (MASM-compatible),
plus the explicit segment macros that replace the DDK generator.

## Fix 1: `&&` → `&`
In the nested macro-def-inside-IRP-inside-MACRO contexts, change all
double-ampersand token pasting to single. Affected identifiers:
`VxD_&segname&_CODE_SEG`, `_&segname`, `$$&Procedure`, `@@&Procedure`,
`@32&Name`, `cparm&Name`, `?&arg&_...`. Verified: `&&` fails with
"Syntax error", `&` assembles clean in JWasm 2.12.

## Fix 2: replace MakeCodeSeg generator with explicit macros
JWasm rejects `segname SEGMENT` inside the trebly-nested generator.
Use explicit per-segment definitions (see below) for the 6 segments
FOSSIL.ASM uses. Same semantics, JWasm-clean.

## The explicit segment macros (drop-in replacement for MakeCodeSeg)
See `vmm_jwasm_segment_macros.inc` — the JWasm-safe VMM.inc header
region (segment declarations + VxD_*_SEG/_ENDS macros).

## Still required
The bundled VMM.INC is corrupted in its last ~290 lines (BeginProc/
EndProc + data/init generators). Obtain the genuine Win95 DDK VMM.INC
or reconstruct that region, then apply Fixes 1 & 2. Validate the
resulting .vxd on real Win98 hardware.
