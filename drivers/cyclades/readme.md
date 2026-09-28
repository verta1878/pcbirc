# cyclades — Cyclades Cyclom-Y multi-port serial driver

Multi-port serial hardware driver for the **Cyclades Cyclom-Y** family
of ISA and PCI serial cards (4/8/16/32-port RS-232 concentrators).
Cyclades cards were staples of larger BBS systems and Unix serial
console farms in the 1990s and early 2000s; adding them to a modern
PC gives PCBoard (and any other DOS/Win BBS) real multi-line dial-up
or console capacity again.

## Status

**Work in progress.** A v1.0.0 binary release is here as a self-contained
tree at `cyclades-1.0.0-bin/`, unpacked from `cyclades-1_0_0-bin.zip`
alongside. The binary bundle ships with GPLv3 source (`SOURCE/`), Win32
+ Win64 drivers, DOS test utilities, docs, and an INF-installer flow.
This top-level `drivers/cyclades/` directory does not yet have its own
build/test integration for pcbirc — see roadmap below.

License: **GPLv3** (per the bundle's `LICENSE.TXT` and `SOURCE/`).

## Contents (`cyclades.zip`)

| Path | What it holds |
|------|---------------|
| `i386/` | Win32 driver (Win2K – Win11), install tool, INFs, signing scripts |
| `x64/` | Win64 driver skeleton (requires the Windows WDK to build) |
| `DOS/` | DOS utilities: `CYTEST` (probe), `CYFTST` (port test) |
| `DOC/` | Docs: BUILD, CROSS_COMPILATION, CYFOSSIL_AUDIT, INSTALL_GUIDE, PLATFORM_SIZES, RELEASE_BUILD, TROUBLESHOOTING |
| `SOURCE/` | GPLv3 source, plus `build/`, `build.sh`, `inc/`, `inf/`, `src/`, `test/`, `tools/` |
| `README.TXT` | Original install-and-run notes |
| `EMERGENCY_RECOVERY.txt` | Recovery instructions if a driver install goes wrong |
| `LICENSE.TXT` | GPLv3 license |
| `FILE_ID.DIZ` | Short BBS-style description |

## Hardware supported

Cyclades **Cyclom-Y** family — ISA and PCI variants, 4/8/16/32 ports
per card. The classic sysop hardware for driving many modems off a
single machine. Cyclades also made Cyclom-Z (Ethernet-attached
console servers) — the v1.0.0 driver here targets **Cyclom-Y only**.

## Platform support

- **Windows**: 2000, XP, Vista, 7, 8, 10, 11 (both 32-bit x86 and
  64-bit x64 paths documented; 64-bit driver needs a local WDK build
  and a signed cert)
- **DOS**: real-mode utilities in `DOS/` for probing and per-port
  testing (`CYTEST`, `CYFTST`) — the actual FOSSIL/BBS driver path is
  the topic of `DOC/CYFOSSIL_AUDIT.md`

## Where this fits in pcbirc

Cyclades cards give PCBoard (or the 1541 successor) real multi-line
capacity beyond what a single COM port or WNFOSSIL/RLFOSSIL setup can
carry. Complements `drivers/netfosdl/` (a networked FOSSIL for
DOSBox-X testing) and the `drivers/SIO/` OS/2 serial family: DOS-era
BBSes running on modern hardware with real serial fan-out.

## Roadmap

- [x] v1.0.0 binary bundle + GPLv3 source vendored (`cyclades-1_0_0-bin.zip`)
- [x] Bundle extracted for reference (`cyclades-1.0.0-bin/`)
- [ ] Audit `DOC/CYFOSSIL_AUDIT.md` and decide whether Cyclom-Y needs
  its own FOSSIL wrapper for use with PCBoard, or whether an existing
  FOSSIL driver (RLFOSSIL / WNFOSSIL / netfosdl) already covers it
- [ ] Cross-build path from Linux (see `DOC/CROSS_COMPILATION.md`)
  documented for the pcbirc toolchain (likely OpenWatcom 2.0 for the
  DOS side)
- [ ] Integration test: PCBoard + Cyclom-Y under emulation
  (86Box / PCem — DOSBox-X does not model this hardware) or on real
  hardware if the crew has a card
