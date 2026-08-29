# PCBBLDBT.IMG — Build Toolkit Image Roadmap

`PCBBLDBT.IMG` (PCB Build Toolkit) is the golden bootable disk image
that ships the full build environment for the PCBoard SDK matrix.
Same image, growing contents at each milestone.

Bootable under DOSBox-X (`BOOT -l c`), QEMU, 86Box, PCem, or on real
hardware. Format: FAT16 hard-disk image with FreeDOS 1.3.

## Milestones

| Milestone | Contents | Status |
|-----------|----------|--------|
| **v1 (now)** | FreeDOS 1.3 boot, CWSDPMI, Borland C++ 3.1, Turbo C 2.01, Microsoft C 7.0, TASM, toolkit/pwa153 source, pcb153 source, BUILD.BAT dispatcher + BLDKBC/BLDKIT/BLDKMS/MKLIB scripts + all RSPs, empty OUT tree | Image assembled; MSC7 DPMI wiring in test |
| **v2** | + recreated 15.22 toolkit source (`toolkit/pwa1522/`), + PWA Clark shipped binaries for regression comparison | Blocked on 15.22 toolkit reconstruction |
| **v3 (1541)** | + verta1878/ow2irc binaries — the full 1541 build-and-ship environment on a single image | Blocked on ow2irc completing |

## Dependency ordering (real work sequence, updated 2026-08-29)

Phase 0 (image rebuild) is done. Remaining phases in order:

1. **`.RED` extractor / creator** (`archivers/lha/` + `archivers/redx/`)
   — the PCB installer wraps files in a Clark-proprietary `.RED`
   container (RR magic + LH5-family compression). Vendored LHA 1.14i
   source provides the LH5 decoder; the crew's thin wrapper handles
   Clark's per-record framing. Deliverables: `redx` (extract) and
   `redc` (create) as cross-platform C. Unlocks:
   - Extraction of COMMDRV.RED (feeds phase 2)
   - Extraction of PCBOARD.RED, PCBMAIL.RED, PPLC.RED, PCBCFGS.RED
     (reference material for the SDK matrix)
   - Ability to CREATE `.RED` files (needed for phase 5)
2. **commdrv decompile** (separate repo, NOT part of pcbirc) —
   commdrv was a commercial serial-comm library from the DOS era,
   never included with PCBoard. Decompile it as its own project,
   out of this repo, deleted from public view after use. Output
   feeds phase 4.
3. **PCBKMS finalize** — MSC 7.0 Route A verified (proof:
   `PCBKMS-ROUTE-A-PROOF-TINY.OBJ` + first 4 real toolkit OBJs).
   Continue per-module iteration: Borland-idiom → MSC7 adaptations
   (inline asm syntax, near/far/huge pointers, MSC vs BC runtime
   function names). Estimate 40-80 hours real port work across
   119 modules. Land 12/12 SDK matrix. **NOTE: the source is
   Borland-native (Clark wrote it for BC++ 3.1); MSC7 is a
   completeness goal, not a native fit.**
4. **pcbdcom** — native PCBoard-authored hardware serial layer.
   Built using phase 2's commdrv analysis as reference, plus
   existing crew drivers in `drivers/` (netfosdl, SIO, cyclades,
   rlfossil) and GPL Linux driver ports (pcxx.c, epca.c, rocket.c,
   istallion.c) per `LINUX_BSD_HUNT.md`. Single `PCBDCOM.DRV`
   output. Ships as native crew asset with pcb1541.
5. **PCB installer recreation** — own the whole PCBoard install
   experience. Uses `archivers/redx` + `archivers/redc` from
   phase 1 for the container operations; DOS + OS/2 stub for
   target platforms. Lets us distribute PCBoard end-to-end on
   our own installer.
6. **IC rebuild** — byte-exact PPE reconstruction for RUNINET.PPE
   (1808 bytes). Uses `toolkit/pplc/3.20/`.

## v3 image milestone — under review

The prior draft treated an "add ow2irc binaries, full 1541
build-ship image" step as phase 9. That mixed two different things
(a packaging milestone vs a workstream that isn't ours to schedule)
and is currently under review — do not treat it as a committed
phase until the roadmap discussion around image milestones vs code
phases lands.

For the tool itself, **OpenWatcom 1.9 is used now under DOSBox-X**
for any Watcom target (Delta 15.4, etc.) — it's free (Sybase OWPL),
runs cleanly under DOSBox-X, and doesn't depend on bob's ow2irc
fork. Watcom availability is a toolchain fact, not a roadmap phase.

## Release naming

The image ships as **`pcbbldbt.zip`** (contains `PCBBLDBT.IMG` +
[`PCBBLDBT.CONF`](PCBBLDBT.CONF) for DOSBox-X + a README). Version stamp goes in the
zip name when it matters (`pcbbldbt-v2.zip`, etc.), not in the
image filename (which stays generic across milestones).

The shipped [`PCBBLDBT.CONF`](PCBBLDBT.CONF) already includes the
DOSBox-X config knobs we've tested and validated (currently: `zero
unused int 68h=true` under `[dos]`, which frees INT 68h for HX
HDPMI use — see `todo/dosboxx-dpmi-failures.md` Failure #1).
Additional config knobs get added there as they're proven, so a
fresh clone gets the working environment automatically.

## Why not name it per toolkit version

Earlier candidate names (`PWA153BT.IMG`, `PWA1522BT.IMG`, etc.)
implied one image per toolkit version. That's not the plan: one
image accumulates content across milestones, ending up with
everything needed to build every version in the matrix. Generic
name matches generic scope.
