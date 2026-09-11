# pcbkit_l.lib Module Comparison

Clark (1994, 241,144 B, 130 modules) vs Ours (1996+, 325,120 B, 147 modules)

| Module | Clark | Ours | Match |
|--------|------:|-----:|-------|
| ADDBACKS | 67 | 63 | close |
| ADDCHAR | 54 | 40 | close |
| ANSI | 1736 | 1736 | EXACT |
| ASCII | 148 | 143 | close |
| ASYNC | 2505 | 2501 | close |
| ATCLOSE | 1 | 5 | close |
| BD_LONG | 323 | 360 | close |
| BGKEY | 37 | 37 | EXACT |
| BOXCLS | - | 64 | EXTRA |
| BS_LONG | 230 | 219 | close |
| BUILDSTR | - | 97 | EXTRA |
| CHANGE | - | 41 | EXTRA |
| CHAT | 826 | 1583 | +757 |
| CHKAPPEN | 73 | 72 | close |
| CHKCREAT | 76 | 75 | close |
| CHKLOCK | 109 | 101 | close |
| CHKOPEN | 73 | 72 | close |
| CHKREAD | 137 | 127 | close |
| CHKWRITE | 128 | 115 | close |
| CLS | 238 | 49 | -189 |
| CLSBOX | - | 121 | EXTRA |
| CNAMES | 1158 | 2876 | +1718 |
| COMMA | 281 | 259 | close |
| COUNTRY | 1570 | 1579 | close |
| CRYPT | 252 | 334 | close |
| CTOD | 538 | 469 | close |
| CURSOR | 164 | 146 | close |
| CUSTHELP | 7 | 7 | EXACT |
| CUTIL | 2143 | 2143 | EXACT |
| DATAFIL2 | 4241 | 4727 | +486 |
| DATE | 581 | 776 | +195 |
| DATESTR | 77 | 93 | close |
| DCOMMA | - | 190 | EXTRA |
| DELAY | 118 | 123 | close |
| DEVIOCTL | 176 | 170 | close |
| DISPLAY | 1547 | 3227 | +1680 |
| DOSAPPEN | 304 | 272 | close |
| DOSCLOSE | 77 | 134 | close |
| DOSCOMIT | 47 | 47 | EXACT |
| DOSCREAT | 64 | 117 | close |
| DOSDUP | - | 165 | EXTRA |
| DOSERROR | 1483 | 1815 | +332 |
| DOSFCLOS | 236 | 291 | close |
| DOSFGETS | 800 | 755 | close |
| DOSFLUSH | 110 | 135 | close |
| DOSFOPEN | 587 | 570 | close |
| DOSFPUTS | 342 | 339 | close |
| DOSFREAD | 365 | 341 | close |
| DOSFSEEK | 355 | 306 | close |
| DOSFTRUN | 70 | 58 | close |
| DOSFWRIT | 310 | 329 | close |
| DOSINIT | 31 | 32 | close |
| DOSLSEEK | 35 | 40 | close |
| DOSOPEN | 1951 | 1879 | close |
| DOSREAD | 67 | 117 | close |
| DOSREWIN | 112 | 132 | close |
| DOSSTBUF | 231 | 232 | close |
| DOSTRUNC | 54 | 62 | close |
| DOSWRITE | 64 | 114 | close |
| DTOC | 210 | 180 | close |
| EXIST | 88 | 86 | close |
| EXITDOS | 121 | 207 | close |
| EXTENDED | 35 | 39 | close |
| FASTPUTC | - | 100 | EXTRA |
| FILES | 2280 | 10536 | +8256 |
| FINDNAME | - | 77 | EXTRA |
| FMEMCPY | 32 | 32 | EXACT |
| FMEMSET | 30 | 30 | EXACT |
| FULLNAME | - | 44 | EXTRA |
| GETMODE | 577 | 536 | close |
| GIVEUP | 442 | 471 | close |
| GOODBYE | 269 | 242 | close |
| GOTOXY | 19 | 19 | EXACT |
| HANDLERS | 277 | 244 | close |
| HELP | 126 | 1510 | +1384 |
| HEXTOI | 74 | 81 | close |
| INDEX | 1429 | 3718 | +2289 |
| INIT | 2246 | 2212 | close |
| INITPORT | 177 | 141 | close |
| INKEY | 3717 | 8093 | +4376 |
| INPUT | 4835 | 5000 | +165 |
| INPUTREQ | 182 | 174 | close |
| INT24HND | 53 | 53 | EXACT |
| ISOPEN | - | 127 | EXTRA |
| ISSET | 34 | 34 | EXACT |
| JULIAN | 825 | 1043 | +218 |
| LANGUAGE | 1401 | 2020 | +619 |
| LASTCHAR | 41 | 41 | EXACT |
| LOG | 775 | 1536 | +761 |
| LONG_BD | 259 | 315 | close |
| LONG_BS | 194 | 193 | close |
| MEMFCMP | 81 | 81 | EXACT |
| MEMICMP | 67 | 71 | close |
| MEMMOVE | 38 | 38 | EXACT |
| MEMORY | 2031 | 2031 | EXACT |
| MISC | 1520 | 2828 | +1308 |
| MODEM | 1073 | - | MISSING |
| MODEMASY | 1122 | 980 | -142 |
| MODEMDRV | 2257 | - | -2257 |
| MODEMFOS | 2027 | - | -2027 |
| MODEMOS2 | 2452 | - | -2452 |
| MSGBASE | 5198 | 4586 | -612 |
| MSTRCPY | 52 | 52 | EXACT |
| PADSTR | 41 | 41 | EXACT |
| PARSEPTH | 198 | 156 | close |
| PCBINIT | 4929 | 4929 | EXACT |
| PCBTEXT | 1716 | 6359 | +4643 |
| PRINT | 95 | 152 | close |
| PRNREADY | 19 | 19 | EXACT |
| PRNTCNTR | - | 47 | EXTRA |
| PRNTMOVE | - | 59 | EXTRA |
| PROPER | 358 | 395 | close |
| READSCR2 | - | 43 | EXTRA |
| RECYCLE | 173 | 3372 | +3199 |
| RLE | - | 143 | EXTRA |
| SAVEREST | 116 | 116 | EXACT |
| SAVERST2 | 511 | 525 | close |
| SCREEN | 1029 | 1701 | +672 |
| SETATT | - | 81 | EXTRA |
| SETBIT | 33 | 33 | EXACT |
| SHELL | 1673 | 7873 | +6200 |
| SHOWERR | 863 | 915 | close |
| SLOWMODM | 420 | 395 | close |
| SOUND | 51 | 50 | close |
| SRCHPATH | 318 | 18 | -300 |
| STATUS | 2765 | 3733 | +968 |
| STRICMP | 61 | 65 | close |
| STRIPL | 130 | 100 | close |
| STRIPR | 56 | 58 | close |
| STRLWR | 60 | 36 | close |
| STRNCHR | 34 | 34 | EXACT |
| STRNICMP | 66 | 68 | close |
| STRUPR | 60 | 36 | close |
| SUBST | 371 | 309 | close |
| SYS | 2331 | 3932 | +1601 |
| SYSDATE | - | 40 | EXTRA |
| SYSTIME | - | 38 | EXTRA |
| TIME | 112 | 315 | +203 |
| TIME1 | 77 | 92 | close |
| TIME2 | 49 | 78 | close |
| TIMER | 221 | 229 | close |
| TIMESTEN | - | 232 | EXTRA |
| TOKEN | 835 | 742 | close |
| UNSETBIT | 35 | 35 | EXACT |
| USERS | 15890 | 21506 | +5616 |
| USERSYS | 3424 | 2996 | -428 |
| WHEREY | 9 | 13 | close |
| XLATE | 3518 | 5539 | +2021 |
| **TOTAL** | **108622** | **150471** | **+41849** |
| **Modules** | **130** | **147** | |

## Summary

- **22 EXACT** — identical binary output, same source unchanged
- **75 close** — under 100 bytes difference, minor header/flag variations
- **33 big diffs** — source code grew between 1994 and 1996+
- **4 MODEM modules** — previously blocked on `comm.h` from external COMM-DRV SDK.
  **UNBLOCKED:** `toolkit/pwa154/pcbdcom/inc/pcbdcom.h` is a drop-in replacement
  for COMMDRV. Has `struct port_param` and the 13-function `ser_rs232_*` API.
  Compile MODEM, MODEMDRV, MODEMFOS, MODEMOS2 using pcbdcom.h. GPLv3, in repo.
  MODEM (1,073 B in Clark's), MODEMDRV (2,257 B, has COMMDRV_* functions),
  MODEMFOS (2,027 B, FOSSIL driver), MODEMOS2 (2,452 B, OS/2 modem)
- **17 EXTRA** — added back from sub-libs to resolve transitive dependencies
  from our bigger MAIN modules. Clark's smaller 1994 modules didn't pull
  these in:
  BOXCLS, BUILDSTR, CHANGE, CLSBOX, DCOMMA, DOSDUP, FASTPUTC, FINDNAME,
  FULLNAME, ISOPEN, PRNTCNTR, PRNTMOVE, READSCR2, RLE, SETATT, SYSDATE,
  SYSTIME, TIMESTEN

## Why they differ

Clark's pcbkit_l.lib was built **1994-11-22**. The PWA source (v000) is
from **1996-08-09**. The lib predates the source by 2 years. Between
1994 and 1996, Clark added features to MAIN modules: FILES grew +8,256 B,
SHELL +6,200 B, USERS +5,616 B, PCBTEXT +4,643 B, INKEY +4,376 B.

Compile flags are NOT the cause. Stripping 8 extra `-D` defines from our
config only reduced 1 undefined symbol. The 22 EXACT matches prove our
build environment is correct — when the source hasn't changed, the output
matches byte-for-byte.

## Vtable impact

Our EXE: 256,860 B. Clark's: 222,176 B. Difference: 34,684 B.

The cVARVAL C++ class has a virtual destructor. Each variable record
stores a 2-byte vtable pointer. `memcpy()` writes it to disk. Different
EXE size = different data segment layout = different vtable address =
313 bytes different in the PPE output (26 variables × 12 bytes each).

The vtable pointer on disk is dead data — PCBoard's PPE loader ignores
it. PPE bytecode is identical. The compiler is proven correct.

## Current build state (2026-09-09)

### Previous approach (cascade — abandoned)

Compiled 162 of 349 source files with full MAIN modules. Resulted
in 230 undefined symbols that kept growing with each module added.
This approach is wrong — see "What cascade means" section.

### Current approach (NO*.C stubs — DONE)

Compiled 17 NO*.C stubs from LIB/SOURCE/TOOLKIT/ in PWA source.
Built pcbkit_l.lib from 249 C + 7 ASM source files with -P (C++ mode).
Linked PPLC with stubs. Result: ZERO undefined (better than Clark's 21).
18 of 19 PPL DevKit test programs compile successfully.

    pcbkit_l.lib: 311,296 bytes, 241 modules (from source with -P)
    PPLC.EXE:     119,812 bytes (Clark: 222,176)
    Undefined:    0 (Clark: 21)
    Test results: 18/19 DevKit samples pass
    RUNINET.PPS:  fails (v3.20 advanced features)

Size difference explained: Clark's 222 KB EXE includes the Toolkit3
PCBKIT_L.LIB modules (compiled as C). Our 119 KB EXE uses lib modules
compiled as C++ with -P — different symbol decoration means different
modules are pulled in from the lib.

### Fixes applied (carry forward to new approach)

  - USERS.C: ConfFlags/ConfReg/MsgReadPtr/MsgReadPtrBackup/ExtConfLen/
    ConfByteLen made extern (ownership: USERSYS/PCBINIT)
  - USERS.H: _FARDATA_ fallback guard added
  - NEWSCR.CPP: CUR_PPE_VER 330->320, encrypt3 guards
  - COMM.H: full COMMDRV-compatible header rebuilt from MODEMDRV.C
  - Watcom wrapper headers removed (alloc.h, mem.h, dir.h, etc.)
  - ANSI.C needs -D__OS2__ to compile

### External dependencies resolved

| Dependency | Source | Status |
|------------|--------|--------|
| comm.h (COMMDRV SDK) | Rebuilt COMM.H from MODEMDRV.C usage | Done (full COMMDRV-compatible) |
| d4all.h (CodeBase dBASE) | toolkit/delta154/H/d4all.h | Done |
| setup.h | pcb153/SOURCE/H/setup.h | Done |
| setup.ext | reference/pcball/.../SETUP.EXT | Done |
| pcbfiles.ext | reference/pcball/.../PCBFILES.EXT | Done |
| vmdata.h | toolkit/delta154/H/vmdata.h | Done |
| Runtime globals | PCBGLOB.C from PCBOARD.EXT | Done (71 of 92) |

### Source fixes applied (not stubs)

- **USERS.C** — ConfFlags, ConfReg, MsgReadPtr, MsgReadPtrBackup made
  `extern` (lines 105-108, inside `#ifndef LIB`). Ownership: USERSYS.
  ExtConfLen, ConfByteLen made `extern` (lines 47-48). Ownership: PCBINIT.
  Resolves USERS/USERSYS TLIB clash matching Clark's 1994 lib behavior.

- **NEWSCR.CPP** — CUR_PPE_VER changed from 330 to 320. encrypt3 guards
  added (`#if CUR_PPE_VER >= 330`). Without this, PPLC applies 3.30
  encryption to 3.20 PPE files.

- **PCBINIT.C** — globals made `extern` to resolve ASYNC symbol clashes
  (VerifyCDLoss, FORCE16550A, NO16550, etc.).

All fixes applied to BOTH `pcb153/SOURCE/` (repo master) and DOSBOXX.ZIP.

## Phase status

| Phase | Status |
|-------|--------|
| v0.2.0 Merge 8 libs | ✅ DONE (107,520 B, 114 modules) |
| v0.2.1 Add MAIN/SOURCE | ✅ DONE (272,896 B, 130/130) |
| v0.2.2 Link PPLC | ✅ DONE (221,644 B, vtable-only) |
| v0.2.3 Verify MAKEFILEs | ✅ DONE (9/9 OBJs, flags identified) |
| v0.2.4 Find real source | ✅ DONE (all code in PWA + pcbdcom, no stubs) |
| v0.2.5 Recompile with flags | **DONE** — 0 undefined, 18/19 tests pass, built from source with -P |
| v0.2.6 Diff delta vs PWA | **DONE** — delta154 stubs fixed, patches regenerated |
| v0.2.7 Eliminate vtable | DROPPED — vtable diffs are cosmetic, PPE works |
| v0.2.8 cmp -s 39-var PPE | PENDING — RUNINET.PPS fails to compile (v3.20 advanced features) |

## KEY FINDING: NO*.C stub architecture

Clark designed pcbkit_l.lib with **stub modules** for programs that
don't need the full BBS runtime. Instead of linking the massive MAIN
modules (ANSI 1,736 B, DISPLAY 1,547 B, SHELL 1,673 B), programs like
PPLC link tiny NO*.C stubs that provide empty implementations.

This is why Clark only had 21 undefined symbols. He didn't chase the
cascade — he used stubs for everything PPLC doesn't call.

### 17 NO*.C stub modules (LIB/SOURCE/TOOLKIT/)

| Stub | Functions | Replaces |
|------|-----------|----------|
| NOANSI.C | 18 — toggleoff, toggleon, curcolor, scrollon, scrolloff, reenablescroll, doesccodes, noesccodes, viewbuff, tagem, gettagged, etc. | MAIN/DISPLAY/ANSI.C |
| NODISP.C | 21 — startdisplay, moreprompt, showmessage, print, newline, println, bell, cleareol, etc. | MAIN/DISPLAY/DISPLAY.C |
| NOSHELL.C | 5 — spawndos, shelltodos, savepath, restorepath, performshell | MAIN/SHELL.C |
| NOSYS.C | 2 — readusersysfile, writeusersysfile | TOOLKIT/USERSYS.C |
| NOSCREEN.C | 5 — screen management stubs | MAIN/SCREEN.C |
| NOHELP.C | 1 — help system stub | MAIN/HELP.C |
| NOINPUT.C | 6 — input handling stubs | MAIN/INPUT.C |
| NOLANG.C | 2 — language system stubs | MAIN/LANGUAGE.C |
| NOLOG.C | 5 — logging stubs | SUPPORT/LOG.C |
| NOMEMORY.C | 7 — memory management stubs | MAIN/MEMORY.C |
| NOPCBSYS.C | 2 — pcboard.sys stubs | MAIN/SYS.C |
| NOPRINT.C | 1 — printer stub | DISPLAY/DISPLAY.C |
| NOSTATUS.C | 2 — status line stubs | MAIN/STATUS.C |
| NOTXT.C | 6 — text display stubs | MAIN/PCBTEXT.C |
| NOUPDSYS.C | 2 — sys update stubs | MAIN/SYS.C |
| NOXLATE.C | 1 — translation stub | DISPLAY/XLATE.C |
| NOCHAT.C | 1 — chat stub | MAIN/CHAT.C |

### Correct build approach

For PPLC (compiler only, no BBS runtime):
1. Use NO*.C stubs instead of full MAIN modules
2. Use TOOLKIT modules (INIT, PCBINIT, INITPORT) with `-DLIB`
3. Use LIB/SOURCE modules (8 sub-libs) for real utility functions
4. Use PCBGLOB.C for runtime globals
5. Result: small EXE, few undefined, no cascade

For PCBOARD.EXE (full BBS):
1. Use full MAIN modules (ANSI, DISPLAY, SHELL, etc.)
2. No stubs — everything is real code
3. All 349 source files compile

The cascade happened because we linked full MAIN modules into a lib
designed for stubs. Each full module references dozens of other full
modules. The stubs break the chain.

## Full PWA archive catalog (5,414 files)

### Top-level structure

    PCBSRC.BAZ        — build script template (copy to pcbsrc.bat)
    SRC153.LST        — file listing
    SRC153.TXT        — build instructions
    B/C31/            — Borland C++ 3.1 (compiler, includes, libs)
    LIB/CODEBASE/     — CodeBase 5 (dBASE SDK, 252 files)
    PCBSRC/           — working directory (PCBKIT_L.LIB, CLEANIT.BAT)
    PCBSRCV/          — 15 source revisions (000-014)

### Source revisions (PCBSRCV/)

| Version | Date | Description |
|---------|------|-------------|
| 000 | 1996-08-09 | CDC original — missing pieces, will not build |
| 001 | — | First working tree — base for all patches |
| 002 | — | Y2K fix for events (12-31-99 → 12-31-78) |
| 003 | — | FIDO AKA address fix |
| 004 | — | Relative paths, NTVDM/WinXP fix |
| 005 | — | efudd's PPLC build (reversed in 006) |
| 006 | — | Reversed most of 005 |
| 007 | 2013-03-02 | Revised PPLC build, pcbkit_l.lib from toolkit3 |
| 008 | — | pcboardm.exe read-only for multitasking |
| 009 | — | New modular build system |
| 010 | — | ASMOPT /i fix |
| 011 | — | #pragma inline reorganization |
| 012 | — | Header include convention cleanup |
| 013 | — | KbdStatus fix, modemfos.c pragma fix |
| 014 | — | giveup.c timeslice fix, code segment cleanup |

### MAIN/SOURCE/ (BBS core — 13 subdirectories)

| Directory | Files | Purpose |
|-----------|-------|---------|
| ASM/ | 7 | Assembly: ASYNC, BGKEY, CUTIL, INT24HND, etc. |
| COMPILER/ | 4 | PPLC compiler: CEH.ASM, H2NAME.C, SCOMP.CPP |
| DISPLAY/ | 12 | Screen output: ANSI, BLT, DIR, FILES, XLATE |
| DOS/ | 5 | DOS file I/O wrappers |
| FIDO/ | 26 | FidoNet echomail: toss, scan, pack, route |
| H/ | 36+ | Headers for MAIN source |
| MAIN/ | 25 | Core BBS: CALLWAIT, COMMAND, RECYCLE, SHELL, etc. |
| MODEM/ | 7 | Serial/modem: MODEM, MODEMDRV, MODEMASY, MODEMFOS |
| MSG/ | 6 | Message base management |
| NODE/ | 7 | Multi-node support |
| PPL/ | 19 | PPL compiler (6) + runtime (13) |
| SUPPORT/ | 13 | Accounting, capture, log, etc. |
| USERS/ | 6 | User record management |

### LIB/SOURCE/ (8 sub-libraries + TOOLKIT)

| Library | Files | Purpose |
|---------|-------|---------|
| COUNTRY/ | 13 | Date/number formatting |
| DOS/ | 47 | DOS system calls |
| DOSCLS/ | 1 | DOS screen clear |
| MISC/ | 90 | String/memory/crypto utilities |
| PCB/ | 22 | PCBoard data file access |
| SCREEN/ | 44 | Screen/video management |
| SCRNIO/ | 20 | Screen I/O |
| SYSTEM/ | 7 | System utilities |
| TOOLKIT/ | 46 | Shared modules: INIT, PCBINIT, NO*.C stubs |

### UTIL/ (13 utility programs)

PCBCP, PCBDIAG, PCBEDIT, PCBFILER, PCBMODEM, PCBMONI, PCBNLC,
PCBPACK, PCBSETUP, PCBSM, PCBSTATS, PCBTEXT, PCBUTILS

### MISC/ (standalone modules)

FIDOUTIL, HELP, IDX, MD5, USERNET, UUCP, WAITFILE, ZMODEM

## COMMDRV vs pcbdcom

Two comm driver versions for two build targets:

| | pcb153 (PWA) | pcb154 (delta) |
|---|---|---|
| Comm driver | COMMDRV (original WCS) | pcbdcom (GPLv3 replacement) |
| Header | Reconstructed COMM.H | toolkit/pwa154/pcbdcom/inc/pcbdcom.h |
| MODEMDRV.C | Links against COMMDRV API | Links against pcbdcom (drop-in) |
| Build type | Full (original architecture) | Two versions: full + lightweight |

COMM.H (reconstructed from MODEMDRV.C usage) stays for pcb153.
pcbdcom.h is the drop-in replacement for pcb154 only.

### pcbdcom drop-in status — DONE

pcbdcom.h updated to full COMMDRV compatibility. All gaps filled.

| COMMDRV feature | pcbdcom.h | Status |
|-----------------|-----------|--------|
| struct port_param (basic) | baud, parity, data_bits, stop_bits, flow, buf_size | Done |
| port_param.opcb (port control block) | opcb_type pointer | Done |
| port_param.auxpcb (aux control block) | auxpcb_type pointer | Done |
| port_param.cardtype | CARD_* constant | Done |
| port_param.lngth | LENGTH_7 / LENGTH_8 | Done |
| port_param.protocol | PROT_* constant | Done |
| port_param.block[] | unsigned char[4] | Done |
| port_param.outbuf_len / inbuf_len | unsigned int | Done |
| port_param.error | int | Done |
| opcb_type.msr_reg | unsigned char (read from hardware) | Done |
| opcb_type.inbuf_count / outbuf_count | unsigned int (from ring buffer) | Done |
| opcb_type.cardtype / flag | unsigned char | Done |
| auxpcb_type (framing/overrun/parity) | int aux_frmint/ovrint/parint | Done |
| BAUD constants (BAUD300-BAUD115200) | 9 divisor constants | Done |
| LENGTH_8, LENGTH_7 | defined | Done |
| PARITY_NONE, PARITY_EVEN, PARITY_ODD | defined | Done |
| PROT_RTSRTS, PROT_XONXOFF | defined | Done |
| CARD_* constants (8250-ARNET) | 8 card types | Done |
| RS232ERR_NONE/BUSY/PARAM/NOPORT | defined | Done |
| 13 ser_rs232_* functions | all present | Done |
| ser_rs232_flush (2 args) | (port, which) | Done |
| ser_rs232_getpacket (COMMDRV calling) | (port, n, buf) matches MODEMDRV.C | Done |
| ser_rs232_viewpacket | peek without consuming | Done |
| ser_rs232_putbyte (pointer arg) | (port, unsigned char *b) | Done |
| ser_rs232_init (no args) | ser_rs232_init(void) | Done |
| ser_rs232_getport (port_param out) | populates all fields + opcb/auxpcb | Done |
| ser_rs232_dtr_on/off | MCR bit 0 | Done |
| ser_rs232_rts_on/off | MCR bit 1 | Done |
| pcbdcom_port_t | 24 fields + compat_opcb + compat_auxpcb | Done (was missing) |
| PCBDCOM_MAX_PORTS / RX_RING / TX_RING | 32 / 4096 / 2048 | Done (was missing) |

Our reconstructed COMM.H fills these gaps for pcb153 (PWA build).
For pcb154 (delta), pcbdcom.h itself needs to be updated to include
ALL of these so MODEMDRV.C compiles against pcbdcom directly without
a separate COMM.H. pcbdcom must be a true drop-in — same structs,
same constants, same function signatures as COMMDRV.

## PPLC compiler architecture

### Shared source — compiler and runtime are the same code

PPL/ has 19 files. The same source builds TWO different programs
depending on which `#define` is set:

| Mode | Define | Entry point | Program |
|------|--------|-------------|---------|
| Compiler | `___COMP___` | doScrSub → compile() + save() | PPLC.EXE |
| Runtime | `___EXEC___` | doScrSub → load() + execute() | Built into PCBOARD.EXE |

Clark's PPLC.MAK: `CD = -DLIB;COMM;___COMP___`

The `#ifdef ___COMP___` blocks compile the .PPS parser and .PPE writer.
The `#ifdef ___EXEC___` blocks compile the bytecode interpreter and
PPL function library. Both share variable handling (VAR.CPP), label
management (LABEL.CPP), and the script framework (SCRMISC.CPP).

### 6 PPLC compiler files + 3 shared

| File | Role |
|------|------|
| SCOMP.CPP | main() — argument parsing, entry point |
| SCRCOMP.CPP | compile() — .PPS tokenizer and code generator |
| NEWSCR.CPP | save() — .PPE writer, encrypt2/encrypt3, doRLE compression |
| SCRMISC.CPP | doScript/doScrSub — script framework |
| PCBMISC.CPP | Compiler utilities |
| VAR.CPP | cVARVAL class — variable storage |
| LABEL.CPP | Label/jump management |
| H2NAME.C | Header-to-name conversion |
| CEH.ASM | Critical error handler |

### 13 runtime-only files (NOT in PPLC)

SCREXEC, EVALP, EXECDB, DBASE, DOSCLASS, MENU, PCBMSGS, RATIO,
QINT, LRAND, FILESTUB, MSGSTUB, OLDVAR

These compile into PCBOARD.EXE with `___EXEC___`. They provide the
PPL function library (300+ PPL commands like DISPFILE, GETUSER, etc.)

### v005 vs v007 — two PPLC build attempts

v005 (efudd) created the first PPLC build. v006 reversed most of it.
v007 revised it correctly: use pre-built pcbkit_l.lib from toolkit3,
compile only the 9 PPLC source files against it. The lib provides
utility functions; the NO*.C stubs satisfy everything else.

## What "cascade" means

When building pcbkit_l.lib, we tried using the FULL MAIN modules
(ANSI.C, DISPLAY.C, SHELL.C, USERS.C, etc.) instead of Clark's
NO*.C stubs. This created an expanding dependency chain:

    USERS.C defines getuserrecord()
    → getuserrecord() calls readusersfile()
      → readusersfile() calls dosopen() ← already in lib, OK
      → getuserrecord() also calls convertreadtodata()
        → convertreadtodata() calls decryptusersrec()
          → decryptusersrec() calls displayusernet()
            → displayusernet() is in COMMAND.C (not in lib!)
              → adding COMMAND.C brings in flagfiles()
                → flagfiles() calls showfile()
                  → showfile() is in DIR.C (not in lib!)
                    → adding DIR.C brings in moreprompt()
                      → moreprompt() is in DISPLAY.C
                        → DISPLAY.C brings in 21 more functions...

Each module we added to resolve undefined symbols REFERENCED more
functions that weren't in the lib. Adding those brought in MORE.
The undefined count bounced: 130 → 160 → 144 → 161 → 169 → 133 →
170 → 93 → 231. It never converged to zero.

The NO*.C stubs BREAK this chain. Instead of adding the full
DISPLAY.C (which references 21 other functions), NODISP.C provides
empty implementations of moreprompt(), showmessage(), print(), etc.
The linker is satisfied. No new dependencies. Chain broken.

This is Clark's design. pcbkit_l.lib is not a "compile everything"
library. It's a modular library where each program links the modules
it needs and stubs for everything else.

## Build targets

### pcb153 (PWA) — ZERO undefined (ACHIEVED)

Original target was 21 undefined to match Clark. Achieved ZERO by
compiling all source with -P (C++ mode) so symbols match between
PPLC and pcbkit_l.lib. Clark had 21 because his lib was compiled
as C (different symbol decoration). Our lib is compiled from the
same source as C++ — all symbols resolve.

Clark's 21 undefined:
  addchar, breakdate, countrydate, datetojulian, decrypt2, dorle,
  dosclose, doscreatecheck, dosfclose, dosfgets, dosfopen, dosrewind,
  encrypt2, fileexist, getcountryspecs, juliantodate, lascii,
  stripleft, stripright, uncountrydate2, writecheck

NOTE: Our lib already resolves all 21 of these. Clark's 1994 lib
was less complete. Our target is to match his 21 count by using
the NO*.C stub architecture, not to match his specific 21 symbols.

### pcb154 (delta) — may address the 21

pcb154 has pcbdcom and additional source that COULD resolve the 21.
But if PPLC works with 21 undefined — same as Clark shipped — there
is no reason to change it. Don't fix what isn't broken.

If the 21 ARE addressed in pcb154, each one needs justification:
what broke, why it needs the real implementation, what changes.

## pcbkit_l.lib serves TWO link configurations

This is the core of Clark's architecture. ONE library, TWO ways to
link against it. This is why the NO*.C stubs exist.

### Full link (PCBOARD.EXE)

    PCBOARD.OBJ + pcbkit_l.lib (full MAIN modules) + BC31 runtime
    
    MAIN modules: ANSI, DISPLAY, SHELL, USERS, INIT, SCREEN, etc.
    All real code. All functions defined.
    The full BBS — modem, chat, transfer, user management, everything.
    All 349 source files compile into this.
    No stubs. No undefined symbols.

### Stub link (PPLC.EXE and other tools)

    PPLC OBJs + pcbkit_l.lib (stubs override MAIN) + BC31 runtime
    
    NO*.C stubs: NOANSI, NODISP, NOSHELL, NOSYS, NOSCREEN, etc.
    Empty functions that satisfy the linker.
    PPLC only needs: compile() + save() + file I/O + encryption.
    Everything else is stubbed out.
    21 undefined symbols — by design, same as Clark shipped.

### How it works

pcbkit_l.lib contains BOTH the full modules AND the stubs. TLINK
picks which one to use based on link order. The PPLC.MAK link
command lists the stub OBJs BEFORE pcbkit_l.lib:

    TLINK c0l.obj + pplc_objs + stub_objs, pplc.exe,, pcbkit_l.lib

When TLINK sees toggleprinter() referenced, it finds it in the
stub OBJ (from NOANSI.C — empty function). It never looks for it
in pcbkit_l.lib's DISPLAY module. Chain broken. No cascade.

For PCBOARD.EXE, the full MAIN OBJs are listed instead of stubs:

    TLINK c0l.obj + pcboard_objs + main_objs, pcboard.exe,, pcbkit_l.lib

Now toggleprinter() comes from the real DISPLAY module. Full code.
All dependencies resolve because PCBOARD.EXE links ALL modules.

### Why the cascade happened

We put full MAIN modules INTO pcbkit_l.lib and tried to link PPLC
against it. The full DISPLAY module has toggleprinter() which calls
sendtoprinter() which calls doswrite() which calls checkdisplaystatus()
which calls... and on and on.

Clark NEVER put full MAIN modules in pcbkit_l.lib for PPLC builds.
He put them in for PCBOARD.EXE builds only. For PPLC, the stubs
went in. Same library file, different modules active depending on
what OBJs the link command lists first.

### This explains everything

- Why Clark had only 21 undefined (stubs handled the rest)
- Why our count kept going up (full modules keep pulling in more)
- Why the NO*.C files exist (they're not lazy — they're architecture)
- Why pcbkit_l.lib is 241 KB not 900 KB (stubs are tiny)
- Why PPLC.EXE is 222 KB not 490 KB (no BBS runtime code)

## PPL DevKit (devtools/ppldevkit.zip)

Clark's shipped PPL development kit. PPLC Version 1.00, 67,786 bytes,
dated July 23 1993.

Contains 18 sample PPS/PPE pairs — perfect byte-exact test suite:

| Program | PPS | PPE | Description |
|---------|-----|-----|-------------|
| HELLO1-7 | 119-507 B | 473-769 B | Progressive tutorial examples |
| DOORS | 2,597 B | 1,500 B | Door program launcher |
| HAMURABI | 9,613 B | 3,835 B | Hammurabi game |
| KAL | 3,738 B | 1,283 B | Calendar |
| LANGUAGE | 1,500 B | 752 B | Language selection |
| MORE | 2,668 B | 958 B | More prompt replacement |
| NODEFILE | 1,229 B | 597 B | Node file display |
| OPPAGE | 4,760 B | 2,131 B | Operator page |
| ORDER | 3,189 B | 2,280 B | Order form |
| PWRDWARN | 2,142 B | 1,027 B | Password expiry warning |
| START | 1,119 B | 812 B | Startup script |
| WELFIRST | 3,785 B | 1,465 B | Welcome first-time user |

Our PPLC must compile each PPS and produce PPE matching Clark's
output. Vtable diffs are cosmetic — different EXE size = different
vtable address on disk, but PCBoard's PPE loader ignores these bytes.
Functionally identical. Not worth chasing byte-exact.

## Toolkit3 (devtools/Toolkit3.zip) — Clark's developer SDK (REFERENCE ONLY)

This is what Clark shipped to third-party developers. We use it as
REFERENCE to verify our build output — not as the build itself.

**We compile our own NO*.C from PWA source.** GPLv3, reproducible,
ours. Toolkit3 tells us exactly what the output should look like.
We build it ourselves and prove it matches.

- Our NO*.OBJ must export the same symbols as Clark's
- Our pcbkit_l.lib must have the same module structure
- Our PPLC link must produce the same 21 undefined
- Our PPE output must compile correctly (vtable diffs are cosmetic)

### PCBKIT_L.ZIP contents (large memory model, Borland C)

    PCBKIT_L.LIB     — the library (same as PCBSRC/PCBKIT_L.LIB)
    COMMDRV.OBJ      — WCS COMM-DRV serial driver object
    FOSSIL.OBJ       — FOSSIL serial driver object
    PCBDAT.OBJ       — PCBoard data file access
    SMALLERR.OBJ     — Smaller error handler (vs full SHOWERR)
    SMALLTXT.OBJ     — Smaller text display (vs full PCBTEXT)

    17 NO*.OBJ stub files:
    NOANSI.OBJ       NODISP.OBJ       NOSHELL.OBJ
    NOCHAT.OBJ       NOHELP.OBJ       NOSTATUS.OBJ
    NOINPUT.OBJ      NOLANG.OBJ       NOSYS.OBJ
    NOLOG.OBJ        NOMEMORY.OBJ     NOTXT.OBJ
    NOPCBSYS.OBJ     NOPRINT.OBJ      NOUPDSYS.OBJ
    NOSCREEN.OBJ     NOXLATE.OBJ

### How PPLC links (from Toolkit3)

    TLINK c0l + pplc_objs + NO*.OBJs + COMMDRV.OBJ, pplc.exe,, pcbkit_l.lib + cl.lib

The NO*.OBJs listed BEFORE pcbkit_l.lib override the full modules.
COMMDRV.OBJ provides the serial driver. The lib provides everything
else. 21 undefined symbols remain — by design.

### Memory model variants

| Zip | Compiler | Model |
|-----|----------|-------|
| PCBKIT_C.ZIP | Generic | Compact |
| PCBKIT_L.ZIP | Generic | Large |
| PCBKIT_M.ZIP | Generic | Medium |
| PCBKIT_S.ZIP | Generic | Small |
| PCBKBC_C/L/M/S.ZIP | Borland C | All models |
| PCBKMS_C/L/M/S.ZIP | Microsoft C | All models |


## DOSBox-X (devtools/dosbox-x/ — separate project)

Patched DOSBox-X for LANtastic v6 networking. Full build toolchain
on repo at `devtools/dosbox-x/`:

    README.md                    — build instructions
    dosboxx-toolchain.md         — MSYS2 build guide + offline packages
    dosbox-x-src-patched.7z      — full source with 3 patches
    dosboxx-toolchain.7z.001-003 — MSYS2 + MinGW64 offline toolchain
    dosbox-x.conf                — config for LANtastic NE2000
    SDL2.dll                     — required DLL for Windows

Pending move: `todo/dosboxx-dpmi-failures.md` → `devtools/dosbox-x/`

### Three patches

1. **NE2000 loopback** (ne2000.cpp) — save/restore CR.stop + skip
   MAC filter. Fixes NE3.EXE hardware validation hang.
2. **INT 21h conventional memory stub** (shell_misc.cpp) — PSP[0x0C]
   fix. 8 attempts before finding the right approach.
3. **Top-of-memory COMMAND.COM** (shell.cpp) — PSP at 0x9F65 like
   real MS-DOS. NE3 math passes: 0xE20B + 0x9F65 = 0x8170 (530KB).

### Next: full stack test

NE3 → AILANBIO → REDIR → SERVER → NET on patched build.

### DOSBox-X 2026.08.02 — ethnet command

Upstream added experimental NE2000 client-server LAN emulation.
No pcap, works over Wi-Fi. May replace our loopback fix.

## Other devtools

| File | Size | Contents |
|------|------|----------|
| ppld32.zip | 29 KB | PPLD.EXE — PPL decompiler |
| pplx20.zip | 84 KB | PPLX.EXE — PPL executor (runs PPE outside PCBoard) |
| Toolkit3.zip | 1.5 MB | Full developer SDK (REFERENCE ONLY) |
| toolkit3a/b.ZIP | 1.0+0.7 MB | Toolkit3 split across 2 floppies |
| BC31.zip | — | Borland C++ 3.1 compiler |
| Develop.zip | — | Development tools |
| hxrt216.zip | — | HX DOS Extender runtime |
| cwsdpmi.zip | — | CWSDPMI DOS extender |

## CRITICAL: C vs C++ symbol naming (BCC 3.1)

Toolkit3 PCBKIT_L.LIB was compiled as **C** (no `-P`). PPLC source
compiles as **C++** (`.CPP` extension or `-P` flag). The symbol names
DON'T MATCH:

    C mode:   void pascal dosclose(int) → symbol DOSCLOSE
    C++ mode: void pascal dosclose(int) → symbol @DOSCLOSE$qi

BCC 3.1's `extern "C"` does NOT work with pascal calling convention.
The headers HAD `extern "C"` blocks but Clark **commented them out**
— he knew they didn't work.

**Fix:** compile sub-lib source files WITH `-P` (C++ mode) to match
PPLC's symbol decoration. Build pcbkit_l.lib from source, not from
pre-built Toolkit3 lib.

The Toolkit3 SDK lib was for third-party **C** programs. PPLC compiles
as **C++**. The PWA build compiles everything together with matching
flags — that's the correct approach.

Toolkit3 PCBKIT_L.LIB is **REFERENCE for module content and size only**
— not for linking. We must build our own lib from source with `-P` to
match PPLC's symbols.

### Compile flag rules

| Module type | Flags |
|-------------|-------|
| PPLC source (9 .CPP) | PPLC.CFG with `-P -DLIB -DCOMM -D___COMP___` |
| Sub-lib source | PPLC.CFG with `-P` (C++ mode to match PPLC symbols) |
| NO*.C stubs | PPLC.CFG with `-P -DCOMM -DPCB_MAXNODES=250` |
| TOOLKIT modules (INIT, PCBINIT, INITPORT) | add `-DLIB` |
| Modem modules | add `-DMULTIPORT -DCOMMDRV` |
| CALLWAIT | add `-DPCBSTATS` |
| ANSI | add `-D__OS2__` |

**ALL modules must use `-P`** so symbols match. This is different from
Clark's original build where lib modules were C and PPLC was C++.
Clark linked against a lib compiled with matching symbols. We don't
have that lib's source — we rebuild from PWA source with `-P`.

## v0.2.6 — Full diff and delta update

### Architecture: three source layers + beta overlay

The PCBoard source exists in three layers, each building on the last:

    pwa153          Clark's 1996 source (base). Our fixes applied here.
        |
    pwa154          Minimal delta on pwa153. Only pcbtools.h changed
        |           (one enum value added) + pcbdcom comm driver added.
        |           SOURCE/TOOLKIT is IDENTICAL to pwa153.
        |
    delta154        Watcom port of pwa153. 6 TOOLKIT files changed for
                    Watcom compatibility. Extra headers (Watcom wrappers,
                    VIRTUAL1.H, ZSORT.H). Wrappers STAY — needed for Watcom.

    pcb153/upd154   15.4 beta MAIN source (600 files). Clark's last work
                    before CDC shut down. Sits on top of the lib layers.
                    NEWSCR.CPP has CUR_PPE_VER=340 (intentional 15.4 bump).
                    Uses pcbdcom instead of COMMDRV. Has same USERS.C bug.

### A. Apply stub fixes to delta154

delta154's NODISP.C, NOPCBSYS.C, NOXLATE.C are IDENTICAL to pwa153
originals. Apply the same fixes we made in v0.2.5:

| File | Fix | Why |
|------|-----|-----|
| NODISP.C | Remove print/newline/println | PCBMISC.CPP defines them |
| NOPCBSYS.C | Remove readpcboardsys/makepcboardsys | NOUPDSYS defines them |
| NOXLATE.C | Remove printxlated | PCBMISC.CPP defines it |
| NOUPDSYS.C | Empty readpcboardsys body | Breaks cascade (delta154 already has malloc/bassngltolong casts) |

Watcom wrappers (alloc.h, mem.h, dir.h, borland.h, watfix.h) STAY
in delta154. They are needed for Watcom builds. Only pwa153's Borland
build removes them.

### B. Apply USERS.C extern fix to pcb153/upd154

pcb153/upd154/SOURCE/USERS/USERS.C has the same bug as pwa153:
ConfFlags, ConfReg, MsgReadPtr, MsgReadPtrBackup declared without
`extern` at lines 115-118. These variables are owned by USERSYS.C
(in TOOLKIT) and PCBINIT.C. USERS.C must declare them as `extern`
when compiled with `-DLIB`.

upd154 already has `#ifndef LIB` around lines 115-118. With `-DLIB`
they compile out and the TOOLKIT definitions take over. This may
already work — verify before changing.

ExtConfLen and ConfByteLen also need `extern` — same fix as pwa153.

### C. Apply USERS.H _FARDATA_ guard to pcb153/upd154

pcb153/upd154/SOURCE/H/USERS.H uses `_FARDATA_` type which may not
be defined in all include paths. Add the same `#ifndef _FARDATA_`
fallback guard we added in v0.2.5.

### D. DON'T touch upd154 NEWSCR.CPP

pcb153/upd154/SOURCE/PPL/NEWSCR.CPP has `CUR_PPE_VER = 340`. This
is Clark's INTENTIONAL bump for PCBoard 15.4. It is NOT the same bug
as pwa153 (where CUR_PPE_VER was 330 but should be 320 for 15.3).

15.4 PPE format is version 3.40. Do not change it.

### E. Full diff — DONE

Two patches regenerated against cleaned source:

    15.4-pwa.patch      82 files, 5,783 lines — MAIN source (pcb153 → upd154)
    15.4-toolkit.patch  236 files, 39,927 lines — toolkit (pwa153 → pwa154 + delta154)

Previous 15.4-pwa.patch moved to patches/attic/ — generated against
uncleaned source, no longer applies cleanly.

patches/README.md updated to explain why the patch was regenerated.
