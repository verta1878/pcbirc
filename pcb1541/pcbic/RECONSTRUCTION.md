# IC Reconstruction — bit by bit

Rebuild each IC component byte-for-byte, verifying against the shipped
originals in decrypted/. Fix bugs only AFTER byte-exact restoration.

## Targets (byte-exact) and status

| Component | Original | Status |
|---|---|---|
| RUNINET.PPE | 1,808 bytes | PPLC 3.20 in hand; source is decompiled (see below) |
| Pcbic.exe | 313,310 bytes | no source yet — binary reference |
| Pcbic2.exe | 217,111 bytes | no source yet |
| PCBICCFG.EXE | 185,398 bytes | no source yet |
| PCBICEVT.EXE | 89,612 bytes | no source yet |
| TESTIC.EXE | 40,104 bytes | no source yet |
| TESTIC2.EXE | 46,627 bytes | no source yet |

## RUNINET.PPE — first verifiable target

We have RUNINET.PPS (the PPL source, 4,016 bytes). Compiling it should
reproduce RUNINET.PPE byte-for-byte.

**Finding:** Clark's RUNINET.PPE header says **PPL 3.20** (PCBoard
15.22 era). Our compilers are newer:
- pcb153 PPLC = 3.30 (15.3)
- pcb153/upd154 PPLC = 3.40 (15.4)

Compiling with 3.40 produces a 2,286-byte PPE (vs 1,808) — different
bytecode because the compiler version differs. To match byte-for-byte
we need **PPLC 3.20**.

Options:
1. Find/obtain PPLC 3.20 (PCBoard 15.22 PPL kit).
2. Build PPLC 3.20 from 15.22-era source if we can locate it.
3. Accept a 3.30/3.40 rebuild as functionally-equivalent (NOT
   byte-exact) and note the version gap.

Next: locate a PCBoard 15.22 / PPL 3.20 distribution for PPLC 3.20.

## Update — PPLC 3.20 located and tested

**PPLC 3.20 is in the tree** at `toolkit/pplc/3.20/PPLC320.EXE` (222 KB,
extracted from `reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`, MD5
`2a23e7686f79ea07bbb3c4d04e064a75`, identifies as "PPLC Version 3.20 -
Copyright (C) 1993-95, Clark Development Company, Inc.").

**Compile test:** Ran PPLC 3.20 against `decrypted/RUNINET.PPS` under
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
question is open. See `toolkit/pplc/3.20/out/` for future RUNINET.PPE
build outputs.

## The EXEs

Pcbic.exe etc. have no recovered source. They remain byte-exact
reference targets. Source hunt continues; until then they're preserved
as-is in decrypted/.
