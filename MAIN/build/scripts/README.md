# pwa153 Toolkit SDK — Build Scripts

Runnable DOS batch files that build the PCBoard 15.3 (pwa153) toolkit
libraries inside DOSBox. Each build compiles all 119 modules across the
4 memory models (small/medium/compact/large) and assembles the four
`.LIB` files, echoing each step so you can watch progress.

## Scripts

**Entry point: `BUILD.BAT`.** Everything starts from `MAIN/`, and inside
that from this dispatcher. Per-compiler and per-version scripts are
called *by* `BUILD.BAT` — you rarely invoke them directly.

| Script | Builds | Compiler needed |
|--------|--------|-----------------|
| `BUILD.BAT` | top-level dispatcher (targets: `borland` / `turbo` / `msc` / `pwa153` / `all` / `clean` / `mrproper` / `status` / `help`, plus stubs for `pwa1522` / `pwa154` / `delta154` / `irc1541`) | — |
| `BLDKBC.BAT` | PCBKBC{S,M,C,L}.LIB | Borland C++ 3.1 (`C:\BC31`) |
| `BLDKIT.BAT` | PCBKIT{S,M,C,L}.LIB | Turbo C 2.01 (`C:\TC201`) |
| `BLDKMS.BAT` | PCBKMS{S,M,C,L}.LIB (476 steps, needs 386MAX for DPMI) | Microsoft C 7.0 (`C:\MSC70`) + TASM |
| `MKLIB.BAT` | assembles OBJ → LIB (called by the above) | BC31 TLIB |

`*.RSP` are TLIB response files (one per model per compiler, ANSI-first
ordering) consumed by `MKLIB.BAT`.

`attic/` holds superseded scripts (currently `BLDMENU.BAT`, the old
interactive CHOICE-based menu that `BUILD.BAT` replaced). See
`attic/README.md`.

## How to run

Inside DOSBox / real DOS (with the repo staged and the compiler present):

```
C:
cd \BUILD\SCRIPTS
BUILD help          rem see all targets
BUILD borland       rem PCBKBC only
BUILD msc           rem PCBKMS only  (needs 386MAX loaded via CONFIG.SYS)
BUILD pwa153        rem all three compilers, clean first
BUILD all           rem alias for pwa153 today (only working version)
BUILD status        rem which .LIBs are present
```

Unknown target → prints help + exits 1. Every real target propagates
errorlevel from the called script, so `BUILD` is safe to chain (the
`pwa153` target stops on the first failing compiler).

The DOSBOXX.ZIP environment sets the paths up for you. On the FreeDOS
golden build image (see `MAIN/build/PCBKMS-BUILD-SETUP.md`),
`AUTOEXEC.BAT` calls `BUILD` with whatever target the operator passes.

### Target reference

Per-compiler (pwa153 only today):

- `borland` (aliases: `bldkbc`) → BLDKBC.BAT → PCBKBC libs
- `turbo` (aliases: `bldkit`) → BLDKIT.BAT → PCBKIT libs
- `msc` (aliases: `mvc`, `bldkms`) → BLDKMS.BAT → PCBKMS libs

Per-version (build all applicable compilers for one version tree):

- `pwa153` — the working target. clean + borland + turbo + msc.
- `pwa1522` — **STUB**. Source for the pre-15.3 toolkit hasn't been
  located; see `toolkit/PWA1522-FUTURE.md`.
- `pwa154` — **STUB**. Framework in place (toolkit/pwa154 +
  pcb153/upd154 + OUT/pwa153/upd154) but needs a build-fix pass (STATS
  OBJ, a few header resolutions) before compile succeeds.
- `delta154` — **STUB**. Toolkit needs updating before compile is
  possible. When it is, output goes to `OUT/delta154/` (its own tree,
  not shared with pwa153).
- `irc1541` — **STUB**. OpenWatcom2irc path; built with
  `verta1878/ow2irc`, not this dispatcher.

Meta:

- `all` — currently aliases `pwa153` (only working version). Will
  expand as more versions come online.
- `clean` — delete all OBJs under `OUT\LIB\*\OBJ\`, keep .LIB files.
- `mrproper` — delete OBJs *and* .LIB files, keep the directory tree.
- `status` — list .LIB files present per version.

Stubs exit with errorlevel 2 and a message pointing at the doc that
explains why they aren't buildable yet.

## Expected layout (mounted as C:)

```
C:\BC31              Borland C++ 3.1   (for PCBKBC)
C:\TC201             Turbo C 2.01      (for PCBKIT; incl. TASM + TURBOC.CFG)
C:\TOOLKIT\PWA153    toolkit source + H
C:\PCB153\SOURCE     main source + H (18 shared modules)
C:\BUILD\SCRIPTS     these scripts
```

## Notes

- Turbo C builds rely on `TURBOC.CFG` in `TC201\BIN` for include paths
  (keeps command lines short) and on the `malloc.h`/`direct.h` shims in
  `TC201\INCLUDE`.
- The shared headers are compiler-guarded (VIRTUAL.C-style): the same
  source builds under both Borland C++ and Turbo C. See the toolkit
  notes for the bool / EXTERN_C / CDCCONST / `__TURBOC__` guards.
- Libraries are assembled with BC31's TLIB (Turbo C 2.0's TLIB has a
  small page-size limit).


## Object output convention (compiler-first, Clark-inspired)

Objects and libraries are grouped under each compiler at the top level
of `OUT\LIB\PWA153\` — this matches Clark's own `bcdos\bc31\` root
convention, just simplified for the shipped SDK (one flat model dir
instead of per-source-subdir):

```
OUT\LIB\PWA153\
  bc31\                       Borland C++ 3.1
    PCBKBCS.LIB              .LIB files at compiler root
    PCBKBCM.LIB              (Clark convention)
    PCBKBCC.LIB
    PCBKBCL.LIB
    OBJ\
      small\*.OBJ
      medium\*.OBJ
      compact\*.OBJ
      large\*.OBJ
    loose-obj\*.OBJ           NODISP, PCBDAT, SMALLERR, NO* stubs
  tc201\                      Turbo C 2.01
    PCBKITS.LIB
    PCBKITM.LIB
    PCBKITC.LIB
    PCBKITL.LIB
    OBJ\{small,medium,compact,large}\*.OBJ
    loose-obj\*.OBJ
  msc70\                      Microsoft C 7.0
    PCBKMSS.LIB
    PCBKMSM.LIB
    PCBKMSC.LIB
    PCBKMSL.LIB
    OBJ\{small,medium,compact,large}\*.OBJ
    loose-obj\*.OBJ
```

Everything for one compiler lives under its own root — easy to zip,
ship, or nuke per compiler. `MKLIB.BAT` takes `<LIBNAME> <CC> <COMPDIR>`
so the LIBs land at `PWA153\<COMPDIR>\<LIBNAME>_?.LIB`.

## AUTOEXEC.BAT

DOSBOXX.ZIP ships a real `BUILD\AUTOEXEC.BAT` (the DOSBOX.CFG
[autoexec] section just mounts C: and calls it). Sysops can read and
edit AUTOEXEC.BAT directly. On start it offers (via CHOICE) to launch
the build menu, and always prints the manual commands.

## Response files

TLIB response files use 8.3-compliant names: `<CC><M><chunk>.RSP`
e.g. `TCLA.RSP` = Turbo C, Large model, chunk A. Each model is built
in two chunks (a/b) because TLIB's response buffer can't take all 119
deep paths at once. MKLIB handles this automatically.


## Compiler location

The Borland C++ 3.1 (BC31) and Turbo C 2.01 (TC201) toolchains are NOT
shipped at the repo root. Get them one of two ways:

1. Use DOSBOXX.ZIP - it bundles both compilers, dosbox-x, and these
   build scripts. Just extract and run (the autoexec sets everything up).
2. Extract the standalone archives into place: PCB153BT.ZIP -> C:\BC31,
   TC201BT.ZIP -> C:\TC201 (when the repo is mounted as C:).

The scripts expect C:\BC31\BIN and C:\TC201\BIN on the path.
