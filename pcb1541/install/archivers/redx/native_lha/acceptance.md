# Native LHA encoder — ACCEPTANCE TESTS

Four tests must all pass before native encoder is promoted to default
in `red_pack.c`. Each test is scripted and reproducible.

## Test 1 — round-trip integrity

**Setup**: for each of the 6 install `.RED` archives:

    COMMDRV.RED  PCBCFGS.RED  PCBMAIL.RED
    PCBOARD.RED  PCBOARD2.RED  PPLC.RED

**Procedure**:
1. Extract all records with `redx extract` → per-record file
2. Repack each record with `redx pack --native --record <file>`
3. Extract the repacked output with `redx extract`
4. `cmp` original record bytes vs re-extracted bytes

**Pass criterion**: `cmp -s` succeeds on every record. Zero
mismatches across all 6 archives.

**Rationale**: proves our encoder + our decoder are consistent,
regardless of external tools.

## Test 2 — interop out (external lha decodes our output)

**Procedure**:
1. Take one record repacked with `--native` (level-0 LHA stream
   inside `.RED` method-0x000B envelope)
2. Strip the WCSC 2-byte prefix + `.RED` framing → raw level-0 LHA
3. Save as `.lzh` file
4. `lha x file.lzh` (using vendored `../../lha/src/lha`)
5. `cmp` extracted bytes vs original record

**Pass criterion**: lha 1.14i decodes cleanly, extracted bytes match.
Run against all 6 archives × ≥3 records each = 18+ test cases.

**Rationale**: proves we're emitting valid LH5, not just something
our own decoder happens to accept.

## Test 3 — interop in (regression: existing decoder still works)

**Procedure**: run the current test harness against all 6 install
archives without any changes to `red_pack.c` or the decoder path:

    for arc in COMMDRV.RED PCBCFGS.RED PCBMAIL.RED PCBOARD.RED PCBOARD2.RED PPLC.RED; do
        redx extract "$arc" -o /tmp/extract_$arc/
        # verify byte-perfect against known-good MD5s
        md5sum -c "expected_$arc.md5"
    done

**Pass criterion**: all 481 target files byte-perfect (matches
current install v1.7.2 state).

**Rationale**: adding native encoder must not break decode path.
Regression test to be run after every commit in the v1.8.1.x series.

## Test 4 — compression effectiveness

**Procedure**: for each of the 6 install archives, compare compressed
sizes:

    orig_size  = payload size in original WCSC .RED archive
    lha_size   = payload size when repacked via shell-out to lha 1.14i
    native_size = payload size when repacked with --native

Compute `native_size / lha_size` per archive.

**Pass criterion**: mean ratio ≤ 1.10 (native within 10% of
lha 1.14i's ratio). No individual archive above 1.20.

**Rationale**: we don't need to beat lha 1.14i, but we must be
competitive — otherwise `.RED` archives balloon and PCBoard 15.41
install disks won't fit their historical 1.44 MB budget.

## Test harness location

Once implemented:

    pcb1541/install/archivers/redx/native_lha/tests/
    ├── run_all.sh              (drives all 4 tests)
    ├── test1_roundtrip.sh
    ├── test2_interop_out.sh
    ├── test3_interop_in.sh
    ├── test4_ratio.sh
    └── expected/
        ├── COMMDRV.RED.md5
        ├── PCBCFGS.RED.md5
        └── ... (one .md5 file per install archive, 481 lines total)

Not built yet — lands with install v1.8.1.1.

## Promotion criteria

Native encoder becomes the default (`--legacy-lha` for the shell-out
path) when:

- [ ] Test 1 passes on all 6 archives, all records
- [ ] Test 2 passes on all 6 archives, ≥3 records each
- [ ] Test 3 passes (no regression, all 481 files byte-perfect)
- [ ] Test 4 passes (ratio ≤ 1.10 mean, ≤ 1.20 max)

Target: install v1.8.1.5.

Until then: shell-out to lha 1.14i remains the default. Native
encoder available via `--native` flag for testing.
