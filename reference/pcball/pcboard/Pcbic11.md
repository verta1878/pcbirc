# PCBIC v1.1 — 05-28-96

`Pcbic11   05-28-96.zip` — the PCBoard Internet Collection v1.1 release, as
shipped. The release before the v1.2 in `Pcbic12   04-30-97.zip`.

## Contents (4 files, as distributed)

| File | Bytes | Notes |
|---|---|---|
| INSTALL.EXE | 35,292 | Clark's installer |
| PCBIC001.EXE | 743,746 | self-extracting ZIP — the 36-file payload |
| HISTORY | 2,952 | v1.1 changelog |
| README.1ST | 10,306 | setup instructions |

All stamped 1996-05-28 01:10.

## Source

Recovered from the Team GLoW release dated 04/28/97 (`GLOW/PCBIC/`), which
bundled PCBoard 15.3, the 15.4 beta, PCBIC and MetaWorlds 1.02 — archived as
`pcb-metaworlds` on the Internet Archive. Not previously in this repo: no
copy in `reference/`, and no commit in the repo's history ever contained it.

## Why it is worth keeping

Diffing the v1.1 payload against the v1.2 reconstruction in
`pcb1541/pcbic12/` narrows what actually changed between releases:

**Byte-identical in both releases** — PCBIC.HLP, PCBIC.DOC, PCBIC.PDF,
README.1ST, PCBICEVT.EXE, TESTIC.EXE, TESTIC2.EXE, RUNINET.PPE, and every
DATA file (FING, FTP, FTPSCRN, GOPH, ICPROFS.DAT, MENU, MENU.DAT, PING, RLOG,
TELN, TROU, WHO, TCPTEXT), TPA.BAT, OS2SLIP.CMD, WIN31PPP.SCP, WIN95PPP.SCP.

**Changed 1.1 → 1.2** — only three binaries:

| | v1.1 | v1.2 |
|---|---|---|
| PCBIC.EXE | 311,422 | 313,310 |
| PCBIC2.EXE | 206,359 | 217,111 |
| PCBICCFG.EXE | 185,574 | 185,398 |

**In v1.1, not in the v1.2 set** — `RUNINET.DOC` (5,986) and `W95PPP.SCP`
(922, distinct from WIN95PPP.SCP at 1,921).

PCBIC.HLP being byte-identical across a release that changed three
executables settles how it should be treated: a shipped data file, not
something the build compiles. It ships as-is from `OUT/data/ic12/`.

The v1.1 HISTORY is a genuine changelog — SLIP/PPP header-compression packet
corruption, FTP memory leaks and LS output, OS/2 parameter passing with long
paths, PPP negotiation ("nearly all PPP client packages on the market should
work with PCB/IC now"), security-level handling for expired accounts. Useful
context when reading the v1.2 disassembly, since it says what changed
underneath between the two.

SHA256 of the zip:
`ea09eeade302134aad36617d58b9bacd5dfa90643204244184862abc6fb1e254`
