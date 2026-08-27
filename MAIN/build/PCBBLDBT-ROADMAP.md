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

## Dependency ordering (real work sequence)

1. **v1 finalize** — prove MSC7 CL.EXE runs under CWSDPMI, run
   `BUILD all` end-to-end on pwa153, complete the 12-lib matrix.
2. **v2** — start once pwa153 Path A (byte-exact vs shipped 15.3
   binaries) is complete. Recreate 15.22 toolkit source using the
   pwa153 method as reference. Add PWA Clark binaries to the image.
3. **IC rebuild** — byte-exact PPE reconstruction for RUNINET.PPE
   (1808 bytes). Falls out of the v2 work: 15.22 material gives us
   the era-correct compilation context. Uses `toolkit/pplc/3.20/`.
4. **v3** — add ow2irc binaries once bob's compiler work is far
   enough along. Full 1541 build-ship image.

## Release naming

The image ships as **`pcbbldbt.zip`** (contains `PCBBLDBT.IMG` +
`PCBBLDBT.CONF` for DOSBox-X + a README). Version stamp goes in the
zip name when it matters (`pcbbldbt-v2.zip`, etc.), not in the
image filename (which stays generic across milestones).

## Why not name it per toolkit version

Earlier candidate names (`PWA153BT.IMG`, `PWA1522BT.IMG`, etc.)
implied one image per toolkit version. That's not the plan: one
image accumulates content across milestones, ending up with
everything needed to build every version in the matrix. Generic
name matches generic scope.
