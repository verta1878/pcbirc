# IC Rebuilt Components

Our rebuilds from source, for comparison against the shipped originals
in ../bin/.

## STATUS: byte-exactness DEFERRED (decided 2026-09-16, v0.3.0)

RUNINET.PPE is **accepted as functionally equivalent, not byte-exact.**
Clark's is 1,808 B; ours is 2,261 B (PPLC 3.30) / 2,286 B (3.40). The
program is correct and runs; the header matches through byte 42 and the
divergence is bytecode encoding, not behaviour.

Two blockers, only one of them solvable:

1. **Compiler version** — Clark used PPLC 3.20. Solvable: build it from
   source (below). The 15.3 library chain this needs is already done
   (pcbsrc v0.1, arc closed 2026-09-07).
2. **The source itself** — Clark's original `.PPS` is lost. Ours was
   decompiled from the PPE and carries 63 implicit variables against
   Clark's 39. `todo/pcb-libchain-build.md` v0.2.8 records both PPLC
   3.20 and 3.30 emitting 2,261 B from it, and marks the phase
   "Blocked — original source lost."

So building PPLC 3.20 alone cannot reach 1,808 B. Closing the gap would
mean recovering the 39-variable form from Clark's PPE bytecode by hand
and rewriting the PPS to match — a separate piece of work, not a build
step.

**Decision: leave it here for now; revisit later.** This does not block
Phase B or the IC release — RUNINET.PPE ships as Clark's original 1,808 B
binary from `OUT/data/ic12/`, exactly as it always has. What is deferred
is only our ability to *reproduce* it from source.

Everything below is the standing detail on how that reproduction would
work when someone picks it up.

## RUNINET.3.30.PPE

RUNINET.PPS compiled with PPLC 3.30 (from pcb1541/install/dist/target/).
- Our build: 2,261 bytes (PPL 3.30 bytecode)
- Clark's original: 1,808 bytes (PPL 3.20 bytecode)

NOT byte-exact — Clark compiled with PPL 3.20 (PCBoard 15.22 era).
Header identical through byte 42; divergence is bytecode instruction
encoding, not source. The programs are functionally equivalent.

## RUNINET.3.40.PPE

RUNINET.PPS compiled with our PPLC 3.40 (from pcb153/upd154).
- Our build: 2,286 bytes (PPL 3.40 bytecode)
- Clark's original: 1,808 bytes (PPL 3.20 bytecode)

Also NOT byte-exact — same compiler-version bytecode difference.

## Next: PPLC 3.20

PPLC 3.20 is the compiler Clark used for the 1,808 B original.

**Getting 3.20 is necessary but not sufficient.** Per HISTORY.md
(pcbic v1.0.1): the decompiled PPS has 63 implicit variables against
Clark's 39, and *both* PPLC 3.20 and 3.30 emit 2,261 B from it. The
compiler version and the implicit-variable gap are two separate
blockers; closing only the first will not reach byte-exact.

**Build it from source** — the standing decision in
`toolkit/pplc/README.md`: "build PPLC from source rather than extract the
shipped binaries. We own the compiler end-to-end from Clark's source."

    1. Edit pcb153/SOURCE/PPL/NEWSCR.CPP:
         HDR_TXT      -> "PCBoard Programming Language Executable  3.20..."
         CUR_PPE_VER  -> 320
    2. MAKE -f PPLC.MAK   (BC 3.1, TLINK 5.1, under DOSBox-X)

Clark's shipped `PPLC320.EXE` (222,176 B, md5
`2a23e7686f79ea07bbb3c4d04e064a75`) exists only inside
`reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`, sub-path
`CSBACKUP-Clean/PCB/PPLC320.EXE`. Reference only — not extracted into
the tree.

**Correction:** an earlier version of this file said PPLC320.EXE was at
`toolkit/pplc/3.20/PPLC320.EXE`. It never was — that directory holds
only `out/`, which is for compiled `.PPE` output, not compiler
binaries. Compiler distributions live in `devtools/` (`ppld32.zip`,
`ppldevkit.zip`, `pplx20.zip`).
