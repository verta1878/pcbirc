# pcbdcom compatibility plan

## Two-phase, two-team approach

**Phase 1 — Feature discovery (internal, unrestricted).**
We study COMM-DRV to answer *what does it do that pcbdcom doesn't
yet*. This is a private read for gap identification, not code
production. Any team member can look at COMM-DRV binaries,
COMMDRV.RED contents, DRVSETUP behavior, live traces — whatever
tells us the feature list.

Deliverable: `pcb154/pcbdcom/GAP-ANALYSIS.md` — a plain-English
list of features COMM-DRV has that pcbdcom is missing, plus
notes on which cards / INT 14h functions / config options each
feature touches. No code, no register values from COMM-DRV, no
byte sequences — just *what features exist*.

**Phase 2 — Implementation (clean room, strict).**
A second reader takes the GAP-ANALYSIS document and implements
each missing feature using only:

- Public FOSSIL spec (INT 14h)
- Public hardware datasheets (16550, CD1400, SC26198, Digi FEP,
  Comtrol AIOP, Arnet register specs where published)
- GPL Linux driver source (already staged in `ref/linux/`)
- PCBoard's own source for what it expects on the interface

The implementation reader does not look at COMM-DRV, its
disassembly, its config file bytes, or anything derived from
COMM-DRV internals. This is the wall that keeps our
implementation legally clean regardless of what Phase 1 found.

## Practical workflow in this repo

Phase 1 output lives in `GAP-ANALYSIS.md` (or a private note if
we want to be extra careful — not committed to public repo).

Phase 2 commits reference only public sources in file headers:
```
/* Ported from Linux drivers/char/cyclades.c v2.6.32 (GPLv2).
 * Public datasheet: Cirrus Logic CD1400 Register Reference. */
```
Never:
```
/* From COMM-DRV COMMDRV.EXE +0x2A40 */    /* ← would poison it */
```

## Where firmware / BIOS blobs come from

Card manufacturer firmware (Digi FEPCODE, Comtrol microcode,
Arnet XABIOS, etc.) is owned by the card vendor, not WCSC. Our
model matches Linux:
- pcbdcom (GPLv3) knows how to talk to the card once firmware
  is loaded.
- Sysop supplies firmware from vendor disk at load time via
  `PCBDCOM.CFG` (`FIRMWARE=path/FEPCODE.BIN` line).
- We do not redistribute vendor firmware unless the vendor
  explicitly permits it.

## Missing-feature candidates (starter list to expand in Phase 1)

Educated guesses at what pcbdcom v1.1 might not yet cover:
- 8th card: Arnet SmartPort Plus (PCBoard docs list it)
- Cold-boot BIOS upload for Digi cards (v1.1 assumes warm boot)
- Comtrol MUDBAC IRQ routing (v1.1 uses IRQ-disabled mode)
- Cyclades multi-chip (16Y/32Y) — v1.1 has structure, needs test
- COMM-DRV extended INT 14h functions beyond standard FOSSIL
  (functions 0x10+; PCBoard source will show which it uses)
- Hardware flow control config nuances per card
- 15.41 TCP socket backend (deferred by design; port later)

Phase 1 replaces this list with real findings.
