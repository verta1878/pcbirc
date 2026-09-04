# PCBIC 1.2 — PCBoard InterCom

Clark Development's InterCom add-on — an internet/TCP-IP suite for
PCBoard (Telnet, FTP, Gopher, Finger, Ping, PPP/SLIP dialup, WHO,
RLOGIN). Sold separately as a commercial add-on, never bundled with
the main PCBoard distribution. Version 1.2, released April 30, 1997.

This directory is our byte-exact reconstruction workspace for that
release.

## Directory layout

```
pcb1541/pcbic12/
├── bin/           Clark's original 1.2 binaries + all shipped support files
├── src/           Our reconstruction source (grows as reverse-engineering lands)
├── rebuilt/       Our compiled outputs (byte-diffed against bin/)
├── README.md      (this file)
├── ROADMAP.md     Phase plan for byte-exact reconstruction
└── RECONSTRUCTION.md
                   Per-target status log (RUNINET.PPE, 6 EXEs)
```

### `bin/` — reference material (38 files)

Clark's original 1997 distribution, extracted from `Pcbic12.zip`.
These are the byte-exact targets. Do not modify.

**Executables (6, our reconstruction targets):**
| File | Size | Format | Role |
|---|---:|---|---|
| `Pcbic.exe`    | 313,310 | MS-DOS MZ | main InterCom binary |
| `Pcbic2.exe`   | 217,111 | OS/2 LX 32-bit | OS/2 sibling of main |
| `PCBICCFG.EXE` | 185,398 | MS-DOS MZ | configuration UI |
| `PCBICEVT.EXE` |  89,612 | MS-DOS MZ | event handler |
| `TESTIC2.EXE`  |  46,627 | OS/2 LX 32-bit | test utility (OS/2) |
| `TESTIC.EXE`   |  40,104 | MS-DOS MZ | test utility (smallest — Ghidra warm-up) |

**Source we can rebuild directly:**
- `RUNINET.PPS` (3,895 B) — PPL source for the SLIP/PPP launcher
- `RUNINET.PPE` (1,808 B) — compiled PPE, target for byte-exact rebuild

**Data / config (TCP/IP menu screens, ANSI-decorated):**
- `DATA/TCPTEXT` (13,962 B) — main IC text bundle
- `DATA/MENU` + `MENU.DAT` — top menu
- `DATA/FTP`, `FTPSCRN`, `TELN`, `GOPH`, `FING`, `PING`, `RLOG`,
  `PPP`, `SLIP`, `TROU`, `WHO` — per-service prompts and screens
- `DATA/ICPROFS.DAT` — service profile database

**Launcher batches:**
- `PCBIC`, `PPP`, `SLIP`, `TPA.BAT` — DOS batches invoked by PCBoard

**Help:** `PCBIC.HLP` (16,636 B)

**Docs (`bin/DOCS/`):**
- `PCBIC.DOC` (109,101 B), `PCBIC.PDF` (339,182 B), `README.1ST` (10,092 B)

**Dial-up scripts (`bin/SCRIPTS/`):**
- `WIN31PPP.SCP` + `.TXT`, `WIN95PPP.SCP` + `.TXT`, `OS2SLIP.CMD` +
  `.TXT` — sample dialer scripts for the era's OSes

### `src/` — our reconstruction source

See [`src/README.md`](src/README.md) for what's in there and what's
planned per binary. Currently a Phase 27 stub (`pcbic.c`, 187 lines);
grows into per-binary subdirs as Ghidra work lands.

### `rebuilt/` — our compiled outputs

Byte-diffed against `bin/`. Currently: `RUNINET.3.40.PPE` (early
compile test with the wrong compiler version — see RECONSTRUCTION.md
for why PPLC 3.20 is the right one).

## Provenance

The original `Pcbic12.zip` (1,441,417 B, 1997-04-30) came to us
ZipCrypto-encrypted by a third party — not Clark. Whoever repackaged
it added the encryption; that's not how Clark originally distributed
InterCom. The full 42-entry archive was unlocked and delivered as
`Pcbic12d.zip` in a separate effort — this is recovered material.

The untouched encrypted archive is preserved in the reference tree at
`reference/pcball/pcboard/Pcbic12   04-30-97.zip` (md5 `ecc17649`).
We no longer keep a working copy in this directory — everything we
work from is already extracted into `bin/`.

## Rebuild directive

Rebuild byte-for-byte with the same bugs. Fix bugs only AFTER
byte-exact restoration.

**Byte-exact acceptance bar for every target:** `cmp -s built.<ext>
bin/<original>` exits 0.

Order:
1. `RUNINET.PPE` — source (`RUNINET.PPS`) IS in hand, compiler
   (PPLC 3.20) IS in tree at `toolkit/pplc/3.20/PPLC320.EXE`. Just
   needs source-fidelity tweaks to close the 2,261→1,808-byte gap.
2. `TESTIC.EXE` — smallest EXE, best Ghidra warm-up.
3. `TESTIC2.EXE` — OS/2 LX sibling; diff against TESTIC to isolate
   DOS/OS2 delta.
4. `PCBICEVT.EXE` — moderate complexity, event-handler patterns.
5. `PCBICCFG.EXE` — config UI, larger.
6. `Pcbic.exe` → `Pcbic2.exe` — the big two, multi-month efforts each.

## Toolchain

- **PPLC 3.20** — `toolkit/pplc/3.20/PPLC320.EXE` (extracted from
  `reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`, md5
  `2a23e7686f79ea07bbb3c4d04e064a75`). Released compiler; we're **not**
  clean-rooming it.
- **Ghidra Linux** — reverse-engineering side, driven off-sandbox.
- **OpenWatcom / Borland C++ 4.5-5.0** — compile side for the C
  reconstruction (target-dependent — Ghidra output tells us which).

## See also

- [`ROADMAP.md`](ROADMAP.md) — phase plan (pcbic v1.0.x)
- [`RECONSTRUCTION.md`](RECONSTRUCTION.md) — per-target status log
- [`src/README.md`](src/README.md) — reconstruction source layout
- `reference/pcball/pcboard/Pcbic12   04-30-97.zip` — untouched
  provenance copy of the original encrypted archive
- `../pcbis/` — Phase 6 successor project (descendant of this codebase)
