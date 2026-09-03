@echo off
rem ============================================================================
rem  rebuild.bat -- regenerate the PCBoard 15.41 install target/ tree
rem
rem  Extracts all 8 archives from pcb1541/install/INSTALL.zip and places each
rem  source file into its target/ location per INSTALL.DAT.
rem
rem  Run from repo root or from pcb1541/install/dist/target/.
rem  Requires: python 3, a C compiler (mingw gcc), unzip in PATH.
rem ============================================================================

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%..\..\..\.."
set "REPO_ROOT=%CD%"
popd

set "INSTALL_ZIP=%REPO_ROOT%\pcb1541\install\INSTALL.zip"
set "REDX_DIR=%REPO_ROOT%\pcb1541\install\archivers\redx"
set "TARGET_DIR=%SCRIPT_DIR%"
set "WORK_DIR=%TEMP%\pcbirc_rebuild_%RANDOM%"

echo   Repo root: %REPO_ROOT%
echo   Working:   %WORK_DIR%
mkdir "%WORK_DIR%" 2>nul

if not exist "%INSTALL_ZIP%" (
    echo ERROR: %INSTALL_ZIP% not found
    exit /b 1
)

echo   Building redx...
gcc -O2 -o "%WORK_DIR%\redx.exe" "%REDX_DIR%\redx.c" "%REDX_DIR%\red_pack.c" "%REDX_DIR%\red_decompress.c" >nul 2>&1
if not exist "%WORK_DIR%\redx.exe" (
    echo ERROR: gcc build failed. Install MinGW or run rebuild.sh under WSL/Git Bash.
    exit /b 1
)

echo   Extracting archives from INSTALL.zip...
mkdir "%WORK_DIR%\ext" 2>nul

rem 6 .RED archives
for %%A in (COMMDRV PCBCFGS PCBMAIL PCBOARD PCBOARD2 PPLC) do (
    unzip -p "%INSTALL_ZIP%" "%%A.RED" > "%WORK_DIR%\%%A.RED" 2>nul
    mkdir "%WORK_DIR%\ext\%%A" 2>nul
    pushd "%WORK_DIR%\ext\%%A"
    "%WORK_DIR%\redx.exe" extract "%WORK_DIR%\%%A.RED" >nul
    popd
)

rem PCBDISK.002 and PCBDISK.003 (no .RED extension but same format)
for %%A in (PCBDISK.002 PCBDISK.003) do (
    unzip -p "%INSTALL_ZIP%" "%%A" > "%WORK_DIR%\%%A" 2>nul
    mkdir "%WORK_DIR%\ext\%%A" 2>nul
    pushd "%WORK_DIR%\ext\%%A"
    "%WORK_DIR%\redx.exe" extract "%WORK_DIR%\%%A" >nul
    popd
)

unzip -p "%INSTALL_ZIP%" INSTALL.DAT > "%WORK_DIR%\install.dat" 2>nul

echo   Placing files into target/...
python "%SCRIPT_DIR%rebuild_place.py" "%WORK_DIR%" "%TARGET_DIR%"

rmdir /S /Q "%WORK_DIR%"

echo.
echo   Done. target/ has been rebuilt from INSTALL.zip.
