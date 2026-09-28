# PCBoard 15.x Binary Catalog

Complete inventory of every binary shipped in a PCBoard install, cross-checked
against available source code.

Method: **Research, Examine, Test, Isolate, Repeat.**

Source archives: `Pcb-main.zip`, `Pcb-misc.zip`, `Pcb-libs.zip`, `Pcb-util.zip`
Password: **`pcb153`** (lowercase — the PWD.txt shows it uppercase, which fails).

---

## A. HAVE SOURCE — build, don't analyse

| Binary | Size | Source location |
|---|---|---|
| PCBOARD.EXE | 1.0 MB | `Pcb-main/153/PCBOARD.MAK` + `Pcb-main/SOURCE/` |
| PCBOARD2.EXE | 670 K | `Pcb-main/153/PCBOARD2.MAK` |
| PCBOARDM.EXE | 984 K | `Pcb-main/153/` (multinode target) |
| PPLC.EXE | 197 K | `Pcb-main/153/PPLC.MAK` + `SOURCE/COMPILER/` |
| PCBSETUP.EXE | 401 K | `Pcb-util/PCBSETUP/PCBSETUP.MAK` |
| PCBSM.EXE | 271 K | `Pcb-util/PCBSM/PCBSM.MAK` |
| PCBFILER.EXE | 311 K | `Pcb-util/PCBFILER/PCBFILER.MAK` |
| PCBEDIT.EXE | 117 K | `Pcb-util/PCBEDIT/MAK/PCBEDIT.MAK` |
| PCBNLC.EXE | 294 K | `Pcb-util/PCBNLC/PCBNLC.MAK` |
| PCBPACK.EXE | 65 K | `Pcb-util/PCBPACK/153/PCBPACK.MAK` |
| PCBDIAG.EXE | 118 K | `Pcb-util/PCBDIAG/PCBDIAG.MAK` |
| PCBMONI.EXE | 33 K | `Pcb-util/PCBMONI/PCBMONI.MAK` |
| PCBSTATS.EXE | 12 K | `Pcb-util/PCBSTATS/PCBSTATS.MAK` |
| MKPCBTXT.EXE | 61 K | `Pcb-util/PCBTEXT/MKPCBTXT.MAK` |
| **PCBMODEM.EXE** | **155 K** | **`Pcb-util/PCBMODEM/PCBMODEM/PCBMODEM.MAK`** |
| MSETUP.EXE | — | `Pcb-util/PCBMODEM/MSETUP/MSETUP.MAK` |
| PCBCP.EXE | — | `Pcb-util/PCBCP/1522/PCBCP.MAK` (OS/2) |
| MAKEIDX.EXE | 76 K | `Pcb-misc/IDX/MAKEIDX.MAK` |
| USERNET.EXE | 12 K | `Pcb-misc/USERNET/USERNET.MAK` |
| FIDOUTIL.EXE | — | `Pcb-misc/FIDOUTIL/FIDOUTIL.MAK` |
| UUIN / UUOUT / UUUTIL / UUXFER | 253/137/137/172 K | `Pcb-misc/UUCP/*/` |
| ZMRECV / ZMSEND | 135/126 K | `Pcb-misc/ZMODEM/ZMODEM.MAK` |
| MAKEHELP.EXE | — | `Pcb-misc/HELP/MAKEHELP.C` |
| WAITFILE.EXE | — | `Pcb-misc/WAITFILE/WAITFILE.MAK` |
| OFFLINE / PCBTITLE / WAITBU | — | `Pcb-util/PCBUTILS/` |
| MD5 | — | `Pcb-misc/MD5/` |

**PCBMODEM correction.** Was on the analysis list. Full C++ source exists,
including `MODEMS.H` documenting MODEMS.DAT exactly. An earlier scaffold
guessed the record layout and got every field wrong. Deleted.

---

## B. THIRD-PARTY — not Clark code

| Binary | Origin | Disposition |
|---|---|---|
| DOORWAY.EXE | **TriMark Engineering**, shareware | Shipped by default, invoked via REMOTE.SYS. Source now available from contact. |
| BC450RTL.DLL, BWCC.DLL | Borland C++ 4.50 redistributables | Needed by PCBMAIL only |
| COMMDRV package | WCSC COMM-DRV v15.0b | Optional add-on, not installed by default. Superseded by `pcbcomm` — see section F |

**COMM-DRV was optional, not default.** INSTALL.DAT line 109:
*"The First Time Installation does not automatically install COMM-DRV, PPL,
or PCBMail. They must be selected manually if desired."* It is install group
`c`, alongside PPL (`d`) and PCBMail (`e`). It shipped in the box on disk 1
as COMMDRV.RED and installs to `COMMDRV\`, but a stock install has no
COMM-DRV at all. So it was never load-bearing — which is exactly why
replacing it with `pcbcomm` costs us nothing in compatibility.

**DOORWAY — confirmed not menu-integrated.** Zero references in Clark's
source. Doors launch generically through DOORS.LST via `executedoor()` →
`spawndos()` in `Pcb-main/SOURCE/MAIN/DOORS.C`; the engine never names
DOORWAY. Its actual hook is **REMOTE.SYS**, the batch file run by the
`9` SysOp command (remote drop to DOS). From BATCH.TXT: *"Included with your
PCBoard package is a shareware copy of DOORWAY by TriMark Engineering and a
REMOTE.SYS that is pre-configured to use DOORWAY on COM1."* So it is a
bundled default the sysop can swap out by editing one batch file.

---

## C. NO SOURCE — binary-analysis targets

| Binary | Size | Documented in | Notes |
|---|---|---|---|
| PCBMAIL.EXE | 333 K | PCBMAIL.HLP (759 K) | Win16 GUI, Borland C++ 4.50 + BWCC |
| PCBIC.EXE / PCBIC2.EXE | 313/217 K | PCBIC.DOC 112 K, PCBIC.PDF 339 K | Internet Component |
| PCBICCFG.EXE | 185 K | PCBIC.DOC | IC configurator |
| PCBICEVT.EXE | 90 K | PCBIC.DOC | IC event scheduler |
| TESTIC / TESTIC2 | 40/47 K | — | IC test harnesses |
| INSTALL.EXE | 331 K | INSTALL.TXT + INSTALL.DAT | NE/OS2, MS C runtime. `.RED` decompressor is the only hard part — 482-member size oracle available, see E2 |
| MKPCBMNU.EXE | 32 K | CUSTBBS.TXT | Menu builder |
| OVLSIZE.EXE | 9 K | UTILITY.TXT | Sets overlay buffer size in an EXE header |
| PCBDESC.EXE | 17 K | UTILITY.TXT | FILE_ID.DIZ extractor |
| RDPCBTXT.EXE | 6.7 K | UTILITY.TXT | PCBTEXT → PCBTEXT.LST |
| TESTFILE.EXE | 2.5 K | UTILITY.TXT, BATCH.TXT | Extension → errorlevel |
| VIEWARCH.COM | 2.2 K | UTILITY.TXT, SETUP.TXT | Archive lister |
| VIEWZIP.EXE | 7.8 K | UTILITY.TXT, SETUP.TXT | ZIP lister |
| ENCRYPT.EXE | — | UTILITY.TXT, SETUP.TXT | PKLITE-packed. **Algorithm already ours** |
| PACKFIDO.EXE | 23 K | — | FidoNet packer, undocumented |
| UPGRADE.EXE | 7.7 K | — | Version upgrade |
| FIXTEXT.EXE | — | — | PCBTEXT language fixer |
| PCBNET.EXE | 40 K | — | 1993 date, predates 15.x. Keep. |
| VIEWFIX.EXE | 5.7 K | — | 2000 date, post-Clark. Recreate for preservation — see below. |

### Specs recovered from UTILITY.TXT

**OVLSIZE** `OVLSIZE [filename] [buffer size]` — PCBoard's main EXE is
overlaid; only part is resident at a time. This sets the overlay buffer size
by writing into the EXE header. A setting of 16 gives a 64 K buffer. Under
DOS/32A flat model it becomes a no-op, but it must still exist and behave
identically.

**PCBDESC** — checks uploads for FILE_ID.DIZ and replaces the user-supplied
description with the author's. Shells to PKXARC / ARJ / LHA / PAK / PKUNZIP
by extension.

**RDPCBTXT** `RDPCBTXT [filename]` — dumps a PCBTEXT file to PCBTEXT.LST as
ASCII, one record per line, so a sysop can find record numbers to edit. The
exact inverse of MKPCBTXT, whose source we have.

**TESTFILE** `TESTFILE [filename] [ext] [ext] ...` — returns a unique
errorlevel identifying which extension matched. Pure string work, no file I/O.
Called from PCBVIEW.BAT and PCBTEST.BAT with `%1`.

**ENCRYPT** `ENCRYPT [location of user file]` — encrypts password, city,
data/business phone, home/voice phone in USERS. PCBoard and System Manager
decrypt transparently on open.

### ENCRYPT downgraded — the algorithm is already ours

`Pcb-libs/SOURCE/MISC/CRYPT.C` (464 lines) exports `encrypt()`/`decrypt()`,
`encrypt2()`/`decrypt2()`, `encrypt3()`/`decrypt3()`, built on `srotl`/`crotl`
/`srotr`/`crotr` rotate primitives. Since PCBoard itself decrypts USERS on
open, the algorithm had to live in the shared library — and it does.
ENCRYPT.EXE is a thin CLI wrapper over code we hold, not a genuine analysis
target. Unpack the PKLITE only to confirm which variant it calls and which
field offsets it walks.

---

## D. Batch-file chain

PCBoard shells out at defined points. From BATCH.TXT:

| Batch file | Trigger | Parameters | Result checked |
|---|---|---|---|
| BOARD.BAT | Startup / errorlevel return | — | — |
| NODE.BAT | Per-node startup | — | — |
| $$LOGON.BAT / $$LOGOFF.BAT | Caller logon/logoff | — | — |
| PCBTEST.BAT | Upload verification | `%1` path, `%2` UPLOAD\|TEST\|ATTACH, `%3` description file | **PCBPASS.TXT** or **PCBFAIL.TXT** |
| PCBVIEW.BAT | `F;V` view-file command | `%1` path | **PCBVIEW.TXT** |
| PCBCMPRS.BAT | Compression | 2 params | target file exists |
| PCBQWK.BAT | QWK packet build | 4 params | — |
| REMOTE.SYS | `9` SysOp remote DOS drop | — | — |

**PCBVIEW lists the contents of an archive.** A caller browsing file
descriptions can hit `F;V` to see what is *inside* a ZIP before spending
download time on it — filenames, sizes, dates. UTILITY.TXT: it *"will list
the files that are stored in ZIP, ARJ, ARC, and PAK compressed files. In
addition, it will view any filename that has a TXT extension to the screen."*
So VIEWARCH/VIEWZIP are directory listers for archives, not extractors.

**PCBVIEW.TXT is the output file, not documentation.**
PCBVIEW.BAT writes the archive listing to PCBVIEW.TXT and PCBoard displays
that to the caller; if it is absent, PCBoard assumes the view failed. The
stock PCBVIEW.BAT uses TESTFILE.EXE to branch on extension, then calls
VIEWARCH/VIEWZIP to produce the listing. PCBTEST.BAT has the same shape but
writes PCBPASS.TXT or PCBFAIL.TXT — and if both exist, PCBFAIL wins.

This chain — PCBVIEW.BAT → TESTFILE → VIEWARCH/VIEWZIP → PCBVIEW.TXT — is one
contained unit. Reproducing TESTFILE, VIEWARCH and VIEWZIP together restores
it whole and gives an end-to-end test: hand it a ZIP, diff the resulting
PCBVIEW.TXT against Clark's.

### Recovered batch files — `Pcb-main/TEST/`

Clark's own development set, in the source tree all along. Paths point at his
build tree (`\proj\pcb\obj\%bccompiler%\`) but the contracts are exact.

**BOARD.BAT** — the outer loop:

```bat
:top
set pcb=%1
set dszlog=pcbdsz.log
if exist remote.bat ren remote.bat remote.sys
if exist door.bat   del door.bat
if exist event.bat  del event.bat
if exist endpcb     del endpcb
pcboard.exe
if exist remote.bat CALL remote
if exist door.bat   CALL door
if exist event.bat  CALL event
if NOT exist endpcb GOTO top
```

**This resolves ENDPCB.** It appeared in the 15.4 file listing with no
extension and no explanation. It is not a binary — it is the exit sentinel.
PCBoard creates it to break the loop; its absence means recycle.

**PCBRZ.BAT** → `zmrecv %3`  **PCBSZ.BAT** → `zmsend %3`
Both call ZMRECV/ZMSEND, which we have source for. `%3` is the filename.

**PCBQWK.BAT** — `%1` = COMPRESS or EXTRACT, `%2` = archive, `%3` = file,
`%4` = list file:

```bat
if %1==COMPRESS pkzip -ex -m %2 @%4
if %1==EXTRACT  pkunzip -o %2 %3
```

**PCBDOS.BAT** — OS/2 VDM wrapper. Sets PCBDRIVE, PCBDIR, PCBDAT, PCBOS2,
PCBHANDLE, DSZLOG, runs `pcbtitle.com`, then `call %1 %2 ... %9`.

The remaining `PCBR*`/`PCBS*` variants (RB, RH, SB, SH — batch/Ymodem,
HS/Link) follow the same one-line shape, calling external drivers such as DSZ
or HS-Link. Since RZ/SZ call binaries we already build, the transfer chain is
substantially in hand.

---

## E. Data formats already documented

**MODEMS.DAT** — `Pcb-util/PCBMODEM/SOURCE/MODEMS.H`

```c
typedef struct {                 /* 16 bytes */
   unsigned Date, NumOfModems;
   long     NextMdmOffset;
   char     Version[8];
} headertype;

typedef struct {                 /* 48 bytes */
   char     Manufs[15], Name[25];
   unsigned StdCfg, Number;
   long     Offset;
} manufdatatype;

typedef struct {                 /* 448 bytes */
   char     V42, Fax, CallID, Seconds, Factory, Eprom, Lock,
            Hspd[15], Lspd[15],
            Send1[40], Send2[40], Send3[40],
            Cmnt1[50], Cmnt2[50], Cmnt3[50],
            Init1[40], Init2[40], Offhook[40],
            Who[15];
   unsigned Date;
   long     Baud;
} modemdatatype;
```

Others in `PCBXDOT/DOCDEV/`: PCBDAT, USERS, USERSYS, PCBSYS, MSGS, CNAMES,
HEADERS, DIR, FILEIDX, DIRIDX, PWRD, CALLERS, PCBSTATS, FIDO.

**Rule: read `DOCDEV/` and the source headers before declaring any struct.**

---

## E2. `.RED` archives — INSTALL.DAT is the complete manifest

Every `.RED` is declared in INSTALL.DAT with member names, **uncompressed
sizes**, and destinations. Eight archives, 482 members:

| Archive | Members | Uncompressed | Contents |
|---|---:|---:|---|
| PCBOARD.RED | 4 | — | PCBOARD.EXE and friends |
| PCBDISK.002 | 202 | 2,943,999 | Utilities, DOC set, FIDO config, batch files |
| PCBDISK.003 | 23 | 1,769,168 | — |
| COMMDRV.RED | 22 | 249,695 | WCSC driver (optional group `c`) |
| PCBMAIL.RED | 4 | 1,477,176 | PCBMAIL.EXE + Borland DLLs + HLP |
| **PCBCFGS.RED** | **171** | **134,350** | **Sample board configurations** |
| PPLC.RED | 46 | 204,737 | PPL compiler + sample PPEs with source |
| PCBOARD2.RED | 10 | 1,272,524 | OS/2 set |

**This is a validation oracle for the decompressor.** 482 members with known
uncompressed sizes. Any candidate LZH implementation can be checked against
every one of them without needing a reference decompressor — if all 482 sizes
come out right, the algorithm is correct.

### PCBCFGS.RED — sample board configurations

Confirmed: it is not program config, it is four ready-made **example boards**,
selectable as install groups:

| Group | Board | Conferences |
|---|---|---|
| `n` | Technical Support Bulletin Board | EMPLOYEE, GEN, PROD1, PROD2 |
| `o` | Sales Bulletin Board | GEN, MAIN |
| `p` | Corporate Bulletin Board | ADMIN, CUSTSRVC, EMPLOYEE, ENGINRNG, GEN, HMNRSRC, MIS, SLSMKTNG |
| `q` | Hobby Bulletin Board | FILES, GEN, GRAPHICS |

Each conference ships CNAMES / CNAMES.ADD / CNAMES.IDX (conference
definitions), BLT + BLT.LST (bulletins), DIR1..DIRn + DIR.LST (file directory
lists), DLPATH.LST (download paths), NEWS, PRIVATE and PUBLIC (message base
seeds), BRDM/BRDS (board menus). The Hobby board additionally ships HISTORY
and RULES.

These are worth preserving as-is. They are the only surviving record of how
Clark themselves laid out a board, and the Corporate set in particular shows
PCBoard being sold as internal company groupware rather than a hobby BBS.

### Correction: the format is uniform, my grep was wrong

Earlier note claimed PCBCFGS.RED had "compressed filenames". It does not.
The name field is a fixed 11 bytes in every archive. PCBCFGS members are
simply named `0`, `1`, … `Z`, `00`, `10`, … `Q3` — positional labels, with
the real destination carried by INSTALL.DAT's `@Out`. `strings -n 5` could
not see one- and two-character names. Verified at offset 26: `30 00 00 00
00 00 00 00 00 00 00` — `"0"` in an 11-byte field.

### A Clark bug to preserve

INSTALL.DAT `@File 91`:

```
@File 91 @Size 142 @Out CUSTSRVC\DIR1   @Group p
```

Double backslash where every sibling line has one. Under bug-for-bug
reproduction this stays — whatever INSTALL.EXE did with that malformed path,
ours does too.

---

## F. pcbcomm — unified serial layer

### What COMM-DRV did, and why pcbcomm replaces it

The DRVSETUP screen columns — Port Number, Card Type, Sub-Port, Base Address,
IRQ, Card Segment, FOSSIL — plus the shipped `.DAT` files (ARNETSP4/8,
DIGI4E/8E) and `.BIN` overlays (BOCA1610, XABIOS, XACOOK, XACOMX) give it away.
COMM-DRV is a **port multiplexer and hardware abstraction layer for multiport
serial cards**.

A 16-node board needs 16 serial ports. The PC BIOS knows about four. Multiport
cards supply the rest, but each vendor lays its registers out differently, and
the *intelligent* boards — Arnet SmartPort Plus, DigiBoard COM/Xi — carry an
onboard CPU and dual-ported RAM and **no UART chips at all**. You cannot drive
those with 16550 register code; you write commands into a shared-memory
structure and let the board's own processor do the work. Hence the **Card
Segment** column: memory-mapped boards, not I/O-mapped.

PCBoard's own documentation names the targets: Arnet SmartPort Plus and
DigiBoard COM/Xi are *"supported by the /M version of PCBoard"* — the multinode
build, which is exactly the configuration that needs many ports.

### Proposed design

`pcbcomm` — one abstraction, dispatched to pluggable backends:

| Backend | Status | Notes |
|---|---|---|
| UART 16550 | have it — `serial.c` | direct register access, FIFO detect |
| FOSSIL INT 14h | have it — `serial.c` | BNU / X00 / ADF |
| Win32 / POSIX tty | have it — `serial.c` | host builds |
| Multiport dumb (Boca 16) | new | banked 16550s, I/O-mapped |
| Multiport intelligent (Digi, Arnet) | new | shared-memory command interface |
| TCP socket | **15.41 only** | telnet / rlogin inbound |

The port table maps node number → backend + parameters. That is precisely
DRVSETUP's screen layout, which is a good sign: Clark's configuration model
already fits the design, so sysop-facing config stays familiar.

### Three deliberate divergences

1. **TSR in 15.4, linked-in from 15.41.** COMMTSR existed because a 1994
   real-mode build could not link everything into one image and still fit in
   conventional memory. Under DOS/32A flat model that constraint is gone and
   the backend can link straight in.

   But sysops running 15.4 expect a TSR. It is how the driver has always
   worked, it is what their CONFIG.SYS and BOARD.BAT are built around, and
   it lets them load the driver once and run several nodes against it. So
   **15.4 ships `pcbcomm` as a loadable TSR** hooking INT 14h — familiar
   shape, familiar config, drop-in for anyone who had COMM-DRV. **15.41
   additionally offers the linked-in build** for sysops who want the memory
   back and the simpler failure modes. Same source, two link targets.

2. **TCP is 15.41-only.** 15.4 must stay byte-compatible and real-mode
   capable. Same source tree, `#ifdef`-gated, so 15.4 compiles with no socket
   code present at all. This falls out naturally from the TSR split above:
   the 15.4 TSR is serial hardware only.

3. **Not bug-for-bug.** This is the one Phase 27 component where exact
   reproduction is explicitly *not* the goal. COMM-DRV is someone else's
   product; we are writing our own driver that does the same job, which is
   clean-room reimplementation and entirely legitimate. What we owe is
   *interface* compatibility — a sysop's existing port configuration should
   carry over — not instruction-level fidelity.

### pcbcomm SDK — Clark already solved the layering

Full plain-language SDK documentation lives in `sdk/README.md`. Summary
follows.

The apparent circularity — *"pcbcomm needs an SDK so we can compile pcbcomm"* —
comes from one name covering two artifacts. Clark's own toolkit separates them
cleanly, and has since February 1994.

`Toolkit3/PCBKIT_S.ZIP` ships `PCBKIT_S.LIB` plus a set of link-time object
modules. Among them:

```
12726  1994-02-15 17:53   COMMDRV.OBJ
11762  1994-02-15 17:53   FOSSIL.OBJ
```

Identical timestamps — added as a pair, eighteen months after the other stubs
(1993-07 / 1993-12). **The serial backend was already a link-time choice.** A
door author links `COMMDRV.OBJ` or `FOSSIL.OBJ` into the same slot; the library
API above it is unchanged either way.

So there are two artifacts, not one:

| Artifact | Role | Clark's equivalent |
|---|---|---|
| `PCBCOMM.EXE` / `.SYS` | the driver — resident TSR in 15.4 | COMMDRV.EXE, COMMTSR.EXE |
| `PCBCOMM.OBJ` | link-time client stub, talks to the driver | COMMDRV.OBJ |
| `PCBCOMM.H` | API header | folded into PCBTOOLS.H |

The SDK does not compile pcbcomm. pcbcomm builds from its own sources. The SDK
merely *consumes* it, by supplying an `.OBJ` that drops into the slot
`COMMDRV.OBJ` and `FOSSIL.OBJ` already occupy. Every existing door relinks
against pcbcomm with a one-line project-file change and no source edits — which
is what "interface compatibility, not instruction fidelity" buys us.

**The link-out idiom is the model to follow.** Appendix D of the 1992 toolkit
docs explains it: a "Hello world" door linked against the full library is 49 K,
so Clark shipped stub objects — `NOCHAT.OBJ`, `NOHELP.OBJ`, `NOSCREEN.OBJ`,
`NOSHELL.OBJ`, `NOTXT.OBJ` and the rest — that satisfy the symbols with empty
bodies. You list the ones you don't need ahead of the `.LIB` and the linker
takes the stubs. pcbcomm's backends work the same way: link only the backend
you use, and the multiport code costs nothing on a single-modem board.

**Memory-model matrix.** Toolkit3 ships S/M/C/L across three compilers
(`PCBKIT_*`, `PCBKBC_*`, `PCBKMS_*`) — twelve library variants. A drop-in
`PCBCOMM.OBJ` needs the same matrix to serve existing 16-bit doors. Our own
OpenWatcom flat-model build is a thirteenth target, not a replacement for them.

**Calling convention.** Toolkit functions use Pascal calling conventions, not C
— chosen for code size, since the callee cleans the stack once rather than the
caller cleaning it at every call site. Names are case-folded to uppercase as a
consequence. `PCBCOMM.OBJ` must match, or nothing links.

### Reference material for the multiport backends

- **Digi ClassicBoard hardware spec** — `ftp1.digi.com/support/utilities/9200282B.doc`.
  Vendor-published register tables. Notably the ClassicBoard implements both
  DigiBoard *and* Arnet interrupt modes: IRQ Status Register A/B for DigiBoard
  and StarGate modes, Register C for Arnet mode, selected via the IRQ Status
  Control Register at offset 03. One backend can therefore cover both vendors.
  UART clock switches 1.8432 → 7.3728 MHz at offset 04, quadrupling the baud
  ceiling to 460.8 K.
- **FreeBSD `digi` driver** — BSD-licensed, covers PC/Xe and PC/Xi, and is
  polling-based rather than interrupt-driven precisely because these boards
  carry more than 1 KB of buffer per port. Good architectural precedent for the
  intelligent-board backend.
- **Linux `epca` / Digi drivers** — GPL, same hardware families.

Vendor documentation plus two independent open-source implementations is ample
to write our own without touching WCSC's code.

### Crew

| Area | Owner |
|---|---|
| pcbcomm serial core, UART + FOSSIL backends | kiddo, wrench |
| Multiport board backends (Digi, Arnet, Boca) | evga |
| PCBDraw (source from sysop/0 + Mystic) | sysop/0 |
| SDK packaging, memory-model matrix, docs | hexadecimal |

### Licensing note

The free download on wcscnet.com is COMM-DRV/**Lib** — the *Windows* library,
and the page states plainly it *"does not include the source code."* Free of
charge, not open source, and not the DOS driver Clark shipped (COMM-DRV/**Dos**,
still a paid product). "It's free" therefore does not give us a licence to copy
it. That does not matter: writing our own against published hardware specs was
the better route anyway, and it is unencumbered.

---

## G. Install layout

Currently flat — everything in `C:\PCB`. Proposal:

```
C:\PCB\              core: PCBOARD, PCBSETUP, PCBSM, PCBFILER, PCBEDIT
C:\PCB\GATEWAY\      UUIN, UUOUT, UUUTIL, UUXFER
C:\PCB\FIDO\         FIDOUTIL, PACKFIDO, PCBNLC
C:\PCB\UTIL\         PCBSTATS, PCBMONI, PCBDIAG, MAKEIDX, USERNET, small tools
C:\PCB\QFRONT\       QFront mailer
```

INSTALL.DAT already does some of this: COMM-DRV installs to `COMMDRV\` via
`@Out COMMDRV\*.*`, so `@Out` is the per-file destination mechanism and
subdirectory layout is a solved problem in the installer.

Remaining constraint is hard-coded paths. PCBOARD.DAT carries file locations,
PPEs carry their own, and batch files reference binaries by relative path.
Any move needs a matching PCBOARD.DAT default change and an upgrade path for
existing installs — mechanical, but it has to be done deliberately rather
than by relocating files and hoping.

---

## H. Revised Phase 27 order

1. **PCBVIEW chain** — TESTFILE, VIEWARCH, VIEWZIP. Small, fully specified in
   UTILITY.TXT, self-testing end to end via PCBVIEW.TXT.
2. **RDPCBTXT** — inverse of MKPCBTXT, whose source we hold.
3. **ENCRYPT** — thin wrapper over `CRYPT.C`, already ours.
4. **Remaining small utilities** — OVLSIZE, PCBDESC, MKPCBMNU, UPGRADE,
   FIXTEXT, PACKFIDO, PCBNET, VIEWFIX.
5. **INSTALL.EXE** — gated on the `.RED` decompressor (LZH-family; evidence
   `make_table`, `raw_in`, `expand_file` in the string table). INSTALL.DAT
   supplies uncompressed sizes for all 482 members across 8 archives, so a
   candidate implementation can be validated exhaustively without a reference
   decompressor. The script engine itself is straightforward by comparison —
   ~40 of the 250+ `@` commands are actually used.
6. **PCBIC suite** — six binaries, 112 K of documentation.
7. **PCBMAIL.EXE** — hardest, best-specified, reference design for pcbnav.

Running in parallel, gated on nothing above: **the unified serial layer**
(section F).

Removed: PCBMODEM (source exists), COMMDRV (superseded by section F).

---

## I. Shipped file list — settled

PCBDISK.002's manifest names every batch file and binary that ships, with
destinations. Batch files: BOARD.BAT, PCBCMPRS.BAT, PCBQWK.BAT, PCBRB.BAT,
PCBRH.BAT, PCBRZ.BAT, PCBSB.BAT. Root binaries: FIDOUTIL, FIXTEXT, INIT,
DOORWAY, ENCRYPT, MAKEIDX, MKPCBMNU, MKPCBTXT, OVLSIZE, PACKFIDO, PCBDESC,
PCBDIAG, PCBEDIT, PCBFILER, PCBMODEM, PCBMONI, PCBNLC, PCBPACK, PCBSETUP.
Plus `DOC\` (the full .DOC set), `FIDO\` (AKAS, AREAS, FREQPATH, MAGICNAM,
NODELIST, ORIGINS, PCBFIDO.CFG, PHONEX, RESPONSE\ Areafix templates), and
`DL01\` sample downloads.

FIXTEXT and INIT are confirmed shipped — both were previously undocumented.

---

## J. Provenance still open

- **APPLYCFG.EXE** — appears to belong to SModem, not PCBoard. Confirm before
  any work.
- **PCBNET.EXE** — 1993 date predates 15.x. Kept in scope regardless.
- **VIEWFIX.EXE** — 2000 date, post-Clark. **Recreate regardless.** It is not
  Clark's, but it is a surviving artifact of the post-Clark community keeping
  PCBoard alive, and 5.7 K is a cheap price for preserving that. Whatever it
  fixed in the PCBVIEW chain is worth understanding even if we end up fixing
  the same thing differently in 15.41.
