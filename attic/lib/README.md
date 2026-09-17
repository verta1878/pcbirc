# OUT/lib/ — Toolkit Libraries

Built from the toolkit source. Programs link against these.

```
PCBTK_B.LIB    Borland BC31 main toolkit — 154 objects
PCBTKL_B.LIB   Borland override stubs — ALTMODEM, NODISP, PCBDAT
PCBTK_W.LIB    Watcom OW1 toolkit — 267 objects, 1,681 symbols
```

## Two-Library Architecture (Borland)

Clark's original design, from toolkit makefiles' `lib:` targets:

- **PCBTK_B.LIB** — the main library. Every program links this.
- **PCBTKL_B.LIB** (toolkitl) — override stubs linked at LOWER priority.
  Provides default implementations (ALTMODEM, NODISP, PCBDAT) that a
  program can override by supplying its own version. This is why
  EXITDOS (main lib) and ALTMODEM (override lib) coexist — both export
  errorexittodos, but they're in separate libraries.

Link order matters: program objects → PCBTK_B.LIB → PCBTKL_B.LIB.

## Manifest Source

The authoritative object list comes from Clark's makefiles:
`toolkit/delta154/SOURCE/*/MAKEFILE` — each has a `lib:` target
listing exactly which objects belong in the library. Files in the
`obj:` target are per-program objects, NOT library members (e.g.
CNAMES, DATAFIL2).

## Excluded Alternates

These files are deliberately NOT in the library (documented in their
own source comments as "not included in the DOS library"):
- SHOWERR2.C (stripped version of SHOWERR)
- CI_OTHER.C (alternate country instruction)
- SMALLERR.C (small error handler)
- VIRTUAL.C (huge-pointer version; VIRTUAL1.C is the standard version)

## Special Build Steps

- DOSCLASS.CPP — C++ file, compiled with -P
- INT24HND.ASM — assembled with `tasm /mx /d__l__` (large model define)

## Watcom Library

PCBTK_W.LIB includes all 267 compiled DOS objects via wlib. Watcom's
linker resolves the VIRTUAL/VIRTUAL1 overlap differently (static Vir),
so no separate override lib is needed.


## Current contents

- `pwa153/` — the SDK library matrix (PCBKBC + PCBKIT families, 8 libs;
  PCBKMS pending). This is the real deliverable. See pwa153/STATUS.md.

The older top-level single-build libs (PCBTK_B/PCBTK_W/PCBTKL_B) were
removed — superseded by the per-compiler, per-model matrix under pwa153/.
