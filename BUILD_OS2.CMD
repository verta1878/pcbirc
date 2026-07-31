@echo off
rem ============================================================================
rem  PCBoard 15.4 Source Build -- OS/2 Build Script
rem  Builds PCBOARD2.EXE for OS/2 32-bit flat model
rem
rem  TWO BUILD PATHS:
rem
rem  Option A: Borland C++ 2.0 for OS/2 (BCOS2) -- native OS/2 compiler
rem    Requires: OS/2 Warp 3+, BCOS2 at \BCOS2, CodeBase at \LIBS\CODEBASE
rem    Run this script ON OS/2.
rem
rem  Option B: OpenWatcom cross-compile -- from DOS/Windows/Linux
rem    Requires: OpenWatcom 2.0, OS/2 Toolkit headers, WATCOMPAT.H
rem    Command: wcl386 -bt=os2v2 -mf -I headers... -fo=obj source.c
rem    See BUILD_OS2_OW.SH for the Linux cross-compile script.
rem
rem  Output: \OUT\OS2\PCBOARD2.EXE
rem ============================================================================

if not exist \OUT\OS2 mkdir \OUT\OS2

rem === OPTION A: Borland C++ for OS/2 ===
if not exist \BCOS2\BIN\BCC.EXE goto NO_BCOS2

SET BCROOT=\BCOS2
SET BCPGM=\BCOS2\BIN
SET INCLUDE=\BCOS2\INCLUDE
SET LIBPATH=\BCOS2\LIB
SET COMPILER=\BCOS2\BIN\BCC.EXE
SET LINKER=\BCOS2\BIN\TLINK.EXE
SET MAKE=\BCOS2\BIN\MAKE.EXE
SET BCCOMPILER=bcos2

echo ============================================================================
echo  PCBoard 15.4 OS/2 Build (Borland C++ 2.0 for OS/2)
echo ============================================================================

cd \PROJ\PCB
if not exist OBJ\BCOS2 mkdir OBJ\BCOS2
\BCOS2\BIN\MAKE -f..\153\PCBOARD2.MAK

if exist OBJ\BCOS2\PCBOARD2.EXE (
    copy OBJ\BCOS2\PCBOARD2.EXE \OUT\OS2\PCBOARD2.EXE
    echo Build complete: \OUT\OS2\PCBOARD2.EXE
) else (
    echo Build FAILED.
)
goto END

:NO_BCOS2
echo.
echo BCOS2 not found at \BCOS2. 
echo.
echo To build PCBOARD2.EXE, you need one of:
echo   A) Borland C++ 2.0 for OS/2 running on OS/2
echo   B) OpenWatcom cross-compile -- see BUILD_OS2_OW.SH
echo.
echo The OpenWatcom path works from DOS/Windows/Linux without an OS/2 VM.
echo Source analysis shows: 0 inline asm blocks, 315 __OS2__ conditional
echo blocks, 8 Borland-specific pragmas (wrappable), 0 Borland-only API calls.
echo WATCOMPAT.H in LIB\H provides the Borland-to-Watcom bridge macros.

:END
