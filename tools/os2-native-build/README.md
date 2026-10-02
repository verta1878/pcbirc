# fpc264irc i386-os2 native (-Tos2) unit rebuild — byte, 2026-10-01

Result: 209/209 PPUs rebuilt, all target byte 0x04 (system_i386_OS2). Replaces 180 EMX (0x1C) PPUs.
Same unit set as the repo's bin/units/i386-os2 (unitlist.txt) — no missing, no extras.

Build (Linux host, bin/ppc386 2.6.4):
1. build.sh   — bootstraps system.ppu from src/rtl/os2/system.pas (-Us)
2. drive.py   — iterative build of every other unit until no progress (3-5 passes)
Flags: -Tos2 -Pi386 -s -O2 -Ur -n -FU<out> -Fu<out>, plus -Fi for rtl/inc, rtl/i386, rtl/objpas(+sysutils,classes,unicode), rtl/os2, fv/src,
and per-unit -Fi <srcdir>, <srcdir>/os2, <srcdir>/inc, <srcdir>/../inc.
-Ur (release units) stops the system.pas recompile cascade, so rtl/os2 can be on the include path.

Source choices for ambiguous units (matches repo's existing PPUs):
- unixcp       -> patched/unixcp.pas (copy of rtl/objpas/unicode/unixcp_stub.pas, returns CP437)
- fpwidestring -> patched/fpwidestring.pp (CompareUnicodeStringProc called with 2 params, 2.6.4 signature)
- crc -> packages/hash; resource/dialogs/menus/msgbox/tabs -> packages/fv; rexxsaa -> os2bindings;
  graph -> packages/graph/src/os2 (PM backend); regexpr -> packages/regexpr/src/regexpr.pas
- eventlog/pipes/process/resolve pick up their src/os2/*.inc

Test: graphtest.pas (graph+crt+sysutils+classes+objects+app+dialogs+zipper+fpjson+process) compiles -Tos2 rc=0.
Not linked (no OS/2 linker on host). .o/.s not shipped (build artifacts).
