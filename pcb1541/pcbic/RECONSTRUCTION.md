# IC Reconstruction — bit by bit

Rebuild each IC component byte-for-byte, verifying against the shipped
originals in decrypted/. Fix bugs only AFTER byte-exact restoration.

## Targets (byte-exact) and status

| Component | Original | Status |
|---|---|---|
| RUNINET.PPE | 1,808 bytes | source in hand (.PPS); needs PPLC 3.20 |
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

## The EXEs

Pcbic.exe etc. have no recovered source. They remain byte-exact
reference targets. Source hunt continues; until then they're preserved
as-is in decrypted/.
