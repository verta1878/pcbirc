# patches/386max-tasm — 386MAX downgrade patch

Transforms sudleyplace/386MAX source (originally built with MASM 5.10/6.0
+ CL 8 + LINK 5.60 on BOB's 1997 network-mounted workstation) into a
form the crew's TASM 3.1 + TLINK toolchain (present on `BUILDROOT/`)
can build.

## Files

| File | Purpose |
|---|---|
| `386max-tasm.patch` | Unified diff, 144 files changed, ~35,500 lines. Captures the 12 sed rules + 6 per-file surgeries from `MAXBLD1.BAT`. Human-readable source deltas only — the macro-scoped `@@`/`@F`/`@B` relabeling and bt-family symbol-size annotation performed by `xform.awk` and `xform-bt.awk` happen at build time and are not in this patch (they'd explode the diff to unreviewable size for zero review value). |
| `README.md` | this file |

## Applying

To the sudleyplace source tree (unpacked from `devtools/386max.7z`):

```
cd 386max-src/386MAX          # this is KERNEL/ in the patch
cd 386max-src/INC             # this is INC/ in the patch
patch -p1 < path/to/386max-tasm.patch
```

Or applied against a KERNEL/INC layout mirroring the patch structure.

**Important**: this patch alone is not enough to assemble the source
with TASM. The build pipeline (`MAXBLD1.BAT` + `MAXBLD2.BAT`) also runs
`xform-bt.awk` and `xform.awk` on the sed output before assembly. Skip
those and TASM will fail on macro-scoped labels and undersized bt-family
operands.

## Recipe reference

See `todo/386max-build-downgrade.md` at repo root for:

- The full 12 sed-rule list with rationale
- xform-bt.awk two-pass symbol-size annotator
- xform.awk MACRO-scoped @@/@F/@B relabeling
- Gate 0/1/1.5/2 progress tracking
- Iteration history (94.30% → 95.85% → 102.60%)

## Build status (as of patch generation)

- Gate 1 (all assemblies succeed): ✓ 92/92, 0 fail
- Gate 1 SYS size: 235,224 bytes (102.60% of shipped Qualitas 229,268)
- Gate 1.5 (byte-verified): partial — layout differs due to TASM 3.1
  vs MASM 5.10 encoding, our bt-family word-ptr widening, alignment
- Gate 2 (loads under DEVICE=): pending — needs 86Box/PCem/QEMU with
  real DOS/FreeDOS, or DOSBox-X 2026.06+ with `[devices]` section

## License

GPLv3 — patch derives from sudleyplace/386MAX (Bob Smith's GPLv3
release of the original Qualitas code). Downstream: the built
386MAX.SYS is redistributable under the same terms.
