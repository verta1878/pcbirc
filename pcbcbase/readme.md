# pcbcbase — CODEBASE dBase library (source only)

Third-party dependency: the **CodeBase** dBase/xBase-compatible database
library by Sequiter Software. PCBoard uses CodeBase for its dBase III/IV
file access — the file-directory system and the PPL `DBASE` functions
read and write .DBF/.MDX/.NTX files through this library.

License: LGPL v3.0.

## Layout

```
pcbcbase/
  CODEBASE/
    SOURCE/          the CodeBase library source (114 files)
```

Source only. `PREBUILT/BC31/` used to sit here; it was moved to
`attic/prebuilt-libs/BC31/` on 2026-09-22 (see below).

### CODEBASE/SOURCE

The CodeBase API source, organized by the library's family-letter
convention:

- `c4*` â core / low-level (codes, BCD, constants)
- `d4*` â data files: open/close/append/pack the .DBF tables
- `e4*` â error handling
- `f4*` â fields
- `i4*` â index files (.MDX / .NTX)
- `r4*` â relations / reports
- `s4*`, `u4*`, `m4*` â strings, utilities, memo files
- `d4all.h` â the umbrella header pulling the API together

This is the dBase engine PCBoard links against; it is compiled into the
toolkit and the main program wherever dBase file access is needed.

### What happened to PREBUILT/BC31

Nine prebuilt `.386` libraries used to live in `pcbcbase/PREBUILT/BC31/`.
They were **not CodeBase**: they are PCBoard's own category libraries
(COUNTRYL, DOSCLS_L, DOS_L, MISC_L, PCB_L, SCREEN_L, SCRNIO_L, SYSTEM_L,
TOOLKITL) — the output of `toolkit/pwa153/SOURCE/<DIR>/MAKEFILE`, filed
under the wrong project.

They now live in `attic/prebuilt-libs/BC31/`, with a README recording
each library's module list (read out of the OMF records) and sha256, so
a future rebuild can be checked against them. Their origin is unknown
and the project cannot rebuild them yet — the toolkit MAKEFILEs are
hard-coded to `bc31` / `large.386` and point at a `bcdos\bc31` folder
that does not exist.

There is no prebuilt CodeBase library in the repo: `C4BASE.LIB` (DOS)
and `B4.LIB` (OS/2) have to be built from `CODEBASE/SOURCE`.

## Usage in pcbrevival

- The 15.3 / 15.4 PWA builds (Borland C++ 3.1) compile CodeBase from
  CODEBASE/SOURCE; the result belongs in
  `OUT/<branch>/SDK/<compiler>/LIB/`.
- The PPL `DBASE` statement family depends on this library â PPEs that
  open .DBF files route through CodeBase.
- Build flags reference `-DDBASE` (see MAIN/README.md); CodeBase is the
  library that flag expects.

## Note

This is upstream third-party source kept for building. The pristine
distribution archives for the wider toolset live in `devtools/`; these
extracted CodeBase sources are the form the build uses.
