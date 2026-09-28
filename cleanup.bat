@echo off
rem ============================================================
rem  cleanup.bat   2026-09-22
rem
rem  run from the repo root (the folder holding apply.txt).
rem
rem    cleanup           both passes, then a summary
rem    cleanup dryrun    say what both passes would do.  changes nothing.
rem    cleanup move      pass 1 only
rem    cleanup retire    pass 2 only
rem    cleanup dedupe    pass 3 only - delete duplicate folders
rem    cleanup stray     find checksum files outside the repo root
rem    cleanup verify    hash the tree against checksums.sha256, then stray
rem
rem  pass 1 - move, driven by attic\cleanup.lst
rem    moves files out of the tree into attic\<category>\<path>.
rem      - copied to the attic before it is deleted
rem      - deleted only if the attic copy exists and matches size
rem      - an attic copy already there is never overwritten
rem    add work:  category^|path\relative\to\root.ext
rem
rem  pass 2 - retire, driven by attic\retire.lst
rem    deletes files whose replacement already exists elsewhere -
rem    a renamed attic copy, a relocated source file, a manifest
rem    that absorbed them.  nothing is copied, because nothing
rem    needs copying; the guard is the proof.
rem      - deleted only if its keeper exists
rem      - no keeper, no delete, and the run says so
rem    add work:  keeper\path.ext^|victim\path.ext
rem
rem  both passes are safe to run twice.  nothing listed has to
rem  exist.  del /f does not use the recycle bin, which is why
rem  every line in both lists names something to check first,
rem  and why dryrun exists.
rem
rem  pass 3 - dedupe, driven by attic\dedupe.lst
rem    deletes a whole folder only if every file in it has a
rem    byte-identical twin under its keeper and it holds nothing
rem    the keeper lacks.  compared by sha-256, not size or date.
rem    all or none: one mismatch and the folder is kept whole, with
rem    the offending files named.  a file the keeper has and the
rem    victim does not is fine - the keeper may be ahead.
rem    add work:  keeper\folder^|victim\folder
rem
rem  stray enforces one rule: there is one checksums.sha256, one
rem  checksums.md5 and one checksums.txt, all at the repo root.
rem  verify cannot catch a stray - a file that is not in the
rem  manifest is not checked by the manifest.  that is exactly how
rem  two per-directory lists rotted for weeks unnoticed.  it
rem  ignores todo\checksums.md, which is a release note.
rem ============================================================

setlocal
set repo=%~dp0
set list=%repo%attic\cleanup.lst
set rlist=%repo%attic\retire.lst
set moved=0
set kept=0
set skipped=0
set gone=0
set held=0
set absent=0
set strays=0
set dry=0

if not exist "%repo%apply.txt" goto nroot
if not exist "%repo%attic\." goto noattic

set mode=%~1
if "%mode%"=="" set mode=all

if /i "%mode%"=="verify" goto verify
if /i "%mode%"=="stray"  goto strayonly
if /i "%mode%"=="move"   goto p1
if /i "%mode%"=="retire" goto p2
if /i "%mode%"=="dedupe" goto p3
if /i "%mode%"=="all"    goto p1
if /i "%mode%"=="dryrun" goto setdry
if /i "%mode%"=="-n"     goto setdry
echo error: unknown mode "%mode%".
echo        use dryrun, move, retire, stray, verify, or nothing.
goto end

:setdry
set dry=1
goto p1

rem ============================================================
:p1
echo.
if "%dry%"=="1" echo *** dry run - nothing will be changed ***
echo === pass 1: move to attic ===================================
if not exist "%list%" (
  echo   attic\cleanup.lst not found - nothing to move.
) else (
  for /f "eol=; tokens=1,2 delims=|" %%a in ('type "%list%"') do call :moveit "%%a" "%%b"
)
echo.
if "%dry%"=="1" (
  echo   would move     : %moved%
) else (
  echo   moved to attic : %moved%
)
echo   kept in place  : %kept%
echo   not in tree    : %skipped%
if /i "%mode%"=="move" goto done

rem ============================================================
:p2
echo.
echo === pass 2: retire (guarded delete) =========================
if not exist "%rlist%" (
  echo   attic\retire.lst not found - nothing to retire.
  goto p2done
)
for /f "eol=; tokens=1,2 delims=|" %%a in ('type "%rlist%"') do call :retireit "%%a" "%%b"
:p2done
echo.
if "%dry%"=="1" (
  echo   would delete   : %gone%
) else (
  echo   deleted        : %gone%
)
echo   held back      : %held%
echo   already gone   : %absent%
if /i "%mode%"=="retire" goto done

rem ============================================================
:p3
echo.
echo === pass 3: dedupe (identical folders) ======================
if not exist "%repo%attic\dedupe.lst" (
  echo   attic\dedupe.lst not found - nothing to dedupe.
  goto done
)
powershell -noprofile -executionpolicy bypass -file "%repo%attic\dedupe.ps1" "%repo%" %dry%
goto done

rem ------------------------------------------------------------
rem  :moveit  "<category>"  "<path relative to repo root>"
rem ------------------------------------------------------------
:moveit
set cat=%~1
set rel=%~2
if "%rel%"=="" goto :eof
set src=%repo%%rel%
set dst=%repo%attic\%cat%\%rel%

if not exist "%src%" (
  echo   absent   %rel%
  set /a skipped=skipped+1
  goto :eof
)

if "%dry%"=="1" (
  if exist "%dst%" (
    echo   would    %rel%   [attic copy exists; size checked at run time]
  ) else (
    echo   would    %rel%   -^> attic\%cat%\
  )
  set /a moved=moved+1
  goto :eof
)

for %%d in ("%dst%") do if not exist "%%~dpd." mkdir "%%~dpd"

if exist "%dst%" (
  echo   have     %rel%   [already in attic]
) else (
  copy /y "%src%" "%dst%" >nul
  if not exist "%dst%" (
    echo   failed   %rel%   [copy to attic failed]
    set /a kept=kept+1
    goto :eof
  )
)

set srcsize=
set dstsize=
for %%a in ("%src%") do set srcsize=%%~za
for %%b in ("%dst%") do set dstsize=%%~zb
if not "%srcsize%"=="%dstsize%" (
  echo   kept     %rel%   [attic %dstsize% b, tree %srcsize% b - not deleted]
  set /a kept=kept+1
  goto :eof
)

del /f /q "%src%"
if exist "%src%" (
  echo   failed   %rel%   [delete failed]
  set /a kept=kept+1
) else (
  echo   moved    %rel%   -^> attic\%cat%\
  set /a moved=moved+1
)
goto :eof

rem ------------------------------------------------------------
rem  :retireit  "<keeper>"  "<victim>"   both repo-relative
rem ------------------------------------------------------------
:retireit
set keep=%~1
set vict=%~2
if "%vict%"=="" goto :eof
set kpath=%repo%%keep%
set vpath=%repo%%vict%

if not exist "%vpath%" (
  echo   gone     %vict%
  set /a absent=absent+1
  goto :eof
)

if not exist "%kpath%" (
  echo   held     %vict%
  echo            keeper missing: %keep%
  set /a held=held+1
  goto :eof
)

if "%dry%"=="1" (
  echo   would    %vict%
  echo            keeper:  %keep%
  set /a gone=gone+1
  goto :eof
)

del /f /q "%vpath%"
if exist "%vpath%" (
  echo   failed   %vict%   [delete failed - file in use or read-only?]
  set /a held=held+1
) else (
  echo   deleted  %vict%
  echo            kept:   %keep%
  set /a gone=gone+1
)
goto :eof

rem ============================================================
rem  stray - one manifest, at the root, and nowhere else
rem ============================================================
:strayonly
call :strayscan
goto end

:strayscan
echo.
echo === stray checksum files ====================================
for /r "%repo%." %%f in (checksums.sha256 checksums.md5 checksums.txt) do call :onestray "%%f"
echo.
if not "%strays%"=="0" goto straysfound
echo   none.  one manifest, at the root, as intended.
goto :eof
:straysfound
echo   %strays% stray checksum file/s.  retire them: add a
echo   keeper^|victim line to attic\retire.lst and run cleanup retire.
echo   do not leave a per-directory checksum file in the tree -
echo   nothing checks it, which is how two of them rotted unnoticed.
goto :eof

:onestray
set sf=%~1
if not exist "%sf%" goto :eof
for %%d in ("%sf%") do set sd=%%~dpd
if /i "%sd%"=="%repo%" goto :eof
echo   stray    %sf%
set /a strays=strays+1
goto :eof

rem ============================================================
:verify
if not exist "%repo%checksums.sha256" (
  echo error: checksums.sha256 not found in the repo root.
  goto end
)
echo.
echo === verify against checksums.sha256 =========================
echo this reads every listed file and hashes it.  give it a minute.
echo.
powershell -noprofile -executionpolicy bypass -command ^
  "$root = (get-location).path;" ^
  "$ok=0; $bad=0; $miss=0;" ^
  "foreach ($line in get-content 'checksums.sha256') {" ^
  "  if ($line -match '^\s*#' -or $line -notmatch '\s') { continue }" ^
  "  $h,$p = $line -split '\s\s',2;" ^
  "  $f = join-path $root ($p -replace '/','\');" ^
  "  if (-not (test-path -literalpath $f)) { write-host ('  missing  ' + $p); $miss++; continue }" ^
  "  $a = (get-filehash -literalpath $f -algorithm sha256).hash.tolower();" ^
  "  if ($a -eq $h) { $ok++ } else { write-host ('  failed   ' + $p); $bad++ }" ^
  "}" ^
  "write-host '';" ^
  "write-host ('  ok      : ' + $ok);" ^
  "write-host ('  failed  : ' + $bad);" ^
  "write-host ('  missing : ' + $miss);" ^
  "if ($bad -eq 0 -and $miss -eq 0) { write-host ''; write-host '  tree matches the manifest.' }"
call :strayscan
echo.
echo a clean verify does not mean the tree is clean.  the manifest
echo only checks what it lists; git status shows what it does not.
goto end

rem ============================================================
:done
echo.
echo ------------------------------------------------------------
echo next:
if "%dry%"=="1" goto nextdry
echo   cleanup verify     hash the tree, then scan for strays
echo   git status         see what changed
echo   git add -a ^&^& git commit ^&^& git push
goto end
:nextdry
echo   cleanup            do it for real
goto end

:nroot
echo error: apply.txt is not next to this batch file.
echo        put cleanup.bat in the repo root and run it there.
goto end

:noattic
echo error: attic\ not found.  nothing was changed.
goto end

:end
endlocal
