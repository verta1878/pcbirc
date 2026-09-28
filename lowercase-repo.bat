@echo off
REM lowercase-repo.bat — rename all tracked files and directories to lowercase
REM Run from the root of the pcbirc repo (where .git is)
REM Logs all operations to lowercase-repo.log
REM Windows NTFS is case-insensitive so we do a two-step rename: FILE.C -> FILE.C.tmp -> file.c

setlocal enabledelayedexpansion

set LOGFILE=lowercase-repo.log
echo === lowercase-repo.bat started %DATE% %TIME% === > %LOGFILE%

if not exist .git (
    echo ERROR: Not in a git repo root. Run from pcbirc\ directory.
    echo ERROR: Not in a git repo root. >> %LOGFILE%
    goto :eof
)

echo === Step 1: Renaming files to lowercase === | tee -a %LOGFILE% 2>nul
echo === Step 1: Renaming files === >> %LOGFILE%

set FCOUNT=0
set FERR=0

REM Use PowerShell to process tracked files — batch can't lowercase strings easily
powershell -ExecutionPolicy Bypass -Command ^
  "$log = 'lowercase-repo.log'; " ^
  "$fc = 0; $fe = 0; " ^
  "$files = git ls-files | Sort-Object -Descending; " ^
  "foreach ($f in $files) { " ^
  "  $dir = Split-Path $f -Parent; " ^
  "  $base = Split-Path $f -Leaf; " ^
  "  $lower = $base.ToLowerInvariant(); " ^
  "  if ($base -cne $lower) { " ^
  "    $tmp = $f + '.lctmp'; " ^
  "    $dest = if ($dir) { \"$dir/$lower\" } else { $lower }; " ^
  "    $r1 = git mv $f $tmp 2>&1; " ^
  "    if ($LASTEXITCODE -eq 0) { " ^
  "      $r2 = git mv $tmp $dest 2>&1; " ^
  "      if ($LASTEXITCODE -eq 0) { " ^
  "        Add-Content $log \"OK   file: $f -> $dest\"; " ^
  "        $fc++; " ^
  "      } else { " ^
  "        Add-Content $log \"FAIL file step2: $tmp -> $dest : $r2\"; " ^
  "        $fe++; " ^
  "      } " ^
  "    } else { " ^
  "      Add-Content $log \"FAIL file step1: $f -> $tmp : $r1\"; " ^
  "      $fe++; " ^
  "    } " ^
  "  } " ^
  "} " ^
  "Write-Host \"Files renamed: $fc  Errors: $fe\"; " ^
  "Add-Content $log \"Files renamed: $fc  Errors: $fe\"; "

echo === Step 2: Renaming directories to lowercase === >> %LOGFILE%
echo === Step 2: Renaming directories ===

REM Directories — deepest first, two-step rename
powershell -ExecutionPolicy Bypass -Command ^
  "$log = 'lowercase-repo.log'; " ^
  "$dc = 0; $de = 0; " ^
  "$dirs = Get-ChildItem -Recurse -Directory | Where-Object { $_.FullName -notmatch '[\\/]\.git([\\/]|$)' } | Sort-Object { $_.FullName.Length } -Descending; " ^
  "foreach ($d in $dirs) { " ^
  "  $rel = Resolve-Path -Relative $d.FullName; " ^
  "  $rel = $rel -replace '^\.[\\/]',''; " ^
  "  $rel = $rel -replace '\\','/'; " ^
  "  $parent = Split-Path $rel -Parent; " ^
  "  $parent = $parent -replace '\\','/'; " ^
  "  $base = Split-Path $rel -Leaf; " ^
  "  $lower = $base.ToLowerInvariant(); " ^
  "  if ($base -cne $lower) { " ^
  "    $tmp = $rel + '.lctmp'; " ^
  "    $dest = if ($parent) { \"$parent/$lower\" } else { $lower }; " ^
  "    $r1 = git mv $rel $tmp 2>&1; " ^
  "    if ($LASTEXITCODE -eq 0) { " ^
  "      $r2 = git mv $tmp $dest 2>&1; " ^
  "      if ($LASTEXITCODE -eq 0) { " ^
  "        Add-Content $log \"OK   dir:  $rel -> $dest\"; " ^
  "        $dc++; " ^
  "      } else { " ^
  "        Add-Content $log \"FAIL dir step2: $tmp -> $dest : $r2\"; " ^
  "        $de++; " ^
  "      } " ^
  "    } else { " ^
  "      Add-Content $log \"FAIL dir step1: $rel -> $tmp : $r1\"; " ^
  "      $de++; " ^
  "    } " ^
  "  } " ^
  "} " ^
  "Write-Host \"Dirs renamed: $dc  Errors: $de\"; " ^
  "Add-Content $log \"Dirs renamed: $dc  Errors: $de\"; "

echo === Step 3: Checking for remaining uppercase === >> %LOGFILE%
echo === Checking for remaining uppercase ===

powershell -ExecutionPolicy Bypass -Command ^
  "$remaining = git ls-files | Where-Object { $_ -cmatch '[A-Z]' }; " ^
  "if ($remaining.Count -eq 0) { " ^
  "  Write-Host 'All lowercase. No uppercase remaining.'; " ^
  "  Add-Content 'lowercase-repo.log' 'All lowercase.'; " ^
  "} else { " ^
  "  Write-Host \"WARNING: $($remaining.Count) paths still have uppercase\"; " ^
  "  $remaining | ForEach-Object { Add-Content 'lowercase-repo.log' \"REMAINING: $_\" }; " ^
  "} "

echo.
echo === Summary ===
echo Review with: git status
echo Commit with: git commit -m "Lowercase all filenames and directories repo-wide"
echo Push with: git push
echo Log saved to: %LOGFILE%
echo === Done %DATE% %TIME% === >> %LOGFILE%
