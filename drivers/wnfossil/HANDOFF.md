WINFOSSIL MAINTAINER HANDOFF
============================
To: wrench
From: evga + sysop/0
Date: 2026-08-18

Project: WinFOSSIL v2.0.0 — Modern FOSSIL Driver
License: GPLv3
Repo base: github.com/verta1878/netmodem2irc (upstream GPLv3 source)


WHAT THIS IS
------------
Clean-room recreation of Bryan Woodruff's WinFOSSIL (1996).
FOSSIL driver (FTS-0017) for BBS software on Windows.
Four self-contained releases from one shared codebase:

  v1.12 Win98    out/installer/win98/       VxD (FOSSIL.ASM)
  v1.0  NT       out/installer/nt/          VDD (wf_vdd.c)
  v2.0  i386     out/installer/modern_i386/ Native DLL
  v2.0  x64      out/installer/modern_x64/  Native DLL


ARCHITECTURE
------------
One core engine, three platform wrappers.

  src/core/wf_core.h         API + types + 28 platform callbacks
  src/core/wf_core.c         FOSSIL API Fn 00h-1Bh + VMODEM AT parser
  src/core/registry_compat.c Runtime Windows version detect + registry I/O
  src/core/comport_compat.c  COM port (sync on 9x, overlapped on NT+)
  src/core/tcp_compat.c      Winsock TCP for VMODEM telnet
  src/core/thread_compat.c   Threads, critsec, events, log with rotation

Platform wrappers implement 28 wfp_* callbacks:

  src/modern/wf_dll.c        FOSSIL.DLL — 30 comm* exports
  src/modern/wf_tray.c       WNFOSSIL.EXE — system tray app
  src/nt/wf_vdd.c            FOSSIL.DLL — NT VDD, INT 14h dispatch
  src/nt/wf_tsr_nt.c         WNFOSSIL.EXE — AUTOEXEC.NT loader
  src/vxd/FOSSIL.ASM         FOSSIL.VXD — Win95 ring-0 driver

Shared utilities:

  src/cpl/wf_cpl.c           WNFOSSIL.CPL — Control Panel applet
  src/ctl/wf_ctl.c           WNFOSCTL.EXE — CLI port control
  src/test/wf_test.c         50-test suite (runs under Wine)


BUILD
-----
Prerequisites: MinGW-w64 cross-compiler (i686 + x86_64), Wine for testing.

  CFLAGS="-O2 -Isrc/core"
  CORE="src/core/wf_core.c src/core/registry_compat.c
        src/core/comport_compat.c src/core/tcp_compat.c
        src/core/thread_compat.c"

  # Modern i386
  i686-w64-mingw32-gcc -shared -o FOSSIL.DLL src/modern/wf_dll.c $CORE -lws2_32 -ladvapi32
  i686-w64-mingw32-gcc -mwindows -o WNFOSSIL.EXE src/modern/wf_tray.c $CORE -lws2_32 -ladvapi32 -lshell32
  i686-w64-mingw32-gcc -o WNFOSCTL.EXE src/ctl/wf_ctl.c src/core/registry_compat.c -ladvapi32
  i686-w64-mingw32-gcc -shared -o WNFOSSIL.CPL src/cpl/wf_cpl.c src/core/registry_compat.c -ladvapi32 -lcomctl32

  # Modern x64 — same with x86_64-w64-mingw32-gcc
  # NT VDD — i686 only, use src/nt/wf_vdd.c instead of wf_dll.c
  # Win98 VxD — needs Win95 DDK MASM, see BUILD_VXD.TXT

  # Test
  i686-w64-mingw32-gcc -o wf_test.exe src/test/wf_test.c $CORE -lws2_32 -ladvapi32
  wine wf_test.exe


REGISTRY PATHS
--------------
Runtime detection — one binary handles all:

  Win95/98/ME:  HKLM\System\CurrentControlSet\Services\VxD\FOSSIL
  NT4/2000:     HKLM\Software\Woodruff\WinFOSSIL
  XP-Win11:     HKLM\SOFTWARE\WinFOSSIL
  Security:     HKLM\SOFTWARE\WinFOSSIL\Security (v2.0 only)

registry_compat.c detects at startup. Wrappers call wfp_reg_base_key()
and never hardcode a path.


COM PORT DIFFERENCES
--------------------
comport_compat.c handles:

  Win98:  Synchronous I/O, "COM1" format, fAbortOnError=FALSE by default
  NT+:    Overlapped I/O, "\\\\.\\COM1" format, fAbortOnError forced FALSE
  USB:    High COM numbers (COM5-COM256) need \\.\\ prefix
  Enum:   Brute force on 9x, QueryDosDevice on NT+


VMODEM AT COMMANDS
------------------
Full Hayes-compatible parser in wf_core.c:

  ATZ ATD ATH ATE ATS0= ATA ATI AT&D AT&F A/ +++

Telnet IAC filtering with persistent state across buffer boundaries.
Accepts SGA (opt 3) and BINARY (opt 0), refuses everything else.
TCP keepalive enabled. Non-blocking sockets.


BUGS FIXED (22 total)
---------------------
Three audit rounds. Every bug has a WF-N or MF-N tag in the source.

  HIGH:
    WF-1  strncpy not inside else braces
    WF-2  + bytes consumed by +++ detector, never forwarded

  MEDIUM:
    WF-3  Telnet subneg truncation at buffer boundary
    WF-4  VDD Fn 18h/19h DOS memory copy was TODO stub
    WF-5  IP prefix match too loose (no octet boundary check)
    WF-8  Lone 0xFF at buffer end passed through raw
    WF-9  wf_vm_engine called tcp_write inside critical section
    WF-10 Trailing + chars lost on idle timeout

  LOW:
    WF-6  wfp_log init race (InterlockedCompareExchange fix)
    WF-7  GetTickCount 49-day wrap (underflow detection)
    WF-11 wf_open_com didn't check for existing threads
    WF-12 BUILD.md referenced wrong filenames

  Plus 10 earlier bugs from evga's first audit round.

  MISSING FEATURES ADDED:
    MF-1  WSACleanup on DLL_PROCESS_DETACH
    MF-3  A/ (repeat last AT command)
    MF-4  AT&F (factory defaults)
    MF-5  Telnet SGA+BINARY accepted instead of refused
    MF-2  VxD ships as separate ASM (by design)


WHAT STILL NEEDS WORK
---------------------
1. Win98 VxD binary — FOSSIL.ASM adapted, needs DDK MASM build.
   Source is ready, just needs the toolchain.

2. InnoSetup installer — .iss script exists in src/setup/ but
   needs InnoSetup compiler to produce SETUP.EXE.

3. WinHelp files — WNFOSSIL.HLP + .CNT. Original was a WinHelp
   file. Could recreate as HTML help or CHM.

4. CPL dialog resources — wf_cpl.c uses DialogBoxA with "IDD_PORTCFG"
   but no .rc resource file for the dialog template. Currently uses
   runtime-created controls. A proper .dlg/.rc would be cleaner.

5. Real hardware testing — Wine tests pass but real COM port +
   real modem + real BBS software testing needed.

6. DOSBox integration — v2.0 works without NTVDM. Could add
   DOSBox serial port passthrough documentation.

7. Icons — using netmodem2irc icons (mainicon.ico, comports.ico).
   Could create WinFOSSIL-specific icons.


TESTING
-------
  wine wf_test.exe → 50/50 pass

  T01-T05  Ring buffer
  T06-T10  Baud encode/decode
  T11-T15  Port lifecycle
  T16-T25  FOSSIL API (all Fn 00h-1Bh)
  T26-T30  VMODEM AT parser
  T31-T35  Registry read/write/platform detect
  T36-T39  COM port open/close/status
  T40-T44  DLL export verification
  T45-T48  Access security (whitelist/blacklist)
  T49-T50  Performance counters

  Test cleans registry before run (RegDeleteKeyA) so results
  are consistent across runs.


FILES IN EACH RELEASE
---------------------
  win98 (12):   FOSSIL.ASM FOSSIL.INC FOSSIL.INF INSTALL.INF
                WNFOSCTL.EXE WNFOSSIL.CPL BUILD_VXD.TXT
                LICENSE.TXT README.TXT WHATSNEW.TXT FILE_ID.DIZ

  nt (12):      FOSSIL.DLL WNFOSSIL.EXE WNFOSCTL.EXE WNFOSSIL.CPL
                FOSSIL.INF INSTALL.INF BUGS.TXT VMODEM.TXT
                LICENSE.TXT README.TXT WHATSNEW.TXT FILE_ID.DIZ

  modern (10):  FOSSIL.DLL WNFOSSIL.EXE WNFOSCTL.EXE WNFOSSIL.CPL
                SETUP.ISS VMODEM.TXT LICENSE.TXT README.TXT
                WHATSNEW.TXT FILE_ID.DIZ


LINE COUNTS
-----------
  wf_core.c           1,285   Core engine + VMODEM
  comport_compat.c       568   COM port across all platforms
  registry_compat.c      493   Runtime platform detection
  wf_vdd.c               461   NT VDD INT 14h dispatch
  wf_core.h               424   API header
  wf_test.c               240   Test suite
  wf_dll.c                247   Modern DLL exports
  tcp_compat.c            235   Winsock TCP
  thread_compat.c         192   Thread/sync/log
  wf_ctl.c                120   CLI utility
  wf_cpl.c                115   Control Panel applet
  wf_tray.c               110   System tray app
  wf_tsr_nt.c              93   NT TSR loader
  FOSSIL.ASM            5,712   Win95 VxD (adapted from netmodem2irc)
  FOSSIL.INC              175   VxD include
                       ------
  Total               10,470 lines


UPSTREAM
--------
  netmodem2irc: github.com/verta1878/netmodem2irc (GPLv3)
    VxD source (NETMODEM.ASM → FOSSIL.ASM)
    Icons (config/resources/*.ico)
    CPL reference (cpl/NetModemCPL.pas)
    Registry docs (docs/netmodem2irc_registry.md)

  fpc264irc: github.com/verta1878/fpc264irc (GPL)
    Compiler toolchain for Pascal components
    win32compat.pas pattern used in registry_compat.c

  Original WinFOSSIL archives (included in source zip):
    wnfos112/     v1.12 for Win95 (16 files)
    wntfos10b3/   v1.0 Beta 3 for NT (18 files)
    wnfosnt/      NT variant
    wnfos112key/  Registration key utility


CONTACT
-------
  sysop/0 — project lead
  evga — core engine, all C code, binary analysis, audit fixes
  wrench — maintainer (you)
  verta — upstream repos, fpc264irc toolchain

  The crew built it. 4free. o7
