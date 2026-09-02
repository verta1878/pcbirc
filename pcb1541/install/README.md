# INSTALL — PCBoard Installation Program

Scaffold for reproducing Clark's INSTALL.EXE (331 KB).

The original decompresses .RED archives (LZH-family compression)
and installs PCBoard from INSTALL.DAT (482-member size oracle).

Source: `src/install.c`

## Provenance and distribution

`pcb1541/install/` is derived from the ORIGINAL PWA install disk set —
the same installer source that Clark shipped for every PCBoard variant.
This is why every downstream variant needs it: each PCBoard release
was built by feeding the same INSTALL.EXE machinery a different set of
`.RED` archives.

**Ships with:**
- `pwa/` (PWA 15.22 and earlier — original Clark line)
- `delta/borland/` (Delta 15.4 Borland-built variant)
- `delta/irc/` (Delta 15.4 IRC-focused variant we maintain)
- Any future PCBoard tree built on the PWA lineage

**Do not delete or move `pcb1541/install/`** without updating every
downstream tree that references it. The `INSTALL.zip` inside is the
raw disk-image source; the `dist/disk1..disk5/` breakdown is the
unpacked view for building the modern install experience.

`INSTALL.DAT` inside `INSTALL.zip` is a plain-text manifest that
serves as public documentation of what each `.RED` archive contains —
we used it for pcbdcom Phase 1 discovery (see
`pcb154/pcbdcom/GAP-ANALYSIS.md`).

