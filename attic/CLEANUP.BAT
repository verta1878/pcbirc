@echo off
rem ============================================================
rem  CLEANUP.BAT   2026-09-22
rem
rem  Run from the repo root (the folder holding APPLY.txt).
rem
rem    CLEANUP           both passes, then a summary
rem    CLEANUP MOVE      pass 1 only
rem    CLEANUP RETIRE    pass 2 only
rem    CLEANUP VERIFY    check the tree against CHECKSUMS.sha256
rem
rem  PASS 1 - MOVE, driven by attic\CLEANUP.LST
rem    Moves files out of the tree into attic\<category>\<path>.
rem      - copied to the attic BEFORE it is deleted
rem      - deleted only if the attic copy exists and matches size
rem      - an attic copy already there is never overwritten
rem    Add work:  category^|path\relative\to\root.ext
rem
rem  PASS 2 - RETIRE, driven by attic\RETIRE.LST
rem    Deletes files whose replacement ALREADY exists elsewhere -
rem    a renamed attic copy, a relocated source file, a manifest
rem    that absorbed them.  Nothing is copied, because nothing
rem    needs copying; the guard is the proof.
rem      - deleted only if its keeper exists
rem      - no keeper, no delete, and the run says so
rem    Add work:  keeper\path.ext^|victim\path.ext
rem
rem  Both passes are safe to run twice.  Nothing listed has to
rem  exist.  del /f does not use the Recycle Bin, which is why
rem  every line in both lists names something to check first.
rem ============================================================

setlocal
set REPO=%~dp0
set LIST=%REPO%attic\CLEANUP.LST
set RLIST=%REPO%attic\RETIRE.LST
set MOVED=0
set KEPT=0
set SKIPPED=0
set GONE=0
set HELD=0
set ABSENT=0

if not exist "%REPO%APPLY.txt" goto nroot
if not exist "%REPO%attic\." goto noattic

set MODE=%~1
if "%MODE%"=="" set MODE=ALL

if /i "%MODE%"=="VERIFY" goto verify
if /i "%MODE%"=="MOVE"   goto p1
if /i "%MODE%"=="RETIRE" goto p2
if /i "%MODE%"=="ALL"    goto p1
echo ERROR: unknown mode "%MODE%".  Use MOVE, RETIRE, VERIFY, or nothing.
goto end

rem ============================================================
:p1
echo.
echo === PASS 1: move to attic ===================================
if not exist "%LIST%" (
  echo   attic\CLEANUP.LST not found - nothing to move.
) else (
  for /f "eol=; tokens=1,2 delims=|" %%a in ('type "%LIST%"') do call :MOVEIT "%%a" "%%b"
)
echo.
echo   moved to attic : %MOVED%
echo   kept in place  : %KEPT%
echo   not in tree    : %SKIPPED%
if /i "%MODE%"=="MOVE" goto done

rem ============================================================
:p2
echo.
echo === PASS 2: retire (guarded delete) =========================
if not exist "%RLIST%" (
  echo   attic\RETIRE.LST not found - nothing to retire.
  goto p2done
)
for /f "eol=; tokens=1,2 delims=|" %%a in ('type "%RLIST%"') do call :RETIREIT "%%a" "%%b"
:p2done
echo.
echo   deleted        : %GONE%
echo   held back      : %HELD%
echo   already gone   : %ABSENT%
goto done

rem ------------------------------------------------------------
rem  :MOVEIT  "<category>"  "<path relative to repo root>"
rem ------------------------------------------------------------
:MOVEIT
set CAT=%~1
set REL=%~2
if "%REL%"=="" goto :eof
set SRC=%REPO%%REL%
set DST=%REPO%attic\%CAT%\%REL%

if not exist "%SRC%" (
  echo   absent   %REL%
  set /a SKIPPED=SKIPPED+1
  goto :eof
)

for %%D in ("%DST%") do if not exist "%%~dpD." mkdir "%%~dpD"

if exist "%DST%" (
  echo   have     %REL%   [already in attic]
) else (
  copy /y "%SRC%" "%DST%" >nul
  if not exist "%DST%" (
    echo   FAILED   %REL%   [copy to attic failed]
    set /a KEPT=KEPT+1
    goto :eof
  )
)

set SRCSIZE=
set DSTSIZE=
for %%A in ("%SRC%") do set SRCSIZE=%%~zA
for %%B in ("%DST%") do set DSTSIZE=%%~zB
if not "%SRCSIZE%"=="%DSTSIZE%" (
  echo   KEPT     %REL%   [attic %DSTSIZE% B, tree %SRCSIZE% B - not deleted]
  set /a KEPT=KEPT+1
  goto :eof
)

del /f /q "%SRC%"
if exist "%SRC%" (
  echo   FAILED   %REL%   [delete failed]
  set /a KEPT=KEPT+1
) else (
  echo   moved    %REL%   -^> attic\%CAT%\
  set /a MOVED=MOVED+1
)
goto :eof

rem ------------------------------------------------------------
rem  :RETIREIT  "<keeper>"  "<victim>"   both repo-relative
rem ------------------------------------------------------------
:RETIREIT
set KEEP=%~1
set VICT=%~2
if "%VICT%"=="" goto :eof
set KPATH=%REPO%%KEEP%
set VPATH=%REPO%%VICT%

if not exist "%VPATH%" (
  echo   gone     %VICT%
  set /a ABSENT=ABSENT+1
  goto :eof
)

if not exist "%KPATH%" (
  echo   HELD     %VICT%
  echo            keeper missing: %KEEP%
  set /a HELD=HELD+1
  goto :eof
)

del /f /q "%VPATH%"
if exist "%VPATH%" (
  echo   FAILED   %VICT%   [delete failed - file in use or read-only?]
  set /a HELD=HELD+1
) else (
  echo   deleted  %VICT%
  echo            kept:   %KEEP%
  set /a GONE=GONE+1
)
goto :eof

rem ============================================================
:verify
if not exist "%REPO%CHECKSUMS.sha256" (
  echo ERROR: CHECKSUMS.sha256 not found in the repo root.
  goto end
)
echo.
echo === VERIFY against CHECKSUMS.sha256 =========================
echo This reads every listed file and hashes it.  Give it a minute.
echo.
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$root = (Get-Location).Path;" ^
  "$ok=0; $bad=0; $miss=0;" ^
  "foreach ($line in Get-Content 'CHECKSUMS.sha256') {" ^
  "  if ($line -match '^\s*#' -or $line -notmatch '\S') { continue }" ^
  "  $h,$p = $line -split '\s\s',2;" ^
  "  $f = Join-Path $root ($p -replace '/','\');" ^
  "  if (-not (Test-Path -LiteralPath $f)) { Write-Host ('  MISSING  ' + $p); $miss++; continue }" ^
  "  $a = (Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash.ToLower();" ^
  "  if ($a -eq $h) { $ok++ } else { Write-Host ('  FAILED   ' + $p); $bad++ }" ^
  "}" ^
  "Write-Host '';" ^
  "Write-Host ('  ok      : ' + $ok);" ^
  "Write-Host ('  failed  : ' + $bad);" ^
  "Write-Host ('  missing : ' + $miss);" ^
  "if ($bad -eq 0 -and $miss -eq 0) { Write-Host ''; Write-Host '  Tree matches the manifest.' }"
goto end

rem ============================================================
:done
echo.
echo ------------------------------------------------------------
echo Next:
echo   CLEANUP VERIFY     check the tree against CHECKSUMS.sha256
echo   git status         see what changed
echo   git add -A ^&^& git commit ^&^& git push
goto end

:nroot
echo ERROR: APPLY.txt is not next to this batch file.
echo        Put CLEANUP.BAT in the repo root and run it there.
goto end

:noattic
echo ERROR: attic\ not found.  Nothing was changed.
goto end

:end
endlocal
