@echo off
rem ============================================================
rem  RETIRE-SIDEFILES.BAT   2026-09-22
rem
rem  Finishes retiring the unused build side-files:
rem    1. copies the two PCB_BC50.386 files into the attic
rem    2. deletes the originals whose attic copy already exists
rem
rem  Run from the repo root (the folder that holds APPLY.txt).
rem  Nothing is deleted unless the attic already has a copy of it.
rem  When you are done, move this .BAT into attic\ and commit.
rem ============================================================

setlocal
set REPO=%~dp0
set ATTIC=%REPO%attic\retired-build-files

if not exist "%REPO%APPLY.txt" goto nroot
if not exist "%ATTIC%\README.md" goto nattic

echo.
echo Repo : %REPO%
echo Attic: %ATTIC%
echo.
echo --- copying PCB_BC50.386 into the attic ---
call :COPYIN pcb153\153                          PCB_BC50.386
call :COPYIN pcb154\MAIN\153                     PCB_BC50.386

echo.
echo --- deleting originals already held in the attic ---
call :RETIRE pcb153\153                          PCB_BC50.386
call :RETIRE pcb154\MAIN\153                     PCB_BC50.386
call :RETIRE pcb153\upd154\SOURCE\UUCP\UUIN      UUIN154.CFG
call :RETIRE pcb153\upd154\SOURCE\UUCP\UUOUT     UUOUT154.CFG
call :RETIRE pcb153\upd154\SOURCE\UUCP\UUUTIL    UTIL154.CFG
call :RETIRE pcb153\upd154\SOURCE\UUCP\UUXFER    XFER154.CFG
call :RETIRE pcb154\MAIN\SOURCE\UTIL\PCBSM       INITBUILD.RSP
call :RETIRE pcb154\MAIN\SOURCE\UUCP\UUIN        UUIN154.CFG
call :RETIRE pcb154\MAIN\SOURCE\UUCP\UUOUT       UUOUT154.CFG
call :RETIRE pcb154\MAIN\SOURCE\UUCP\UUUTIL      UTIL154.CFG
call :RETIRE pcb154\MAIN\SOURCE\UUCP\UUXFER      XFER154.CFG

echo.
echo Done.  Review with:  git status
echo Then move this file into attic\ and commit.
goto end

rem ------------------------------------------------------------
rem  :COPYIN  <folder>  <file>    copy into attic, never overwrite
rem ------------------------------------------------------------
:COPYIN
if not exist "%REPO%%~1\%~2" (
  echo   skip     %~1\%~2   [not in tree]
  goto :eof
)
if exist "%ATTIC%\%~1\%~2" (
  echo   have     %~1\%~2   [already in attic]
  goto :eof
)
if not exist "%ATTIC%\%~1" mkdir "%ATTIC%\%~1"
copy /y "%REPO%%~1\%~2" "%ATTIC%\%~1\%~2" >nul
if exist "%ATTIC%\%~1\%~2" (
  echo   copied   %~1\%~2
) else (
  echo   FAILED   %~1\%~2   [copy failed]
)
goto :eof

rem ------------------------------------------------------------
rem  :RETIRE  <folder>  <file>    delete only if attic has a copy
rem ------------------------------------------------------------
:RETIRE
if not exist "%REPO%%~1\%~2" (
  echo   gone     %~1\%~2   [already deleted]
  goto :eof
)
if not exist "%ATTIC%\%~1\%~2" (
  echo   KEPT     %~1\%~2   [no attic copy - not deleted]
  goto :eof
)
del /f /q "%REPO%%~1\%~2"
if exist "%REPO%%~1\%~2" (
  echo   FAILED   %~1\%~2   [delete failed]
) else (
  echo   deleted  %~1\%~2
)
goto :eof

rem ------------------------------------------------------------
:nroot
echo ERROR: APPLY.txt is not next to this batch file.
echo        Put RETIRE-SIDEFILES.BAT in the repo root and run it there.
goto end

:nattic
echo ERROR: attic\retired-build-files\README.md not found.
echo        Nothing was changed.
goto end

:end
endlocal
