@echo off
REM ============================================================
REM  BLDUPD154.BAT - build the 15.4 PWA upgrade (Clark's 15.4)
REM  Source:  pcb153\upd154\SOURCE  (reconstructed from binaries)
REM  Compiler: Borland C++ 3.1 (same as 15.3 PWA base)
REM  Output:  OUT\pwa153\upd154
REM ============================================================
echo.
echo Building 15.4 PWA upgrade (upd154) with Borland C++ 3.1
echo.
echo   NOTE: the reconstructed source needs the build-fix pass
echo   before it compiles 100%% (STATS control OBJ; DOORS.C UData
echo   header resolution - see ..\README.md). This script is the
echo   scaffold: it sets the toolchain + paths and targets
echo   OUT\pwa153\upd154, ready for those fixes.
echo.
set PATH=C:\BC31\BIN;%PATH%
set SRC=C:\PCB153\UPD154\SOURCE
set OUTDIR=C:\OUT\PWA153\UPD154
if not exist %OUTDIR% md %OUTDIR%
REM  Compile pattern -- CORRECTED 2026-09-17.
REM
REM  DO NOT use -DLIB here.  The earlier pattern in this file was
REM  "-c -P -mm -DPCB152 -DLIB -DCOMM", copied from BLDKBC -- which
REM  builds the door SDK, not the program.  Under -DLIB, PCBOARD.H
REM  selects the reduced door-visible structs, so members the program
REM  legitimately uses vanish.  That, not any source defect, produced
REM  the "'Stats' undefined" and "UData/TotUpldBytes" errors this file
REM  used to call the build-fix pass.  Verified 2026-09-17: CALLWAIT.C
REM  and DOORS.C both compile clean with Clark's flags below.
REM
REM  Use Clark's own config files, as the working 15.3 build does:
REM    bcc +C:\TOOLKIT\PWA154\CFG\BC31\PCBOARD.CFG ^
REM        @C:\TOOLKIT\PWA154\CFG\BC31\ALL.RES ^
REM        -IC:\PCB153\UPD154\SOURCE\H -n%OUTDIR% <module>.C
REM  plus what MAIN\COMPILE.BAT adds: -DSTATS -DMP -D386 -DDBASE
REM  -DFIDO, and NODES=25.
REM
REM  Large model (-ml) is what PCBOARD.MAK uses.  See
REM  OUT\pwa153\BUILD-RECIPE.md for the 15.3 build that ran end to end.
echo Scaffold only - see pcb153\upd154\build\README.md
echo.
