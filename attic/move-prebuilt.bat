@echo off
rem ============================================================
rem  MOVE-PREBUILT.BAT   2026-09-22
rem
rem  Deletes pcbcbase\PREBUILT\BC31\*.386 now that the same nine
rem  libraries are held in attic\prebuilt-libs\BC31\ .
rem
rem  Run from the repo root (the folder holding APPLY.txt).
rem  Nothing is deleted unless the attic copy is there AND is the
rem  same size as the original.
rem  When it is done, move this .BAT into attic\ as well.
rem ============================================================

setlocal
set REPO=%~dp0
set SRC=%REPO%pcbcbase\PREBUILT\BC31
set ATTIC=%REPO%attic\prebuilt-libs\BC31

if not exist "%REPO%APPLY.txt" goto nroot
if not exist "%ATTIC%\README.md" goto nattic

echo.
echo From: %SRC%
echo To  : %ATTIC%   (already copied)
echo.

call :RETIRE COUNTRYL.386
call :RETIRE DOSCLS_L.386
call :RETIRE DOS_L.386
call :RETIRE MISC_L.386
call :RETIRE PCB_L.386
call :RETIRE SCREEN_L.386
call :RETIRE SCRNIO_L.386
call :RETIRE SYSTEM_L.386
call :RETIRE TOOLKITL.386

echo.
rem  remove the now-empty folders, innermost first
if exist "%SRC%\*.386" goto leftovers
if exist "%SRC%" rmdir "%SRC%" 2>nul
if exist "%REPO%pcbcbase\PREBUILT" rmdir "%REPO%pcbcbase\PREBUILT" 2>nul
if exist "%REPO%pcbcbase\PREBUILT" echo   note: pcbcbase\PREBUILT not empty, left in place
goto report

:leftovers
echo   note: some .386 files were kept, folders left in place

:report
echo.
echo Done.  Review with:  git status
echo pcbcbase\CODEBASE is untouched - only PREBUILT was moved.
echo Then move this file into attic\ and commit.
goto end

rem ------------------------------------------------------------
rem  :RETIRE  <file>   delete only if the attic holds a same-size copy
rem ------------------------------------------------------------
:RETIRE
if not exist "%SRC%\%~1" (
  echo   gone     %~1   [already deleted]
  goto :eof
)
if not exist "%ATTIC%\%~1" (
  echo   KEPT     %~1   [no attic copy - not deleted]
  goto :eof
)
if not "%~z1"=="" set ORIGSIZE=
for %%A in ("%SRC%\%~1") do set ORIGSIZE=%%~zA
for %%B in ("%ATTIC%\%~1") do set ATTICSIZE=%%~zB
if not "%ORIGSIZE%"=="%ATTICSIZE%" (
  echo   KEPT     %~1   [attic copy is %ATTICSIZE% bytes, original %ORIGSIZE% - not deleted]
  goto :eof
)
del /f /q "%SRC%\%~1"
if exist "%SRC%\%~1" (
  echo   FAILED   %~1   [delete failed]
) else (
  echo   deleted  %~1   [%ORIGSIZE% bytes, copy verified]
)
goto :eof

rem ------------------------------------------------------------
:nroot
echo ERROR: APPLY.txt is not next to this batch file.
echo        Put MOVE-PREBUILT.BAT in the repo root and run it there.
goto end

:nattic
echo ERROR: attic\prebuilt-libs\BC31\README.md not found.
echo        Nothing was changed.
goto end

:end
endlocal
