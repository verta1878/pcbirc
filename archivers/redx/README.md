# redx — RED archive extractor

Extracts files from WCSC's proprietary `.RED` archive format used
by PCBoard's `INSTALL.EXE`.

## Status

Scaffold. `redx_test.c` proves the approach with the first record of
`COMMDRV.RED` (`COMMDV00.DRV`, 1130 bytes). Uses vendored LHA LH5
decoder (bitio + huf + shuf + dhuf + slide + larc + maketbl + maketree)
plus `redx_glue.c` providing the globals `lharc.c` normally owns.

Current issue: LH5 `make_table()` returns "Bad table (case b)" —
Clark's `.RED` framing wraps LH5 payload in a proprietary header
that we need to strip or byte-align differently before feeding
LHA's decoder. Detailed analysis of `.RED` byte layout lives in
Phase 1 private notes (`/tmp/commdrv-work/ANALYSIS.md` on the
build workstation, not committed to public repo per Phase 1 rules).

## Files

- `redx_glue.c` — LHA library glue (globals + xmalloc/xfopen stubs)
- `redx_test.c` — proof-of-concept single-record extractor
- `config.h` — feature flags for the vendored LHA sources
- `redx_test` — compiled binary (rebuild with the gcc line in
  `redx_test.c`'s header comment)

## Build

```
gcc -o redx_test redx_test.c redx_glue.c \
    lha/src/{bitio,huf,shuf,dhuf,slide,larc,maketbl,maketree}.c \
    -I lha/src -DHAVE_CONFIG_H=0
```

Requires an `lha/src/` tree with the LHA reference implementation.
Vendored copy expected at `archivers/lha/src/` (not committed here
since LHA has its own license; download from tukaani.org or similar).

## Roadmap

- [x] Format identification (Phase 1)
- [x] Header byte layout mapped (Phase 1)
- [ ] Fix LH5 framing so single record extracts (`redx_test`)
- [ ] Multi-record extraction (walk the full manifest)
- [ ] CLI: `redx list foo.RED`, `redx extract foo.RED [file...]`
- [ ] Integration: `INSTALL.zip → COMMDRV.RED → 22 files → firmware/`
