# pwa153 Toolkit SDK — Build Scripts

Runnable DOS batch files that build the PCBoard 15.3 (pwa153) toolkit
libraries inside DOSBox. Each build compiles all 119 modules across the
4 memory models (small/medium/compact/large) and assembles the four
`.LIB` files, echoing each step so you can watch progress.

## Scripts

| Script | Builds | Compiler needed |
|--------|--------|-----------------|
| `BLDMENU.BAT` | interactive menu (CHOICE) to pick a target | — |
| `BLDKBC.BAT` | PCBKBC_S/M/C/L.LIB | Borland C++ 3.1 (`C:\BC31`) |
| `BLDKIT.BAT` | PCBKIT_S/M/C/L.LIB | Turbo C 2.01 (`C:\TC201`) |
| `BLDKMS.BAT` | PCBKMS (placeholder) | Microsoft C 7.0 (`C:\MSC70`) |
| `MKLIB.BAT` | assembles OBJ → LIB (called by the above) | BC31 TLIB |

`*.RSP` are TLIB response files (one per model per compiler, ANSI-first
ordering) consumed by `MKLIB.BAT`.

## How to run

Inside DOSBox (with the repo mounted as `C:` and the compiler present):

```
C:
cd \BUILD\SCRIPTS
BLDMENU
```

…or run one directly, e.g. `BLDKIT` for the Turbo C build.

The DOSBOXX.ZIP environment sets this up for you and offers to launch
the menu on start (see its DOSBOX.CFG autoexec).

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


## Object output convention (matches Clark)

Objects are written to Clark's directory convention:

```
OUT\LIB\PWA153\OBJ\<compiler>\<model>\
```

where `<compiler>` is `bc31` / `tc201` / `msc70` (Clark's CVER names)
and `<model>` is `small` / `medium` / `compact` / `large` (Clark's
model dir names, from his MODELS.BAT). So a sysop building with the
scripts gets the same layout Clark used - nothing to change.

The loose override OBJs (NODISP, PCBDAT, the NO* stubs, SMALLERR) live
in `OUT\LIB\PWA153\loose-obj\<compiler>\`.

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
