# DOSBOXX.ZIP — what is inside it

**Status: kept in the repo root on purpose.**
Audited 2026-09-22; updated 2026-10-01 (dosbox.cfg moved into zip, bldkit.bat
mkdir chain fixed, turboc.cfg confirmed working with BUILDROOT as C:;
stale source/output removed, 3 build scripts reconciled with repo).

~28 MB packed / ~72 MB unpacked / ~1,768 files.

It is a self-contained DOSBox-X build appliance from **2026-08-29**, which is
*before* the ROOT=\OUT pass, before `BLDDOS.BAT` / `BLDOS2.CMD`, and before the
attic cleanup. Everything below is measured, not remembered.

---

## 1. What it contains

| Part | Size | Notes |
|---|---:|---|
| `dosbox.cfg` | 1,495 B | DOSBox-X config — mounts BUILDROOT as C:, sets BC31 PATH |
| `dosbox-x.exe`, `dosbox-x.conf` | 23.7 MB | Windows build |
| `LAUNCH.BAT`, `LAUNCH.SH`, `SETUP.SH` | 5 KB | one-click launchers |
| FreeDOS boot chain — `KERNEL.SYS`, `FDCONFIG.SYS`, `FDAUTO.BAT`, `FDOS\` | 1.8 MB | optional real-boot path |
| `pcbirc\PCBBLDBT.CONF` | 2,409 B | the config `LAUNCH.BAT` actually passes to dosbox-x |
| `BUILDROOT\BC31` | 28.7 MB | Borland C++ 3.1, complete with `BIN`, `INCLUDE`, `LIB` |
| `BUILDROOT\BCOS2` | 13.5 MB | Borland C++ for OS/2 — `BIN`, `INCLUDE`, `LIB`, `FILELIST.DOC` |
| `BUILDROOT\MSC70` | 10.6 MB | Microsoft C 7.0 (the PCBKMS leg) |
| `BUILDROOT\TC201` | 1.6 MB | Turbo C 2.01 |
| `BUILDROOT\386MAX_S` | 12.4 MB | 386MAX source (Route B) |
| `BUILDROOT\HX`, `CWSDPMI`, `D32A`, `PCBDCOM` | 0.6 MB | DOS extenders + pcbdcom |
| `BUILDROOT\BUILD\SCRIPTS` (29) and `BUILDROOT\SCRIPTS` (8) | 358 KB | SDK build scripts (reconciled 2026-10-01) |
| top-level `PCBIC\` (50), `TC\` (59), `drivez\` | 1.8 MB | extras, not used by the build |

The compiler trees are the reason to keep this file. `BCOS2`, `MSC70`, `TC201`
and `386MAX_S` exist nowhere else in the repo; only `devtools\BC31.zip`
duplicates any of it.

## 2. Stale source and output removed (2026-10-01)

`BUILDROOT\PCB153` (713 files, 11 MB), `BUILDROOT\TOOLKIT\PWA153` (415 files,
2.9 MB) and `BUILDROOT\OUT` (486 files, 5.6 MB) were pre-ROOT-pass snapshots
from 2026-08-29 that shadowed the repo. All three directories have been
deleted. The repo is the authoritative source.

## 3. Build scripts reconciled (2026-10-01)

`BLDKBC.BAT`, `MAXBLD1.BAT` and `MAXBLD2.BAT` have been replaced with the
repo versions. `XFORM.SED` has 2 extra asm fixup rules (Rules 11–12) not in
the repo — left as-is pending 386MAX rebuild testing. `XFORM-DIF.AWK` is
bundle-only (pre-expands REPT/CATSTR/SUBSTR macros for TASM) — left as-is.
`pcbirc\PCBBLDBT.CONF` still differs from `MAIN\build\PCBBLDBT.CONF`.

## 4. Two routes in the boot menu point at nothing

`BUILDROOT\CONFIG.SYS` offers six routes (PWA / DELTA / IRC1541 / PCBKMS /
386MAX / BARE) and `AUTOEXEC.BAT` dispatches on them. `DELTA` sets
`PATH=C:\WATCOM\BINW` and `IRC1541` sets `PATH=C:\OW2IRC\BIN`; neither tree is
in the bundle. `386MAX` is documented in the file itself as taking effect only
under 86Box/PCem/QEMU, not DOSBox-X. The working routes are PWA, PCBKMS and
BARE.

Note also that the shipped `dosbox-x.conf` has an **empty** `[autoexec]`
section; the mount is done by `pcbirc\PCBBLDBT.CONF`
(`mount c pcbirc/BUILDROOT`), which is what `LAUNCH.BAT` passes.

`dosbox.cfg` (at the zip root, moved from the repo root 2026-10-01) mounts
BUILDROOT as C: (`mount C BUILDROOT`) and sets `ver=7.10`. This is the
correct layout: all compilers (BC31, TC201, MSC70) and source (TOOLKIT,
PCB153, PCBCBASE) are peers at the C: root. TCC 2.01 locates `turboc.cfg`
via argv[0] in `C:\TC201\BIN\`, and the `-I` paths in that cfg
(`-IC:\tc201\include`, `-IC:\toolkit\pwa153\H`, `-IC:\pcb153\source\H`)
resolve correctly. Confirmed working 2026-10-01.

## 5. What a refresh would involve (mostly done)

Completed:

1. ~~Repoint the mount~~ — **DONE 2026-10-01.** `dosbox.cfg` mounts BUILDROOT
   as C:.
2. ~~Delete stale source and output~~ — **DONE 2026-10-01.** 1,614 files
   removed (§2).
3. ~~Reconcile build scripts~~ — **DONE 2026-10-01.** 3 scripts replaced with
   repo versions; `XFORM.SED` and `XFORM-DIF.AWK` left as-is (§3).

Remaining:

4. Trim or annotate the two dead routes in §4.
5. Retest `BLDDOS MAKEIDX` and `BLDDOS FIDOUTIL` inside it.

**Before adding more blobs, decide whether the bundle belongs in git at all.**
A release asset may be the better home.

## 6. Why it is being left alone for now

The bundle's value is its compilers, and the next real work — rebuilding the
category libraries from `toolkit\pwa153\SOURCE` without `-DLIB` — is what will
show whether the `BCOS2` and `MSC70` trees in it are complete enough to keep.
Refreshing it before that answer arrives risks doing the work twice.

Do not retire it: it is the only copy of BCOS2, MSC70, TC201 and 386MAX we
have.
