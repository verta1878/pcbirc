# toolkit/pplc — PCBoard Programming Language Compilers

Three PPL compilers staged as ready-to-use binaries. Each version has
its own `out/` directory for the `.PPE` files it compiles, so
byte-exact comparisons across versions are trivial (compile the same
`.PPS` with each PPLC, diff the outputs).

## Versions

| Version | Binary | Size | MD5 |
|---------|--------|-----:|-----|
| 3.00 | `3.00/PPLC300.EXE` | 121 558 | `86325a0f3ff8006c5a19763fb8e1c267` |
| 3.10 | `3.10/PPLC310.EXE` | 118 328 | `18409677b4a40497001c2848e0616ce2` |
| 3.20 | `3.20/PPLC320.EXE` | 222 176 | `2a23e7686f79ea07bbb3c4d04e064a75` |

## Provenance

All three extracted from
`reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`
(sub-path `CSBACKUP-Clean/PCB/PPLC3??.EXE`, dated 1997-08-01).
This is Roy SAC's clean backup of a PCBoard 15.22-era Clark Support
board install; the three PPLCs shipped side-by-side inside `PCB/` on
that BBS's binary tree.

## Why three versions

- **PPLC 3.00** — PCBoard 15.0 era. Earliest PPL compiler we have.
- **PPLC 3.10** — PCBoard 15.1 era. Middle version, useful when a PPE
  header reports PPL 3.10.
- **PPLC 3.20** — PCBoard 15.22 era. **The IC byte-exact target.**
  Clark's `RUNINET.PPE` header reports PPL 3.20; compiling
  `RUNINET.PPS` with this produces the 1808-byte binary we want.
  See `pcb1541/pcbic12/RECONSTRUCTION.md`.

Our newer PPLCs (3.30 = 15.3, 3.40 = 15.4) live under the toolkit
version they belong to (`toolkit/pwa153/`, etc.) — they're not staged
here because they produce a different bytecode shape and don't match
Clark's 15.22 shipped PPEs byte-for-byte.

## Running them

These are DOS EXEs. Under DOSBox-X:

```
mount c /path/to/pcbirc
c:
cd \toolkit\pplc\3.20
PPLC320 mysource.pps
```

The compiled `.PPE` lands next to the `.PPS` by default. Move it into
`out/` (or redirect the compile there) to keep per-version outputs
tidy.

## Building PPLC from source (future)

The PPL compiler *source* also exists — see
`reference/pcball/pcboard/pcb-main/SOURCE/{PPL,COMPILER}/` and
`reference/pcball/pcboard/pcb-main/153/PPLC.MAK`. Rebuilding PPLC
from that source is a separate project; the binaries here are for
immediate use.

## Samples

20 sample PPL scripts (.PPS source + .PPE Clark-compiled binaries) plus
2 helper .BATs shipped by Clark with PCBoard 15.41. See
[`samples/SAMPLES.md`](samples/SAMPLES.md) for the full catalog.

These serve two purposes:
- **Tutorial** — HELLO1-7 walk through the language progressively.
- **Round-trip parity target** — when PPLC clean-room compilers are
  ready, recompiling each `.PPS` should produce a `.PPE` byte-perfect
  against Clark's originals in `samples/`.

The same files also live at `pcb1541/install/dist/target/PPL/` as the
installer parity target. Both locations are byte-identical.

