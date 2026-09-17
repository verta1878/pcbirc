# Toolkit — recreated bit-for-bit, four versions

## Milestone: the toolkit is rebuilt

We have recreated Clark's PCBoard toolkit from source — bit-for-bit,
with all the original bugs intact. This is the preservation guarantee:
not a cleaned-up reimplementation, but Clark's actual toolkit as it
shipped, bugs and all. Fixes come later and deliberately; first we
match what Clark had exactly.

**pwa153 is first** — the 15.3 PWA toolkit is the base, restored from
Clark's source (via the PWA archive) and confirmed building with
Borland C++ 3.1. Every other toolkit version descends from it.

## The four toolkit versions

Each PCBoard version has its own toolkit, matched to its compiler and
feature set. They live side by side under `toolkit/`:

| Toolkit | Version | Compiler | What it is |
|---|---|---|---|
| `pwa153/` | 15.3 PWA | Borland C++ 3.1 | Clark's 15.3 toolkit. The base. Bit-for-bit. |
| `pwa154/` | 15.4 PWA | Borland C++ 3.1 | Clark's 15.4 toolkit (adds SPACERIGHTAT for @x color). |
| `delta154/` | 15.4 Delta | OpenWatcom 16-bit | Crew's Watcom port of the 15.4 toolkit. |
| `irc1541/` | 15.41 IRC | openwatcom2irc | Crew's 15.41 toolkit — grows with new code. |

## What "bit-for-bit with the bugs" means

The toolkit is Clark's shared library the PCBoard programs link against
(screen I/O, DOS helpers, string/date utilities, user records, country
tables). We rebuilt it to match Clark's source exactly:

- Same functions, same signatures, same behaviour
- Original bugs preserved, not silently fixed
- Build-enabling fixes (missing includes, casts for the C++ compiler,
  hardcoded dev paths) are the only changes, and they're documented as
  such — they let the code compile without changing what it does

This is the difference between preservation and reimplementation. A
door built against this toolkit gets exactly what a 1995 door got.

## How the versions relate

- **15.3 -> 15.4**: tiny toolkit change. Clark's 15.4 added 7 feature
  areas but only ONE touched the toolkit — a single enum value
  (SPACERIGHTAT) for the @x color-code feature. Everything else Clark's
  15.4 added lived in the main source. The 15.3 -> 15.4 PWA patch
  carries that one toolkit line along with the source changes.
- **15.4 PWA -> Delta**: same toolkit content, ported from Borland to
  OpenWatcom. The port needs ~22 Watcom-compat fixes (already solved in
  irc1541, being ported into delta154).
- **Delta -> 15.41 IRC**: the toolkit grows. New crew code (Zmodem,
  QWK, RIP, SMTP, TCP) is added here. A sysop writing a door in 2026
  links irc1541 and gets everything the crew has built.

## Frozen vs growing

- **pwa153 / pwa154** are frozen — exactly what Clark shipped. The
  moment we add a function it is no longer what Clark shipped.
- **irc1541** is where things get added. Same architecture, growing
  content.
- **delta154** is the bridge: Clark's 15.4 content on the Watcom
  toolchain, the crew's active working copy.

Existing door *binaries* don't use the toolkit — they're already
compiled. The toolkit only matters to someone *building* a door, which
is exactly why getting it bit-for-bit correct matters.

## Layout inside each toolkit

```
<version>/
  SOURCE/     C source by area: MISC, DOS, SCREEN, SCRNIO, PCB,
              SYSTEM, COUNTRY, DOSCLS, TOOLKIT
  H/          headers (misc.h, screen.h, pcbtools.h, users.h, ...)
  CFG/        build config / resource files
  <compiler>/lib/   built libraries (see below)
```

Built libraries land in `toolkit/<branch>/<compiler>/lib/`, under
**Clark's own names** — one family per compiler, four memory models
each:

| Family | Compiler | Libraries |
|---|---|---|
| PCBKBC | Borland C++ 3.1 | `PCBKBC{S,C,M,L}.LIB` |
| PCBKIT | Turbo C 2.01 | `PCBKIT{S,C,M,L}.LIB` |
| PCBKMS | Microsoft C 7.0 | `PCBKMS{S,C,M,L}.LIB` |

The `PCBTK_*` / `PCBTKL_*` names this document used before were
invented, not Clark's. Do not reintroduce them.

That same directory also holds the **category libraries** (`DOS_L`,
`MISC_L`, `SCREEN_L`, …) — a different product from the same tree. The
SDK is merged and per-memory-model, for door developers; the category
libraries are split by source area, large model only, and are the link
inputs for `PCBOARD.EXE`. Only the SDK comes from `BLDKBC.BAT`; the
category libraries come from Clark's per-directory MAKEFILEs via
`MAIN/COMPILE.BAT` -> `153/PCBOARD.MAK`.

The override stubs (`ALTMODEM`, `NODISP`, `PCBDAT`, the `NO*` and
`SMALL*` modules) are in **neither** library. They ship as loose .OBJ
and are linked selectively, ahead of it — ALTMODEM first, since it
supplies `_Status` and deliberately replaces INKEY, LOG and EXITDOS.

## Status

| Toolkit | Source | SDK library |
|---|---|---|
| pwa153 | Base restored, bit-for-bit. Borland: 244/262 source files clean. | **PCBKBC BUILT 2026-09-17** — 152 modules x 4 models, `toolkit/pwa153/bc31/lib/`; 6 sample doors in `OUT/pwa153/bins/`. PCBKIT and PCBKMS not built. |
| pwa154 | Clark's 15.4 toolkit. Same base + SPACERIGHTAT. | not built |
| delta154 | Watcom port; needs 22 fixes from irc1541 to reach 267/267. | not built — **and the toolkit is not converted to OpenWatcom yet**, so there is no Watcom SDK to build until that lands |
| irc1541 | 267/273 DOS 100%; 6 OS/2 on OS/2 target. | not built |

Note the two counts measure different things and neither is wrong:
244/262 counts toolkit *source files*; the 152 in PCBKBC counts
*library modules*, which is a different set (it also draws modules from
`pcb153/SOURCE`).

### pwa153 PCBKBC — what the build established

Full account: `toolkit/pwa153/bc31/README.md`. The parts worth knowing
before touching another branch, because all three will recur:

- **Config.** Clark shipped no door-SDK config; `CFG/BC31/PCBOARD.CFG`
  is the PCBOARD.EXE config. The SDK config is that, plus `-DLIB`,
  minus three switches: `-Y` (overlays — Borland rejects them in
  small/compact, which is what capped those models at 6/130 objects),
  `-DPCBCOMM` (guards branches reading `Status.TerseMode`, which does
  not exist in the door-visible `statustype` that `-DLIB` selects), and
  `/DCPU386` (only `BUG.H` reads it, to reach a `long BC386BUG` global
  living in `pcb153/SOURCE/MAIN/PCBOARD.C`, which no door can link).
  Keep `-D_FARDATA_=_FAR_` and `-P`.
- **Manifest.** `pcbkit_l.lib` is **not** the SDK — it is the
  PPLC/utility link target (PCBSETUP links it too), built without
  `-DLIB`, and already recorded as stale in
  `docs/pcboard-internals/PCBKIT-LIB.md`. Using its module list cost 22
  missing modules, 3 wrong-tree picks (CNAMES and HELP must come from
  `SOURCE/TOOLKIT/`; ANSI is `pcb153/SOURCE/ASM/ANSI.ASM`, assembled
  `tasm /m3`, not either `ANSI.C`), and 2 modules that had to come out
  (`SCRNIO/GETKEY.C` and `DOS/SHOWERR.C` are PCBSETUP-side, needing
  host globals no door supplies).
- **Verification is linking, not counting.** Clark's own samples in
  `SOURCE/TOOLKIT/SAMPLES/` link against the library; that is what found
  every one of the above. 150/150 of the functions in
  `SOURCE/TOOLKIT/OTHER/QUICKREF` are present and nothing is left
  unresolved. Small and compact cannot link a *full* door (`_TEXT
  exceeds 64K` — one 64K code segment); they are for doors using a small
  slice of the toolkit, which is why Clark shipped four models.

## Next

1. **PCBKIT (Turbo C 2.01) and PCBKMS (MSC 7.0) for pwa153.** Both were
   previously marked "DONE, all 4 models"; that referred to the old
   119-module manifest and the old flag set, and neither library was
   ever committed — `toolkit/pwa153/{tc201,msc70}/` hold only a
   `.gitkeep`. Re-run both against the corrected manifest and config.
2. Port the 22 Watcom fixes irc1541 -> delta154 (reach 267/267).
3. **Convert the delta154 toolkit to OpenWatcom.** Until that is done
   there is no Watcom SDK to build — this is upstream of any delta154
   `.LIB`, and separate from the Delta *program* build. Note OpenWatcom
   2.0 has a Linux-native 16-bit DOS cross-compiler (verified
   2026-09-17), so this no longer needs DOSBox: `wcc -bt=dos -ml` plus
   `wlink system dos` produces MS-DOS MZ executables directly. See
   `MAIN/build/CLARK-DRIVE-LAYOUT.md`.
4. Build the remaining three toolkits into their .LIB form. pwa154
   should be cheap — it is pwa153 plus SPACERIGHTAT, so the corrected
   manifest and config carry over almost unchanged.
5. That gives the SDK every other component (IC, PCBIS, doors) links
   against.
6. ~~Open layout question — where do the category libraries live?~~
   **Decided 2026-09-17: moved to `toolkit/pwa153/bc31/lib/`** beside
   PCBKBC. Same source tree, same compiler, and Clark kept his in
   `LIB/BCDOS/BC31/` — inside the library tree, not an output dir.
   `OUT/pwa153/lib/` is retired; do not recreate it.

## Milestone log

- **2026-09-17** — pwa153 PCBKBC built, all four memory models, first
  SDK library the project has produced (`toolkit/pwa153/bc31/lib/`),
  with 6 of Clark's 7 sample doors linked against it as proof
  (`OUT/pwa153/bins/`). The 9 category libraries moved into the same
  directory from `OUT/pwa153/lib/`.
