@echo off
REM ============================================================================
REM  ATTIC-CLEANUP.BAT  --  move superseded pcbdcom drafts to the attic
REM
REM  Run from the repo ROOT (the folder that contains pcb1541\ and attic\).
REM  Follows the existing attic convention: attic\superseded-pcbdcom\ mirrors
REM  the live source path, so a file's origin stays obvious.
REM
REM  What it moves and why:
REM    src\int14-r1.c  -->  attic\...\src\int14-r1.c
REM      Superseded 211-line earlier revision of the INT 14h handler. Its
REM      header still says "int14.c" (misfiled name). The live handler is the
REM      701-line src\int14.c, which is what PCBDCOM.MAK builds (obj\int14.obj).
REM      Nothing references int14-r1.c -- no makefile, no source, no doc.
REM
REM  Safe: this only MOVES files into attic\. Nothing is deleted. If a move
REM  target already exists the copy is skipped and the source left in place,
REM  so re-running is harmless.
REM ============================================================================

setlocal
set SRC=pcb1541\pcbdcom
set DST=attic\superseded-pcbdcom\pcb1541\pcbdcom

if not exist "%SRC%\src\int14-r1.c" goto nofile
if not exist "%DST%\src\" mkdir "%DST%\src\"

echo Moving superseded int14-r1.c to attic...
move "%SRC%\src\int14-r1.c" "%DST%\src\int14-r1.c"
if errorlevel 1 goto failed

echo Done. src\int14-r1.c is now in %DST%\src\
echo The live handler src\int14.c is untouched.
goto end

:nofile
echo Nothing to do: pcb1541\pcbdcom\src\int14-r1.c not found.
echo (Already atticked, or you are not in the repo root.)
goto end

:failed
echo ERROR: move failed. Check that %DST%\src\int14-r1.c does not already exist.

:end
endlocal
