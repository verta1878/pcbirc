# toolkit/ — PCBoard Toolkit Libraries

The toolkit is Clark's shared library of routines that the PCBoard
programs link against (screen I/O, DOS helpers, string/date utilities,
user-record handling, country tables, etc.). It's the foundation every
PCBoard executable is built on.

Almost nobody outside Clark knew this toolkit existed as separable
source — it's one of the things that makes this project unusual.

## Recreated bit-for-bit

The toolkit has been rebuilt from Clark's source bit-for-bit, with all
the original bugs intact — not a cleaned-up reimplementation. **pwa153
(15.3) is the base**, restored from Clark's source and building with
Borland C++ 3.1; every other version descends from it. Build-enabling
fixes (missing includes, C++ casts, hardcoded dev paths) are the only
changes, and they're documented as such. See `todo/toolkit.md` for the
full account.

## The 4 toolkit versions

Each PCBoard version has its own toolkit, matched to its compiler and
feature set. They live here side by side:

| Dir | Version | Compiler | Notes |
|---|---|---|---|
| `pwa153/` | 15.3 PWA | Borland C++ 3.1 | The base. No 15.4 features. |
| `pwa154/` | 15.4 PWA (Clark's) | Borland C++ 3.1 | Self-contained copy; adds SPACERIGHTAT (@x color). Independent of pwa153. |
| `delta154/` | 15.4 Delta | OpenWatcom 16-bit | Crew's active Watcom port. |
| `irc1541/` | 15.41 IRC | openwatcom2irc | Toolkit for the IRC branch. |

The 15.3 -> 15.4 difference in the toolkit is tiny: one enum value
(SPACERIGHTAT) for the @x color-code feature. Everything else Clark's
15.4 added lived in the main source, not the toolkit.

## Layout inside each toolkit

Some toolkit versions also carry a `docs/` subdir with Clark's data
structure / file format reference (e.g. `toolkit/pwa153/docs/` has the
devkit and develop9 revisions of DEVELOP.ZIP — the struct specs).

```
<version>/
  SOURCE/     the toolkit C source, grouped by area:
    MISC/       string, date, math, misc helpers
    DOS/        DOS-level file/IO routines
    SCREEN/     screen output
    SCRNIO/     screen I/O primitives
    PCB/        PCBoard-specific helpers
    SYSTEM/     low-level system routines
    COUNTRY/    country/codepage tables
    DOSCLS/     DOS class wrappers (C++)
    TOOLKIT/    top-level toolkit glue + override stubs
  H/          toolkit headers (misc.h, screen.h, pcbtools.h, etc.)
  CFG/        build config / resource files
```

## Building

Each toolkit compiles into a per-compiler SDK matrix under `OUT/lib/`.
For each compiler, four `.LIB` files (one per memory model: small,
medium, compact, large) sit at the compiler root, next to `OBJ/` and
`loose-obj/`:

```
OUT/lib/pwa153/
  bc31/   PCBKBC{S,M,C,L}.LIB   Borland C++ 3.1
  tc201/  PCBKIT{S,M,C,L}.LIB   Turbo C 2.01
  msc70/  PCBKMS{S,M,C,L}.LIB   Microsoft C 7.0
```

= 3 compilers × 4 memory models = 12-library SDK matrix per toolkit
version. See `OUT/lib/README.md` for the library architecture and
`MAIN/build/scripts/README.md` for the full layout inside each
compiler root.

## Relation to source and binaries

Convention: source trees, toolkits, and binaries are kept in parallel
top-level dirs, each named by version:

| Version | Source | Toolkit | Binaries |
|---|---|---|---|
| 15.3 PWA | `pcb153/SOURCE` | `toolkit/pwa153` | `OUT/pwa153` |
| 15.4 PWA | `pcb153/upd154/SOURCE` | `toolkit/pwa154` | `OUT/pwa153/upd154` |
| 15.4 Delta | `pcb154/MAIN/SOURCE` | `toolkit/delta154` | `OUT/delta154` |
| 15.41 IRC | `pcb1541/` | `toolkit/irc1541` | `OUT/irc1541` |

Shared developer tool archives (not per-version) are in `devtools/`.


## Future: pwa1522 (old 15.x toolkit)

An idea worth preserving: build the older 15.0/15.1/15.2 toolkit as
`toolkit/pwa1522` to give earlier-version users an upgrade path to 15.3.
Not yet buildable (we lack pre-15.3 source). Ties into the IC work,
since the IC is 15.2x-era. See `toolkit/PWA1522-FUTURE.md`.
