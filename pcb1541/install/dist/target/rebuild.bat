@echo off
rem ============================================================================
rem  rebuild.bat -- regenerate the pcboard 15.41 install target/ tree
rem
rem  extracts all 8 archives from pcb1541/install/install.zip and places each
rem  source file into its target/ location per install.dat.
rem
rem  run from repo root or from pcb1541/install/dist/target/.
rem  requires: python 3, a c compiler (mingw gcc), unzip in path.
rem ============================================================================

setlocal enabledelayedexpansion

set "script_dir=%~dp0"
pushd "%script_dir%..\..\..\.."
set "repo_root=%cd%"
popd

set "install_zip=%repo_root%\pcb1541\install\install.zip"
set "redx_dir=%repo_root%\pcb1541\install\archivers\redx"
set "target_dir=%script_dir%"
set "work_dir=%temp%\pcbirc_rebuild_%random%"

echo   repo root: %repo_root%
echo   working:   %work_dir%
mkdir "%work_dir%" 2>nul

if not exist "%install_zip%" (
    echo error: %install_zip% not found
    exit /b 1
)

echo   building redx...
gcc -o2 -o "%work_dir%\redx.exe" "%redx_dir%\redx.c" "%redx_dir%\red_pack.c" "%redx_dir%\red_decompress.c" >nul 2>&1
if not exist "%work_dir%\redx.exe" (
    echo error: gcc build failed. install mingw or run rebuild.sh under wsl/git bash.
    exit /b 1
)

echo   extracting archives from install.zip...
mkdir "%work_dir%\ext" 2>nul

rem 6 .red archives
for %%a in (commdrv pcbcfgs pcbmail pcboard pcboard2 pplc) do (
    unzip -p "%install_zip%" "%%a.red" > "%work_dir%\%%a.red" 2>nul
    mkdir "%work_dir%\ext\%%a" 2>nul
    pushd "%work_dir%\ext\%%a"
    "%work_dir%\redx.exe" extract "%work_dir%\%%a.red" >nul
    popd
)

rem pcbdisk.002 and pcbdisk.003 (no .red extension but same format)
for %%a in (pcbdisk.002 pcbdisk.003) do (
    unzip -p "%install_zip%" "%%a" > "%work_dir%\%%a" 2>nul
    mkdir "%work_dir%\ext\%%a" 2>nul
    pushd "%work_dir%\ext\%%a"
    "%work_dir%\redx.exe" extract "%work_dir%\%%a" >nul
    popd
)

unzip -p "%install_zip%" install.dat > "%work_dir%\install.dat" 2>nul

echo   placing files into target/...
python "%script_dir%rebuild_place.py" "%work_dir%" "%target_dir%"

rmdir /s /q "%work_dir%"

echo.
echo   done. target/ has been rebuilt from install.zip.
