# OUT/clark-original — Clark's shipped 15.4 beta

**Clark Development's own binaries. Binaries only, no source.**
The byte-match reference the 15.4 PWA rebuild (`OUT/pwa153/upd154/`) must
reproduce. Nothing here is built by us — do not overwrite, do not regenerate.

Kept at the `OUT/` top level rather than under a version directory: these are
Clark's originals, not our output for any one build leg, and the same
directory is where further original Clark material belongs as it turns up.

## What this is

PCBoard v15.4 **beta**, shipped as an upgrade dropped on top of an installed
PCBoard 15.x — EXEs only, no installer. Clark never shipped 15.4 as a full
release, which is why the reconstructed source lives under `pcb153/upd154/`
as a delta on 15.3 rather than as its own tree.

Newest build stamp: **04/22/97** (PCBOARD.EXE, PCBOARDM.EXE, PCBOARD2.EXE,
PCBSM.EXE, PPLC.EXE, UUIN.EXE, LOCAL.EXE). The rest are 04/15/97.

## Contents

| Dir | Files | What |
|---|---|---|
| `DOS/` | 11 | DOS executables |
| `OS2/` | 1 | PCBOARD2.EXE |
| `docs/` | 5 | HISTORY, README.1ST, REPORT.TXT, WHATSNEW, PCBOARD.SER — as shipped, CRLF |
| `beta-patched/` | 3 | date-check-patched variants — see below, NOT the reference |

SHA256 for everything: `CHECKSUMS.sha256`.

### DOS/

LOCAL.EXE (768,080) · MKPCBTXT.EXE (62,958) · PCBOARD.EXE (1,051,920) ·
PCBOARDM.EXE (1,007,552) · PCBSETUP.EXE (411,344) · PCBSM.EXE (278,160) ·
PPLC.EXE (201,774) · UUIN.EXE (259,728) · UUOUT.EXE (141,272) ·
UUUTIL.EXE (141,170) · UUXFER.EXE (176,998)

### OS2/

PCBOARD2.EXE (891,963)

## Provenance

Two independent sources, cross-verified:

1. **Repo git history.** Committed in `1993ecb` (2026-07-31) at `OUT/DOS/` and
   `OUT/OS2/`, refreshed in `7d22e72` (2026-08-19), deleted in `e4181e5`
   (2026-08-25 toolkit restructure) and never re-added. The `OUT/pwa154/` slot
   named in older notes never existed in the history.
2. **Original 1997 distribution.** `PCB154B/` inside a Team GLoW release dated
   04/28/97 bundling PCBoard 15.3 (250-node), the 15.4 beta, PCBIC and
   MetaWorlds 1.02 — archived as `pcb-metaworlds` on the Internet Archive.

9 of the 12 executables are byte-identical between the two. `PCBOARD.SER` is
identical. The four documents are identical once line endings are ignored —
the repo copies had been normalized CRLF→LF at some point.

## The three that differed — and why this directory now holds the 1997 copies

`LOCAL.EXE`, `PCBOARD.EXE` and `PCBOARDM.EXE` differed from the 1997
distribution by **exactly one byte each**, at the beta date check:

```
... 26 2b 07 89 46 fe 83 7e fe 1e [76|EB] ...
     ^^^^^^^^^^^^^^^^             ^^^^^^^^
     David Terry's documented      76 = JBE (check intact)
     beta-check signature          EB = JMP (check bypassed)
```

| File | Offset | 1997 dist | Repo copy |
|---|---|---|---|
| LOCAL.EXE | 0x03249B | `76` | `EB` |
| PCBOARD.EXE | 0x03D45B | `76` | `EB` |
| PCBOARDM.EXE | 0x03CBE3 | `76` | `EB` |

The byte pattern `26 2b 07 89 46 fe` is the signature David Terry (one of the
original PCBoard programmers) published in June 1997 for disabling the 30-day
beta timer — see `reference/pcb-1997-06-16-terry.txt`. The repo copies had
been patched; the 1997 distribution copies are unpatched.

**This matters for byte-exactness.** A faithful rebuild from source emits the
conditional jump (`76`). Verifying against a patched reference would report a
one-byte failure in three binaries that is not a reconstruction error. So
`DOS/` and `OS2/` now hold the unpatched 1997 copies, and the patched variants
are kept in `beta-patched/` rather than discarded.

## Known gaps

- **No `.HLP` files.** `README.1ST` says to copy `*.EXE` and `*.HLP` into
  `\PCB`, but the 1997 distribution carries no `.HLP` for 15.4 and none was
  ever committed to this repo. Either the beta reused the 15.3 help files or
  they shipped in a different package.
- **No `PCBUUCP.ZIP`.** `README.1ST` refers to it as carrying its own
  `WHATSNEW.UU`. Not in the distribution.
