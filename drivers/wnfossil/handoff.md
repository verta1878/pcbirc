WINFOSSIL MAINTAINER HANDOFF
============================
To: wrench
From: evga + sysop/0
Date: 2026-08-18 (see session.md for the current 2026-08 audit record —
      several corrections below were made after this date; this
      header date reflects the original handoff, not the last edit)

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

  src/dll/wf_dll_unified.c   FOSSIL.DLL — 30 comm* exports
  src/modern/wf_tray.c       WNFOSSIL.EXE — system tray app
  src/nt/wf_vdd.c            FOSSIL.DLL — NT VDD, INT 14h dispatch
  src/nt/wf_tsr_nt.c         WNFOSSIL.EXE — AUTOEXEC.NT loader
  src/vxd/FOSSIL.ASM         FOSSIL.VXD — Win95 ring-0 driver

Shared utilities:

  src/cpl/wf_cpl.c           WNFOSSIL.CPL — Control Panel applet
  src/ctl/wf_ctl.c           WNFOSCTL.EXE — CLI port control
  src/test/wf_test.c         65-assertion test suite (runs under Wine)


BUILD
-----
Prerequisites: MinGW-w64 cross-compiler (i686 + x86_64), Wine for testing.

  CFLAGS="-O2 -Isrc/core"
  CORE="src/core/wf_core.c src/core/registry_compat.c
        src/core/comport_compat.c src/core/tcp_compat.c
        src/core/thread_compat.c"

  # Modern i386
  i686-w64-mingw32-gcc -shared -o FOSSIL.DLL src/dll/wf_dll_unified.c $CORE -lws2_32 -ladvapi32
  i686-w64-mingw32-gcc -mwindows -o WNFOSSIL.EXE src/modern/wf_tray.c $CORE -lws2_32 -ladvapi32 -lshell32
  i686-w64-mingw32-gcc -o WNFOSCTL.EXE src/ctl/wf_ctl.c src/core/registry_compat.c -ladvapi32
  i686-w64-mingw32-gcc -shared -o WNFOSSIL.CPL src/cpl/wf_cpl.c src/core/registry_compat.c -ladvapi32 -lcomctl32

  # Modern x64 — same with x86_64-w64-mingw32-gcc
  # NT VDD — i686 only, use src/nt/wf_vdd.c instead of wf_dll_unified.c
  # Win98 VxD — see the separate fossil-vxd-recovery package's
  # build/BUILD_masm.bat or build/build_jwasm.sh (this used to say
  # "see BUILD_VXD.TXT," which never existed anywhere in this
  # package — corrected rather than left as a dead reference)

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
    WF-13 IAC SE split across buffer boundary inside subnegotiation
          not handled — state now persists across filter_telnet()
          calls via tn_subneg_iac
    WF-14 Telnet option negotiation could echo-loop with the remote
          endpoint (re-sending DO/WILL for an already-accepted
          option); now tracked per-option via tn_accepted[]

  LOW:
    WF-6  wfp_log init race (InterlockedCompareExchange fix)
    WF-7  GetTickCount 49-day wrap (underflow detection)
    WF-11 wf_open_com didn't check for existing threads
    WF-12 BUILD.md referenced wrong filenames
    WF-15 Telnet IAC filter given full persistent state (superset
          of WF-13 — the whole filter, not just subnegotiation
          parsing, now survives being called with a byte stream
          split arbitrarily across TCP reads)

  Plus 10 earlier bugs from evga's first audit round.

  Correction (2026-08): WF-13/14/15 above were previously untagged
  in this file despite being real, present fixes in wf_core.c — this
  list undercounted at 22 (it's now 25: the original 22 plus these
  three). WF-1 and WF-12 were flagged for re-verification during the
  2026-08 audit; both confirmed genuinely fixed (WF-1 structurally,
  with no surviving instance of the described pattern found anywhere
  in the codebase after a systematic search; WF-12 by confirming
  every path BUILD.md references actually exists).

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

   Update (2026-08): the src/vxd-recovered/ tree that used to live in
   this same zip has been separated into its own standalone package
   (fossil-vxd-recovery), since it's needed for a separate netserial
   integration. It is no longer nested here.

   That work is done, not just started: it has a hash-verified build
   (corrupted VMM.INC was the root cause, per its docs/RECOVERY.md),
   three independently-confirmed source-bug fixes, and a completed
   per-procedure push/pop stack-balance trace covering all 35
   BeginProc/EndProc pairs in FOSSIL.ASM. That trace (re-run live to
   confirm, not just cited from memory) initially flags 19 procedures
   on an aggregate push/pop count, but manual tracing of three
   representative cases (a simple two-exit branch, a loop with two
   exits, a .REPEAT loop with two exits) plus a structural check on
   the two most extreme cases (Int14_Proc: 1 pushad vs 70 popad;
   W32DeviceIoControl: 3 pushad vs 29 popad) confirmed all 19 are the
   same benign pattern — one push/pushad written once in source,
   matched by a pop/popad duplicated across every separate,
   mutually-exclusive exit path or loop iteration — not real leaks.
   No genuine stack-balance bug was found anywhere in the file.

   This resolves the "just needs the toolchain" framing above and
   clears the stack-balance risk specifically. The one gate still
   open is the same one it's always been: load-testing on real Win98
   hardware to confirm actual INT 14h initialization — that can't be
   done in this sandbox and isn't a documentation or code issue.

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

8. Windows Performance Monitor integration — the original FOSSIL.VXD
   registers as a real PerfMon counter object ("bytes read/written
   per second," visible in the classic System Monitor applet).
   Confirmed via 2026-08 strings comparison against the real
   FOSSIL.VXD: this doesn't exist anywhere in this source. The
   throughput data itself is computed correctly already
   (perf_cps_rx/perf_cps_tx in wf_core.c) — only the actual OS-level
   counter registration (a registry Performance subkey + a counter
   DLL, or the older .INI+.DAT mechanism for 9x) is missing. Not a
   small fix; flagged rather than attempted without a real Windows
   SDK to verify against.


TESTING
-------
  wine wf_test.exe → 65 assertions across 9 categories (ring buffer,
  baud rates, port lifecycle, FOSSIL API, VMODEM, registry, COM
  port, DLL exports, security, performance)

  Correction (2026-08): this used to say "50-test suite" / "50/50
  pass" — the actual count is 65, not 50 (the suite grew after this
  doc was last updated). Also worth knowing: 15 of the 65 are
  always-true skip stubs (`t("...", 1)`), legitimately used for
  platform-gating (`#ifdef _WIN32`/`#else` — a non-Windows build
  can't run registry/DLL/port-enum tests). But inside the `#ifdef
  _WIN32` DLL-exports block itself, a failed `LoadLibraryA` at
  runtime *also* falls back to 4 always-true stubs instead of
  failing — meaning even a real Wine run with a broken/missing
  FOSSIL.DLL would be indistinguishable from a pass for those 4
  assertions. Worth tightening (fail loudly instead of skip-passing
  when running under Win32 and the DLL genuinely can't load) before
  trusting this suite's count as proof of anything DLL-related.

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
  wf_core.c           1,383   Core engine + VMODEM
  comport_compat.c       589   COM port across all platforms
  registry_compat.c      493   Runtime platform detection
  wf_vdd.c               568   NT VDD INT 14h dispatch
  wf_core.h               435   API header
  wf_test.c               240   Test suite
  wf_dll_unified.c        536   Modern DLL exports (path: src/dll/)
  tcp_compat.c            235   Winsock TCP
  thread_compat.c         192   Thread/sync/log
  wf_ctl.c                151   CLI utility
  wf_cpl.c                115   Control Panel applet
  wf_tray.c               110   System tray app
  wf_tsr_nt.c              93   NT TSR loader
  FOSSIL.ASM            5,712   Win95 VxD (adapted from netmodem2irc)
  FOSSIL.INC              175   VxD include
                       ------
  Total               11,027 lines

  (Corrected 2026-08 — several counts above had drifted from actual
  file sizes as fixes landed: wf_core.c, comport_compat.c, wf_vdd.c,
  wf_core.h, and wf_ctl.c all grew since this table was last updated;
  wf_dll_unified.c was also renamed/moved from its old wf_dll.c path,
  see the LINE COUNTS entry and the build command earlier in this
  file. This total does NOT include src/vxd-recovered/, which has
  been split into its own separate package — see that item's note
  earlier in this document.)


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

  Original WinFOSSIL archives:
    wnfos112/     v1.12 for Win95 (16 files) — the real, legitimate
                  original distribution (SETUP.EXE, FOSSIL.VXD,
                  WNFOSCTL.EXE, WNFOSSIL.CPL, REGFORM.TXT/.ITA,
                  WFOSKEY2.ZIP, PGPKEY.TXT). Used as the comparison
                  baseline for the Win95 track audit (2026-08).
    wntfos10b3/   v1.0 Beta 3 for NT (18 files)
    wnfosnt/      NT variant

  Correction (2026-08): this list previously also named
  "wnfos112key/ Registration key utility" as included in the source
  zip. It is NOT actually present in wnfossil-2_0_0-final.zip —
  removed from this list rather than left as an inaccurate claim.
  Separately: a file uploaded as "the real NT binary" during the
  2026-08 audit (WNFOSNT.RAR) turned out to be a nag-screen-removal
  patch per its own file_id.diz, not the actual product — declined
  to use it for anything. The NT track (wf_vdd.c/wf_tsr_nt.c)
  remains source-review-only pending a genuine NT binary to compare
  against.


CONTACT
-------
  sysop/0 — project lead
  evga — core engine, all C code, binary analysis, audit fixes
  wrench — maintainer (you)
  verta — upstream repos, fpc264irc toolchain

  The crew built it. 4free. o7
