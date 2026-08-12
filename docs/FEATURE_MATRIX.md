# PCBoard Feature Matrix — 15.4 Shipped vs Added vs Planned

## Legend

- [DONE] = Done, shipping
- [NEW]  = Built this session, ready
- [PLAN] = Planned, has phase/spec
- [NO]   = Not present, not planned
- [STRUCT] = Clark designed data structures but never finished the code

---

## 1. Core BBS Engine

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| Main BBS engine | [DONE] PCBOARD.EXE | [DONE] PCBOARD_W.EXE | [DONE] | OpenWatcom port |
| Local login | [DONE] LOCAL.EXE | [DONE] LOCAL_W.EXE | [DONE] | |
| PPL 3.40 compiler | [DONE] PPLC.EXE | [DONE] PPLC_W.EXE | [DONE] | |
| Setup utility | [DONE] PCBSETUP.EXE | [DONE] PCBSETUP_W.EXE | [DONE] | |
| System Manager | [DONE] PCBSM.EXE | [DONE] PCBSM_W.EXE | [DONE] | |
| Index builder | [DONE] MAKEIDX.EXE | [DONE] MAKEIDX_W.EXE | [DONE] | |
| User network | [DONE] USERNET.EXE | [DONE] USERNET_W.EXE | [DONE] | |
| Text generator | [DONE] MKPCBTXT.EXE | [DONE] MKPCBTXT_W.EXE | [DONE] | |
| Help builder | [DONE] MAKEHELP.EXE | [DONE] MAKEHELP_W.EXE | [DONE] | |
| DOS target (BC++ 3.1) | [DONE] 11 binaries | [DONE] 11 binaries | [DONE] | |
| OS/2 target (Watcom) | [NO] | [DONE] 28 Clark binaries | [DONE] | New |
| NT target (Watcom) | [NO] | [NEW] some tools | [PLAN] | pcbbinkp, pcbdraw, pcbpscan |
| Linux target | [NO] | [NO] | [PLAN] Phase 6 | |

---

## 2. Message System

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| Message base (read/write) | [DONE] | [DONE] | [DONE] | |
| Conference system | [DONE] | [DONE] | [DONE] | |
| ConfType 0: Normal | [DONE] | [DONE] | [DONE] | |
| ConfType 1: Internet Email | [STRUCT] | [DONE] via UUIN | [PLAN] UUEMAIL | Structs in CNAMES.ADD |
| ConfType 2: Internet/Usenet Junk | [STRUCT] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | Import only, structs ready |
| ConfType 3: Usenet Moderated | [STRUCT] | [PLAN] NNTP tool | [PLAN] NNTP tool | Structs ready |
| ConfType 4: Usenet Public | [STRUCT] | [PLAN] NNTP tool | [PLAN] NNTP tool | Structs ready |
| ConfType 5: Fido Conference | [STRUCT] | [NO] (external HPT) | [PLAN] pcbtoss | Structs ready |
| Echomail flag | [DONE] CNAMES | [DONE] | [DONE] | |
| Message threading | [DONE] | [DONE] | [DONE] | |
| Carbon copy | [DONE] | [DONE] | [DONE] | |
| Message move | [DONE] | [DONE] | [DONE] | |

---

## 3. UUCP / Internet Email

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| UUIN (UUCP import) | [DONE] | [DONE] UUIN_W.EXE | [DONE] | Clark's tool |
| UUOUT (UUCP export) | [DONE] | [DONE] UUOUT_W.EXE | [DONE] | Clark's tool |
| UUXFER (UUCP transfer) | [DONE] | [DONE] UUXFER_W.EXE | [DONE] | Clark's tool |
| UUUTIL (UUCP maintenance) | [DONE] | [DONE] UUUTIL_W.EXE | [DONE] | Clark's tool |
| UUCP transport (dial-up batch) | [DONE] | [DONE] | [DONE] | Legacy, still works |
| UUEMAIL (modern replacement) | [NO] | [PLAN] Phase 11 | [PLAN] Phase 11 | Standalone tool |
| SMTP client (send email) | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | |
| SMTP server (receive email) | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | |
| POP3 client (fetch email) | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | |
| POP3 server (serve email) | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | |
| RFC 822 header parsing | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | smail reference |
| Email aliases/forwarding | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | smail reference |
| Domain routing | [NO] (bang-path) | [PLAN] UUEMAIL | [PLAN] UUEMAIL | |
| MIME support | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | base64, q-p |
| TLS/STARTTLS | [NO] | [PLAN] UUEMAIL | [PLAN] UUEMAIL | If feasible |
| Email address display | [NO] | [PLAN] | [PLAN] | Port ADRS101/ENAME101 to C |
| FidoNet<->Internet gateway | [NO] | [PLAN] | [PLAN] | Port E-BLT12 to C |

Reference: UUPC/extended (UUPC.zip) has UUSMTPD.EXE + UUPOPD.EXE.
Reference: SMAILSRC.ZIP has RFC 822 parser in C source.
Reference: NX-PCB (NetExpress) was the third-party solution.

---

## 4. NNTP / Usenet Newsgroups

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| Newsgroup->conference mapping | [STRUCT] ConfType 2/3/4 | [PLAN] Phase 11 | [PLAN] Phase 11 | Structs ready! |
| NNTP client (fetch articles) | [NO] | [PLAN] Phase 11 | [PLAN] Phase 11 | Standalone tool |
| NNTP server (serve articles) | [NO] | [PLAN] Phase 11 | [PLAN] Phase 11 | |
| Article import to conference | [NO] | [PLAN] Phase 11 | [PLAN] Phase 11 | ConfType 2/3/4 |
| Article export from conference | [NO] | [PLAN] Phase 11 | [PLAN] Phase 11 | Moderated vs public |
| Cross-posting | [NO] | [PLAN] | [PLAN] | |
| Newsgroup subscription | [NO] | [PLAN] | [PLAN] | |
| Active file / newsrc | [NO] | [PLAN] | [PLAN] | |

Works over dialup — NNTP is TCP, same as pcbbinkp.
Clark designed ConfType 2/3/4 for exactly this — he never built the tools.

---

## 5. FidoNet

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| PCBFIDO.CFG (FidoNet config) | [DONE] | [DONE] | [DONE] | |
| ConfType 5 (Fido Conference) | [STRUCT] | [NO] (external HPT) | [PLAN] pcbtoss | |
| FidoNet address in PCBSETUP | [DONE] | [DONE] | [DONE] | |
| EMSI profile | [DONE] | [DONE] | [DONE] | In PCBFIDO.CFG |
| FREQ restrictions | [DONE] | [DONE] | [DONE] | In PCBFIDO.CFG |
| Archiver config | [DONE] | [DONE] | [DONE] | In PCBFIDO.CFG |
| BinkP mailer | [NO] | [NEW] pcbbinkp | [DONE] pcbbinkp | FTS-1026, CRAM-MD5 |
| Echomail tosser | [NO] (external) | [NO] (external HPT) | [PLAN] pcbtoss | |
| Echomail scanner | [NO] (external) | [NO] (external HPT) | [PLAN] pcbtoss | |
| TIC file processor | [NO] (external) | [NO] (external htick) | [PLAN] pcbtic | |
| Nodelist compiler | [NO] (external) | [DONE] PCBNLC | [DONE] PCBNLC | |
| FidoNet configurator | [NO] | [NO] | [PLAN] pcbfcfg | |
| FidoNet console (pcbfido) | [NO] | [NO] | [PLAN] pcbfido | |
| BSO outbound scanner | [NO] | [NEW] bso.c | [DONE] bso.c | In pcbbinkp |

---

## 6. File Transfer & Upload Processing

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| FTP protocol | [NO] | [DONE] built in | [DONE] | In PCBOARD.EXE |
| Upload verification (PCBTEST) | [DONE] (external) | [NEW] pcbpscan | [DONE] pcbpscan | 4 thdproscan bugs fixed |
| ZIP integrity check | [NO] | [NEW] pcbpscan | [DONE] | Central dir verify |
| ARJ/RAR/LHA/7Z check | [NO] | [NEW] pcbpscan | [DONE] | Via external tools |
| FILE_ID.DIZ extraction | [NO] | [NEW] pcbpscan | [DONE] | From ZIP + external |
| Virus scanning hook | [NO] | [NEW] pcbpscan | [DONE] | ClamAV RC=0/1/2 |
| Banned extension check | [NO] | [NEW] pcbpscan | [DONE] | Configurable list |
| DIR file writer | [NO] | [NEW] pcbpscan | [DONE] | PCBoard DIR format |
| Config file | [NO] | [NEW] pcbpscan.cfg | [DONE] | ClamAV preconfigured |

---

## 7. Offline Mail

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| QWK mail packets | [NO] (external) | [PLAN] pcbwave | [PLAN] pcbwave | Phase 12, 15.4 addon |
| Blue Wave packets | [NO] (external) | [PLAN] pcbwave | [PLAN] pcbwave | Phase 12, 15.4 addon |
| SOUP packets | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | Internet email/Usenet |
| OPX packets | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | Phase 12, 15.4 addon |
| Conference selection | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | Caller picks conferences |
| Personal mail filter | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | Messages to/from caller |
| File attaches | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | QWK + Blue Wave |
| FidoNet in QWK packets | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | Echo + netmail |
| FidoNet<->UUCP gateway | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | PWAVE110 feature |
| Door interface (ANSI/ASCII) | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | DOOR.SYS |
| Reply packet import | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | All 4 formats |
| Sysop config utility | [NO] | [PLAN] pcbwave | [PLAN] pcbwave | |

Reference: PWAVE110.ZIP (feature target), MultiMail (GPL, format reference).

---

## 8. ANSI Art / Drawing

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| ANSI viewer | [DONE] (built-in) | [DONE] | [DONE] | |
| pcbdraw viewer/editor | [NO] | [NEW] viewer | [PLAN] full editor | Ships on both versions |
| PCBoard @X code support | [DONE] | [NEW] pcbdraw | [DONE] | Load + save |
| SAUCE metadata | [NO] | [NEW] pcbdraw | [DONE] | Read + write |
| Binary format | [NO] | [NEW] pcbdraw | [DONE] | .BIN char+attr pairs |
| Teleconference drawing | [NO] | [NEW] pdnet.c (protocol) | [PLAN] Phase 5b | TCP server+client done |
| Serial/modem drawing | [NO] | [PLAN] Phase 10 | [PLAN] Phase 10 | pdserial.c |
| ANSI animation | [NO] | [PLAN] Phase 5e | [PLAN] Phase 5e | DuNoDraw-style |
| Door mode (caller draws) | [NO] | [PLAN] Phase 5d | [PLAN] Phase 5d | |
| Who's online / last callers | [NO] | [NO] | [PLAN] Phase 5f | PCBoard node integration |
| File area save | [NO] | [NO] | [PLAN] Phase 5f | Save ANSI to file base |
| Font editor | [NO] | [PLAN] Phase 5a | [PLAN] Phase 5a | From CIADraw |
| Palette editor | [NO] | [PLAN] Phase 5a | [PLAN] Phase 5a | From CIADraw |

---

## 9. Serial / Hardware

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| SIO OS/2 serial driver | [NO] | [DONE] v1+v2 (31 bugs) | [DONE] | evga |
| FOSSIL socket layer | [NO] | [DONE] 4 platforms | [DONE] | wrench |
| Cyclades CD1400 driver | [NO] | [PLAN] Phase 9a | [PLAN] Phase 9a | evga, in progress |
| DigiBoard support | [NO] | [PLAN] Phase 9b | [PLAN] Phase 9b | evga |
| Modem control (AT commands) | [DONE] (built-in) | [DONE] | [DONE] | |
| pdserial.c (pcbdraw serial) | [NO] | [PLAN] Phase 10 | [PLAN] Phase 10 | modem/FOSSIL |
| VMODEM (virtual modem) | [NO] | [DONE] evga | [DONE] | In SIO package |

---

## 10. OS/2 Integration

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| OS/2 Control Panel | [NO] | [DONE] PCBCP_W.EXE | [DONE] | PM app |
| OS/2 native compile | [NO] | [DONE] all 28 Clark binaries | [DONE] | |
| System tray | [NO] | [NO] | [PLAN] utrayit | |

---

## 11. Installer / Operations

| Feature | Clark 15.4 | Our 15.4 | Our 15.41 | Notes |
|---------|-----------|----------|-----------|-------|
| Installer TUI | [NO] | [DONE] pcbis_ui | [DONE] | |
| FidoNet ops console | [NO] | [NO] | [PLAN] pcbfido | |
| Startup/shutdown scripts | [NO] | [DONE] | [DONE] | OS/2 + Linux |
| 15.3->15.4 upgrade | [NO] | [NO] | [PLAN] upd1541 | |

---

## 12. Documentation

| Document | Clark 15.4 | Our 15.4 | Notes |
|----------|-----------|----------|-------|
| Developer docs (file formats) | [NO] (internal) | [DONE] DEVELOP9.ZIP | Extracted |
| Build guide | [NO] | [DONE] PCB154_BUILD_GUIDE.md | |
| Setup guide | [NO] | [DONE] SETUP_GUIDE.md | |
| FidoNet guide | [NO] | [DONE] FIDONET.md | |
| Spec doc | [NO] | [DONE] PCB1541_DRAFT.md (2,543 lines) | |
| WHATSNEW | [NO] | [DONE] WHATSNEW.md | |
| PCBDraw phase plan | [NO] | [DONE] PCBDRAW_PHASE5.md | |
| Feature matrix | [NO] | [DONE] FEATURE_MATRIX.md | This file |
| Reference catalog | [NO] | [DONE] REFERENCE_CATALOG.md | 313 archives |
| evga driver phases | [NO] | [DONE] PHASE9_EVGA_DRIVERS.md | |
| Offline mail phase | [NO] | [DONE] PHASE12_PCBWAVE.md | |

---

## The Big Discovery

Clark already designed ConfType 1-5 in CNAMES.ADD:

    ConfType 0 = Normal PCBoard Conference
    ConfType 1 = Internet Email (Import & Export)
    ConfType 2 = Internet/Usenet Junk (Import Only)
    ConfType 3 = Usenet Moderated Newsgroup
    ConfType 4 = Usenet Public Newsgroup
    ConfType 5 = Fido Conference

The data structures are IN THE SHIPPED CODE. He just never built
the transport tools for types 2/3/4/5. No data structure upgrade
needed — we just build UUEMAIL (SMTP/POP3), an NNTP tool, pcbtoss,
and pcbwave to use what Clark already designed.

---

## Phase List (updated)

| Phase | What | Version | Status |
|-------|------|---------|--------|
| 1 | 15.4 Source Port (OpenWatcom, 556 files) | 15.4 | [DONE] |
| 2 | Core Features -- FTP, BinkP | 15.4 | [DONE] |
| 3 | OS/2 Native (PCBCP) | 15.4 | [DONE] |
| 4 | Standalone Tools -- pcbbinkp, pcbpscan, PCBNLC | 15.4 | [DONE] |
| 4a | FOSSIL Drivers (wrench, 4 platforms) | 15.4 | [DONE] |
| 5 | PCBDraw (6 sub-phases) | 15.4 + 15.41 | IN PROGRESS |
| 5a | -- Editor core (CIADraw port, PCB UI libs) | 15.4 + 15.41 | TODO |
| 5b | -- Network integration (teleconference) | 15.4 + 15.41 | TODO |
| 5c | -- Serial/FOSSIL/modem + DigiBoard | 15.4 + 15.41 | TODO |
| 5d | -- Door mode (BBS integration) | 15.4 + 15.41 | TODO |
| 5e | -- ANSI animation (DuNoDraw-style) | 15.4 + 15.41 | TODO |
| 5f | -- PCBoard integration (nodes, users, files) | 15.41 | TODO |
| 6 | Linux Native | 15.4 | NOT STARTED |
| 7 | Multi-Platform | -- | HOLD |
| 8 | Community (Roy/SAC) | -- | WAITING |
| 9 | Hardware Drivers & Serial Stack (evga) | 15.4 + 15.41 | IN PROGRESS |
| 9a | -- Cyclades CD1400 driver | 15.4 + 15.41 | IN PROGRESS |
| 9b | -- DigiBoard SDK integration | 15.4 + 15.41 | TODO |
| 10 | pdserial.c -- Serial transport for pcbdraw | 15.4 + 15.41 | TODO |
| 11 | Email/News -- UUEMAIL + NNTP | 15.4 + 15.41 | TODO |
| 12 | Offline Mail -- pcbwave (PWAVE110 replacement) | 15.4 addon | TODO |

---

## Binary Count: 33 programs

### Clark Binaries — Main (Watcom DOS4G port, bin/watcom/)

| # | Binary | Size | Description |
|---|--------|------|-------------|
| 1 | PCBOARD_W.EXE | 1.3M | Main BBS engine |
| 2 | LOCAL_W.EXE | 1.3M | Local login mode |
| 3 | PCBSETUP_W.EXE | 424K | System configuration |
| 4 | PCBSM_W.EXE | 221K | System manager |
| 5 | PPLC_W.EXE | 1.3M | PPL compiler |
| 6 | MKPCBTXT_W.EXE | 86K | Text file builder |
| 7 | PCBTEXT_W.EXE | 39K | Text file editor |
| 8 | UUIN_W.EXE | 1.4M | UUCP inbound |
| 9 | UUOUT_W.EXE | 1.4M | UUCP outbound |
| 10 | UUUTIL_W.EXE | 1.4M | UUCP utilities |
| 11 | UUXFER_W.EXE | 1.4M | UUCP transfer |
| 12 | USERNET_W.EXE | 28K | User network flags |
| 13 | PCBCP_W.EXE | 77K | OS/2 PM control panel |
| 14 | MAKEHELP_W.EXE | 25K | Help file builder |
| 15 | MAKEIDX_W.EXE | 37K | Index file builder |
| 16 | PCBIS_W.EXE | 48K | Installer/config TUI |

### Clark Utilities — Phase 0 (hexadecimal + sysop/0)

| # | Binary | Size | Description |
|---|--------|------|-------------|
| 17 | PCBSTATS_W.EXE | 31K | Statistics generator |
| 18 | PCBPACK_W.EXE | 84K | Message base packer |
| 19 | MSETUP_W.EXE | 108K | Modem database editor |
| 20 | PCBMODEM_W.EXE | 528K | Modem config utility |
| 21 | PCBEDIT_W.EXE | 133K | Text/config editor |
| 22 | PCBMONI_W.EXE | 54K | Node monitor |
| 23 | PCBDIAG_W.EXE | 552K | Diagnostics utility |
| 24 | PCBFILER_W.EXE | 215K | File management utility |
| 25 | PCBNLC_W.EXE | 77K | Nodelist compiler (FidoNet) |
| 26 | OFFLINE_W.EXE | 26K | Offline flag utility |
| 27 | WAITBU_W.EXE | 25K | Wait for backup |
| 28 | PCBTITLE_W.EXE | 15K | OS/2 console title setter |

### New Tools (sysop/0)

| # | Binary | Lines | Description |
|---|--------|-------|-------------|
| 29 | pcbbinkp | 2,008 | BinkP/1.1 FidoNet mailer |
| 30 | pcbdraw | 1,826 | ANSI art viewer/editor |
| 31 | pcbpscan | 770 | File scanner |
| 32 | pcbfido | 778 | FidoNet console (15.41) |

### Internet Services (sysop/0, Pascal)

| # | Binary | Lines | Description |
|---|--------|-------|-------------|
| 33 | pcbis.exe | 5,710 | Internet services daemon (18 units) |

### Drivers

| Component | Lead | Status |
|-----------|------|--------|
| SIO Driver | evga | 31 bugs fixed |
| FOSSIL Driver | sysop/0 + kiddo | platform/fossil/ |

## Gap Summary

### Already closed:

| Gap | How |
|-----|-----|
| No BinkP mailer | pcbbinkp (2,008 lines, 2 binaries) |
| No file scanner | pcbpscan (770 lines, ClamAV hook) |
| No ANSI parser/viewer | pcbdraw (1,826 lines, 2 binaries) |
| No FidoNet console | pcbfido (778 lines, 15.41) |
| No nodelist compiler | PCBNLC (Clark) |
| SIO driver bugs | 31 bugs fixed, 3 audits |
| No FOSSIL layer | wrench, 4 platforms |
| No developer docs | DEVELOP9.ZIP extracted |
| No reference archive | 313 files from mpoli.fi |

### HIGH PRIORITY gaps:

| Gap | Phase | Version | What's needed |
|-----|-------|---------|--------------|
| No SMTP/POP3 email | 11 | 15.4 + 15.41 | UUEMAIL standalone tool |
| No NNTP newsgroups | 11 | 15.4 + 15.41 | NNTP tool, uses ConfType 2/3/4 |
| No offline mail | 12 | 15.4 addon | pcbwave (QWK/BW/SOUP/OPX) |
| No echomail tosser | 15.41 | 15.41 | pcbtoss |
| No pcbdraw editor | 5a | 15.4 + 15.41 | pdeditor.c (CIADraw port, PCB UI) |
| No serial transport | 10 | 15.4 + 15.41 | pdserial.c (modem/FOSSIL) |
| DOSBox test | -- | 15.4 | Relink PCBOARD_W.EXE as dos4g |

### MEDIUM PRIORITY gaps:

| Gap | Phase | Version | What's needed |
|-----|-------|---------|--------------|
| No ANSI animation | 5e | 15.4 + 15.41 | pdanim.c (DuNoDraw-style) |
| No pcbdraw door mode | 5d | 15.4 + 15.41 | pddoor.c |
| No pcbdraw PCB integration | 5f | 15.41 | Nodes, users, file save |
| No TIC processor | 15.41 | 15.41 | pcbtic |
| No DigiBoard | 9b | 15.4 + 15.41 | evga |
| Cyclades CD1400 | 9a | 15.4 + 15.41 | evga (in progress) |
| Email PPE->C ports | 11 | 15.4 | ADRS101, ENAME101, E-BLT12 |
| Linux native | 6 | 15.4 | Full recompile |

### LOW PRIORITY gaps:

| Gap | Phase | Version | What's needed |
|-----|-------|---------|--------------|
| No RIPscrip in pcbdraw | Future | 15.41 | Phase 5 sub-phase |
| USB-serial support | 9 | 15.4 + 15.41 | evga SIO enhancement |
| VMODEM enhancements | 9 | 15.4 + 15.41 | SSH tunnel, telnet nego |
| 1541 branch on GitHub | -- | -- | Create from web UI |
| Multi-platform builds | 7 | -- | HOLD |

---

## Addendum: Additional Phases

### Phase 13: CD-ROM Offline Reader

Standalone tool for browsing PCBoard message bases and file
listings distributed on CD-ROM. Sysops ship monthly/quarterly
CDs with full conference archives and file catalogs.

Features:
- [ ] Read PCBoard message base format from CD (read-only)
- [ ] Browse conferences, search messages, read threads
- [ ] File listing browser with descriptions
- [ ] FILE_ID.DIZ display
- [ ] ANSI viewer for .ANS files on CD
- [ ] Index builder for fast search across CD content
- [ ] Print messages
- [ ] Export messages to QWK/Blue Wave for reply (ties into pcbwave Phase 12)
- [ ] PCB-ATC5 CHANGE.ASM QWK file attach reference for CD->QWK export

Ships as 15.4 addon.

### Updated Phase List

| Phase | What | Version | Status |
|-------|------|---------|--------|
| 1 | 15.4 Source Port (OpenWatcom, 556 files) | 15.4 | [DONE] |
| 2 | Core Features -- FTP, BinkP | 15.4 | [DONE] |
| 3 | OS/2 Native (PCBCP) | 15.4 | [DONE] |
| 4 | Standalone Tools -- pcbbinkp, pcbpscan, PCBNLC | 15.4 | [DONE] |
| 4a | FOSSIL Drivers (wrench, 4 platforms) | 15.4 | [DONE] |
| 5 | PCBDraw (6 sub-phases) | 15.4 + 15.41 | IN PROGRESS |
| 6 | Linux Native | 15.4 | NOT STARTED |
| 7 | Multi-Platform | -- | HOLD |
| 8 | Community (Roy/SAC) | -- | WAITING |
| 9 | Hardware Drivers & Serial Stack (evga) | 15.4 + 15.41 | IN PROGRESS |
| 10 | pdserial.c -- Serial transport | 15.4 + 15.41 | TODO |
| 11 | Email/News -- UUEMAIL + NNTP | 15.4 + 15.41 | TODO |
| 12 | Offline Mail -- pcbwave (QWK/BW/SOUP/OPX) | 15.4 addon | TODO |
| 13 | CD-ROM Offline Reader | 15.4 addon | TODO |

### Phase 12 Update: PCB-ATC5 Reference

PCB-ATC5.ZIP (CHANGE.ASM) added as direct reference for pcbwave
file attach handling. The ASM source shows how QWK .REP packets
handle binary file attachments — needed for pcbwave's file attach
import/export in QWK and Blue Wave formats.

### Phase 13: CD-ROM Offline Reader/Door (pcbcdrom)

Clean-room CD-ROM file browser door for PCBoard. No existing
CD-ROM door had source released. Reference tools (all binary-only):
- MegaByte Manager — CD-ROM file manager
- LaserBoard — CD-ROM BBS door
- SF-ROM v2.10f — CD-ROM door (Spitfire, generic)

Checklist:
- [ ] Read PCBoard DIR/DIRIDX/FILEIDX format from CD-ROM (read-only)
- [ ] Browse file listings with descriptions and FILE_ID.DIZ
- [ ] Search across CD content (filename, description, date)
- [ ] Download files from CD via transfer protocol (Zmodem etc.)
- [ ] Read PCBoard message base from CD (read-only)
- [ ] Browse conferences, read messages, search threads
- [ ] ANSI viewer for .ANS files on CD
- [ ] Print messages / file listings
- [ ] Export messages to QWK/Blue Wave for reply (ties into pcbwave)
- [ ] Multi-CD support (index across multiple discs)
- [ ] Door interface (DOOR.SYS, ANSI/ASCII menus)
- [ ] ISO 9660 / Joliet CD filesystem reading
- [ ] Sysop config: CD drive letter, index path, access levels

Ships as 15.4 addon.

### Final Phase List

| Phase | What | Version | Status |
|-------|------|---------|--------|
| 1 | 15.4 Source Port (OpenWatcom, 556 files) | 15.4 | [DONE] |
| 2 | Core Features -- FTP, BinkP | 15.4 | [DONE] |
| 3 | OS/2 Native (PCBCP) | 15.4 | [DONE] |
| 4 | Standalone Tools -- pcbbinkp, pcbpscan, PCBNLC | 15.4 | [DONE] |
| 4a | FOSSIL Drivers (wrench, 4 platforms) | 15.4 | [DONE] |
| 5 | PCBDraw (6 sub-phases) | 15.4 + 15.41 | IN PROGRESS |
| 6 | Linux Native | 15.4 | NOT STARTED |
| 7 | Multi-Platform | -- | HOLD |
| 8 | Community (Roy/SAC) | -- | WAITING |
| 9 | Hardware Drivers & Serial Stack (evga) | 15.4 + 15.41 | IN PROGRESS |
| 10 | pdserial.c -- Serial transport | 15.4 + 15.41 | TODO |
| 11 | Email/News -- UUEMAIL + NNTP | 15.4 + 15.41 | TODO |
| 12 | Offline Mail -- pcbwave (QWK/BW/SOUP/OPX) | 15.4 addon | TODO |
| 13 | CD-ROM Reader/Door -- pcbcdrom | 15.4 addon | TODO |
