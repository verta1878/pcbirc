# ROADMAP — pcbic12 byte-exact reconstruction

**Scope**: turning `bin/`'s reference binaries into buildable
clean-room source, byte-for-byte identical to the originals.
Covers all 6 EXEs + RUNINET.PPE.

**Order**: easiest wins first (source in hand), reverse-engineering
after. Every phase has a byte-exact acceptance test.

**Version labels (pcbic subsystem):**
- **pcbic v1.0** — reference material landed, structure in place.
  6 EXEs unlocked, `decrypted/` renamed to `bin/`, full doc pass.
- **pcbic v1.0.1** — RUNINET.PPE byte-exact.
- **pcbic v1.0.2** — TESTIC.EXE understood (Ghidra pass, C source
  exported).
- **pcbic v1.0.3** — TESTIC2.EXE understood (OS/2 sibling).
- **pcbic v1.1+** — the big four (PCBICEVT/PCBICCFG/Pcbic/Pcbic2)
  and byte-exact reconstruction beyond the two TESTICs.

---

## Phase 1 — RUNINET.PPE (pcbic v1.0.1)

**Status**: 90% there. See `RECONSTRUCTION.md` for the full trail.

- **Target**: `bin/RUNINET.PPE` (1,808 bytes, PPL 3.20 bytecode)
- **Source**: `bin/RUNINET.PPS` (3,895 bytes — decompiled, not
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
        edit bin/RUNINET.PPS      # ← actually copy to a working file, don't touch bin/
        PPLC320 RUNINET.PPS
        cmp our.PPE bin/RUNINET.PPE
        # note delta trend; keep tweaks that shrink, revert those that grow

**Estimate**: 4-8 focused sessions. Tractable — we have all tools.

**Acceptance**: `cmp -s rebuilt/RUNINET.PPE bin/RUNINET.PPE` exits 0.

---

## Phase 2 — The 6 EXEs (pcbic v1.0.2, v1.0.3, v1.1+)

The pcbic EXEs have no known source. Byte-exact reconstruction requires
disassembly-driven clean-room reimplementation. Same workflow for each.

**Targets** (in complexity order, easiest first):

| # | Binary       | Size        | Version tag | Notes |
|---|--------------|------------:|---|---|
| 1 | TESTIC.EXE   |  40,104 | v1.0.2 | Smallest, single-purpose (ping test). Best warm-up target. |
| 2 | TESTIC2.EXE  |  46,627 | v1.0.3 | OS/2 sibling of TESTIC. Compare with #1 to spot the OS/2 diff. |
| 3 | PCBICEVT.EXE |  89,612 | v1.1   | Event handler. Moderate. |
| 4 | PCBICCFG.EXE | 185,398 | v1.2   | Config UI. Big — lots of screen drawing code. |
| 5 | Pcbic.exe    | 313,310 | v1.3   | Main IC binary. Huge — TCP/IP stack integration, all 10 menu commands. |
| 6 | Pcbic2.exe   | 217,111 | v1.4   | OS/2 sibling of Pcbic.exe. |

### Per-EXE workflow (Ghidra Linux)

For each target:

1. **Import** into Ghidra, auto-analyze
   - Language: x86 16-bit real mode for DOS MZ, x86 32-bit protected
     for OS/2 LX
   - Loader: MS-DOS MZ or OS/2 LX
   - Borland C++ 4.5 or 5.0 likely toolchain (era + WCSC's known
     preferences)
2. **Identify library code** (don't reimplement RTL)
   - Match Borland C++ 4.5 / 5.0 startup + runtime signatures
   - Mark those functions as library, focus on application code
3. **Symbol recovery** from strings, RTTI, error messages
   - PCBoard convention: many functions named after their menu key
   - Cross-reference against `bin/PCBIC.HLP` topics for feature grouping
4. **Function-by-function decompile → C source**
   - Ghidra P-code as starting point
   - Clean up manually, add types, name variables
   - Save cleaned C to `src/<binary>/` in this repo
5. **Compile with matching toolchain**
   - Borland C++ 4.5 or 5.0 (in `reference/` — check availability)
   - Same optimization flags (`-O2 -Ox` typical for BC on DOS)
6. **Byte-diff**
   - `cmp -l rebuilt/<binary> bin/<original>`
   - Iterate on any deltas: usually one of {compiler version,
     library version, code order, string layout, padding}
7. **Acceptance**: `cmp -s`, no mismatches

### Repository layout (planned per binary)

    pcb1541/pcbic12/src/testic/
    ├── SPEC.md              (what this binary does, from Ghidra + docs)
    ├── GHIDRA-NOTES.md      (analysis log: function map, library IDs,
    │                         tricky areas, decisions)
    ├── testic.c             (main)
    ├── icmp.c               (ping implementation)
    ├── tcpio.c              (TCP stack glue)
    ├── Makefile             (or Turbo project file for BC)
    └── (built artifacts go to ../../rebuilt/testic/)

Same structure repeats for testic2, pcbicevt, pcbiccfg, pcbic, pcbic2.

**Estimate per binary**: TESTIC = 2-4 weeks focused work; Pcbic.exe =
2-6 months. This is not fast work but it's tractable one function at
a time.

**Acceptance per binary**: `cmp -s rebuilt/<binary> bin/<original>`.

---

## Phase 3 — INSTALL.EXE completion (adjacent, same workflow)

Not part of `pcbic12/` but same reconstruction pattern. Living at
`pcb1541/install/src/install.c` (518 lines, Phase 27 stub — 7 of
60 @-commands implemented).

**Current state**:
- 7 of 60 @-commands implemented (`@DefineProject`, `@DefineVars`,
  `@Display`, `@Cls`, `@Pause`, `@Abort`, `@Exit`)
- Decompression stubbed (was written BEFORE `redx` existed —
  redx now works, can be linked instead of stubbing)
- Reference INSTALL.EXE: 338,548 bytes, NE format, md5 `5239767b`
  (in `pcb1541/install/reference/INSTALL.EXE`)
- Reference INSTALL.DAT: 42,294 bytes, 72 @-directives, 60 unique
  (in `pcb1541/install/reference/INSTALL.DAT`)

**Approach**:

1. Wire redx into install.c's `@BeginLib` / `@File` path — already
   proven working via `pcb1541/install/dist/target/rebuild_place.py`
2. Ghidra the real INSTALL.EXE to confirm @-command semantics
3. Burn down the remaining 53 @-commands function-by-function
4. End-to-end test: install.c against real INSTALL.DAT under
   DOSBox-X, produce a target/ tree matching
   `pcb1541/install/dist/target/`
5. Byte-diff acceptance is v1.11+ (v1.10 gets us *understanding*
   and *end-to-end runs*; byte-exact is a separate arc)

See `HISTORY.md` "install v1.10" section for the phase breakdown.

---

## What Ghidra work looks like from Claude's side

Claude does not run Ghidra directly (sandbox has no Ghidra + no X
server for the GUI). The workflow:

- **You** (on your Linux box): run Ghidra, work through analysis,
  export decompiled C, commit `SPEC.md`, `GHIDRA-NOTES.md`, cleaned-up
  `*.c` files to `src/<binary>/`
- **Claude**: reviews the exports, spots Borland RTL patterns Ghidra
  didn't recognize, suggests structural refactors, writes missing
  headers/glue, wires up the Makefile, runs compile-diff cycles,
  documents phases in the work log

For the compile side, Claude can:

- Set up cross-compilation via OpenWatcom (in-sandbox) targeting DOS
- Where OpenWatcom byte-diff differs from Borland (it will), document
  the delta and identify what BC-specific patterns need matching
- Vendor Borland C++ 4.5 / 5.0 in a sandboxed DOSBox-X env if we can
  find a clean copy in `reference/`

## Repository additions this arc will introduce

- `pcb1541/pcbic12/src/<binary>/` — Ghidra-driven reverse-eng work,
  one subdir per binary (testic/, testic2/, pcbicevt/, pcbiccfg/,
  pcbic/, pcbic2/)
- `pcb1541/pcbic12/rebuilt/<binary>` — compiled outputs, byte-diffed
- `pcb1541/install/src/install.c` — completion (Phase 3 above)
- `docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md` — canonical
  @-command reference, grown as install v1.10 sub-phases land
- `toolkit/borland/` — vendored BC 4.5 / 5.0 if we can source it clean

## Priority order (recommended)

1. **RUNINET.PPE** (v1.0.1) — source is IN HAND, quick win, closes
   the pcbic loop for the one component we can actually finish soon
2. **INSTALL.EXE completion** (install v1.10) — leverages what we
   already built (redx, 7 @-commands, all decompression working) —
   completes the installer arc
3. **TESTIC.EXE** (v1.0.2) — first Ghidra target, warm-up before
   the big EXEs
4. **TESTIC2.EXE** (v1.0.3) — OS/2 sibling, isolates DOS/OS2 delta
5. **PCBICEVT.EXE** (v1.1) — moderate complexity, event handler
   patterns are common enough to make progress steadily
6. **PCBICCFG.EXE** → **Pcbic.exe** → **Pcbic2.exe** (v1.2, v1.3, v1.4)
   — accepting these are multi-month efforts each

---

*pcbic v1.0 opens the arc — reference in `bin/`, source targets defined,
tooling in place. v1.0.1 starts the burn-down.*
