# PCBIC in PCB154 — Build Pipeline & Installer Integration (DRAFT)

## Current State

PCBIC v1.2 is fully reconstructed (all 6 binaries byte-exact, v0.2.9).
Source lives in `pcb1541/pcbic12/` (standalone, verta + hexadecimal 3 weeks).
This document plans how PCBIC integrates into pcb154 main source,
the build pipeline, and the installer disk packaging.

IC is the 15.4 internet upgrade. pcb153 has no IC — no internet.
toolkit/delta154 is SDK only — IC does not go there.

---

## OUT/ — Canonical Binary + Data Output

All build output lives in OUT/. The installer assembles from here.
Binaries by compiler variant. Data by program — no duplicating directory structures.

```
OUT/
  pwa153/bins/               15.3 base EXEs (Borland)
    COMMDRV/                   WCSC serial driver package
  upd154/                    15.4 upgrade
    pwa/bins/                  15.4 upgrade EXEs (Borland)
    delta/bins/                15.4 upgrade EXEs (OpenWatcom)
    irc/bins/                  15.41 IRC EXEs (ow2irc)
  delta154/bins/             15.4 full build (OpenWatcom)
  irc1541/bins/              15.41 full build (future)
  data/                      ALL data files, by program — NO source code, NO EXEs
    ic12/                      PCBIC: all IC data files (see below)
    commdrv/                   COMMDRV: configs (.DAT), firmware (.BIN), MONITOR.BAT
    pcbmail/                   QWK mailer: HLP only
    pcbos2/                    OS/2: CMD, HLP, sample config
    help/                      63 base HLP files (HLP! through HLPZ)
    doc/                       30 documentation files
    ppl/                       42 PPE+PPS sample files + BATs
    gen/                       95 system files (BLT, BRDS, menus, screens, PPE/PPS)
    fido/                      18 FidoNet configs + RESPONSE/
    graphics/                  7 ANSI dir/path graphics
    files/                     11 file area configs
    main/                      33 message base skeleton (CNAMES.*, MSGS, PWRD, etc.)
    conferences/               Sample conferences (see below)
    root/                      BATs, DATs, configs for install root (see below)
    dl01/                      2 sample download files
```

### OUT/data/ic12/ — All PCBIC data in one place

| File | Original Location | Install Target |
|---|---|---|
| PCBIC.HLP | pcb1541/pcbic12/bin/ | → HELP/ |
| PCBIC.DOC | pcb1541/pcbic12/bin/DOCS/ | → DOC/ |
| PCBIC.PDF | pcb1541/pcbic12/bin/DOCS/ | → DOC/ |
| README.1ST | pcb1541/pcbic12/bin/DOCS/ | → DOC/ (rename to avoid conflict) |
| RUNINET.PPE | pcb1541/pcbic12/bin/ | → PPL/ |
| RUNINET.PPS | pcb1541/pcbic12/bin/ | → PPL/ |
| FING | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| FTP | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| FTPSCRN | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| GOPH | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| ICPROFS.DAT | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| MENU | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| MENU.DAT | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| PING | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| RLOG | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| TCPTEXT | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| TELN | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| TROU | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| WHO | pcb1541/pcbic12/bin/DATA/ | → DATA/ |
| OS2SLIP.CMD | pcb1541/pcbic12/bin/SCRIPTS/ | → SCRIPTS/ |
| OS2SLIP.TXT | pcb1541/pcbic12/bin/SCRIPTS/ | → SCRIPTS/ |
| WIN31PPP.SCP | pcb1541/pcbic12/bin/SCRIPTS/ | → SCRIPTS/ |
| WIN31PPP.TXT | pcb1541/pcbic12/bin/SCRIPTS/ | → SCRIPTS/ |
| WIN95PPP.SCP | pcb1541/pcbic12/bin/SCRIPTS/ | → SCRIPTS/ |
| WIN95PPP.TXT | pcb1541/pcbic12/bin/SCRIPTS/ | → SCRIPTS/ |
| TPA.BAT | pcb1541/pcbic12/bin/ | → root |
| FILE_ID.DIZ | pcb1541/pcbic12/bin/ | → (installer metadata) |
| PPP | pcb1541/pcbic12/bin/ | → root |
| SLIP | pcb1541/pcbic12/bin/ | → root |
| PCBIC (config) | pcb1541/pcbic12/bin/ | → root |

IC binaries do NOT go in data/ — they go in OUT/upd154/pwa/bins/.

### OUT/data/commdrv/ — COMMDRV data (non-EXE, non-DRV)

| File | Description |
|---|---|
| ARNETSP4.DAT | Arnet SmartPort 4-port sample config |
| ARNETSP8.DAT | Arnet SmartPort 8-port sample config |
| DIGI4E.DAT | DigiBoard 4-port sample config |
| DIGI8E.DAT | DigiBoard 8-port sample config |
| BOCA1610.BIN | Boca BB1610 firmware |
| XABIOS.BIN | DigiBoard BIOS firmware |
| XACOMX.BIN | DigiBoard COM/Xi firmware |
| XACOOK.BIN | DigiBoard cooked-mode firmware |
| MONITOR.BAT | Port monitor launcher |

Binaries (COMMDRV.EXE, COMMTSR.EXE, DRVSETUP.EXE, TEST.EXE + 9 .DRV files)
→ OUT/pwa153/bins/COMMDRV/

### OUT/data/pcbmail/ — QWK Mailer data

| File | Description |
|---|---|
| PCBMAIL.HLP | QWK mailer help |

Binaries (PCBMAIL.EXE, BWCC.DLL, BC450RTL.DLL) → OUT/pwa153/bins/PCBMAIL/

### OUT/data/pcbos2/ — OS/2 Support data

| File | Description |
|---|---|
| BOARD.CMD | OS/2 board launcher |
| PCBCP.HLP | OS/2 PM control panel help |
| SAMPLE.OS2 | OS/2 sample config |
| STARTOS2.CMD | OS/2 startup script |

Binaries (PCBCP.EXE, PCBMONI2.EXE, PCBOARD2.EXE, PCBPACK2.EXE,
USERNET2.EXE, PCBTITLE.COM) → OUT/pwa153/bins/PCBOS2/

### OUT/data/help/ — 63 base HLP files

HLP!, HLP1-HLP15, HLPA-HLPZ, HLPALIAS, HLPBRD, HLPCHAT, HLPCMENU,
HLPENDR, HLPFLAG, HLPFSCRN, HLPLANG, HLPNEWS, HLPOPEN, HLPQWK,
HLPREG, HLPREP, HLPRM, HLPSEC, HLPSEL, HLPSRCH, HLPTEST, HLPTS,
HLPUSERS, HLPWHO

### OUT/data/doc/ — 30 documentation files

ADDENDUM.DOC, COUNTRY.TXT, DISCLAIM.TXT, DOORWAY.DOC, ENV2DOS.DOC,
FIDO.DOC, FIDOHUB.DOC, FIDONET.DOC, HOWTODBF.TXT, NEWBOARD.DOC,
PACK.DOC, PCBCP.DOC, PCBEDIT.DOC, PCBMAIL.DOC, PCBOARD2.DOC,
PCBOS2.TXT, PPE_INFO.TXT, PPE_MEM.DOC, PPLCMDS.DOC, UUCP.DOC,
WHATSNEW.151, .152, .200, .300, .310, .521, .522, .FID, .PPL, .UU

### OUT/data/ppl/ — 42 PPE+PPS files

21 PPE/PPS pairs: ACCNTDBF, DBASE, DOORS, HAMURABI, HELLO1-7, KAL,
LANGUAGE, MORE, NODEFILE, OPPAGE, ORDER, PWRDWARN, START, WELFIRST
Plus RUN1.BAT, RUN2.BAT

### OUT/data/gen/ — 95 system files

BLT/BLT1-4 (bulletin screens), BRDS/BRDM (board menus), DIR1-7
(file dir screens), DLPATH (download paths), DOORS (door menu),
CHTM, CLOSED, CMD.LST, CNFN.PPE/PPS, DIR.PPE/PPS, DISPLAY,
EXPIRED, GRAF-D.*, GROUP, HISTORY, MAINEVNT, NEWS, NEWUSER,
NOANSI, PCBML.DAT, PCBPROT.DAT, PCBTEXT, RULES, SAMPLE, SCRIPT,
SCRIPT.LST, SCRIPT1, SCRIPT2.PPE/PPS, STUFF, UPLOAD, USERS15.*,
WARNING, WELCOME
Plus multi-language/node variants (.43, .53, .83, .A3, etc.)

### OUT/data/fido/ — 18 FidoNet configs

AKAS.DAT, AREAS.DAT, AREAS.IDX, FREQDENY.DAT, FREQPATH.DAT,
MAGICNAM.DAT, NODEARC.DAT, NODELIST.DAT, ORIGINS.DAT, PCBFIDO.CFG,
PHONEX.DAT, RESPONSE/ (subdirectory)

### OUT/data/graphics/ — 7 ANSI graphics

DIR.LST, DIR1, DIR2, DIR3, DLPATH.LST, PRIVATE, PUBLIC

### OUT/data/files/ — 11 file area configs

DIR.LST, DIR1-DIR7, DLPATH.LST, PRIVATE, PUBLIC

### OUT/data/main/ — 33 message base skeleton

CNAMES and variants (.62, .72_, .82.ADD, .92.IDX, .ADD, .IDX, .K,
.L3, .L_, .M.ADD, .M3_, .N.IDX, .N3.ADD, .O0, .O3.IDX, .P0_,
.Q0.ADD, .R0.IDX, CNAMES_), FSEC, HOLIDAYS.CFG, MSGS, MSGS.IDX,
PRIVATE (+ .P3, .S0), PUBLIC (+ .Q3, .T0), PWRD, TCAN

### OUT/data/conferences/ — 9 sample conferences

| Conference | Files | Contents |
|---|---|---|
| ADMIN/ | 11 | BLT, BLT.LST, BLT1, BLT2, BRDM, DIR.LST, DIR1, DLPATH.LST, NEWS, PRIVATE, PUBLIC |
| CUSTSRVC/ | 9 | BLT, BLT.LST, BRDM, DIR.LST, DIR1, DLPATH.LST, NEWS, PRIVATE, PUBLIC |
| EMPLOYEE/ | 10 | BLT, BLT.LST, BRDM, DIR.LST, DIR13, DLPATH.LST, INTRO, NEWS, NEWS.G1, PUBLIC |
| ENGINRNG/ | 9 | BLT, BLT.LST, BRDM, DIR.LST, DIR1, DLPATH.LST, NEWS, PRIVATE, PUBLIC |
| HMNRSRC/ | 9 | BLT, BLT.LST, BRDM, DIR.LST, DIR1, DLPATH.LST, NEWS, PRIVATE, PUBLIC |
| MIS/ | 9 | BLT, BLT.LST, BRDM, DIR.LST, DIR1, DLPATH.LST, NEWS, PRIVATE, PUBLIC |
| PROD1/ | 11 | BLT, BLT.LST, CHANGES, DIR.LST, DIR3, DIR4, DLPATH.LST, NEWS, PRIVATE, PUBLIC, TECHTIPS |
| PROD2/ | 11 | BLT, BLT.LST, CHANGES, DIR.LST, DIR3, DIR4, DLPATH.LST, NEWS, PRIVATE, PUBLIC, TECHTIPS |
| SLSMKTNG/ | 10 | BLT, BLT.LST, BRDM, DIR.LST, DIR1, DLPATH.LST, INTRO, NEWS, PRIVATE, PUBLIC |

### OUT/data/root/ — root install files (non-EXE)

| File | Description |
|---|---|
| BOARD.BAT | Board launcher |
| MODEMS.DAT | Modem init strings |
| PCBOARD.DAT | Main PCBoard config |
| PCBCMPRS.BAT | Compression batch |
| PCBFILER.DEF | Filer definitions |
| PCBQWK.BAT | QWK batch |
| PCBRB.BAT | Receive batch (B protocol) |
| PCBRH.BAT | Receive batch (H protocol) |
| PCBRZ.BAT | Receive batch (Zmodem) |
| PCBSB.BAT | Send batch (B protocol) |
| PCBSH.BAT | Send batch (H protocol) |
| PCBSM.CLR | SM color config |
| PCBSM.CNF | SM config |
| PCBSM.HLP | SM help |
| PCBSZ.BAT | Send batch (Zmodem) |
| PCBTEST.BAT | Test batch |
| PCBVIEW.BAT | View batch |
| REMOTE.SYS | Remote system file |

### OUT/data/dl01/ — 2 sample downloads

ALLFILES.ZIP, CALGUIDE.ZIP

---

## Where EXEs Go (bins/)

### OUT/pwa153/bins/ — 15.3 base EXEs

Root (37): PCBOARD.EXE, PCBOARDM.EXE, PCBSETUP.EXE, PCBSM.EXE,
PCBSTATS.EXE, PCBPACK.EXE, PCBMONI.EXE, PCBMODEM.EXE, PCBEDIT.EXE,
PCBFILER.EXE, PCBDIAG.EXE, PCBDESC.EXE, PCBNLC.EXE, DOORWAY.EXE,
ENCRYPT.EXE, FIDOUTIL.EXE, FIXTEXT.EXE, INIT.EXE, MAKEIDX.EXE,
MKPCBMNU.EXE, MKPCBTXT.EXE, OVLSIZE.EXE, PACKFIDO.EXE, RDPCBTXT.EXE,
TESTFILE.EXE, UPGRADE.EXE, USERNET.EXE, UUIN.EXE, UUOUT.EXE,
UUUTIL.EXE, UUXFER.EXE, VIEWARCH.COM, VIEWZIP.EXE, ZMRECV.EXE,
ZMSEND.EXE, PPLC100.EXE, PPLC330.EXE

COMMDRV/ (13): COMMDRV.EXE, COMMTSR.EXE, DRVSETUP.EXE, TEST.EXE + 9 .DRV
PCBMAIL/ (3): PCBMAIL.EXE, BWCC.DLL, BC450RTL.DLL
PCBOS2/ (6): PCBCP.EXE, PCBMONI2.EXE, PCBOARD2.EXE, PCBPACK2.EXE, USERNET2.EXE, PCBTITLE.COM

### OUT/upd154/pwa/bins/ — 15.4 upgrade EXEs (IC)

Pcbic.exe, Pcbic2.exe, PCBICCFG.EXE, PCBICEVT.EXE, TESTIC.EXE, TESTIC2.EXE

---

## PCBoard Installer Architecture

Clark's INSTALL.EXE reads INSTALL.DAT (a manifest/script) and extracts
files from .RED archives (LZH-family compression) onto the target system.

### .RED Archives (8 in base 15.3)

| Archive | Files | Contents |
|---|---|---|
| PCBOARD.RED | 4 | PCBOARD.EXE, PCBOARDM.EXE, PPLC.EXE |
| PCBDISK.002 | 202 | DOC/*, HELP/*, GEN/*, MAIN/*, FIDO/*, GRAPHICS/*, utilities |
| PCBDISK.003 | 22 | PCBSM.EXE, PCBSTATS.EXE, USERNET.EXE, + utilities |
| COMMDRV.RED | 22 | COMMDRV/* |
| PCBMAIL.RED | 4 | PCBMAIL/* |
| PCBCFGS.RED | 171 | Conference configs |
| PPLC.RED | 46 | PPL/* |
| PCBOARD2.RED | 0 | OS/2 binaries |

### Disk Layout

| Disk | Archives |
|---|---|
| Disk 1 | PCBOARD.RED, PCBDISK.002 |
| Disk 2 | PCBDISK.003, COMMDRV.RED, PCBMAIL.RED |
| Disk 3 | PCBCFGS.RED, PPLC.RED |
| Disk 4 | PCBOARD2.RED (OS/2) |
| Disk 5 | (overflow/updates) |

### Source → Install Target File Map (18 matches)

| File | Source | Target | Match |
|---|---|---|---|
| AKAS.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | IDENTICAL |
| AREAS.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | DIFFERENT |
| BOARD.BAT | TEST/ | root | DIFFERENT |
| FREQDENY.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | DIFFERENT |
| FREQPATH.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | IDENTICAL |
| HISTORY | upd154/docs/ | GEN/ | DIFFERENT |
| MAGICNAM.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | IDENTICAL |
| NODEARC.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | IDENTICAL |
| NODELIST.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | DIFFERENT |
| ORIGINS.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | DIFFERENT |
| PCBFIDO.CFG | SOURCE/MISC/FIDOUTIL/ | FIDO/ | DIFFERENT |
| PCBOARD.DAT | SOURCE/MISC/FIDOUTIL/ | root | DIFFERENT |
| PCBQWK.BAT | TEST/ | root | IDENTICAL |
| PCBRZ.BAT | TEST/ | root | DIFFERENT |
| PCBSZ.BAT | TEST/ | root | DIFFERENT |
| PCBTEXT | SOURCE/UTIL/PCBTEXT/ | GEN/ | IDENTICAL |
| PHONEX.DAT | SOURCE/MISC/FIDOUTIL/ | FIDO/ | IDENTICAL |
| FIDO.DOC | toolkit/pwa153/ | DOC/ | (pwa153) |

---

## Build Pipeline (pcb154 end-to-end)

### Step 1: Compile source → EXEs
- pcb154/MAIN/SOURCE/ → all EXEs → OUT/pwa153/bins/ (base) or OUT/upd154/*/bins/ (upgrade)
- **pcb154/MAIN/SOURCE/IC/ → IC EXEs → OUT/upd154/pwa/bins/**

### Step 2: Compile HELP
- MAKEHELP.C → 63 HLP files → OUT/data/help/
- **PCBIC.HLP → OUT/data/ic12/**

### Step 3: Compile PPL
- PPLC .PPS → .PPE → OUT/data/ppl/
- **RUNINET.PPS → RUNINET.PPE → OUT/data/ic12/**

### Step 4: Assemble docs
- DOC files → OUT/data/doc/
- **PCBIC.DOC, PCBIC.PDF → OUT/data/ic12/**

### Step 5: Assemble data
- GEN, FIDO, MAIN, etc. → OUT/data/
- **IC data files → OUT/data/ic12/**

### Step 6: Package .RED archives
```
Input:  OUT/*/bins/ + OUT/data/
Tool:   REDX archiver
Output: .RED archives
```

### Step 7: Update INSTALL.DAT manifest
- @File entries for all files with @Out paths
- Installer knows OUT/data/ic12/PCBIC.HLP → install target HELP/

### Step 8: Build installer disks
```
Tool:   BLDINS.BAT
Input:  .RED archives + INSTALL.EXE + INSTALL.DAT
Output: disk1/ through diskN/
```

### Step 9: Verify installer
- Run INSTALL.EXE in DOSBox-X
- Checksum installed tree

---

## Repository Restructure (restruc-repo.bat)

### Moves:

| What | From | To |
|---|---|---|
| COMMDRV EXEs+DRVs | target/COMMDRV/ | OUT/pwa153/bins/COMMDRV/ |
| COMMDRV data | target/COMMDRV/*.{DAT,BIN,BAT} | OUT/data/commdrv/ |
| HELP/ | target/HELP/ | OUT/data/help/ |
| DOC/ | target/DOC/ | OUT/data/doc/ |
| PPL/ | target/PPL/ | OUT/data/ppl/ |
| GEN/ | target/GEN/ | OUT/data/gen/ |
| FIDO/ | target/FIDO/ | OUT/data/fido/ |
| GRAPHICS/ | target/GRAPHICS/ | OUT/data/graphics/ |
| MAIN/ | target/MAIN/ | OUT/data/main/ |
| PCBMAIL/ | target/PCBMAIL/ | OUT/data/pcbmail/ |
| PCBOS2/ | target/PCBOS2/ | OUT/data/pcbos2/ |
| FILES/ | target/FILES/ | OUT/data/files/ |
| Conferences (9) | target/{ADMIN,...}/ | OUT/data/conferences/ |
| Root data | target/*.{BAT,DAT,...} | OUT/data/root/ |
| DL01/ | target/DL01/ | OUT/data/dl01/ |
| EXEs (37) | target/*.EXE | **DELETE** |
| **Then delete** | pcb1541/install/dist/target/ | gone |

---

## Open Questions

1. **PCBIC.HLP** — Compiled from source or standalone?
2. **README.1ST conflict** — Base has one, IC has one. Rename?
3. **Archive strategy** — Merge IC into existing .RED or new PCBIC.RED?
4. **Disk assignment** — IC on existing disk or new disk?
5. **VMData shared code** — PCBICEVT vs delta154 VMAVL.C/VMFUNCS.C
6. **6 missing toolkit backends** — Copy from pcb154?
7. **COMMDV08 Win95 VxD** — No source exists
8. **PCBMAIL** — EXE is Windows, sits with DLLs. Belongs in data/ or bins/?

---

## Phase Plan

### Phase A: Source + data placement (pcb154 only)
- Copy NASM source into pcb154/MAIN/SOURCE/IC/
- Place IC data files → OUT/data/ic12/
- Place IC binaries → OUT/upd154/pwa/bins/
- Original stays in pcb1541/pcbic12/

### Phase B: Shared code + build integration
- Map VMData funcs vs delta154 VMAVL.C/VMFUNCS.C
- Shared headers with pcb154/MAIN/SOURCE/H/
- Update build scripts for IC targets
- Verify byte-exact builds

### Phase C: Installer packaging
- Add IC files to .RED archives
- Update INSTALL.DAT
- Build complete installer disk set from OUT/
- Verify installer end-to-end

### Phase D: Documentation + release
- Update pcb154 README.md
- Generate patches, CHECKSUMS, APPLY.txt, HISTORY.md

### Phase E: PCBICEVT vs pcbis event comparison
- Clark's batch processor vs pcbis_events.pas

---

## todo/ Cleanup — Duplicate Documentation

Installer/build/placement info is spread across 10+ files between todo/ and
docs/pcboard-internals/. These need consolidating before adding more.

### Overlapping files:

**RED / Installer:**
- todo/RED_FORMAT.md (36 lines)
- docs/RED_ARCHIVES_COVERAGE.md (105 lines)
- docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md (217 lines)
- docs/pcboard-internals/INSTALL-EXE-PARITY.md (337 lines)
- docs/pcboard-internals/INSTALL-EXE-GAP-ANALYSIS.md
- docs/pcboard-internals/INSTALL-EXE-SIZE-CONVERGENCE.md

**Build / Distribution / Placement:**
- todo/BUILD.md (82 lines)
- todo/PCB154-DISTRIBUTION.md (182 lines)
- todo/PCB154_BUILD_GUIDE.md (158 lines)
- todo/PLACEMENT.md (124 lines)
- todo/DATA_DIRECTORY.md (301 lines)
- todo/SETUP_GUIDE.md (199 lines)
- todo/BINARY-CATALOG.md (550 lines)

**Restructure:**
- todo/RESTRUCTURE.LOG
- todo/repo-restructure.bat (already exists — we created a new restruc-repo.bat at root)

**Toolkit:**
- todo/toolkit.md (the real toolkit doc)

### Action needed:
- Read existing files BEFORE adding new content
- Consolidate dupes — one source of truth per topic
- Move finished docs from todo/ to docs/ or docs/pcboard-internals/
- Update existing files with PCBIC/OUT/data info, don't create new files
