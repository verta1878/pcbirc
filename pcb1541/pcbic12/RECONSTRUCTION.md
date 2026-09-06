# IC Reconstruction — bit by bit

Rebuild each IC component byte-for-byte, verifying against the shipped
originals in `bin/`. Fix bugs only AFTER byte-exact restoration.

## Targets (byte-exact) and status

| Component     | Original    | Version tag | Status |
|---|---|---|---|
| RUNINET.PPE   |   1,808 B   | pcbic v1.0.1 | BLOCKED — source is decompiled (63 vs 39 variables, see below) |
| TESTIC.EXE    |  40,104 B   | pcbic v1.0.2 | disassembly pending — smallest, easiest warm-up |
| TESTIC2.EXE   |  46,627 B   | pcbic v1.0.3 | OS/2 LX sibling — compare to TESTIC to isolate delta |
| PCBICEVT.EXE  |  89,612 B   | pcbic v1.0.4 | no source yet |
| PCBICCFG.EXE  | 185,398 B   | pcbic v1.0.5 | no source yet |
| Pcbic.exe     | 313,310 B   | pcbic v1.0.6 | no source yet (main binary) |
| Pcbic2.exe    | 217,111 B   | pcbic v1.0.7 | no source yet (OS/2 sibling of Pcbic.exe) |

**pcbic v1.1** = whole arc complete (all 7 targets `cmp -s` verified).

## RUNINET.PPE — first verifiable target (pcbic v1.0.1)

We have `bin/RUNINET.PPS` (the PPL source, 3,895 bytes). Compiling it
should reproduce `bin/RUNINET.PPE` (1,808 bytes) byte-for-byte.

**Finding:** Clark's RUNINET.PPE header says **PPL 3.20** (PCBoard
15.22 era). Our newer compilers produce different bytecode:
- `pcb153/PPLC` = 3.30 (15.3)     → 2,286 B output
- `pcb153/upd154/PPLC` = 3.40 (15.4) → 2,286 B output
- `toolkit/pplc/3.20/PPLC320.EXE`   → **2,261 B** output (correct compiler, wrong source)

**Root cause (found 2026-09-06):** PPE header byte 48 = variable count.
Ours: 0x3F (63 variables). Clark's: 0x27 (39 variables). The decompiler
created 24 extra implicit temporaries by expanding inline expressions
into named variables. This accounts for the entire 453-byte gap.
The fix is source refactoring, not compiler changes.

To match byte-for-byte we needed **PPLC 3.20** — and we have it.

### PPLC 3.20 located and tested

**PPLC 3.20**: build from `pcb153/SOURCE/PPL/` source (change
`CUR_PPE_VER` to 320 in `NEWSCR.CPP`). Shipped binary also in repo at
`reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`
(222 KB, extracted from `reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`,
md5 `2a23e7686f79ea07bbb3c4d04e064a75`, identifies as "PPLC Version
3.20 - Copyright (C) 1993-95, Clark Development Company, Inc.").

**Compile test:** Ran PPLC 3.20 against `bin/RUNINET.PPS` under
DOSBox-X. Result: clean compile ("Source compilation complete"), output
**2,261 bytes** — different from both the 1,808-byte reference AND
from the 2,286-byte outputs we get from PPLC 3.30/3.40. So the
compiler version was necessary but not sufficient: the *source* has
also drifted.

**What that means for byte-exact reconstruction:** the `RUNINET.PPS`
we have is a decompile (from `.PPE` via PPLD/similar). Decompilers
approximate — they emit valid PPL that recompiles to the same
behavior, not necessarily the same bytecode. Every whitespace or
statement-order choice by the decompiler shows up in the bytecode. To
recover Clark's exact 1,808-byte PPE we'd need:

- Clark's *original* handwritten `.PPS` source (probably lost unless
  it turns up in someone's archive), OR
- A better decompile that matches Clark's stylistic choices more
  closely (partial-recovery approach), OR
- Iterative source tweaking (adjust whitespace, statement grouping,
  variable declaration order) until output shrinks to 1,808 —
  labor-intensive but tractable in principle since we can now compile
  cheaply and diff.

The compiler-version question is *closed*; the source-fidelity
question is open.

Our tweak loop writes to a local working file (not `bin/`),
preserving Clark's originals as reference:

```
cp bin/RUNINET.PPS <local-work>/RUNINET.PPS      # working copy
edit <local-work>/RUNINET.PPS
PPLC320 <local-work>/RUNINET.PPS
cmp -l <local-work>/RUNINET.PPE bin/RUNINET.PPE  # measure the gap
```

For historical reference, an earlier compile with the WRONG compiler
version (PPLC 3.40 instead of 3.20) produced 2,286 B — versus 2,261 B
from PPLC 3.20 and the 1,808 B target from Clark's `bin/RUNINET.PPE`.
Compiler-version question is closed; source-fidelity is the open work.

## The EXEs (pcbic v1.0.2 through v1.0.7)

Pcbic.exe etc. have no recovered source. They remain byte-exact
reference targets. Source hunt continues; until then they're preserved
as-is in `bin/`.

a disassembler Linux is the primary tool for the reverse-engineering side —
you drive it off-sandbox, export cleaned C into `src/<binary>/`,
and Claude wires up the compile-diff loop on this side. See
[`ROADMAP.md`](ROADMAP.md) Phase 2 for the per-EXE workflow.

**First real a disassembler target: TESTIC.EXE** (40,104 B). Smallest,
single-purpose (ping test). Once we've walked a full a disassembler →
C source → OpenWatcom / Borland compile → byte-diff loop end-to-end
on TESTIC, the same workflow scales up to the four bigger EXEs.

**Second target: TESTIC2.EXE** (46,627 B, OS/2 LX 32-bit). Diffing
against TESTIC (DOS MZ 16-bit) isolates the pure DOS/OS2 delta from
the actual test-utility logic — that's leverage for the big two
(Pcbic.exe vs Pcbic2.exe).

---

**See also**:
- [`ROADMAP.md`](ROADMAP.md) — full pcbic v1.0.x phase plan
- [`src/README.md`](src/README.md) — reconstruction source layout
- [`README.md`](README.md) — pcbic12 directory overview
- `../install/src/install.c` — adjacent Phase 3 reconstruction
  (INSTALL.EXE, tracked as install v1.10 arc)
