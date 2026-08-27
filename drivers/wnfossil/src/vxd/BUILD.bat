@echo off
REM ============================================================
REM  FOSSIL.VXD build script — Windows 95/98/ME Virtual FOSSIL
REM ============================================================
REM  Requires: Windows 95 DDK + MASM 6.11 (ML.EXE) + LINK.EXE
REM            (the 16-bit VxD linker with -vxd support)
REM
REM  The DDK provides VMM.inc, VPICD.inc, VComm.inc, etc. Our
REM  copies in this directory are clean-room reconstructions of
REM  the DDK structures — for a real build, put the genuine DDK
REM  includes on the ML include path FIRST.
REM ============================================================

set DDKINC=%W95DDK%\inc32
set MLFLAGS=-c -coff -DBLD_COFF -DIS_32 -Sg -DMASM6 -W2 -Zd -Zp1

echo Assembling FOSSIL.ASM...
ml %MLFLAGS% -I%DDKINC% -I. -Fo FOSSIL.obj FOSSIL.ASM
if errorlevel 1 goto error

echo Linking FOSSIL.VXD...
link -vxd -def:FOSSIL.DEF -out:FOSSIL.VXD FOSSIL.obj
if errorlevel 1 goto error

echo.
echo FOSSIL.VXD built successfully.
echo Copy to %%WINDIR%%\SYSTEM\ and reference in SYSTEM.INI [386Enh]:
echo   device=FOSSIL.VXD
goto done

:error
echo BUILD FAILED.
exit /b 1

:done
