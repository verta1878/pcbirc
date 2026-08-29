# 386MAX build scripts

Runtime transforms + build orchestrator for building
sudleyplace/386MAX 8.03 under TASM 3.1 + TLINK 5.1 in a DOSBox-X
guest booting `PCBBLDBT.IMG`.

## Files

**Transforms** (run at build time, source stays untouched on disk):

- `xform.sed` — sed rules: comment `.xcref`, rename `@Version` to
  `?VERSION`, drop `at`-address group definitions (RGROUP/AGROUP/
  PSPGRP/CGROUP) and rename references, rewrite `loop dword ptr`
  to `loopd`, strip `.EDD` type-hint from `loop` targets.
- `xform.awk` — expand MASM 6 anonymous labels: `@@:` becomes
  `LBL_1:`/`LBL_2:`/..., `@F` becomes next label, `@B` becomes
  previous label. Word-bounded so `@8042_ST` stays intact.
- `xform-bt.awk` — two-pass symbol-size annotator for
  `bt`/`bts`/`btr`/`btc` instructions. Pass 1 scans all sources
  for `SYMBOL db/dw/dd` definitions and emits a symbol-size
  table. Pass 2 uses the table to inject `byte ptr` / `word ptr`
  / `dword ptr` before memory operands that TASM 3.1 can't
  size-infer from context.

**Build orchestrators** (pick ONE workflow):

- `MAXBUILD.BAT` — full 9-phase pipeline in one boot. Convenient
  but may exceed DOSBox-X boot budget on slower hosts.

- `MAXBLD1.BAT` + `MAXBLD2.BAT` — two-boot split for slower
  environments:
    - `MAXBLD1.BAT` runs phases 0-5 (preprocess only). Populates
      `\386MAX_S\XFORM\` and `\386MAX_S\XINC\` with transformed
      source. Writes `\TMP\P1_DONE.TXT` on success.
    - `MAXBLD2.BAT` runs phases 6-9 (HDPMI16 load, TASMX
      assemble, HDPMI16 unload, TLINK). Writes `\TMP\P2_DONE.TXT`
      on success and copies `386MAX.SYS` + `386MAX.MAP` to
      `\386MAX_S\BIN\`.

Filenames use `MAXBLD1`/`MAXBLD2` rather than `MAXBLD-P1`/`MAXBLD-P2`
because DOS 8.3 truncates the dashed names to `MAXBLD~1`/`MAXBLD~2`.
