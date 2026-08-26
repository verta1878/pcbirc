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
REM  Compile pattern (per module, mirrors BLDKBC):
REM    BCC.EXE -c -P -mm -DPCB152 -DLIB -DCOMM ^
REM      -IC:\BC31\INCLUDE -IC:\BC31\INCLUDE\SYS ^
REM      -IC:\TOOLKIT\PWA154\H -IC:\PCB153\UPD154\SOURCE\H ^
REM      -n%OUTDIR% <module>.C
REM  (medium model = what PCBoard ships; see MAIN\build for the
REM   full per-module driver once the build-fixes are in.)
echo Scaffold only - see pcb153\upd154\build\README.md
echo.
