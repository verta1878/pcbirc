# SIO — Clean-Room Serial I/O Driver Suite for OS/2

**License:** GPLv3 (clean-room reimplementation)
**Author:** evga

A clean-room reimplementation of the SIO serial driver family for OS/2 —
designed from published documentation, with zero lines from the original
shareware and no reverse engineering. Replaces COM.SYS and provides
FOSSIL/virtual-modem support for DOS VDMs.

## Versions

| Version | Release | Status |
|---------|---------|--------|
| v1/ | V1.1.0 — Aug 7, 2026 | Feature complete. 9 binaries, 0 compile errors. |
| v2/ | V2.0 Alpha — Aug 8, 2026 | Core architecture built, layered redesign. All modules compile clean. |

### v1 — feature-complete suite

The complete V1.1.0 release: SIO.SYS (OS/2 PDD, replaces COM.SYS),
VSIO.SYS (VDD, virtualizes COM for DOS VDMs), VX00.SYS (FOSSIL / INT 14h
driver), VMODEM.EXE (telnet/VMP virtual modem), plus utilities — SU
(port status), PMLM (line monitor), VIEWPMLM (trace viewer), INSTALL,
and D4TEST (37-test conformance harness). See `v1/README.md`.

### v2 — layered rewrite (alpha)

A clean-room rewrite with a layered architecture: SIO2K.SYS (logical
layer — IOCtl, DCB, buffering, config file) over UART.SYS (physical
layer), plus VSIO2K (VDD) and supporting modules (esp, pci, logger,
modes). See `v2/README.md` and `v2/STATUS.md` for the honest
module-by-module status, including documented functionality gaps.

## Layout

Each version is a self-contained source tree with its own Makefile,
README, LICENSE, and component subdirectories. Build from within the
version directory.

## Note

This suite is under active development by evga; further updates are
expected. The two versions are kept side by side so the feature-complete
v1 remains available while v2's layered design matures.
