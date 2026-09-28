# DOSBOXX.ZIP — what is inside it, and what in it is stale

**Status: kept in the repo root on purpose, not updated, not retired.**
Audited 2026-09-22 by unpacking a copy outside the repo. The archive itself
was not opened for writing and not one byte of it was changed.

43,775,080 bytes packed / 104,463,142 unpacked / 3,065 files.

It is a self-contained DOSBox-X build appliance from **2026-08-29**, which is
*before* the ROOT=\OUT pass, before `BLDDOS.BAT` / `BLDOS2.CMD`, and before the
attic cleanup. Everything below is measured, not remembered.

---

## 1. What it contains

| Part | Size | Notes |
|---|---:|---|
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
| `BUILDROOT\BUILD\SCRIPTS` (30) and `BUILDROOT\SCRIPTS` (9) | 358 KB | SDK build scripts |
| `BUILDROOT\PCB153` (629 files) | 7.2 MB | **stale copy of our source — see §2** |
| `BUILDROOT\TOOLKIT\PWA153` (393 files) | 1.9 MB | **stale copy of our toolkit — see §2** |
| `BUILDROOT\OUT` | 1.8 KB | **old output layout — see §3** |
| top-level `PCBIC\` (50), `TC\` (59), `drivez\` | 1.8 MB | extras, not used by the build |

The compiler trees are the reason to keep this file. `BCOS2`, `MSC70`, `TC201`
and `386MAX_S` exist nowhere else in the repo; only `devtools\BC31.zip`
duplicates any of it.

## 2. The embedded source copy is a pre-ROOT-pass snapshot — the repo wins

`BUILDROOT\PCB153` and `BUILDROOT\TOOLKIT\PWA153` are a copy of our own tree
taken on 2026-08-29. They were hashed file-by-file against the repo:

* Of the 14 makefiles present in both, **all 14 differ.** The bundle has the
  versions driven by `BCDOS.BAT` environment variables, with no `ROOT` guard,
  no `BRANCH`/`CVER`/`SRC`/`TKIT`/`LIBSDIR` block and no `CLEAN` target —
  i.e. everything the ROOT=\OUT pass replaced:
  `153\PCBOARD.MAK`, `153\PCBOARD2.MAK`, `153\TURBOC.CFG`,
  `SOURCE\MISC\FIDOUTIL\FIDOUTIL.MAK`, `SOURCE\MISC\IDX\MAKEIDX.MAK`,
  `SOURCE\MISC\USERNET\USERNET.MAK`, `USERNET2.MAK`,
  `SOURCE\UTIL\PCBSETUP\PCBSETUP.MAK`, `SOURCE\UTIL\PCBSM\PCBSM.MAK`,
  `SOURCE\UTIL\PCBTEXT\MKPCBTXT.MAK`,
  `SOURCE\UUCP\{UUIN,UUOUT,UUUTIL,UUXFER}\*.MAK`.
* It still carries the side files retired to `attic\retired-build-files`:
  `UUIN154.CFG`, `UUOUT154.CFG`, `UTIL154.CFG`, `XFER154.CFG`, plus `T.BAT`,
  `G.BAT` and the `EMPTY.BAT` copies.
* It has **no `APPLY.txt`**, which is the marker `BLDDOS.BAT` checks to confirm
  it is standing at the build root. `BLDDOS` therefore cannot run inside this
  bundle as shipped — the bundle predates it.

**Consequence:** anyone who boots this appliance and types `BUILD all` is
compiling the August source, not the repo. Treat the repo as authoritative and
this copy as a historical snapshot.

## 3. The output layout in it is the old one

`BUILDROOT\OUT\LIB\PWA153\{bc31,msc70,tc201}\OBJ\{small,compact,medium,large}`
— superseded by `OUT\pwa153\SDK\<CVER>\{LIB,OBJ}`. Only a directory skeleton
plus four stray `.OBJ` files; nothing depends on it.

## 4. The build scripts are nearly current

Compared against `MAIN\build\scripts`:

* `BUILDROOT\BUILD\SCRIPTS` — 28 of 29 files are byte-identical to the repo.
  Only `BLDKBC.BAT` differs.
* `BUILDROOT\SCRIPTS` — 4 identical; `MAXBLD1.BAT`, `MAXBLD2.BAT` and
  `XFORM.SED` differ; `XFORM-DIF.AWK` is in the bundle only.
* `pcbirc\PCBBLDBT.CONF` (2,409 B) differs from `MAIN\build\PCBBLDBT.CONF`
  (2,160 B).

So the scripts are not the problem; §2 is.

## 5. Two routes in the boot menu point at nothing

`BUILDROOT\CONFIG.SYS` offers six routes (PWA / DELTA / IRC1541 / PCBKMS /
386MAX / BARE) and `AUTOEXEC.BAT` dispatches on them. `DELTA` sets
`PATH=C:\WATCOM\BINW` and `IRC1541` sets `PATH=C:\OW2IRC\BIN`; neither tree is
in the bundle. `386MAX` is documented in the file itself as taking effect only
under 86Box/PCem/QEMU, not DOSBox-X. The working routes are PWA, PCBKMS and
BARE.

Note also that the shipped `dosbox-x.conf` has an **empty** `[autoexec]`
section; the mount is done by `pcbirc\PCBBLDBT.CONF`
(`mount c pcbirc/BUILDROOT`), which is what `LAUNCH.BAT` passes.

## 6. What a refresh would involve (not done — deferred)

Recorded here so it does not have to be re-derived:

1. Repoint the mount: `mount c <repo>` and `mount d <bundle>\BUILDROOT`, then
   `SET BC31PATH=D:\BC31` so `BLDDOS.BAT` finds the compiler off-drive. This is
   the one piece that needs testing — `BLDDOS` currently probes `%BC31PATH%`,
   then `\BC31`, then `\B\C31`, and a drive-qualified value has not been tried.
2. Delete §2 and §3 from the bundle (~1,022 files, 9.1 MB) so nothing shadows
   the repo.
3. Reconcile the four differing script files in §4 against `MAIN\build\scripts`
   and keep one copy.
4. Trim or annotate the two dead routes in §5.
5. Retest `BLDDOS MAKEIDX` and `BLDDOS FIDOUTIL` inside it.
6. Re-zip. Result would be a ~66 MB compilers-and-emulator bundle instead of a
   104 MB part-copy of the repo.

**Before doing that, decide whether the bundle belongs in git at all.**
Re-zipping writes a second ~40 MB blob into history next to the one already
there; a release asset may be the better home.

## 7. Why it is being left alone for now

The bundle's value is its compilers, and the next real work — rebuilding the
category libraries from `toolkit\pwa153\SOURCE` without `-DLIB` — is what will
show whether the `BCOS2` and `MSC70` trees in it are complete enough to keep.
Refreshing it before that answer arrives risks doing the work twice.

Do not retire it: it is the only copy of BCOS2, MSC70, TC201 and 386MAX we
have.
