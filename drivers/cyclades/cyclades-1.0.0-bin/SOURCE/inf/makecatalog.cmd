@echo off
REM Generate catalog file for driver package signing
REM Requires WDK inf2cat.exe in PATH
REM Run from the directory containing the INF and SYS files

echo Creating driver catalog...

inf2cat /driver:. /os:2000,XP_X86,Vista_X86,7_X86,7_X64,8_X86,8_X64,10_X86,10_X64

if errorlevel 1 (
    echo ERROR: inf2cat failed. Check INF for errors.
    exit /b 1
)

echo Catalog created: cyclom-y.cat
echo Sign it with: signtool sign /f cert.pfx cyclom-y.cat
