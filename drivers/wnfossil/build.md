# WinFOSSIL Build Guide (v2.0.0 — Unified Source)

## Prerequisites
- MinGW-w64 cross-compiler (i686 + x86_64)
- Wine (for testing)
- For the Win95 VxD: Windows 95 DDK + MASM 6.11 (or DSEO for signing)

## Architecture

Since v2.0.0 all Windows targets build from **one** entry-point source,
`src/dll/wf_dll_unified.c`, selected by a `-DWF_TARGET_*` flag. The core
engine (`src/core/`) is identical across every platform.

```
                        src/dll/wf_dll_unified.c
                                  |
        +----------------+--------+--------+----------------+
        |                |                 |                |
  WF_TARGET_MODERN  WF_TARGET_WIN98  WF_TARGET_VDD   (VxD companion)
   XP-Win11 DLL      Win9x DLL        NT4/2000 VDD    Option B bridge
   (i386 + x64)      (ANSI)           (+ wf_vdd.c)    (DeviceIoControl)
```

Two integration paths, both supported:

- **Path A (ring-3):** the unified DLL provides FOSSIL for every platform,
  including Win98. No kernel driver required.
- **Path B (ring-0):** on Win98 the classic `src/vxd/FOSSIL.ASM` handles
  INT 14h at ring-0 for direct hardware I/O, and forwards VMODEM/TCP work
  to the ring-3 DLL via `DeviceIoControl` (Section 4 of the unified file).
  Use **DSEO** (`dseo13b.exe`) to sign the VxD / enable test-signing on x64.

## Source Layout
```
src/core/wf_core.h          API + types + platform callbacks
src/core/wf_core.c          FOSSIL engine + VMODEM + perf + telnet filter
src/core/registry_compat.c  Runtime Win version detect + registry
src/core/comport_compat.c   COM port (sync 9x / overlapped NT+)
src/core/tcp_compat.c       Winsock TCP/VMODEM transport
src/core/thread_compat.c    Threads, critsec, events, logging
src/dll/wf_dll_unified.c    UNIFIED entry point - all Windows targets
src/dll/fossil.def          Clean export names (undecorated comm*)
src/nt/wf_vdd.c             NT VDD INT 14h dispatch (links with unified)
src/nt/wf_tsr_nt.c          NT TSR loader (WNFOSSIL.EXE for NT)
src/modern/wf_tray.c        System tray app (WNFOSSIL.EXE v2.0)
src/cpl/wf_cpl.c            Control Panel applet (WNFOSSIL.CPL)
src/ctl/wf_ctl.c            CLI utility (WNFOSCTL.EXE)
src/test/wf_test.c          50-test suite
src/vxd/FOSSIL.ASM          Win95 VxD (ring-0, Option B)
src/res/wnfossil.rc         Version info + icons
src/docs/WNFOSSIL.html      Help documentation
```

## Build Commands

Shared definitions:
```
CFLAGS="-O2 -Isrc/core"
CORE="src/core/wf_core.c src/core/registry_compat.c \
      src/core/comport_compat.c src/core/tcp_compat.c \
      src/core/thread_compat.c"
```

### Modern i386 (Win98-Win11, 32-bit) - Path A
```
i686-w64-mingw32-gcc $CFLAGS -DWF_TARGET_MODERN -shared -o FOSSIL.DLL \
    src/dll/wf_dll_unified.c $CORE src/dll/fossil.def \
    -lws2_32 -ladvapi32
```
The `fossil.def` is required - it exports the clean `commOpenPort` names
that BBS software links against (mingw would otherwise emit
`commOpenPort@4`).

### Modern x64 (Win7-Win11, 64-bit) - Path A
```
x86_64-w64-mingw32-gcc $CFLAGS -DWF_TARGET_MODERN -shared -o FOSSIL.DLL \
    src/dll/wf_dll_unified.c $CORE -Wl,--kill-at \
    -lws2_32 -ladvapi32
```
x64 has no `@N` decoration; `--kill-at` (or an x64 def) keeps names clean.

### Win98/ME (ANSI, Path A standalone)
```
i686-w64-mingw32-gcc $CFLAGS -DWF_TARGET_WIN98 -shared -o FOSSIL.DLL \
    src/dll/wf_dll_unified.c $CORE src/dll/fossil.def \
    -lws2_32 -ladvapi32
```
For Path B (VxD + DLL companion) build the DLL as above AND assemble
`src/vxd/FOSSIL.ASM` with the Win95 DDK. The DLL auto-detects the VxD
via `commVxDDetect()` and falls back to standalone if absent.

### NT VDD (NT4/2000 - i386 only) - Path A
```
i686-w64-mingw32-gcc $CFLAGS -DWF_TARGET_VDD -shared -o FOSSIL.DLL \
    src/dll/wf_dll_unified.c src/nt/wf_vdd.c $CORE src/dll/fossil.def \
    -lws2_32 -ladvapi32
i686-w64-mingw32-gcc $CFLAGS -o WNFOSSIL.EXE \
    src/nt/wf_tsr_nt.c
```
The VDD build links the unified DLL (for `comm*` exports and the shared
port table via `wf_dll_get_ports`) together with `wf_vdd.c` (which
provides `VDDInitialize` / `VDDRegisterInit` / `VDDI14Dispatch`).
`vddsvc.h` and `vdmdbg.lib` from the NT DDK are needed for the final
link on a real NT toolchain.

### Utilities (all platforms)
```
i686-w64-mingw32-windres src/res/wnfossil.rc -o res.o
i686-w64-mingw32-gcc $CFLAGS -mwindows -o WNFOSSIL.EXE \
    src/modern/wf_tray.c $CORE res.o \
    -lws2_32 -ladvapi32 -lshell32 -lcomctl32
i686-w64-mingw32-gcc $CFLAGS -o WNFOSCTL.EXE \
    src/ctl/wf_ctl.c src/core/registry_compat.c res.o -ladvapi32
i686-w64-mingw32-windres src/res/wnfossil_cpl.rc -o cpl.o
i686-w64-mingw32-gcc $CFLAGS -shared -o WNFOSSIL.CPL \
    src/cpl/wf_cpl.c src/core/registry_compat.c cpl.o \
    -ladvapi32 -lcomctl32
```

### Win95 VxD (Path B, requires Win95 DDK)
See `out/installer/win98/BUILD_VXD.TXT`. Sign with `dseo13b.exe` for
loading on 64-bit test-signing hosts.

### Test Suite
```
i686-w64-mingw32-gcc $CFLAGS -DWF_TARGET_MODERN -o wf_test.exe \
    src/test/wf_test.c $CORE -lws2_32 -ladvapi32
cp FOSSIL.DLL .          # test loads it via LoadLibrary
wine wf_test.exe         # expect: 50/50 passed
```

## Output
```
out/installer/modern_i386/   v2.0 32-bit release  (Path A)
out/installer/modern_x64/    v2.0 64-bit release  (Path A)
out/installer/nt/            NT4/2000 release      (Path A VDD)
out/installer/win98/         Win98 release         (Path A or A+B)
```

## Verification Status (2026-08-19)
- Modern i386 DLL: builds clean, 50/50 tests pass in Wine
- Win98 DLL: builds clean
- x64 DLL: builds clean
- NT VDD: unified compiles clean in VDD mode (final link needs NT DDK)
- All 26 FOSSIL INT 14h functions implemented (incl. keyboard/screen)
- WF-1 ... WF-15 all fixed
