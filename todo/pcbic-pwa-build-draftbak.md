# PCBIC in PCB 15.4 — Build Pipeline & Installer Integration (DRAFT)

## Current State

PCBIC v1.2 is fully reconstructed (all 6 binaries byte-exact, v0.2.9).
Original source stays in `pcb1541/pcbic12/` (standalone, verta + hexadecimal 3 weeks).
This document plans how PCBIC integrates into the 15.4 source tree,
the build pipeline, and the installer disk packaging.

IC is the 15.4 internet upgrade. pcb153 has no IC — no internet.
toolkit/delta154 is SDK only — IC does not go there.

**IC belongs to PWA 15.4.** Delta 15.4 inherits it — IC was missing for 20
years and is being added back on both legs. Phase A places IC on the PWA leg.
Delta inherits Clark's 6 binaries unchanged, plus RUNINET.PPS as real source
and the docs/data; see open question 10 and pcb154/DOCS/SYSOP_154.TXT §8.

### Phase A — DONE

- IC source → `pcb153/upd154/SOURCE/IC/` (20 files, `dos/` + `os2/` preserved)
- IC EXEs → `OUT/pwa153/upd154/` (6 files)
- IC data → `OUT/data/ic12/` (32 files, subdirs mirrored)
- Originals in `pcb1541/pcbic12/` untouched

---

## OUT/ — Canonical Binary + Data Output

> **Reality check, 2026-09-17.** The `OUT/data/` layout described in the
> next ~15 sections is a **plan, not a state**. `OUT/data/` currently
> contains one directory — `ic12/` — and nothing else. `data/root`,
> `data/gen`, `data/help`, `data/doc`, `data/ppl`, `data/main`,
> `data/fido`, `data/graphics`, `data/files`, `data/conferences`,
> `data/dl01`, `data/commdrv`, `data/pcbmail` and `data/pcbos2` do not
> exist. Populating them needs the same 15.3 distribution that the Tier 4
> binaries need, so it is blocked on the same acquisition.


All build output lives in OUT/. The installer assembles from here.
Binaries by version. Data by program — no duplicating directory structures.

**`bins/` is NOT where program EXEs go.** Per `OUT/*/bins/README.md`, every
`bins/` subdirectory holds *SDK example binaries* — the small buildable sample
add-ons (a door, a monitor, a front-end/echo demo, sample .PPEs) that prove the
SDK builds end to end. Shipping EXEs live at the **version top level**.

Version layout follows `OUT/README.md` (Convention A):

| Version | Source | Toolkit | Binaries |
|---|---|---|---|
| 15.3 PWA | pcb153/SOURCE | toolkit/pwa153 | OUT/pwa153 |
| 15.4 PWA | pcb153/upd154/SOURCE | toolkit/pwa154 | OUT/pwa153/upd154 |
| 15.4 Delta | pcb154/MAIN/SOURCE | toolkit/delta154 | OUT/delta154 |
| 15.41 IRC | pcb1541/ | toolkit/irc1541 | OUT/irc1541 |

```
OUT/
  pwa153/                    15.3 base EXEs (Borland)
    COMMDRV/                   WCSC serial driver package
    PCBMAIL/                   QWK mailer binaries
    PCBOS2/                    OS/2 binaries
    bins/                      SDK example binaries (NOT program EXEs)
    upd154/                    15.4 PWA upgrade EXEs (Borland) — incl. IC
      bins/                      SDK example binaries
  clark-original/            Clark's shipped 15.4 beta binaries (12 EXEs,
                             no source) — the byte-match reference
  delta154/                  15.4 full build (OpenWatcom)
    bins/                      SDK example binaries
  irc1541/                   15.41 full build (future, ow2irc)
    bins/                      SDK example binaries
  lib/                       toolkit libraries
  support/                   shared runtime data (PCBOARD.SER, PCBSM.CLR/CNF, ENDPCB)
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

**Subdirectories are mirrored, not flattened.** A flat `ic12/` is impossible:
`bin/PPP` (150 b) and `bin/SLIP` (151 b) are different files from `bin/DATA/PPP`
and `bin/DATA/SLIP` (377 b each), and would overwrite each other. Mirroring also
removes the `README.1ST` conflict — it only gets renamed at install time.

```
OUT/data/ic12/
  PCBIC.HLP, RUNINET.PPE, RUNINET.PPS, FILE_ID.DIZ
  DATA/      15 files
  DOCS/       3 files
  SCRIPTS/    6 files
  root/       4 files (PCBIC, PPP, SLIP, TPA.BAT)
```

32 files total. Source for all of them: `pcb1541/pcbic12/bin/`.

| File | In ic12/ | Install Target |
|---|---|---|
| PCBIC.HLP | (top) | → HELP/ |
| RUNINET.PPE | (top) | → PPL/ |
| RUNINET.PPS | (top) | → PPL/ |
| FILE_ID.DIZ | (top) | → (installer metadata) |
| PCBIC.DOC | DOCS/ | → DOC/ |
| PCBIC.PDF | DOCS/ | → DOC/ |
| README.1ST | DOCS/ | → DOC/ (rename to avoid conflict) |
| FING | DATA/ | → DATA/ |
| FTP | DATA/ | → DATA/ |
| FTPSCRN | DATA/ | → DATA/ |
| GOPH | DATA/ | → DATA/ |
| ICPROFS.DAT | DATA/ | → DATA/ |
| MENU | DATA/ | → DATA/ |
| MENU.DAT | DATA/ | → DATA/ |
| PING | DATA/ | → DATA/ |
| PPP | DATA/ | → DATA/ |
| RLOG | DATA/ | → DATA/ |
| SLIP | DATA/ | → DATA/ |
| TCPTEXT | DATA/ | → DATA/ |
| TELN | DATA/ | → DATA/ |
| TROU | DATA/ | → DATA/ |
| WHO | DATA/ | → DATA/ |
| OS2SLIP.CMD | SCRIPTS/ | → SCRIPTS/ |
| OS2SLIP.TXT | SCRIPTS/ | → SCRIPTS/ |
| WIN31PPP.SCP | SCRIPTS/ | → SCRIPTS/ |
| WIN31PPP.TXT | SCRIPTS/ | → SCRIPTS/ |
| WIN95PPP.SCP | SCRIPTS/ | → SCRIPTS/ |
| WIN95PPP.TXT | SCRIPTS/ | → SCRIPTS/ |
| PCBIC (config) | root/ | → root |
| PPP | root/ | → root |
| SLIP | root/ | → root |
| TPA.BAT | root/ | → root |

IC binaries do NOT go in data/ — they go in `OUT/pwa153/upd154/`.
Not staged into ic12/: `bin/rebuilt/*.exe` — reconstruction comparison
artifacts, they stay in `pcb1541/pcbic12/bin/rebuilt/`.

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
→ OUT/pwa153/COMMDRV/

### OUT/data/pcbmail/ — QWK Mailer data

| File | Description |
|---|---|
| PCBMAIL.HLP | QWK mailer help |

Binaries (PCBMAIL.EXE, BWCC.DLL, BC450RTL.DLL) → OUT/pwa153/PCBMAIL/

### OUT/data/pcbos2/ — OS/2 Support data

| File | Description |
|---|---|
| BOARD.CMD | OS/2 board launcher |
| PCBCP.HLP | OS/2 PM control panel help |
| SAMPLE.OS2 | OS/2 sample config |
| STARTOS2.CMD | OS/2 startup script |

Binaries (PCBCP.EXE, PCBMONI2.EXE, PCBOARD2.EXE, PCBPACK2.EXE,
USERNET2.EXE, PCBTITLE.COM) → OUT/pwa153/PCBOS2/

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

## Where EXEs Go (version top level — NOT bins/)

### OUT/pwa153/ — 15.3 base EXEs

Root (37): PCBOARD.EXE, PCBOARDM.EXE, PCBSETUP.EXE, PCBSM.EXE,
PCBSTATS.EXE, PCBPACK.EXE, PCBMONI.EXE, PCBMODEM.EXE, PCBEDIT.EXE,
PCBFILER.EXE, PCBDIAG.EXE, PCBDESC.EXE, PCBNLC.EXE, DOORWAY.EXE,
ENCRYPT.EXE, FIDOUTIL.EXE, FIXTEXT.EXE, INIT.EXE, MAKEIDX.EXE,
MKPCBMNU.EXE, MKPCBTXT.EXE, OVLSIZE.EXE, PACKFIDO.EXE, RDPCBTXT.EXE,
TESTFILE.EXE, UPGRADE.EXE, USERNET.EXE, UUIN.EXE, UUOUT.EXE,
UUUTIL.EXE, UUXFER.EXE, VIEWARCH.COM, VIEWZIP.EXE, ZMRECV.EXE,
ZMSEND.EXE, PPLC100.EXE, PPLC330.EXE


#### That 37 is a SHIP list, not a BUILD list

Audited 2026-09-17 against every `.MAK` in the source archive
(`reference/pcb153src0014.zip` -> `PCBSRCV/000/`), the repo tree, and the
packaged build root (`DOSBOXX.ZIP` -> `pcbirc/BUILDROOT/`). The 37 is
what the installer puts on disk. Four different numbers were being
conflated:

| | count | meaning |
|---|---|---|
| shipped | **37** | what the 15.3 installer writes to the target |
| has a makefile somewhere | **24** | buildable in principle from surviving source |
| present in the build root | **11** | what `DOSBOXX.ZIP` can reach today |
| actually built | **2** | `PCBOARD.EXE`, `PCBOARDM.EXE` |

##### Tier 1 — in the repo AND the build root (11 + MAKEHELP)

PCBSETUP, PCBSM, MKPCBTXT, FIDOUTIL, MAKEIDX, USERNET, USERNET2, UUIN,
UUOUT, UUUTIL, UUXFER. Plus `MISC\HELP\MAKEHELP.C`, which has its own
`COMPILE.BAT` and no `.MAK`.

**FIDOUTIL was wrongly listed as blocked.** Its 11th module,
`packfido.obj`, supplies one symbol — `do_pack()` — whose only call site
is commented out (`CONVERT.CPP:124`). Dropped from the makefile
2026-09-17; FIDOUTIL builds with its ten real modules. See
`pcb153/SOURCE/MISC/PACKFIDO/README.md` (PACKFIDO-MISSING.md resolved, moved to attic 2026-09-25).

##### Tier 2 — in the repo, NOT in the build root (blocked by a packaging gap)

PCBOARD, PCBOARDM, PCBOARD2, PPLC100, PPLC330.

`BUILDROOT/PCB153/` has **no `153\` directory**. It carries the batch
drivers (`COMPILE.BAT`, `BCDOS.BAT`, `ASMCOMP.BAT`, `TKCOMP.BAT`,
`PPLC.BAT`) but none of the makefiles they invoke — no `PCBOARD.MAK`, no
`PCBOARD2.MAK`, no `PPLC.MAK`, no `PCBOARD.CFG`, none of the
`.RES`/`.386` link-response files. `COMPILE.BAT` runs and fails. The two
binaries in `OUT/pwa153/` came from a separately staged tree, not from
the zip. **Fixing the zip is the cheapest single unblock in this list.**

##### Tier 3 — source EXISTS in the archive, never imported (13)

Complete source and Clark's own makefiles, sitting in `PCBSRCV/000/` and
in neither the repo nor the build root. Importing is a **copy**, not a
reconstruction:

| Target | Archive path |
|---|---|
| PCBCP | `UTIL/PCBCP/1522/PCBCP.MAK` |
| PCBDIAG | `UTIL/PCBDIAG/PCBDIAG.MAK` |
| PCBEDIT | `UTIL/PCBEDIT/MAK/PCBEDIT.MAK` |
| PCBFILER | `UTIL/PCBFILER/PCBFILER.MAK` |
| PCBMODEM, MSETUP | `UTIL/PCBMODEM/{PCBMODEM,MSETUP}/*.MAK` |
| PCBMONI, PCBMONI2 | `UTIL/PCBMONI/PCBMONI{,2}.MAK` |
| PCBNLC | `UTIL/PCBNLC/PCBNLC.MAK` |
| PCBPACK, PCBPACK2 | `UTIL/PCBPACK/153/PCBPACK{,2}.MAK` |
| PCBSTATS | `UTIL/PCBSTATS/PCBSTATS.MAK` |
| WAITFILE | `MISC/WAITFILE/WAITFILE.MAK` |
| ZMRECV, ZMSEND | `MISC/ZMODEM/ZMODEM.MAK` |

Two things bite on import: **small-model category libraries**
(`DOS_S.LIB` etc.) have never been built, and PCBSTATS, PCBMONI,
USERNET, WAITFILE, MKPCBTXT are small-model; and several want objects at
`<subdir>\large\` where the 15.3 build writes `<subdir>\large.386\`.

##### Tier 4 — NO SOURCE AND NO BINARY (13)

No makefile in `PCBSRCV/000/`, no source in the repo — **and no binary
either**, which is the part that decides the method.

**Nothing in this tier can be decompiled today.** Every Clark binary the
repo holds was inventoried 2026-09-17:

| Location | Contents |
|---|---|
| `OUT/clark-original/DOS/` | LOCAL, MKPCBTXT, PCBOARD, PCBOARDM, PCBSETUP, PCBSM, PPLC, UUIN, UUOUT, UUUTIL, UUXFER (11, the **15.4 beta**) |
| `OUT/clark-original/OS2/` | PCBOARD2 |
| `OUT/pwa153/upd154/` | the 6 PCBIC 1.2 binaries |

Eighteen binaries, and **not one of them is a Tier 4 target**. Every
binary we hold is for a program whose source we already have. The
overlap is exactly zero, which is not a coincidence — the 15.4 beta
distribution shipped the main programs, and the utilities came on the
15.3 install disks, which this project does not have.

So the backlog splits by method, and the first job is acquisition, not
engineering:

1. **Find a PCBoard 15.3 distribution.** The installer `.RED` archives
   (`PCBDISK.002`, `PCBDISK.003`) carry these EXEs. With them, twelve of
   the thirteen become a decompile — tractable work, and the project has
   done it before (RIPterm 1.54, PCBIC 1.2, NetSerial, INSTALL.EXE).
   Without them it is clean-room reimplementation from behaviour nobody
   has observed in twenty years.
2. **Only then decompile.** All twelve are small single-purpose DOS
   utilities built with the same Borland C++ 3.1 the rest of the tree
   uses, so the existing decompile workflow applies directly.
3. **DOORWAY is not ours either way** — see below.

| Target | Source | Binary | Method |
|---|---|---|---|
| PACKFIDO.EXE | none | none | acquire, then decompile |
| PCBDESC.EXE | none | none | acquire, then decompile |
| ENCRYPT.EXE | none | none | acquire, then decompile |
| FIXTEXT.EXE | none | none | acquire, then decompile |
| INIT.EXE | none | none | acquire, then decompile |
| MKPCBMNU.EXE | none | none | acquire, then decompile |
| OVLSIZE.EXE | none | none | acquire, then decompile |
| RDPCBTXT.EXE | none | none | acquire, then decompile |
| TESTFILE.EXE | none | none | acquire, then decompile |
| UPGRADE.EXE | none | none | acquire, then decompile |
| VIEWARCH.COM | none | none | acquire, then decompile |
| VIEWZIP.EXE | none | none | acquire, then decompile |
| DOORWAY.EXE | third-party | none | **not ours** — license or drop |

What is known about each, for when a binary turns up:

| Target | What is known | Notes |
|---|---|---|
| `PACKFIDO.EXE` | `void do_pack(void)` — one prototype | Source lived at `E:\TC\PACKFIDO\PACKFIDO.C`, a developer's Turbo C scratch drive, per two `.DSK` desktop files in the archive. Never in archive scope. FidoNet outbound packer. |
| `PCBDESC.EXE` | name only | file-description utility |
| `ENCRYPT.EXE` | name only | user-record encryption; `LIB/SOURCE/MISC/CRYPT.C` may be the algorithm |
| `FIXTEXT.EXE` | name only | PCBTEXT repair |
| `INIT.EXE` | name only | first-run initialisation |
| `MKPCBMNU.EXE` | name only | menu compiler; pairs with MKPCBTXT |
| `OVLSIZE.EXE` | name only | overlay-size reporter for the `-Y` build |
| `RDPCBTXT.EXE` | name only | PCBTEXT reader; inverse of MKPCBTXT |
| `TESTFILE.EXE` | name only | archive integrity tester |
| `UPGRADE.EXE` | name only | version upgrade; NOT the toolkit sample door of the same name |
| `VIEWARCH.COM` | name only, `.COM` | archive viewer — `.COM`, so tiny model, likely assembly |
| `VIEWZIP.EXE` | name only | ZIP viewer |
| `DOORWAY.EXE` | **third-party** | Marshall Dudley's door driver, shipped under licence. Never Clark's and not ours to recreate. Source it or drop it from the target. |

Three of the thirteen could in principle be written clean-room without a
binary, because their file formats are already known to us:

- **MKPCBMNU** and **RDPCBTXT** — their inverse, `MKPCBTXT`, survives
  with source, so the PCBTEXT format is documented by working code.
- **OVLSIZE** — reads the Borland overlay header, which is publicly
  documented, and the 15.3 build uses `-Y` overlays so there is a real
  file to test against.
- **PACKFIDO** is a near-fourth: FTS-0001 is public, so the packet
  format is not the obstacle. What is missing is knowing what Clark's
  version actually did with it.

Everything else needs a binary first. Writing them from a name alone
would produce programs that share a filename with Clark's and nothing
else, which is worse than an honest gap.

COMMDRV/ (13): COMMDRV.EXE, COMMTSR.EXE, DRVSETUP.EXE, TEST.EXE + 9 .DRV
PCBMAIL/ (3): PCBMAIL.EXE, BWCC.DLL, BC450RTL.DLL
PCBOS2/ (6): PCBCP.EXE, PCBMONI2.EXE, PCBOARD2.EXE, PCBPACK2.EXE, USERNET2.EXE, PCBTITLE.COM

### OUT/pwa153/upd154/ — 15.4 PWA upgrade EXEs (IC)

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

## Build Pipeline (15.4 PWA end-to-end)

### Step 1: Compile source → EXEs
- pcb153/SOURCE/ → 15.3 base EXEs → OUT/pwa153/
  - **2 of 37 built.** See "That 37 is a SHIP list" above for the
    tier breakdown: 11 buildable now, 5 blocked on the build root's
    missing `153\`, 13 importable from the archive, 13 to be written
    from scratch (1 of those third-party).
- pcb153/upd154/SOURCE/ → 15.4 upgrade EXEs → OUT/pwa153/upd154/ (BLDUPD154.BAT)
- **pcb153/upd154/SOURCE/IC/ → IC EXEs → OUT/pwa153/upd154/**
- pcb154/MAIN/SOURCE/ → 15.4 Delta EXEs (OpenWatcom) → OUT/delta154/

### Step 2: Compile HELP
- MAKEHELP.C → 63 HLP files → OUT/data/help/
- **PCBIC.HLP → OUT/data/ic12/** (provenance unresolved — open question 1)

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
Input:  OUT/<version>/ (EXEs) + OUT/data/
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
| COMMDRV EXEs+DRVs | target/COMMDRV/ | OUT/pwa153/COMMDRV/ |
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
2. ~~**README.1ST conflict**~~ — RESOLVED. Mirrored subdirs keep it at
   `ic12/DOCS/README.1ST`; the rename happens at install time only.
3. **Archive strategy** — Merge IC into existing .RED or new PCBIC.RED?
4. **Disk assignment** — IC on existing disk or new disk?
5. **VMData shared code** — PCBICEVT vs delta154 VMAVL.C/VMFUNCS.C
6. **6 missing toolkit backends** — Copy from pcb154?
7. **COMMDV08 Win95 VxD** — No source exists
8. **PCBMAIL** — EXE is Windows, sits with DLLs. Belongs in data/ or OUT/pwa153/PCBMAIL/?
9. ~~**clark-original/**~~ — RESOLVED. Recovered from git history: Clark's
   shipped 15.4 beta binaries (BETA.ZIP), 12 EXEs (11 DOS + 1 OS/2), newest
   build stamp 04/22/97. They had lived at `OUT/DOS/` and `OUT/OS2/`, were
   deleted in `e4181e5` (2026-08-25 toolkit restructure) and never re-added;
   restored from `e4181e5^` into `OUT/clark-original/` with
   SHA256 sums. The `OUT/pwa154/` slot in older notes never existed.
   Still open: `README.1ST` says BETA.ZIP also carried `*.HLP` files and a
   separate `PCBUUCP.ZIP`. No 15.4 beta `.HLP` was ever committed — if the
   original BETA.ZIP can be re-sourced, those are the missing pieces.
   Note: the 4–5 disk `.RED` set described above is the **15.3** installer,
   a separate artifact. Clark shipped 15.4 as a beta upgrade onto an
   installed 15.x, not as a full disk release (see pcb153/upd154/README.md).
10. ~~**Delta inheritance**~~ — RESOLVED. Delta ships Clark's 6 IC binaries
    unchanged (Borland output, no C source exists; the NASM reconstruction is
    3,888 functions of raw `db` bytes and `pcbic.c` is a stub). RUNINET.PPE is
    inherited as real PPL source, docs and data ship from OUT/data/ic12/.
    Recorded in pcb154/DOCS/SYSOP_154.TXT §8 next to PCBSETUP and LOCAL, which
    are shipped-binary for the same reason. Writing C for the six remains
    possible later, as its own project.

---

## Phase Plan

### Phase A: Source + data placement (PWA 15.4) — DONE
- [x] Copy NASM source into pcb153/upd154/SOURCE/IC/ (20 files)
- [x] Place IC data files → OUT/data/ic12/ (32 files, subdirs mirrored)
- [x] Place IC binaries → OUT/pwa153/upd154/ (6 files)
- [x] Original stays in pcb1541/pcbic12/ — untouched

### Phase B: Build integration
- ~~Decide how Delta 15.4 inherits IC~~ — DECIDED: Delta ships Clark's 6 IC
  binaries unchanged; RUNINET.PPS and the data files are inherited as source
  and data. Recorded in pcb154/DOCS/SYSOP_154.TXT §8 and DELTA154-CHANGES.md.
- ~~RUNINET.PPE byte-exactness~~ — DEFERRED (decided 2026-09-16). Accepted as
  functionally equivalent, not byte-exact. Clark 1,808 B; ours 2,261 B (3.30) /
  2,286 B (3.40). Two blockers: PPLC 3.20 (solvable — lib chain is done) and
  the decompiled PPS's 63 implicit variables vs Clark's 39 (todo/
  pcb-libchain-build.md v0.2.8: "Blocked — original source lost"; both 3.20 and
  3.30 emit 2,261 B from it). Does not block the release — RUNINET.PPE ships as
  Clark's original from OUT/data/ic12/. See pcb153/upd154/SOURCE/IC/RUNINET.md
- Map VMData funcs vs VMAVL.C/VMFUNCS.C — NOTE: these are in pcb154/LIB/SOURCE/
  and toolkit/delta154/SOURCE/, NOT pcb154/MAIN/SOURCE/
- Shared headers with pcb153/upd154/SOURCE/H/
- Update BLDUPD154.BAT for IC targets
- Verify byte-exact builds against OUT/clark-original/ (12 reference EXEs in place)

### Phase C: Installer packaging
- Add IC files to .RED archives
- Update INSTALL.DAT
- Build complete installer disk set from OUT/
- Verify installer end-to-end

### Phase D: Documentation + release
- Update pcb153/upd154/README.md and pcb154 README.md
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
