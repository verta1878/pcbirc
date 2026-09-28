@echo off
rem ============================================================================
rem  attic-cleanup.bat  --  move superseded pcbcomm drafts to the attic
rem
rem  run from the repo root (the folder that contains pcb1541\ and attic\).
rem  follows the existing attic convention: attic\superseded-pcbcomm\ mirrors
rem  the live source path, so a file's origin stays obvious.
rem
rem  what it moves and why:
rem    src\int14-r1.c  -->  attic\...\src\int14-r1.c
rem      superseded 211-line earlier revision of the int 14h handler. its
rem      header still says "int14.c" (misfiled name). the live handler is the
rem      701-line src\int14.c, which is what pcbcomm.mak builds (obj\int14.obj).
rem      nothing references int14-r1.c -- no makefile, no source, no doc.
rem
rem  safe: this only moves files into attic\. nothing is deleted. if a move
rem  target already exists the copy is skipped and the source left in place,
rem  so re-running is harmless.
rem ============================================================================

setlocal
set src=pcb1541\pcbcomm
set dst=attic\superseded-pcbcomm\pcb1541\pcbcomm

if not exist "%src%\src\int14-r1.c" goto nofile
if not exist "%dst%\src\" mkdir "%dst%\src\"

echo moving superseded int14-r1.c to attic...
move "%src%\src\int14-r1.c" "%dst%\src\int14-r1.c"
if errorlevel 1 goto failed

echo done. src\int14-r1.c is now in %dst%\src\
echo the live handler src\int14.c is untouched.
goto end

:nofile
echo nothing to do: pcb1541\pcbcomm\src\int14-r1.c not found.
echo (already atticked, or you are not in the repo root.)
goto end

:failed
echo error: move failed. check that %dst%\src\int14-r1.c does not already exist.

:end
endlocal
