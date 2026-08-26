# pcbcbase — CODEBASE dBase library + prebuilt toolkit libs

Third-party dependency: the **CodeBase** dBase/xBase-compatible database
library by Sequiter Software, plus a set of prebuilt Borland toolkit
libraries. PCBoard uses CodeBase for its dBase III/IV file access — the
file-directory system and the PPL `DBASE` functions read and write
.DBF/.MDX/.NTX files through this library.

License: LGPL v3.0.

## Layout

```
pcbcbase/
  CODEBASE/
    SOURCE/          the CodeBase library source (114 files)
  PREBUILT/
    BC31/            prebuilt .LIB files (Borland C++ 3.1, large model)
```

### CODEBASE/SOURCE

The CodeBase API source, organized by the library's family-letter
convention:

- `c4*` — core / low-level (codes, BCD, constants)
- `d4*` — data files: open/close/append/pack the .DBF tables
- `e4*` — error handling
- `f4*` — fields
- `i4*` — index files (.MDX / .NTX)
- `r4*` — relations / reports
- `s4*`, `u4*`, `m4*` — strings, utilities, memo files
- `d4all.h` — the umbrella header pulling the API together

This is the dBase engine PCBoard links against; it is compiled into the
toolkit and the main program wherever dBase file access is needed.

### PREBUILT/BC31

Ready-built Borland C++ 3.1 **large-model** libraries, so you can link
without rebuilding CodeBase and the toolkit from source. Includes the
CodeBase-backed toolkit libs (PCB_L, TOOLKIT_L, SCREEN_L, SCRNIO_L,
DOS_L, DOSCLS_L, SYSTEM_L, MISC_L, COUNTRY_L) and `PCBKIT_L.LIB`, each
with its matching `.386` overlay. Both upper- and lower-case names are
present for case-sensitive (Linux/OpenWatcom) build hosts.

## Usage in pcbrevival

- The 15.3 / 15.4 PWA builds (Borland C++ 3.1) can link the prebuilt
  BC31 libraries directly, or rebuild CodeBase from CODEBASE/SOURCE.
- The PPL `DBASE` statement family depends on this library — PPEs that
  open .DBF files route through CodeBase.
- Build flags reference `-DDBASE` (see MAIN/README.md); CodeBase is the
  library that flag expects.

## Note

This is upstream third-party source kept for building. The pristine
distribution archives for the wider toolset live in `devtools/`; these
extracted CodeBase sources and prebuilt libs are the form the build
uses.
