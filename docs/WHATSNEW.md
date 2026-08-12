
                    PCBoard Version 15.41
                    =====================
                    What's New / Changes
                    
                    pcbrevival / pcbirc crew
                    August 2026


OVERVIEW
--------
Version 15.41 is a maintenance and feature release of PCBoard 15.4.
It adds FidoNet file echo (TIC) support, FTP as a file transfer protocol, BinkP mailer for FidoNet over TCP/IP,
protocol for telnet callers, FTP server configuration, file source
tracking, and completes the OpenWatcom 2.0 source port.

The existing data formats are fully backward compatible — no
conversion is needed for existing BBS installations.


NEW FEATURES
------------

  FidoNet TIC File Echo Support (pcbtic)
  ──────────────────────────────────────
  PCBoard now supports FidoNet file echo distribution through the
  new pcbtic utility. This fills the gap between PCBoard's built-in
  PCBTOSS echomail tosser and the file distribution network.

    PabloDraw Pascal   sysop/0 — full rewrite of PabloDraw (414 C#
                       files → 20 Pascal units, 4,460 lines)
                       Original by Curtis Wensley (MIT)
                       github.com/verta1878/fpc264irc/examples/pablodraw

  - Toss inbound .TIC files from BinkleyTerm XE
  - Move files to correct PCBoard file directories
  - Update DIR listing files automatically
  - Forward TIC + files to downlinks (passthrough)
  - Hatch new files into the TIC network
  - Configurable via pcbtic.cfg with area-to-directory mapping
  - FTS-5006.001 compliant TIC format

  See docs/PCBTIC.md for full documentation.


  File Source Tracking
  ────────────────────
  New optional DIRxxx.SRC companion files track how each file
  entered the system:

    FSRC_LOCAL   (0)  - Sysop upload / local file
    FSRC_UPLOAD  (1)  - Remote user upload via modem/telnet
    FSRC_FTP     (2)  - FTP server upload
    FSRC_TIC     (3)  - FidoNet TIC file echo
    FSRC_FREQ    (4)  - FidoNet file request
    FSRC_IMPORT  (5)  - Batch import / sysop tool

  Source tracking is automatic — pcbtic and the FTP server write
  .SRC entries as files arrive. Existing file areas continue to
  work without .SRC files (all files show as LOCAL).

  The .SRC file format is documented in docs/PCB1541_DRAFT.md.


  FTP Server Configuration
  ────────────────────────
  pcbis (installer TUI) now includes FTP server configuration:

    - Enable/disable FTP server
    - FTP port (default 21)
    - File root directory
    - Anonymous login toggle
    - Maximum concurrent connections
    - Welcome banner message

  Settings saved to pcbis.cfg. See docs/PCBIS_UI.md.


  FTP File Transfer Protocol
  ──────────────────────────
  FTP is now available as a download/upload protocol for callers
  connected via telnet. Select (F) at the protocol prompt:

    Protocol for Transfer (Enter)=Z?

       (A) ASCII
       (C) Xmodem CRC
       (O) 1K-Xmodem
       (Y) Ymodem Batch
       (Z) Zmodem Batch
    => (F) FTP (Internet)
       (N) None

  FTP handles both single and batch file transfers. SyncTerm and
  NetRunner terminals support FTP over telnet sessions natively.

  Setup: add one line to PCBPROT.DAT:

    F,D,0,FTP (Internet),N,N,N

  FTP is automatically hidden from the protocol list for modem
  (non-TCP) callers. Requires FTP server configuration in
  PCBOARD.DAT (FtpHost, FtpPort) and the PCBSF.BAT/PCBRF.BAT
  protocol handler batch files. See docs/PCB1541_DRAFT.md
  section 19 for full implementation details.

  New PCBTEXT string:
    #788  TXT_FTPNOTSUPPORTED  "FTP transfer not available on
                                this connection."

  All existing transfer prompts (filename display, download time,
  protocol type, transfer successful/aborted) work unchanged
  with FTP.


  BinkP Mailer Integration
  ────────────────────────
  PCBoard 15.41 includes integrated BinkP (FidoNet over TCP/IP)
  mailer support. No separate mailer (FrontDoor, BinkleyTerm)
  needed for TCP-based FidoNet connections.

    - Outbound: FIDOPOLL command or ALT-F sysop hotkey
    - Inbound: BinkP listener on configurable port (default 24554)
    - CRAM-MD5 authentication (password never sent in clear)
    - Full-duplex file exchange
    - BSO (Binkley-Style Outbound) directory structure
    - Auto-toss received .PKT files after session

  Configure per-node in PCBSETUP → FidoNet → Node Editor:
    Mail Type, BinkP Host, Timeout, Block Size, MD5 Auth

  PCBOARD.DAT settings:
    BinkpEnable, BinkpPort, BinkpInbound, BinkpOutbound

  PCBTEXT strings 775-778 (TXT_BINKP*).
  See docs/PCB1541_DRAFT.md section 20 for protocol details.

  This is a 15.41 feature — not present in 15.4.


  Startup and Shutdown Scripts
  ────────────────────────────
  Platform-specific scripts for running PCBoard as a service:

    Linux:    pcbis_startup / pcbis_shutdown (bash)
    Windows:  pcbis_startup.bat / pcbis_shutdown.bat
    OS/2:     pcbis_startup.cmd / pcbis_shutdown.cmd (REXX)

  Linux/Windows: starts netmodem2irc → DOSBox → PCBoard in order.
  OS/2: starts PCBoard natively (no DOSBox needed).

  First-time setup: pcbis_initv / pcbis_initv.bat creates the
  directory structure, dosbox.conf, and default pcbis.cfg.

  Configuration TUI: PCBIS_W.EXE (48KB, OpenWatcom). ANSI
  menu-driven editor for pcbis.cfg — general settings, paths,
  FTP, FidoNet, DOSBox config.

  Can run as a systemd service (Linux), Task Scheduler task
  (Windows), or from STARTUP.CMD (OS/2).

  See docs/PCB1541_DRAFT.md section 21.


  PCBCP — OS/2 Control Panel
  ──────────────────────────
  PCBoard Control Panel for OS/2 Presentation Manager, recovered
  from pcball.zip and ported to OpenWatcom 2.0. PCBCP_W.EXE (77KB)
  provides windowed GUI monitoring of all PCBoard nodes.

  Source: 8 C files (6,241 lines), originally compiled with IBM
  C Set/2. Now compiles under wpp386 -bt=os2 with 6 fixes.

  IMPORTANT: PCBCP.INI is an OS/2 binary INI file that must be
  configured by the sysop on first run. Default paths point to
  Clark's dev machine. Update all node paths through PCBCP's
  Options menu. Future: PCBSETUP will generate PCBCP.INI.

  See docs/PCB1541_DRAFT.md section 22.


  Web Server Configuration
  ────────────────────────
  pcbis now includes web server configuration:

    - Enable/disable web server
    - HTTP port (default 8080)
    - Web root directory (DATA/default/www/)
    - Page title
    - ANSI art preview toggle
    - File area browser toggle

  Default index.htm included with full PCBoard feature listing.


  Nodelist Compiler (PCBNLC — Clark)
  ──────────────────────────
  Standalone nodelist compiler reads NODELIST.### and produces
  NODELIST.DBF + NODELIST.NDX for PCBoard's FidoNet subsystem.
  Replaces external nodelist compilers.


  Nodelist Lookup (NL command)
  ────────────────────────────
  New user-facing command lets callers search the compiled
  nodelist by FTN address (with wildcards) or by keyword
  (sysop name, BBS name, location). Configurable minimum
  security level. Based on Mystic BBS TNodeListSearch design.


  Message Reader Enhancements
  ───────────────────────────
  New reader commands:

    I  - Message Info: show FidoNet MSGID, REPLY, SEEN-BY,
         PATH, and origin for echomail/netmail messages
    B  - Thread Browser: visual tree display of message
         thread with navigation (next/prev/read/leave)
    W  - Move Message: move message to another conference
         (sysop security required)
    ^  - Thread Root: jump to first message in thread

  FidoNet-aware message display shows sender/recipient FTN
  addresses, area tags, tearlines, and origin lines for
  echomail and netmail. SEEN-BY/PATH hidden by default,
  shown with I command.


  Echomail Toss/Scan Improvements
  ───────────────────────────────
  PCBTOSS and PCBMSG updated with:

    - MSGID-based duplicate detection (30,000 entry circular
      buffer in DUPES.DAT, configurable size)
    - Proper SEEN-BY merging per FTS-0004
    - PATH line maintenance
    - MSGID/REPLY kludge generation for outbound
    - Passthrough area support (forward without storing)
    - Improved bad packet logging with reject reason
    - Tearline: "--- PCBoard 15.41/OpenWatcom"

  Reference: Mystic BBS mutil_echoimport/echoexport (g00r00)


  New PCBTEXT Strings (751-780)
  ─────────────────────────────
  30 new text strings for nodelist lookup, FidoNet message
  display, thread browser, echomail toss/scan statistics,
  file source display, and BinkP status. All support existing
  @X color codes and @-variable pipe code substitution.

  New pipe code variables: @ZONE@, @NET@, @NODE@, @FROMADDR@,
  @TOADDR@, @AREATAG@, @MSGID@, @TREE@, @TOSSED@, @DUPES@,
  @SCANNED@, @AREAS@, @SOURCE@, @ADDR@, @HOST@, @STATUS@, @DIR@


  FREQ and Magic Name Documentation
  ──────────────────────────────────
  PCBoard 15.4 already includes full FREQ (file request) and
  magic file name support via FREQPATH.DAT, MAGICNAM.DAT, and
  FREQDENY.DAT, managed through PCBSETUP's FidoNet menu (items
    PabloDraw Pascal   sysop/0 — full rewrite of PabloDraw (414 C#
                       files → 20 Pascal units, 4,460 lines)
                       Original by Curtis Wensley (MIT)
                       github.com/verta1878/fpc264irc/examples/pablodraw

  J/K/L). BinkleyTerm XE supports magic names through its OKFILE
  system with *@+$ prefixes. QFront 1.20a has its own FREQ.CFG.

  Standard FidoNet magic names (NODELIST, NODEDIFF, FILES, ABOUT)
  are now documented in PCB1541_DRAFT.md section 15 with the full
  data structures and pcbis installer integration.

  pcbis now includes a FREQ configuration screen for easy initial
  setup of FREQ paths, magic names, and session restrictions.


  New Menu Commands
  ─────────────────
  Main prompt:
    NL           Nodelist lookup
    FIDO STATUS  FidoNet mail status (sysop)
    FIDO TOSS    Force echomail toss (sysop)
    FIDO SCAN    Force echomail scan (sysop)
    FIDO POLL    Force BinkP poll (sysop)


  PPE Collection
  ──────────────
  5,703 PPE (PCBoard Programming Language Executable) files
  included with full FILE_ID.DIZ index. Covers doors, login
  screens, utilities, games, sysop tools, and more.


TOOLS ADDED
-----------

  pcbtic      TIC file processor (toss/hatch/list)
  pcbis       Installation system TUI (7 config screens inc. FREQ)
  pcbfcfg     Standalone FidoNet configurator (mirrors PCBSETUP A-L)
  PCBNLC      Nodelist compiler (Clark) (NODELIST.### → .DBF/.NDX)
  upd1541     Backup + version upgrade (15.4 → 15.41)
  pcbdraw     ANSI art editor with PCBoard @X codes & animation
  pcbis_ui    Full-screen ANSI configuration interface


SOURCE PORT
-----------

  OpenWatcom 2.0 Port: 556/556 files, 28 Clark binaries linked

  All PCBoard 15.4 source files compile under OpenWatcom 2.0
  and all 13 original Clark binaries link successfully:

    PCBOARD_W.EXE   1.3MB  Main BBS engine
    LOCAL_W.EXE     1.3MB  Local login mode
    PPLC_W.EXE      1.3MB  PPL 3.40 compiler
    PCBSETUP_W.EXE  424KB  Setup utility
    PCBSM_W.EXE     221KB  System Manager
    UUIN_W.EXE      1.4MB  UUCP import
    UUOUT_W.EXE     1.4MB  UUCP export
    UUUTIL_W.EXE    1.4MB  UUCP utilities
    UUXFER_W.EXE    1.4MB  UUCP transfer
    MAKEIDX_W.EXE    37KB  Index builder
    USERNET_W.EXE    28KB  User network
    MKPCBTXT_W.EXE   27KB  Text generator
    MAKEHELP_W.EXE   25KB  Help builder

  The PCBoard 15.4 source code has been ported from Borland C++
  3.1 to OpenWatcom 2.0. Key changes:

  - WATCOMPAT.H compatibility layer (~300 lines)
    farmalloc→malloc, bioskey→_bios_keybrd, Borland pseudo-
    registers (_AH/_AL etc.) mapped to union REGS + int386()

  - OS/2 guard extension: ~40 files unlocked by extending
    #ifdef __OS2__ to include __WATCOMC__, exposing existing
    C code branches that replace inline assembly

  - Phase 3 ASM→C conversion: 8,251 lines TASM → 309 lines C
    ASYNC.ASM → FOSSIL driver (181 lines), CUTIL, TIMER,
    BGKEY, NOSCROLL, MEMMOVE, INT24HND, XMODEM stubbed

  - CNAMES.C atexit() fix: closecnames is __pascal but atexit
    expects __cdecl. Added cdecl wrapper. Original Clark source
    bug that only manifests under OpenWatcom. pcb.lib complete.

  - ASYNC.C CPU hog fix: added INT 2Fh/1680h DPMI timeslice
    release in COMMINKEY and CHECKCOMM polling loops. Fixes the
    known FOSSIL busy-wait bug (alt.bbs.pcboard, Dec 2014).

  - Inline ASM → C conversion: 8 files hand-converted
    (CONFFUNC, WILDCARD, ZSWAPVIR, ZSWAPSTR, EVALUATE,
    SETROWS, PCBMISC, DLPATH)

  - constrea.h: Borland constream replacement for Watcom
  - dir.h: Borland directory function shim
  - struct ffblk, struct date/time, text_info compatibility
  - C++ keyword conflicts resolved (xor → xor_val)
  - Enum/macro conflicts resolved (YESNO, PROGRAM, TEXT,
    ALLCONF, DEFAULTS, WORDWRAP, displaytype)

    PabloDraw Pascal   sysop/0 — full rewrite of PabloDraw (414 C#
                       files → 20 Pascal units, 4,460 lines)
                       Original by Curtis Wensley (MIT)
                       github.com/verta1878/fpc264irc/examples/pablodraw

  BinkleyTerm XE: 99/99 DOS files compile under Watcom (100%)


FRONTEND MAILER
---------------

    PabloDraw Pascal   sysop/0 — full rewrite of PabloDraw (414 C#
                       files → 20 Pascal units, 4,460 lines)
                       Original by Curtis Wensley (MIT)
                       github.com/verta1878/fpc264irc/examples/pablodraw

  BinkleyTerm XE included as the FidoNet frontend mailer.
  Full protocol support: EMSI, Hydra, Janus, ZModem, YooHoo.
  Compiles 100% under OpenWatcom 2.0 for DOS.

  QFront 1.20a freeware binaries also included as alternative.


COMPATIBILITY
-------------

  - All existing PCBOARD.DAT configurations work unchanged
  - All existing DIR listing files work unchanged  
  - All existing user records work unchanged
  - All existing message bases work unchanged
  - All 5,703 PPEs continue to function
  - All existing doors and external programs work
  - New .SRC files created on demand, not required


KNOWN ISSUES
------------

  - 14 source files remain to be ported (C++ class structure,
    UUCP Borland conio deps, pure inline ASM, code fragments)
  - FTP server and web server are configuration-only in this
    release (actual server daemons are external)
  - File source tracking display in PCBSETUP/PCBSM planned
    for future release



  PCBDRAW — ANSI Art Editor (based on CIADraw)
  ──────────────────────────────────────────────
  PCBoard-native ANSI art editor based on CIADraw by CiA/Strider
  (1994-1996), ported to Free Pascal by sysop/0 (fpc264irc).

  Extends CIADraw with:
    - PCBoard @X color code output (.PCB display files)
    - PCBoard display tags (@CLS@, @DELAY:n@, @PAUSE@, @MORE@)
    - Frame-based animation support with @DELAY@ timing
    - .PCA animation project format (multi-frame)
    - Saves as .ANS (standard ANSI) or .PCB (PCBoard format)

  Original CIADraw features preserved:
    - TheDraw font support with built-in font editor
    - VGA palette editor with RGB control
    - Mouse support, block operations, box drawing
    - CP437 charset, 16-color CGA palette

  Teleconference mode (TCP client/server):
    - Up to 32 concurrent artists on shared canvas
    - Real-time drawing sync, cursor broadcast, chat
    - Three access levels: Viewer → Editor → Operator
    - Pure Pascal sockets (Linux + DOS with packet driver)
    - Based on sysop/0 PabloDraw Pascal port (pdnet.pas)

  Credits: CiA/Strider (original), sysop/0 (FPC port +
  PabloDraw Pascal port), Curtis Wensley (PabloDraw MIT)
CREDITS
-------

  PCBoard 15.4 source code: Clark Development Company, Inc.
  Fred Clark — original PCBoard author (1983)

  pcbrevival project:
    hexadecimal        Project lead, OpenWatcom port
    verta1878          netmodem2irc, OpenOLMS
    wrench             netmodem2irc engine, OpenWatcom dev
    sysop/0            fpc264irc compiler, CIADraw FPC port,
                       PabloDraw Pascal port (20 units, 4,460 lines,
                       utrayit system tray class (Win2k-Win11 + Unix + DOS stubs)
                       9 format parsers, TCP teleconference)
    kiddo/evga         Mystic/RIPscrip/RIPView
    evga               Mystic/RIPView/display

  Third-party tools included by reference:
    PPLD 3.20          astuder (Adrian Studer) — PPE decompiler
                       Original 1994, public domain
                       github.com/astuder/ppld

    PPLEngine          mkrueger (Mike Krueger) — PPE decompiler,
                       compiler, runner, LSP (Rust rewrite)
                       github.com/mkrueger/PPLEngine

    PabloDraw Pascal   sysop/0 — full rewrite of PabloDraw (414 C#
                       files → 20 Pascal units, 4,460 lines)
                       Original by Curtis Wensley (MIT)
                       github.com/verta1878/fpc264irc/examples/pablodraw

    BinkleyTerm XE     Vince Perriello, Bob Hartman, et al.
                       FidoNet mailer

  PCBoard TUI libraries (SCREEN.LIB, SCRNIO.LIB):
    Clark Development Company — used for pcbis installer UI

  CodeBase library:     Sequiter Software Inc. (LGPL v3.0)

