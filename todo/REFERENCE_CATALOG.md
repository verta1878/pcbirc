# PCBoard Reference Materials Catalog

Source: https://files.mpoli.fi/software/DOS/BBS/
Downloaded: 2026-08-08

All files placed in `reference/` directory.

---

## DEVELOP9.ZIP — PCBoard Developer Documentation Kit (68KB)

**Status: CRITICAL — extracted to reference/develop9/**

Clark Development's official file structure documentation.
Every PCBoard binary file format documented with field offsets,
data types, and record layouts.

### Contents and Backport Plan

| File | Size | What | Backport to |
|------|------|------|------------|
| PCBDAT.DOC | 16KB | PCBOARD.DAT field-by-field layout (220+ lines) | Phase 5f: pcbdraw node I/O |
| USERSYS.DOC | 34KB | USERS.SYS record layout (per-node user state) | Phase 5f: pcbdraw user auth |
| PCBSYS.DOC | 11KB | PCBOARD.SYS record layout (per-node BBS state) | Phase 5f: who's online, node status |
| USERS.DOC | 5KB | USERS file record layout (user database) | Phase 5f: pcbdraw user lookup |
| MSGS.DOC | 16KB | Message base format (headers, index, text) | 15.41 pcbtoss (message import) |
| HEADERS.DOC | 21KB | Message header record fields | 15.41 pcbtoss |
| CNAMES.DOC | 9KB | CNAMES conference config format | Phase 5f: conference file area mapping |
| CALLERS.DOC | 1KB | CALLERS log record format | Phase 5f: activity log, last callers |
| DIR.DOC | 5KB | DIR file listing format | Phase 5f: file area save, pcbpscan DIR writer |
| DIRIDX.DOC | 2KB | DIR index format | Phase 5f: file indexing |
| FILEIDX.DOC | 5KB | FIDX/FSIDX file index format | Phase 5f: file search |
| USERNET.DOC | 4KB | USERNET.DAT/XXX node chat format | Phase 5f: teleconference node comms |
| STRUCTS.DOC | 6KB | C struct definitions for all records | All phases — direct code reference |
| FIDO.DOC | 9KB | PCBoard FidoNet integration fields | Phase 2: pcbbinkp config, 15.41 pcbtoss |
| ACCOUNT.DOC | 6KB | Accounting/billing record format | Future |
| PWRD.DOC | 2KB | Password file format | Phase 5f: user auth |
| 145INFO.DOC | 27KB | PCBoard 14.5 info (historical reference) | Reference only |
| OS2PORTS.DOC | 6KB | OS/2 port-specific fields and structures | Phase 3: OS/2 native |
| PCBSTATS.DOC | 1KB | Statistics file format | Future |
| FLIST.DOC | 2KB | File list format | Phase 5f |
| DIRLST.DOC | 1KB | Directory list format | Phase 5f |

### Immediate Use

STRUCTS.DOC contains C struct definitions we can drop directly
into pcbdraw.h for PCBOARD.SYS, USERS.SYS, CALLERS, USERNET.DAT
record access. This is exactly what phase 5f needs.

---

## FP14S.ZIP — FidoPCB 1.4s (217KB)

**Status: Reference for pcbbinkp and 15.41 toss/scan**

FidoPCB by Rick Hobbs — FidoNet echomail/netmail interface for
PCBoard. Includes tosser, scanner, and configurator.

### Contents

| File | What |
|------|------|
| FP8086.EXE | FidoPCB main (8086 version) |
| FP386.EXE | FidoPCB main (386 version) |
| FPCONFIG.EXE | FidoPCB configuration utility |
| AREAS2FP.EXE | Area list converter |
| FIDOPCB.DOC | Full documentation (79KB) |
| FIDOPCB.BT | BinkleyTerm config template |
| FIDOPCB.FD | FrontDoor config template |

### Backport Value

- [ ] Study FIDOPCB.DOC for how it maps FidoNet areas → PCBoard conferences
- [ ] Study .BT/.FD configs for BSO outbound integration patterns
- [ ] Reference for 15.41 pcbtoss conference mapping
- [ ] Reference for echomail SEEN-BY/PATH handling with PCBoard

---

## CFOS097H.ZIP — CAPI FOSSIL Driver 0.97h (86KB)

**Status: Reference for wrench's FOSSIL stack**

FOSSIL driver for ISDN CAPI (Common ISDN API) adapters.
Provides FOSSIL INT 14h interface over ISDN instead of analog modem.

### Contents

| File | What |
|------|------|
| CFOS.EXE | CAPI FOSSIL driver TSR |
| CFOS.DOC | Full documentation (109KB) |
| CFOS.FAQ | FAQ |
| MODEM.DOC | Modem emulation settings |

### Backport Value

- [ ] Study CFOS.DOC for FOSSIL INT 14h implementation patterns
- [ ] Reference for pcbdraw serial/FOSSIL transport (sub-phase 5c)
- [ ] Reference for wrench's platform/fossil/ stack
- [ ] ISDN CAPI → useful if anyone still has ISDN hardware

---

## HOSTV12A.ZIP — KSP-HOST v1.2a (117KB)

**Status: Reference for pcbis telnet serving**

KSP-HOST by Keith S. Pechilis — inbound Telnet server for DOS BBSes.
Accepts TCP/IP Telnet connections and routes them to BBS nodes via
FOSSIL ports. Predecessor to what pcbis does.

### Contents

| File | What |
|------|------|
| KSP-SRVR.EXE | Telnet server (listener) |
| KSP-CALL.EXE | Outbound telnet caller |
| KSP-NODE.EXE | Node manager |
| KSP-HOST.DOC | Full documentation (48KB) |

### Backport Value

- [ ] Study KSP-HOST.DOC for DOS telnet→FOSSIL bridging patterns
- [ ] Reference for pcbbinkp answer mode (TCP listener)
- [ ] Reference for pcbdraw door mode telnet handling (sub-phase 5d)
- [ ] KSP-CALL outbound pattern may inform pcbbinkp poll mode

---

## NX-PCB.ZIP — NetExpress for PCBoard (1.1MB)

**Status: Reference for SMTP/NNTP/UUCP integration**

NetExpress by MicroDot Systems — full Internet gateway for PCBoard.
SMTP (email), NNTP (Usenet), UUCP, Gopher, and TCP/IP stack.
This is exactly what PCBoard's UUIN/UUOUT/UUUTIL replaces.

### Contents

| File | What |
|------|------|
| UUCICO.MSA | UUCP transport engine |
| MTA.MSA | Mail Transfer Agent (SMTP) |
| NETXPRES.MSA | Main NetExpress package |
| ONC.MSA | ONC/RPC networking |
| GOPHER.MSA | Gopher server/client |
| FAQDIAG.MSA | FAQ and diagnostics (269KB) |
| DOCS.MSA | Documentation |

Note: .MSA files are self-extracting archives.

### Backport Value

- [ ] Study UUCICO implementation for UUCP transport reference
- [ ] Study MTA for SMTP integration patterns with PCBoard
- [ ] Reference for any future NNTP gateway for PCBoard
- [ ] Historical: shows how Internet was bolted onto DOS BBSes in 1995

---

## PWAVE110.ZIP — PCBWAVE 1.10 (368KB)

**Status: Reference for offline mail and FidoNet↔UUCP gateway**

PCBWAVE — Blue Wave and QWK compatible offline mail door for PCBoard
with built-in FidoNet↔UUCP gateway.

### Contents

| File | What |
|------|------|
| INSTALL.EXE | Installer |
| INSTALL.PCW | Install package (1MB expanded) |
| INSTALL.DOC | Installation guide |
| WHATSNEW.110 | Version history |

### Backport Value

- [ ] Study offline mail packet format (Blue Wave / QWK)
- [ ] Study FidoNet↔UUCP gateway implementation
- [ ] Reference for any future QWK door on 15.4
- [ ] Historical: shows the mail format landscape of 1996

---

## Backport Priority Matrix

### HIGH — Direct code use in current phases

| File | From | Phase | What to extract |
|------|------|-------|----------------|
| STRUCTS.DOC | DEVELOP9 | 5f | C struct definitions → pcbdraw.h |
| PCBSYS.DOC | DEVELOP9 | 5f | PCBOARD.SYS record → who's online |
| USERS.DOC | DEVELOP9 | 5f | USERS record → user auth |
| USERSYS.DOC | DEVELOP9 | 5f | USERS.SYS record → node user state |
| CALLERS.DOC | DEVELOP9 | 5f | CALLERS record → last callers, activity log |
| USERNET.DOC | DEVELOP9 | 5f | USERNET.DAT → node-to-node chat |
| DIR.DOC | DEVELOP9 | 5f/4 | DIR format → pcbpscan DIR writer, file save |
| CNAMES.DOC | DEVELOP9 | 5f | Conference config → file area mapping |
| FIDO.DOC | DEVELOP9 | 2/4 | FidoNet fields → pcbbinkp config |

### MEDIUM — Reference for upcoming work

| File | From | Phase | What to study |
|------|------|-------|--------------|
| MSGS.DOC | DEVELOP9 | 15.41 | Message base format → pcbtoss |
| HEADERS.DOC | DEVELOP9 | 15.41 | Message headers → pcbtoss |
| CFOS.DOC | CFOS097H | 4a/5c | FOSSIL INT 14h patterns → pdserial.c |
| FIDOPCB.DOC | FP14S | 15.41 | Area↔conference mapping → pcbtoss |
| KSP-HOST.DOC | HOSTV12A | 4/5d | Telnet→FOSSIL bridge → door mode |
| PCBDAT.DOC | DEVELOP9 | 5f | PCBOARD.DAT layout → config reading |

### LOW — Historical reference

| File | From | Phase | Value |
|------|------|-------|-------|
| NX-PCB docs | NX-PCB | Future | SMTP/NNTP/UUCP gateway patterns |
| PWAVE110 | PWAVE110 | Future | QWK/Blue Wave offline mail format |
| 145INFO.DOC | DEVELOP9 | — | Historical only |
| OS2PORTS.DOC | DEVELOP9 | 3 | OS/2-specific fields (already done) |

---

## Checklist: Backport Tasks

### From DEVELOP9 (immediate)

- [ ] Extract C structs from STRUCTS.DOC into pcbdraw.h
- [ ] Add PCBOARD.SYS reader to pcbdraw.c (who's online)
- [ ] Add USERS record reader to pcbdraw.c (user auth)
- [ ] Add CALLERS record writer to pcbdraw.c (activity log)
- [ ] Add USERNET.DAT reader to pcbdraw.c (node chat)
- [ ] Verify pcbpscan DIR writer matches DIR.DOC format
- [ ] Add CNAMES reader for conference→file area mapping (15.41)
- [ ] Cross-reference FIDO.DOC with pcbbinkp config fields
- [ ] Feed MSGS.DOC + HEADERS.DOC into 15.41 pcbtoss design

### From FP14S (medium priority)

- [ ] Read FIDOPCB.DOC — document area↔conference mapping approach
- [ ] Compare FidoPCB BSO layout with pcbbinkp's bso.c

### From CFOS097H (medium priority)

- [ ] Read CFOS.DOC — document FOSSIL INT 14h call patterns
- [ ] Feed into pdserial.c design (sub-phase 5c)
- [ ] Share with wrench for platform/fossil/ reference

### From HOSTV12A (low priority)

- [ ] Read KSP-HOST.DOC — document telnet→FOSSIL bridge approach
- [ ] Feed into pddoor.c design (sub-phase 5d)

### From NX-PCB and PWAVE110 (future)

- [ ] Extract and read documentation when SMTP/NNTP phase starts
- [ ] Study QWK format if offline mail door is added

---

## Additional Downloads (2026-08-08, batch 2)

Source: https://files.mpoli.fi/software/DOS/BBS/

### Email/Mail Tools

| File | Size | What | Backport Value |
|------|------|------|---------------|
| SMAILBIN.ZIP | 111KB | smail/PC v2.5 UUCP mailer binary | UUCP transport reference |
| SMAILSRC.ZIP | 119KB | smail/PC v2.5 UUCP mailer **SOURCE CODE** | Direct reference for UUIN/UUOUT |
| NS11.ZIP | 432KB | NetXpress Server v1.1 DEMO (WWW/SMTP/NNTP) | SMTP/NNTP server patterns |
| QM4_0604.ZIP | 194KB | QMail 4.00 for PCBoard 15.0 | QWK mail door reference |
| PCB-ATC5.ZIP | 29KB | QWK file attach import/export (w/ source) | QWK attach handling |
| IZ_QS_13.ZIP | 51KB | qwkscan v1.30 complete QWK mail system PPE | QWK scanning reference |
| PCBNET71.ZIP | 124KB | PCB-Net v7.1 — updates mail waiting flags | Mail flag format reference |

### Reference Tools

| File | Size | What | Backport Value |
|------|------|------|---------------|
| TDRAW463.ZIP | 289KB | TheDraw 4.63 ANSI editor | Reference for pcbdraw editor (phase 5a) |
| CIMRG100.ZIP | 39KB | CIMERGE v1.00 — Clark Dev conference merge | Clark's own conference handling code |
| FD212.ZIP | 633KB | FrontDoor 2.12 FidoNet mailer (shareware) | Mailer/BSO reference for pcbbinkp |
| PCBCK325.ZIP | 128KB | pcbcheck 3.25 upload processor | Upload processing reference for pcbpscan |

### Checklist: New Downloads

- [ ] Extract SMAILSRC.ZIP — study UUCP source for PCBoard UUIN/UUOUT reference
- [ ] Study NS11.ZIP SMTP server for future email gateway
- [ ] Study TDRAW463.ZIP for editor UI patterns (phase 5a)
- [ ] Study CIMRG100.ZIP for Clark's conference merge code
- [ ] Study PCBCK325.ZIP for upload processing comparison with pcbpscan
- [ ] Study FD212.ZIP BSO outbound handling vs our bso.c
- [ ] Study PCBNET71.ZIP for MSGS.IDX mail waiting flag format
- [ ] Extract QM4_0604.ZIP for QWK packet format reference

### Files on mpoli.fi NOT Downloaded (lower priority)

These are available but not critical for current phases:

| File | What | Why skipped |
|------|------|------------|
| PWAVE100.ZIP | Older PCBWAVE v1.00 | Already have v1.10 |
| QM4.ZIP | QMail update | Have base v4.00 |
| QMSM.ZIP | QMail sub-update | Have base v4.00 |
| TIEAB38.ZIP | Email address book PPE | PPE, not C tool |
| ADRS101.ZIP | Email address PPE | PPE, not C tool |
| ENAME101.ZIP | Email name PPE | PPE, not C tool |
| E-BLT12.ZIP | FidoNet email bulletin PPE | PPE, not C tool |
| BAQWK10.ZIP | QWK command PPE | PPE, not C tool |
| EZQWK1.ZIP | QWK replacement PPE | PPE, not C tool |
| QWKBLT12.ZIP | QWK bulletin PPE | PPE, not C tool |
| QWKP095A.ZIP | QWK packer PPE | PPE, not C tool |
| JM_QT_10.ZIP | QWK transfer PPE | PPE, not C tool |
| MP_2B.ZIP | Msg-Pack QWK PPE | PPE, not C tool |
| MAILL501.ZIP | Mailing list PPE | PPE, not C tool |
| PCB15DMO.ZIP | PCBoard v15.0 DEMO (1.4MB) | Have full source already |

---

## Full Inventory: reference/

| File | Size | Category |
|------|------|----------|
| CFOS097H.ZIP | 86KB | FOSSIL driver reference |
| CIMRG100.ZIP | 39KB | Clark conference merge |
| DEVELOP9.ZIP | 68KB | **Developer docs (CRITICAL)** |
| FD212.ZIP | 633KB | FrontDoor mailer reference |
| FP14S.ZIP | 217KB | FidoPCB reference |
| HOSTV12A.ZIP | 117KB | Telnet server reference |
| IZ_QS_13.ZIP | 51KB | QWK mail system |
| NS11.ZIP | 432KB | NetXpress SMTP/NNTP server |
| NX-PCB.ZIP | 1.1MB | NetExpress gateway |
| PCB-ATC5.ZIP | 29KB | QWK file attach |
| PCBCK325.ZIP | 128KB | Upload processor |
| PCBNET71.ZIP | 124KB | Mail waiting flags |
| PWAVE110.ZIP | 368KB | Blue Wave/QWK mail door |
| QM4_0604.ZIP | 194KB | QMail 4.00 |
| SMAILBIN.ZIP | 111KB | UUCP mailer binary |
| SMAILSRC.ZIP | 119KB | **UUCP mailer SOURCE** |
| TDRAW463.ZIP | 289KB | TheDraw ANSI editor |
| develop9/ | — | Extracted developer docs |
| **Total** | **~4.2MB** | **17 archives + 1 extracted dir** |

---

## Full mpoli.fi Archive Mirror (2026-08-08, batch 3)

Downloaded the complete DOS/BBS (260 files) and DOS/OFFLINE (55 files)
directories from https://files.mpoli.fi/software/DOS/

After deduplication: **313 unique archives, 42MB total**

### Directory Layout

```
reference/
  *.ZIP (21)              Original hand-picked reference files + extracted develop9/
  mpoli_bbs/ (237)        Complete DOS/BBS directory from mpoli.fi
  mpoli_offline/ (55)     Complete DOS/OFFLINE directory from mpoli.fi
```

### Deduplication Results

| Check | Dupes Found | Action |
|-------|-------------|--------|
| mpoli_bbs vs tarball | 2 (PCBWIN95.ZIP, PCBMODEM.ZIP) | Removed from mpoli_bbs |
| mpoli_bbs vs reference/ root | 0 | — |
| mpoli_offline vs tarball | 0 | — |

### Categories in mpoli_bbs/ (237 files)

| Category | Count | Examples |
|----------|-------|---------|
| PCBoard PPE programs | ~120 | Login, menu, chat, file list, stats PPEs |
| PCBoard utilities | ~40 | Upload processors, DIR tools, converters |
| PCBoard patches | 7 | 15.22→15.23 patches (all node counts) |
| FidoNet tools | ~10 | FidoPCB, PCB-Net, mailers |
| Internet/email tools | ~10 | NetExpress, smail, KSP suite, QWK doors |
| FOSSIL/modem | ~5 | CAPI FOSSIL, DoorWay, modem tools |
| ANSI/art tools | ~5 | TheDraw, converters |
| BBS software (non-PCB) | ~15 | triBBS, Concord, UltraBBS, Sapphire |
| PPL compilers/tools | ~10 | PowerPPL, PPLX decompiler, PPL docs |
| Misc utilities | ~15 | File converters, backup, HTML export |

### Categories in mpoli_offline/ (55 files)

| Category | Count | Examples |
|----------|-------|---------|
| QWK mail readers | ~25 | Blue Wave, 1stReader, OLX, Speed Read |
| SOUP/USENET readers | ~5 | Yarn, WSOMR, NewsWerthy |
| Internet email | ~3 | Pegasus Mail, Wang's OMR |
| QWK utilities | ~10 | Tagline managers, packet mergers, converters |
| Finnish readers | ~5 | ASO, Lucifer, Rontti, SkyReader |
| Misc | ~7 | PGP integration, signature tools |

### Key Files for Backport (sorted by priority)

**CRITICAL (use now):**
- develop9/ — PCBoard file structure docs (already extracted)
- SMAILSRC.ZIP — UUCP mailer with source code
- TDRAW463.ZIP — TheDraw reference for pcbdraw editor

**HIGH (reference for current phases):**
- FP14S.ZIP — FidoPCB reference for pcbbinkp
- CFOS097H.ZIP — CAPI FOSSIL reference for pdserial.c
- HOSTV12A.ZIP — KSP-HOST telnet server reference
- KSPFTP30.ZIP — KSP FTP door reference
- KSPTEL50.ZIP — KSP Telnet door reference
- PCBNET71.ZIP — PCB-Net mail flag updater
- PCBCK325.ZIP — pcbcheck upload processor (compare with pcbpscan)
- CIMRG100.ZIP — Clark's conference merge utility
- FD212.ZIP — FrontDoor 2.12 mailer reference
- PMAIL322.ZIP (in mpoli_offline) — Pegasus Mail, internet email reference

**MEDIUM (future phases):**
- NX-PCB.ZIP — NetExpress SMTP/NNTP gateway
- PWAVE110.ZIP — PCBWAVE Blue Wave/QWK mail door
- QM4_0604.ZIP — QMail 4.00 QWK door
- NS11.ZIP — NetXpress SMTP/NNTP server demo
- KSPSLP37.ZIP — KSP SLIP (TCP/IP over serial for BBS callers)
- YARN_076.ZIP (in mpoli_offline) — PC Yarn USENET reader
- BW23_386.ZIP (in mpoli_offline) — Blue Wave 2.30 reader

**LOW (preservation/historical):**
- PCB15DMO.ZIP — PCBoard v15.0 demo (we have full source)
- 1523_*.ZIP — PCBoard 15.23 patches (7 files, different node counts)
- PPL_DOCS.ZIP — PPLC v1.0 documentation
- All PPE files — PCBoard extensions, useful for testing PPLC compiler

---

## Roy/SAC Contribution (2026-08-11)

Source: https://www.roysac.com/roy-sac_downloads_links.html
Photos: https://drive.google.com/file/d/15v3Y19lkaDXbeI8JLVdsy2tjkJk5Qbp-

Carsten (Roy/SAC) provided physical PCBoard package documentation
and his complete PPE/DOOR collection. License chain: original sysop
→ Roy/SAC via POB (German distributor), upgraded 15.21→15.22, 5→10 nodes.

### Files at reference/roysac/

| File | Size | What |
|------|------|------|
| PCB_box_photos.zip | 87MB | 23 photos of physical PCBoard package |
| PCB1522_D1.ZIP | 1.4MB | PCBoard 15.22 Install Disc 1 |
| PCB1522_D2.ZIP | 1.4MB | PCBoard 15.22 Install Disc 2 |
| PCB1522_D3.ZIP | 924KB | PCBoard 15.22 Install Disc 3 |
| CSBACKUP.ARJ | 22MB | Complete running BBS backup |
| PCB1522-CS2BACKUP-Clean.ZIP | 12MB | Clean BBS backup |
| RoySAC_PPE_Collection.zip | 190MB | 4,548 PPE/DOOR archives |
| PCBSTAT2.ZIP | 645KB | PCBStat v2 statistics generator |
| OS2CAPI.ZIP | 646KB | OS/2 CAPI ISDN driver |
| GLOWSIO.ZIP | 211KB | GlowSIO OS/2 serial driver |

### Key Items

- PCB1522 install discs: 15.22 reference install (pre-15.4)
- GLOWSIO: OS/2 serial driver, reference for evga's SIO work
- OS2CAPI: CAPI/ISDN on OS/2, reference for FOSSIL stack
- PCB_box_photos: physical package provenance documentation
- PPE Collection: 4,548 archives for PPLC compiler testing
- CSBACKUP: complete running BBS snapshot for integration testing

---

## Complete Archive Inventory

### PPE/Archive Comparison

| Source | Total | Unique | Dupes |
|--------|-------|--------|-------|
| Roy/SAC PPE Collection | 4,548 | 3,612 | 936 (overlap with pcbppes) |
| pcbppes.zip (PPL/ppe/) | 2,757 | 1,821 | 936 (overlap with Roy) |
| mpoli.fi BBS | 176 | 176 | 61 removed (were in Roy) |
| mpoli.fi OFFLINE | 55 | 55 | 0 |
| ppe-examples/ | 7 | 7 | 0 |
| ppfilesrcs.zip | 11 | 11 | 0 |
| reference/ root | 21 | 21 | 0 |

**Total unique archives: ~5,703**

### Directory Layout

```
reference/
  *.ZIP (21)              Hand-picked reference files
  develop9/               Extracted PCBoard developer docs
  mpoli_bbs/ (176)        DOS/BBS from mpoli.fi (61 dupes removed)
  mpoli_offline/ (55)     DOS/OFFLINE from mpoli.fi
  roysac/ (10)            Roy/SAC's PCBoard files + photos

PPL/ppe/
  pcbppes.zip (2,757)     PCBoard PPE collection

ppe-examples/ (7)         PPE examples with source
```

### Source Code Found Across All Archives

| File | Source Type | Lines | What |
|------|-----------|-------|------|
| SMAILSRC.ZIP | C source | ~8,500 | UUCP mailer (smail/PC) |
| TDRAW463.ZIP | ASM+PAS | ~2,000 | TheDraw ANSI editor |
| PCB-ATC5.ZIP | ASM | ~500 | QWK file attach |
| E-BLT12.ZIP | PPL source | ~200 | FidoNet email address (FREEWARE) |
| ENAME101.ZIP | PPL source | ~100 | Username→email converter |
| ADRS101.ZIP | PPL source | ~100 | Email address display |
| BAQWK10.ZIP | PPL source | ~100 | QWK mail command |
| QWKBLT12.ZIP | PPL source | ~200 | QWK bulletin |
| PPC1B3AA.ZIP | PPL source | 38 files | PowerPPL compiler library |
| MSGTAG12.ZIP | PPL source | 7 files | Message reader/tagger |
| IEMSI120.ZIP | PPL source | 4 files | IEMSI auto-login |

---

## Roy/SAC Contribution (2026-08-11)

Source: https://www.roysac.com/roy-sac_downloads_links.html
Photos: https://drive.google.com/file/d/15v3Y19lkaDXbeI8JLVdsy2tjkJk5Qbp-/view

Carsten (Roy/SAC) donated his complete PCBoard archive including
physical box photos, install discs, BBS backup, and PPE collection.
License transferred via POB (German distributor), upgraded from
15.21 to 15.22, 5 to 10 nodes.

### Files Downloaded

| File | Size | What |
|------|------|------|
| PCB_box_photos.zip | 87MB | 23 photos of physical PCBoard package |
| PCB1522_D1.ZIP | 1.4MB | Install disc 1 |
| PCB1522_D2.ZIP | 1.4MB | Install disc 2 |
| PCB1522_D3.ZIP | 924KB | Install disc 3 |
| CSBACKUP.ARJ | 22MB | Complete running BBS backup |
| PCB1522-CS2BACKUP-Clean.ZIP | 12MB | Clean BBS backup |
| PCBSTAT2.ZIP | 645KB | PCBStat v2 stats generator |
| OS2CAPI.ZIP | 646KB | OS/2 CAPI ISDN driver |
| GLOWSIO.ZIP | 211KB | GlowSIO OS/2 serial driver |
| RoySAC_PPE_Collection.zip | 190MB | 4,548 PPE/door archives |

Total: 314MB in reference/roysac/

### PPE Collection Comparison

Roy's collection is the authoritative PPE archive. Our pcbppes.zip
contributes 1,821 unique PPEs not in Roy's collection.

| Source | Total | Unique | Dupes |
|--------|-------|--------|-------|
| Roy/SAC PPE Collection | 4,548 | 3,612 | 936 (overlap with pcbppes) |
| pcbppes.zip (our contribution) | 2,757 | 1,821 | 936 (overlap with Roy) |
| mpoli.fi BBS | 176 | 176 | 61 removed (were in Roy's) |
| mpoli.fi OFFLINE | 55 | 55 | 0 |
| ppe-examples/ | 7 | 7 | 0 |
| ppfilesrcs.zip | 11 | 11 | 0 |

**Combined unique: ~5,703 archives**

Our 1,821 unique PPEs from pcbppes.zip are our contribution back
to Roy's collection — filling gaps in the most complete PCBoard
PPE archive that exists.

### Backport Value

| File | Use |
|------|-----|
| PCB1522 install discs | 15.22 baseline reference (pre-15.4) |
| GLOWSIO.ZIP | OS/2 serial driver — reference for evga's SIO work |
| OS2CAPI.ZIP | CAPI/ISDN on OS/2 — reference for FOSSIL stack |
| CSBACKUP.ARJ | Complete BBS snapshot — testing reference |
| PCB_box_photos.zip | Physical package provenance documentation |
| PPE Collection | 4,548 PPEs for PPLC compiler testing |

### Credits

Carsten (Roy/SAC) — PCBoard package donation, photos, PPE collection
License chain: original sysop → Roy/SAC (transferred via POB Germany)
PCBoard 15.22, 10-node license
