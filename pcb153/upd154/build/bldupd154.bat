@echo off
rem ============================================================
rem  bldupd154.bat - build the 15.4 pwa upgrade (clark's 15.4)
rem  source:  pcb153\upd154\source  (reconstructed from binaries)
rem  compiler: borland c++ 3.1 (same as 15.3 pwa base)
rem  output:  out\pwa153\upd154
rem ============================================================
echo.
echo building 15.4 pwa upgrade (upd154) with borland c++ 3.1
echo.
echo   note: the reconstructed source needs the build-fix pass
echo   before it compiles 100%% (stats control obj; doors.c udata
echo   header resolution - see ..\readme.md). this script is the
echo   scaffold: it sets the toolchain + paths and targets
echo   out\pwa153\upd154, ready for those fixes.
echo.
set path=c:\bc31\bin;%path%
set src=c:\pcb153\upd154\source
set outdir=c:\out\pwa153\upd154
if not exist %outdir% md %outdir%
rem  compile pattern (per module, mirrors bldkbc):
rem    bcc.exe -c -p -mm -dpcb152 -dlib -dcomm ^
rem      -ic:\bc31\include -ic:\bc31\include\sys ^
rem      -ic:\toolkit\pwa154\h -ic:\pcb153\upd154\source\h ^
rem      -n%outdir% <module>.c
rem  (medium model = what pcboard ships; see main\build for the
rem   full per-module driver once the build-fixes are in.)
echo scaffold only - see pcb153\upd154\build\readme.md
echo.
