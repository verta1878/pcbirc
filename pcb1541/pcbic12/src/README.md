# pcbic12/src/ — byte-exact reconstruction source

Only source that **compiles** and **byte-matches** its target in
`../bin/` lives here. Nothing else — no scratch, no notes, no
disassembler exports.

## Layout

Flat by default — one file per binary. Subdirs appear only when a
single binary genuinely decomposes into multiple source files (main
IC binary likely will; smaller utilities probably won't).

    src/
    ├── README.md              (this file)
    ├── pcbic.c                pcbic v1.0.6 target — Pcbic.exe (313 KB, main)
    │                          Currently a Phase 27 stub; grows into a
    │                          real reproduction as the disassembly pass
    │                          on Pcbic.exe lands. May split into
    │                          src/pcbic/*.c later if the source fans out
    │                          (menu.c, tcpio.c, etc.)
    │
    ├── runinet.pps            pcbic v1.0.1 target — RUNINET.PPE (1,808 B)
    │                          PPL 3.20 source. Tweak-loop working file.
    │
    ├── testic.c               pcbic v1.0.2 target — TESTIC.EXE (40 KB)
    ├── testic2.c              pcbic v1.0.3 target — TESTIC2.EXE (47 KB, OS/2)
    ├── pcbicevt.c             pcbic v1.0.4 target — PCBICEVT.EXE (90 KB)
    ├── pcbiccfg.c             pcbic v1.0.5 target — PCBICCFG.EXE (185 KB)
    └── pcbic2.c               pcbic v1.0.7 target — Pcbic2.exe (217 KB, OS/2)

## Reconstruction pattern (every target)

Two acceptance steps per target:

**Step a — decompile.** Extract a starting-point source from the
binary using the right decompiler for the target format. This is a
**local** step — decompiler output is not part of the tracked source
tree. Only cleaned, compilable source lands here in `src/`.

**Step b — clean, compile, diff.** Rewrite the decompile output into
real source (fix types, name variables, fold obvious patterns),
commit as `src/<target>.<ext>`, compile with the matching toolchain,
byte-diff against `../bin/<target>`. Iterate the deltas until match.

    edit src/<target>.<ext>
    <compile>                       # PPLC320 or OpenWatcom/Borland
    cmp -l <built-output> ../bin/<target>
    # note delta trend; keep tweaks that shrink, revert those that grow

**Acceptance:** `cmp -s <built-output> ../bin/<target>` exits 0.
Build outputs go wherever your local build lands them — nothing to
track in this repo except source.

## RUNINET.PPE — pcbic v1.0.1 (nearest target)

The .PPS/.PPE pair is the shortest path to a first byte-exact win —
source language is small, compile time is instant.

**pcbic v1.0.1a — decompile baseline**
- Run a PPL 3.20 decompiler against `../bin/RUNINET.PPE` locally
- Compare against `../bin/RUNINET.PPS` (the .PPS Clark shipped
  alongside the .PPE — same size range, may or may not match a fresh
  decompile)
- Whichever gives the closer compile output becomes the baseline
  copied into `src/runinet.pps`

**pcbic v1.0.1b — compile-diff loop**
- Compiler: `../../../toolkit/pplc/3.20/PPLC320.EXE`
  (in tree, runs under DOSBox-X)
- Last measured: PPLC 3.20 on `bin/RUNINET.PPS` → 2,261 B
- Target:                                          1,808 B
- Gap: 453 B, closed via whitespace / statement order / declaration
  order tweaks
- Compiler-version question is CLOSED; source-fidelity is the open work

## The 6 EXEs — pcbic v1.0.2 onward

No shipped source. Reconstruction is disassembly-driven clean-room
reimplementation. Two-step pattern applies per binary:

**Step a — decompile (local).** Disassemble each `../bin/<BINARY>.EXE`
with an appropriate tool; produce a starting-point source. Work
happens locally, not in the tracked tree.

**Step b — clean, compile, diff (in this folder).** Rewrite into
real source with proper types, named variables, structured control
flow. Commit as `src/<binary>.c` (or `src/<binary>/*.c` if
multi-file). Compile via OpenWatcom (sandbox-native) or Borland
C++ 4.5/5.0 via DOSBox-X. Byte-diff against `../bin/<BINARY>.EXE`.
Iterate — usual delta culprits: compiler version, library version,
code order, string layout, alignment padding.

**One binary at a time.** Focus one target, get it to `cmp -s`, land
cleaned source here, then start the next.

## The existing `pcbic.c` (Phase 27 stub — legacy)

`pcbic.c` predates this restructure. Was a scaffold before we had
the unlocked EXEs to analyze. Its header directive:

    /* Exact reproduction of Clark's PCBIC v1.2 (April 30, 1997).
     *   Pcbic.exe     313K   Main IC program
     *   Pcbic2.exe    217K   IC v2
     *   PCBICCFG.EXE  185K   IC configurator
     *   PCBICEVT.EXE   90K   IC event scheduler
     *   TESTIC.EXE     40K   IC test
     *   TESTIC2.EXE    47K   IC test 2
     *   RUNINET.PPE     2K   PPE launcher (source: RUNINET.PPS)
     *
     * Services: FTP, Gopher, Finger, Ping, Telnet, RLOGIN, PPP/SLIP, WHO
     *
     * This is the ancestor of our pcbis (Phase 6). Behavior must match
     * exactly before we extend it.
     */

Stays as a placeholder + intent marker. Real content lands (and
supersedes it) when pcbic v1.0.6 work delivers cleaned source to
`src/pcbic.c`.

## Toolchain

- **PPLC 3.20** — `../../../toolkit/pplc/3.20/PPLC320.EXE`
  (in tree, DOSBox-X). Compiler for RUNINET.PPE reconstruction.
- **OpenWatcom 2.0** — Claude's default cross-compiler for EXE
  compile-diff loops. Sandbox-native, targets DOS + OS/2.
- **Borland C++ 4.5 / 5.0** — target-truth compiler where byte-exact
  requires it (vendored in `reference/` when sourced clean).

Decompiler tooling for step-a runs locally and is not published as
part of this reconstruction — only its cleaned output does, in this
folder.

## See also

- [`../README.md`](../README.md) — pcbic12 directory overview + provenance
- [`../ROADMAP.md`](../ROADMAP.md) — full phase plan (pcbic v1.0.0 → v1.1)
- [`../RECONSTRUCTION.md`](../RECONSTRUCTION.md) — per-target status log
- [`../bin/`](../bin/) — Clark's originals (reference targets, read-only)
