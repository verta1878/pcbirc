@echo off
REM ============================================================
REM  FOSSIL.VXD build — genuine MASM 6.11d + DDK LINK.EXE
REM  Uses the PRISTINE DDK includes in ..\src\ddk-genuine
REM ============================================================
REM  Requires the Win95/98 DDK: ML.EXE 6.11d + LINK.EXE (-vxd)
REM  NOTE: MASM build must NOT use OPTION NOKEYWORD (that line is
REM  JWasm-only; genuine MASM 6.11 does not know SYSEXIT/VMRESUME
REM  as reserved words, so it is absent from the MASM code path).
REM ============================================================
set DDKINC=..\src\ddk-genuine
set MLFLAGS=-c -coff -DBLD_COFF -DIS_32 -Sg -DMASM6 -W2 -Zd -Zp1 -DNODECOUNT=16

ml %MLFLAGS% -I%DDKINC% -I..\src -Fo FOSSIL.obj ..\src\FOSSIL.ASM
if errorlevel 1 goto error
link -vxd -def:..\src\FOSSIL.DEF -out:FOSSIL.vxd FOSSIL.obj
if errorlevel 1 goto error
echo Build OK.
goto end
:error
echo BUILD FAILED
:end
