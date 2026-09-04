# pcbic12/src/ — reconstruction source

Our byte-exact reconstruction targets, one subdirectory per binary
as Ghidra work lands.

## Current contents

```
src/
├── README.md         (this file)
└── pcbic.c           Phase 27 stub — placeholder for the main IC binary
                      reproduction. 187 lines, opens with a directive to
                      match Clark's PCBIC v1.2 byte-for-byte. Real
                      content lands as Ghidra output is cleaned and
                      merged in.
```

## Planned layout as reverse-engineering lands

Each of the 6 reference binaries in `../bin/` gets its own subdirectory
here, following the same shape:

```
src/
├── testic/            pcbic v1.0.2 target — TESTIC.EXE (40 KB, MS-DOS MZ)
│   ├── SPEC.md            What the binary does (from Ghidra + docs)
│   ├── GHIDRA-NOTES.md    Analysis log: function map, RTL IDs, tricky bits
│   ├── testic.c           Main entry
│   ├── icmp.c             Ping implementation
│   ├── tcpio.c            TCP stack glue
│   └── Makefile           Or Turbo project file for BC
│
├── testic2/           pcbic v1.0.3 target — TESTIC2.EXE (47 KB, OS/2 LX)
│   └── (mirrors testic/ structure; diff to isolate OS/2 delta)
│
├── pcbicevt/          pcbic v1.1 target — PCBICEVT.EXE (90 KB)
├── pcbiccfg/          pcbic v1.2 target — PCBICCFG.EXE (185 KB)
├── pcbic/             pcbic v1.3 target — Pcbic.exe (313 KB, main binary)
└── pcbic2/            pcbic v1.4 target — Pcbic2.exe (217 KB, OS/2 main)
```

## Build outputs

Compiled binaries go to `../rebuilt/<binary>/<output>.EXE`, not into
`src/`. This keeps source and artifacts separated for cleaner git
diffs and easier `cmp -s rebuilt/<binary>/foo.EXE bin/FOO.EXE`
acceptance checks.

## The existing `pcbic.c` (Phase 27 stub)

`pcbic.c` predates this restructure — a scaffold written before we had
the unlocked EXEs to actually analyze. Its header lays out the
directive:

```
/* Exact reproduction of Clark's PCBIC v1.2 (April 30, 1997).
 *   Pcbic.exe     313K   Main IC program
 *   Pcbic2.exe    217K   IC v2
 *   PCBICCFG.EXE  185K   IC configurator
 *   PCBICEVT.EXE   90K   IC event scheduler
 *   TESTIC.EXE     40K   IC test
 *   TESTIC2.EXE    47K   IC test 2
 *   RUNINET.PPE     2K   PPE launcher (source: RUNINET.PPS)
 *   PCBIC.DOC     112K   Documentation (text)
 *   PCBIC.PDF     339K   Documentation (PDF)
 *
 * Services: FTP, Gopher, Finger, Ping, Telnet, RLOGIN, PPP/SLIP, WHO
 *
 * This is the ancestor of our pcbis (Phase 6). Behavior must match
 * exactly before we extend it.
 */
```

Once Pcbic.exe gets its own Ghidra pass (pcbic v1.3), this stub
migrates into `src/pcbic/pcbic.c` and grows from Ghidra-cleaned
output. Until then it stays as a placeholder + intent marker.

## Ghidra workflow

Off-sandbox (your Linux box):
1. Import `../bin/<BINARY>.EXE` into Ghidra
2. Auto-analyze, then manual clean-up (name functions, mark library
   code, recover types from strings/RTTI)
3. Export decompiled C via Ghidra's C-decompile-and-save-selection
4. Clean up manually, add types, name variables
5. Commit `SPEC.md`, `GHIDRA-NOTES.md`, cleaned `.c/.h` files here

In-sandbox (Claude):
1. Reviews the exports, flags Borland RTL patterns Ghidra missed
2. Writes missing headers/glue, wires up the Makefile
3. Runs OpenWatcom or Borland compile via DOSBox-X
4. Byte-diffs against `../bin/<BINARY>.EXE`
5. Iterates on deltas (compiler version, library version, code order,
   string layout, padding are the usual culprits)

See [`../ROADMAP.md`](../ROADMAP.md) for the full phase plan.

## Toolchain

- **PPLC 3.20** at `../../../toolkit/pplc/3.20/PPLC320.EXE` — for
  RUNINET.PPE compilation (pcbic v1.0.1, non-C target)
- **OpenWatcom 2.0** — Claude's default cross-compiler (sandbox-native)
- **Borland C++ 4.5 / 5.0** — target-truth compiler where byte-exact
  requires it (vendored in `reference/` if we can source clean copies)
- **Ghidra Linux** — reverse-engineering, off-sandbox
