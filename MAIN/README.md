# PCBoard Source Code — Master Guide

## What's in MAIN/

MAIN/ holds the project's model and build system — the cross-version
"how it all fits together" layer (not any single version's source).

```
MAIN/
  README.md              this file - the master guide
  DELTA-MODEL.md         the 4-version model (15.3 / 15.4 PWA /
                         15.4 Delta / 15.41 IRC) in detail
  build/                 the build system (moved here from repo root)
    scripts/             DOS build scripts + TLIB response files:
                           BLDMENU.BAT   menu driver
                           BLDKBC.BAT    build PCBKBC (Borland C++ 3.1)
                           BLDKIT.BAT    build PCBKIT (Turbo C 2.01)
                           BLDKMS.BAT    build PCBKMS (Microsoft C 7.0)
                           MKLIB.BAT     assemble OBJs into the .LIBs
                           <CC><M><chunk>.RSP  8.3 response files
                                          (BC/TC/MS x S/M/C/L x A/B)
    manifests/           the 119-module build manifest + category lists
                           pwa153-COMPLETE-118.json is the master
    backups/             header backups kept during the build-fix work
    borland/ watcom/ dosbox/   compiler-specific build notes/helpers
    SDK-BUILD-STATUS.md  current state of the 12-library SDK matrix
    IC-TOOLKIT-SDK-BUILD-PLAN.md   the toolkit/IC build plan
```

The DOS scripts embed their module lists directly (they don't read the
JSON at runtime), so they're self-contained. In DOSBox they run from
`C:\BUILD\SCRIPTS` (the DOSBOXX.ZIP bundle stages `MAIN/build/scripts`
there as `BUILD/SCRIPTS`).


## Four Versions, One Tree

| Version | Source | Toolkit | Binaries | Compiler |
|---|---|---|---|---|
| **15.3 PWA** | `pcb153/` | `toolkit/pwa153` | `OUT/pwa153` | Borland C++ 3.1 |
| **15.4 PWA** | `pcb153/upd154/` | `toolkit/pwa154` | `OUT/pwa153/upd154` | Borland C++ 3.1 |
| **15.4 Delta** | `pcb154/` | `toolkit/delta154` | `OUT/delta154` | OpenWatcom 16-bit |
| **15.41 IRC** | `pcb1541/` | `toolkit/irc1541` | `OUT/irc1541` | openwatcom2irc |

**15.3 PWA** — Corey Blake's licensed source, preserved by PWA (Pirates
with Attitude). The base. No 15.4 features.

**15.4 PWA** — Clark's authentic 15.4, released as binary upgrades on
top of 15.3, reconstructed into source form. Borland lineage. Represents
Clark's shipped 15.4 (all features present); not yet compiling 100% —
needs a build-fix pass (missing STATS OBJ, a few header resolutions).

**15.4 Delta** — the crew's active work: fixing, adding, changing,
moving. OpenWatcom. Ongoing.

**15.41 IRC** — our work beyond 15.4: new programs, restored binaries,
FidoNet/TCP/RIP additions, the openwatcom2irc port.

See `MAIN/DELTA-MODEL.md` for the full version model, and `patches/`
for the 15.3 -> 15.4 PWA patch.

**Toolkit milestone:** Clark's toolkit has been recreated from source
bit-for-bit, with all original bugs intact — pwa153 (15.3) first, as the
base. Four toolkit versions now live under `toolkit/`. See
`toolkit/README.md` and `todo/toolkit.md`.

## Layout

```
pcb153/                 15.3 PWA source (the base)
  pcb153/upd154/        15.4 PWA = Clark's 15.4 upgrade, in source form
                        (an update layered on 15.3, with its own build/)
pcb154/                 15.4 Delta source (crew's OpenWatcom work)
pcb1541/                15.41 IRC source (our new work)
toolkit/                toolkits by branch: pwa153 pwa154 delta154 irc1541
OUT/                    binaries by version: pwa153 delta154 irc1541
  OUT/pwa153/upd154/    the 15.4 PWA upgrade binaries (parallel to pwa153)
  OUT/lib/pwa153/       the built SDK library matrix
  OUT/support/          shared PCBoard runtime data
devtools/               shared dev-tool + compiler archives
patches/                15.4-pwa.patch = the 15.3 -> 15.4 PWA delta
reference/              archives, excluded from release
```

Each version owns its source, toolkit, and binaries in parallel
top-level dirs. See `toolkit/README.md` for the toolkit layout and
`OUT/README.md` for the binary layout.

## Toolkit

Clark's shared library the PCBoard programs link against (screen I/O,
DOS helpers, string/date utilities, user records, country tables). Each
version has a matching toolkit under `toolkit/`. The 15.3 -> 15.4
toolkit difference is tiny — one enum value (SPACERIGHTAT, for @x color
codes). Everything else Clark's 15.4 added lived in the main source.

Built libraries live in `OUT/lib/pwa153/` as the SDK matrix: Clark's
toolkit compiled by three compilers x four memory models (PCBKBC =
Borland C++ 3.1, PCBKIT = Turbo C 2.01 - both built; PCBKMS = Microsoft
C 7.0 - pending an OS/2 host). See `MAIN/build/SDK-BUILD-STATUS.md` and
`devtools/COMPILERS.md`.

## Third-Party Dependencies

```
pcbcbase/
  CODEBASE/   Sequiter Software dBase library (LGPL v3.0)
  PREBUILT/BC31/   prebuilt .LIB files (Borland C++ 3.1, large model)
```

CODEBASE provides dBase III/IV file access used by PCBoard's file
directory system and PPL DBASE functions.

## Build — DOSBox-X + Borland C++ 3.1 (15.3 / 15.4 PWA)

Everything needed to compile is in the repo:

| File | What |
|---|---|
| `DOSBOXX.ZIP` | **All-in-one**: DOSBox-X (32-bit) + Borland C++ 3.1 compiler + DOSBOX.CFG |
| `PCB153BT.ZIP` | Borland C++ 3.1 build tools alone (PCBKBC) |
| `TC201BT.ZIP` | Turbo C 2.01 build tools alone (PCBKIT) |
| `MSC70BT.ZIP` | Microsoft C 7.0 build tools alone (PCBKMS), incl. the OS/2 add-on |

Setup: extract `DOSBOXX.ZIP` and run its DOSBox-X against the bundled
`DOSBOX.CFG` (it mounts the repo as C:, the repo's `OUT\` as E: for
build output, and hands off to `BUILD\AUTOEXEC.BAT`). Compilers are
expected at `C:\BC31`, `C:\TC201`, `C:\MSC70` - use the DOSBOXX bundle
or extract the standalone *BT.ZIP archives.

Build flags (Borland, 15.3/15.4 PWA):
`-c -P -ml -Od -V -Vmp -Vmd -ff -DPCB152 -DCOMM -DSTATS -DMP -D386 -DDBASE -DFIDO`
Include paths: `BC31\INCLUDE`, `BC31\INCLUDE\SYS`, `toolkit/<ver>/H`

## Build — OpenWatcom (15.4 Delta / 15.41 IRC)

OpenWatcom runs natively on Linux — DOSBox only needed to test the
resulting DOS executables.

```bash
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl64:$PATH
wcc -zq -w1 -ml -2 -za99 -fi=WATCOMPAT.H \
  -Itoolkit/delta154/H -Ipcb154/MAIN/SOURCE/H -I$WATCOM/h \
  -dPCB152 -dLIB -dCOMM -dSTATS -dMP -d386 -dDBASE -dFIDO \
  <file>.c
```

WATCOMPAT.H bridges Borland-to-Watcom differences.

## OS/2 Support

| Directory | What |
|---|---|
| `OS2TK/` | OS/2 toolkit headers |
| `pcb1541/pcbcp/` | Clark's OS/2 PM control program source (1996) |
| `BUILD_OS2.CMD` | OS/2 build script |
| `BUILD_OS2_OW.SH` | OpenWatcom cross-compile for OS/2 |

## Drivers

| Directory | What |
|---|---|
| `drivers/netfosdl/` | wrench's DOS FOSSIL driver (GPLv3, verified in DOSBox-X) |

## Build Status

| Target | Compiler | Status |
|---|---|---|
| 15.3 PWA toolkit | BC31 | DOS 244/262 clean; rest need build-path fixes |
| 15.4 PWA | BC31 | features present; build-fix pass pending (STATS OBJ, headers) |
| 15.4 Delta toolkit | wcc | DOS 267/267 (100%) once 22 Watcom fixes ported from irc1541 |
| 15.41 IRC toolkit | wcc | 267/273 DOS 100%; 6 OS/2 on OS/2 target |

## The Crew

| Handle | Role |
|---|---|
| verta1878 | project lead |
| sysop/0 | compilers (openwatcom2irc, fpc264irc), codecs, utilities |
| hexadecimal | PCBoard port, toolkit, docs |
| wrench | FOSSIL, networking |
| kiddo | protocols, RIP engines, serial |
| evga | SIO driver, display, multiport |
| byte | program recovery; decrypted PCBIC 1.2 (Pcbic12.zip) |
| dotmatrix | crew |

## Documentation

Per-version docs live inside each source tree:
- `pcb153/docs/` — 15.3 PWA docs (incl. the full 15.3 manual/)
- `pcb153/upd154/docs/` — Clark's 15.4 docs (WHATSNEW, HISTORY, Reference)
- `pcb154/docs/` — Delta crew notes

Shared manuals and guides that span versions are in `docs/`.
Planning docs in `todo/`.

## License

Clark Development source: proprietary (licensed copy)
Our additions: GPLv3
CODEBASE: LGPL v3.0
