# ROADMAP — pcbic and adjacent byte-exact reconstructions

**Scope**: turning our reference binaries into buildable clean-room
source, byte-for-byte identical to the originals. Covers pcbic
(6 EXEs + 1 PPE) and the adjacent `INSTALL.EXE` completion (same kind
of work, same Ghidra workflow).

**Order**: easiest wins first (source in hand), reverse-engineering
after. Every phase has a byte-exact acceptance test.

---

## Phase 1 — RUNINET.PPE (source in hand, needs fidelity tweaks)

**Status**: 90% there. See `RECONSTRUCTION.md` for the full trail.

- **Target**: `decrypted/RUNINET.PPE` (1,808 bytes, PPL 3.20 bytecode)
- **Source**: `decrypted/RUNINET.PPS` (3,895 bytes — decompiled, not
  original)
- **Compiler**: `toolkit/pplc/3.20/PPLC320.EXE` (in tree, works under
  DOSBox-X)
- **Current build**: 2,261 bytes when we compile RUNINET.PPS with
  PPLC 3.20 — 453 bytes bigger than target

**Gap**: source drift. The decompiled PPS emits valid PPL that
recompiles to correct behavior, but different bytecode. Every
whitespace / statement-order / declaration-order choice by the
decompiler shows up in the compiled output.

**Approach**: iterative source tweaking with cheap compile-diff loop.

    for tweak in whitespace, ordering, style:
        edit RUNINET.PPS
        PPLC320 RUNINET.PPS
        cmp our.PPE decrypted/RUNINET.PPE
        # note delta trend; keep tweaks that shrink, revert those that grow

**Estimate**: 4-8 focused sessions. Tractable — we have all tools.

**Acceptance**: `cmp -s our.PPE decrypted/RUNINET.PPE` exits 0.

---

## Phase 2 — The 6 EXEs (no source, need reverse engineering)

The pcbic EXEs have no known source. Byte-exact reconstruction requires
disassembly-driven clean-room reimplementation. Same workflow for each.

**Targets** (in complexity order, easiest first):

| # | Binary       | Size        | Notes |
|---|--------------|------------:|-------|
| 1 | TESTIC.EXE   | 40,104      | Smallest, single-purpose (ping test). Best warm-up target. |
| 2 | TESTIC2.EXE  | 46,627      | OS/2 sibling of TESTIC. Compare with #1 to spot the OS/2 diff. |
| 3 | PCBICEVT.EXE | 89,612      | Event handler. Moderate. |
| 4 | PCBICCFG.EXE | 185,398     | Config UI. Big — lots of screen drawing code. |
| 5 | Pcbic.exe    | 313,310     | Main IC binary. Huge — TCP/IP stack integration, all 10 menu commands. |
| 6 | Pcbic2.exe   | 217,111     | OS/2 sibling. Compare with #5. |

### Per-EXE workflow (Ghidra Linux)

For each target:

1. **Import** into Ghidra, auto-analyze
   - Language: x86 16-bit real mode (Borland C++ likely, based on
     PCBoard's toolchain)
   - Loader: MS-DOS MZ or NE (check header)
2. **Identify library code** (don't reimplement RTL)
   - Match Borland C++ 4.5 / 5.0 startup + runtime signatures
   - Mark those functions as library, focus on application code
3. **Symbol recovery** from strings, RTTI, error messages
   - PCBoard convention: many functions named after their menu key
   - Cross-reference against `PCBIC.HLP` topics for feature grouping
4. **Function-by-function decompile → C source**
   - Ghidra P-code as starting point
   - Clean up manually, add types, name variables
   - Save cleaned C to `src/<binary>/` in this repo
5. **Compile with matching toolchain**
   - Borland C++ 4.5 or 5.0 (in `reference/` — check)
   - Same optimization flags (`-O2 -Ox` typical for BC on DOS)
6. **Byte-diff**
   - `cmp -l our.exe decrypted/original.exe`
   - Iterate on any deltas: usually one of {compiler version,
     library version, code order, string layout, padding}
7. **Acceptance**: `cmp -s`, no mismatches

### Repository layout (planned per binary)

    pcb1541/pcbic/reversed/testic/
    ├── SPEC.md              (what this binary does, from Ghidra + docs)
    ├── GHIDRA-NOTES.md      (analysis log: function map, library IDs, tricky
    │                         areas, decisions)
    ├── src/
    │   ├── testic.c         (main)
    │   ├── icmp.c           (ping implementation)
    │   ├── tcpio.c          (TCP stack glue)
    │   └── ...              (as decomposed from Ghidra)
    ├── build/               (build outputs — gitignored)
    ├── Makefile             (or Turbo project file for BC)
    └── BYTE-DIFF.log        (results of cmp against decrypted/TESTIC.EXE)

Same structure repeats for testic2, pcbicevt, pcbiccfg, pcbic, pcbic2.

**Estimate per binary**: TESTIC = 2-4 weeks focused work; Pcbic.exe =
2-6 months. This is not fast work but it's tractable one function at
a time.

**Acceptance per binary**: `cmp -s built.exe decrypted/original.exe`.

---

## Phase 3 — INSTALL.EXE completion (adjacent, same workflow)

Not part of `pcbic/` but same reconstruction pattern. Living at
`pcb1541/install/src/install.c` (518 lines, Phase 27 stub).

**Current state**:
- ~5 of 40 @Commands implemented, rest are TODO stubs
- Decompression stubbed (was written BEFORE `redx` existed —
  redx now works, can be linked instead of stubbing)
- Target: `pcb1541/install/dist/target/INIT.EXE` and the real
  `INSTALL.EXE` from disk 1 (need to identify — probably in the
  extracted `abpb1531.zip` set that shipped install v1.7.1)

**Approach**:

1. Ghidra the real INSTALL.EXE (331,310 bytes per install.c header
   comment)
2. Map all 40 @Commands used by PCBoard's actual INSTALL.DAT — most
   already have names from the string scan
3. Rewrite `install.c` function-by-function, using redx for the
   decompression paths that were previously stubbed
4. Byte-diff against original
5. Iterate

Same acceptance bar: `cmp -s`.

---

## What Ghidra work looks like from Claude's side

Claude does not run Ghidra (sandbox has no Ghidra + no X server). The
workflow:

- **You** (on your Linux box): run Ghidra, work through analysis,
  export decompiled C, commit `SPEC.md`, `GHIDRA-NOTES.md`, cleaned-up
  `src/*.c`
- **Claude**: review the exports, spot Borland RTL patterns Ghidra
  didn't recognize, suggest structural refactors, write missing
  headers/glue, wire up the Makefile, run compile-diff cycles,
  document phases in the work log

For the compile side, Claude can:

- Set up cross-compilation via OpenWatcom (in-sandbox) targeting DOS
- Where OpenWatcom byte-diff differs from Borland (it will), document
  the delta and identify what BC-specific patterns need matching
- Vendor Borland C++ 4.5 / 5.0 in a sandboxed DOSBox-X env if we can
  find a clean copy in `reference/`

## Repository additions this phase will introduce

- `pcb1541/pcbic/reversed/` — Ghidra-driven reverse-eng work per binary
- `pcb1541/install/src/install.c` — completion of Phase 27 with real
  decompression via redx
- `docs/GHIDRA-WORKFLOW.md` — shared workflow doc, referenced from
  every `reversed/<binary>/GHIDRA-NOTES.md`
- `toolkit/borland/` — vendored BC 4.5 / 5.0 if we can source it clean

## Priority order (recommended)

1. **RUNINET.PPE** (Phase 1) — source is IN HAND, quick win, closes
   the pcbic loop for the one component we can actually finish soon
2. **INSTALL.EXE completion** (Phase 3) — leverages what we already
   built (redx, 5 @Commands, all decompression working) — completes
   the installer arc
3. **TESTIC.EXE** (Phase 2, target #1) — first Ghidra target,
   warm-up before the big EXEs
4. **PCBICEVT.EXE** (Phase 2, target #3) — moderate complexity, event
   handler patterns are common enough to make progress steadily
5. **PCBICCFG.EXE** → **Pcbic.exe** → **Pcbic2.exe** (rest of Phase 2)
   — accepting these are multi-month efforts each

`TESTIC2.EXE` slots in with #1 (same code, OS/2 variant) and
`Pcbic2.exe` with #5 (same code, OS/2 variant).

---

*install v1.9+ arc opens here.*
