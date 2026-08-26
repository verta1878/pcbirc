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
```

Built libraries land in `OUT/lib/`:
- `PCBTK_B.LIB` / `PCBTKL_B.LIB` — Borland (main + override stubs)
- `PCBTK_W.LIB` — Watcom

## Status

| Toolkit | Build status |
|---|---|
| pwa153 | Base restored, bit-for-bit. Borland: 244/262 clean; rest need documented build-path fixes. |
| pwa154 | Clark's 15.4 toolkit. Same base + SPACERIGHTAT. |
| delta154 | Watcom port; needs 22 fixes from irc1541 to reach 267/267. |
| irc1541 | 267/273 DOS 100%; 6 OS/2 on OS/2 target. |

## Next

1. Port the 22 Watcom fixes irc1541 -> delta154 (reach 267/267).
2. Build all four toolkits into their .LIB form.
3. That gives the SDK every other component (IC, PCBIS, doors) links
   against.
