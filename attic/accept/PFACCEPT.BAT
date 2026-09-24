@ECHO OFF
REM ===========================================================================
REM PFACCEPT.BAT - the PACKFIDO acceptance test.
REM
REM Runs BOTH Clark's shipped PACKFIDO.EXE and our reconstruction over
REM identical copies of the same file and compares the two results byte for
REM byte.  PASS means they produced the same bytes.  Nothing less counts -
REM "it compiles" is not the test.
REM
REM     PFACCEPT              run in \PFTEST
REM     PFACCEPT \MYDIR       run in \MYDIR instead
REM
REM The three files in this directory:
REM     MKPFDATA.C   makes the test data - a version 2 PCBFIDO.CFG, ORIG.CFG,
REM                  CNAMES.@@@, CNAMES.ADD and a PCBOARD.DAT
REM     FILEOPS.C    copy and byte-compare that set ERRORLEVEL
REM     PFACCEPT.BAT this - runs the two programs and asks FILEOPS /D
REM
REM Build the three EXEs first:
REM     BCC -ml -ePACKFIDO.EXE PACKFIDO.C      (in the parent directory)
REM     BCC -ml -eMKPFDATA.EXE MKPFDATA.C
REM     BCC -ml -eFILEOPS.EXE  FILEOPS.C
REM
REM ---------------------------------------------------------------------------
REM WHY THERE ARE NO ..\..\ PATHS HERE
REM
REM This repo's DOS scripts assume the repo folder is MOUNTED AS THE DRIVE
REM ROOT, the same as BLDTK.BAT, and they check for \APPLY.txt to prove it.
REM So every path below starts at \ and this file works from any directory,
REM on any drive letter, under any mount name.  A relative walk like
REM ..\..\..\..\.. works from one exact directory and breaks the moment the
REM file is moved or the tree is reorganised - and it passes its own test,
REM because you ran the test from that one directory.
REM
REM     DOSBox-X:  mount c <the pcbircrevival folder>
REM     real DOS:  SUBST, or a drive whose root IS the repo folder
REM
REM ---------------------------------------------------------------------------
REM WHY IT DOES NOT RUN IN THIS DIRECTORY
REM
REM PcbData.FidoConfig is char[33] and PcbData.CnfFile is char[32], so the
REM paths PCBOARD.DAT carries have 32 and 31 usable characters.
REM
REM     \PCB153\SOURCE\MISC\PACKFIDO\ACCEPT\PCBFIDO.CFG    47 characters
REM
REM Clark's binary would truncate that, open nothing, print "done." and
REM change no file - a silent pass proving nothing.  The test therefore runs
REM in a SHORT directory, \PFTEST by default, which this script creates and
REM leaves in place so the outputs can be inspected.  MKPFDATA refuses, with
REM the arithmetic printed, if you point it somewhere too long.  Nothing in
REM the source tree is written to or deleted.
REM
REM ---------------------------------------------------------------------------
REM WHY THERE IS NO COPY, REN, DEL, FC OR REDIRECTION
REM
REM Measured under DOSBox-X: COPY works from the autoexec and SILENTLY DOES
REM NOTHING from inside a batch file - no error, no file, ERRORLEVEL
REM unchanged.  A harness built on it compares whatever the previous step
REM left lying around and reports PASS for the wrong reason.  FC is an
REM external command that is not always present.  So FILEOPS.EXE does the
REM copying and the comparing, and this script uses nothing but EXE calls,
REM IF ERRORLEVEL, IF EXIST, CD and ECHO.
REM
REM One consequence: Clark's PACKFIDO.EXE reads a bare "PCBOARD.DAT" from the
REM CURRENT directory, so this script has to CD into the test directory and
REM cannot change back - COMMAND.COM has nowhere to remember the old one.
REM You are left in the test directory when it finishes.
REM ===========================================================================

IF NOT EXIST \APPLY.txt GOTO NOROOT

SET PFDIR=%1
IF "%PFDIR%"=="" SET PFDIR=\PFTEST

SET PFCLARK=\pcb1541\install\dist\target\PACKFIDO.EXE
SET PFOURS=\pcb153\SOURCE\MISC\PACKFIDO\PACKFIDO.EXE
SET PFMK=\pcb153\SOURCE\MISC\PACKFIDO\ACCEPT\MKPFDATA.EXE
SET PFOPS=\pcb153\SOURCE\MISC\PACKFIDO\ACCEPT\FILEOPS.EXE
SET PFDAT=\pcb153\SOURCE\MISC\PACKFIDO\EXAMPLES\PCBOARD.DAT

IF NOT EXIST %PFCLARK% GOTO NOCLARK
IF NOT EXIST %PFOURS%  GOTO NOOURS
IF NOT EXIST %PFMK%    GOTO NOMK
IF NOT EXIST %PFOPS%   GOTO NOOPS
IF NOT EXIST %PFDAT%   GOTO NODAT

IF NOT EXIST %PFDIR%\NUL MKDIR %PFDIR%
IF NOT EXIST %PFDIR%\NUL GOTO NODIR

ECHO.
ECHO --- making the test data in %PFDIR% --------------------------------------
%PFMK% %PFDIR% %PFDAT%
IF ERRORLEVEL 1 GOTO FAILED

CD %PFDIR%
IF NOT EXIST PCBFIDO.CFG GOTO FAILED
IF NOT EXIST ORIG.CFG GOTO FAILED

ECHO.
ECHO --- Clark's PACKFIDO.EXE -------------------------------------------------
%PFCLARK%
IF NOT EXIST PCBFIDO.CFG GOTO FAILED
%PFOPS% /C PCBFIDO.CFG CLARK.OUT
IF ERRORLEVEL 1 GOTO FAILED

ECHO.
ECHO --- our PACKFIDO.EXE -----------------------------------------------------
%PFOPS% /C ORIG.CFG PCBFIDO.CFG
IF ERRORLEVEL 1 GOTO FAILED
%PFOURS% PCBFIDO.CFG CNAMES 800
IF ERRORLEVEL 1 GOTO FAILED
%PFOPS% /C PCBFIDO.CFG OURS.OUT
IF ERRORLEVEL 1 GOTO FAILED

ECHO.
ECHO --- comparing ------------------------------------------------------------
%PFOPS% /D CLARK.OUT OURS.OUT
IF ERRORLEVEL 1 GOTO MISMATCH
ECHO.
ECHO   PASS - byte for byte identical.
ECHO   outputs left in %PFDIR%
GOTO END

:MISMATCH
ECHO.
ECHO   FAIL - the two outputs differ.  Both are in %PFDIR%
GOTO END

:NOROOT
ECHO.
ECHO   \APPLY.txt not found.
ECHO   Mount the repo folder as the drive root first, eg under DOSBox-X:
ECHO       mount c ^<the pcbircrevival folder^>
GOTO END

:NOCLARK
ECHO.
ECHO   Cannot find Clark's PACKFIDO.EXE at
ECHO   %PFCLARK%
ECHO   Without it there is no oracle and no acceptance test.
GOTO END

:NOOURS
ECHO.
ECHO   Cannot find %PFOURS%
ECHO   Build it first:  BCC -ml -ePACKFIDO.EXE PACKFIDO.C
GOTO END

:NOMK
ECHO.
ECHO   Cannot find %PFMK%
ECHO   Build it first:  BCC -ml -eMKPFDATA.EXE MKPFDATA.C
GOTO END

:NOOPS
ECHO.
ECHO   Cannot find %PFOPS%
ECHO   Build it first:  BCC -ml -eFILEOPS.EXE FILEOPS.C
GOTO END

:NODAT
ECHO.
ECHO   Cannot find %PFDAT%
ECHO   The test data copies Clark's own PCBOARD.DAT from EXAMPLES\.
GOTO END

:NODIR
ECHO.
ECHO   Could not create %PFDIR%
GOTO END

:FAILED
ECHO.
ECHO   FAIL - a step did not complete.

:END
SET PFDIR=
SET PFCLARK=
SET PFOURS=
SET PFMK=
SET PFOPS=
SET PFDAT=
