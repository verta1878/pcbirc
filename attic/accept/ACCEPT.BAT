@ECHO OFF
REM ===========================================================================
REM ACCEPT.BAT - the PACKFIDO acceptance test.
REM
REM Builds a version 2 PCBFIDO.CFG fixture, runs BOTH Clark's shipped
REM PACKFIDO.EXE and our reconstruction over identical copies of it, and
REM compares the two results byte for byte.
REM
REM   ACCEPT              run in \PFTEST
REM   ACCEPT \MYDIR       run in \MYDIR instead
REM
REM PASS means the two programs produced the same bytes.  Nothing less
REM counts - "it compiles" is not the test.
REM
REM ---------------------------------------------------------------------------
REM WHY THERE ARE NO ..\..\ PATHS HERE
REM
REM This repo's DOS scripts assume the repo folder is MOUNTED AS THE DRIVE
REM ROOT, the same as BLDTK.BAT and BLDDOS.BAT, and they check for
REM \APPLY.txt to prove it.  So every path below starts at \ and this file
REM works from any depth, on any drive letter, under any mount name.  A
REM relative walk like ..\..\..\..\.. only works from one exact directory
REM and breaks the moment the file is moved or the tree is reorganised.
REM
REM   DOSBox-X:  mount c <the repo folder>
REM   real DOS:  SUBST or a drive whose root IS the repo folder
REM
REM ---------------------------------------------------------------------------
REM WHY IT DOES NOT RUN IN THIS DIRECTORY
REM
REM PcbData.FidoConfig is char[33] and PcbData.CnfFile is char[32], so the
REM paths PCBOARD.DAT carries have 32 and 31 usable characters.
REM
REM   \PCB153\SOURCE\MISC\PACKFIDO\ACCEPT\PCBFIDO.CFG    47 characters
REM
REM Clark's binary would silently truncate that and then fail to open
REM anything.  The test therefore runs in a SHORT directory - \PFTEST by
REM default - which ACCEPT.BAT creates and leaves in place afterwards so
REM the outputs can be inspected.  Nothing in the source tree is written
REM to or deleted.
REM
REM Clark's PACKFIDO.EXE reads a bare "PCBOARD.DAT" from the CURRENT
REM directory, so this script has to CD into the test directory and cannot
REM change back - COMMAND.COM has nowhere to remember the old one.  You are
REM left in %PFDIR% when it finishes.
REM
REM ---------------------------------------------------------------------------
REM WHY THERE IS NO COPY, REN, DEL OR FC BELOW
REM
REM Measured under DOSBox-X: COPY works from the autoexec and SILENTLY DOES
REM NOTHING from inside a batch file - no error, no file, ERRORLEVEL
REM unchanged.  A harness built on it compares whatever the previous step
REM left lying around and reports PASS for the wrong reason.  FC is an
REM external command that is not on every DOS.  So MKACCEPT.EXE does the
REM copying (/C) and the comparing (/D) itself, and this script uses nothing
REM but EXE calls, IF ERRORLEVEL, IF EXIST, CD and ECHO.  No redirection
REM anywhere either - that is unreliable from batch for the same reason.
REM ===========================================================================

IF NOT EXIST \APPLY.txt GOTO NOROOT

SET PFDIR=%1
IF "%PFDIR%"=="" SET PFDIR=\PFTEST

SET PFCLARK=\pcb1541\install\dist\target\PACKFIDO.EXE
SET PFOURS=\pcb153\SOURCE\MISC\PACKFIDO\PACKFIDO.EXE
SET PFMK=\pcb153\SOURCE\MISC\PACKFIDO\accept\MKACCEPT.EXE
SET PFDAT=\pcb153\SOURCE\MISC\PACKFIDO\examples\PCBOARD.DAT

IF NOT EXIST %PFCLARK% GOTO NOCLARK
IF NOT EXIST %PFOURS%  GOTO NOOURS
IF NOT EXIST %PFMK%    GOTO NOMK
IF NOT EXIST %PFDAT%   GOTO NODAT

IF NOT EXIST %PFDIR%\NUL MKDIR %PFDIR%
IF NOT EXIST %PFDIR%\NUL GOTO NODIR

ECHO.
ECHO --- building the fixture in %PFDIR% --------------------------------------
%PFMK% %PFDIR% %PFDAT%
IF ERRORLEVEL 1 GOTO FAILED

CD %PFDIR%
IF NOT EXIST PCBFIDO.CFG GOTO FAILED
IF NOT EXIST ORIG.CFG GOTO FAILED

ECHO.
ECHO --- Clark's PACKFIDO.EXE -------------------------------------------------
%PFCLARK%
IF NOT EXIST PCBFIDO.CFG GOTO FAILED
%PFMK% /C PCBFIDO.CFG CLARK.OUT
IF ERRORLEVEL 1 GOTO FAILED

ECHO.
ECHO --- our PACKFIDO.EXE -----------------------------------------------------
%PFMK% /C ORIG.CFG PCBFIDO.CFG
IF ERRORLEVEL 1 GOTO FAILED
%PFOURS% PCBFIDO.CFG CNAMES 800
IF ERRORLEVEL 1 GOTO FAILED
%PFMK% /C PCBFIDO.CFG OURS.OUT
IF ERRORLEVEL 1 GOTO FAILED

ECHO.
ECHO --- comparing ------------------------------------------------------------
%PFMK% /D CLARK.OUT OURS.OUT
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
ECHO   Build it first:  BCC -ml -eMKACCEPT.EXE MKACCEPT.C
GOTO END

:NODAT
ECHO.
ECHO   Cannot find %PFDAT%
ECHO   The fixture copies Clark's own PCBOARD.DAT from examples\.
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
SET PFDAT=
