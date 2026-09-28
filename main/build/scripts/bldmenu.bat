@echo off
REM ============================================================
REM  PCBoard pwa153 SDK - Build Menu
REM ============================================================
:MENU
echo.
echo  =====================================================
echo   PCBoard 15.3 (pwa153) Toolkit SDK - Build Menu
echo  =====================================================
echo.
echo   Each build compiles 119 modules x 4 memory models
echo   and assembles the 4 .LIB files. Steps echo as they run.
echo.
echo   1. PCBKBC  - Borland C++ 3.1   (needs BC31)
echo   2. PCBKIT  - Turbo C 2.01      (needs TC201)
echo   3. PCBKMS  - Microsoft C 7.0   (needs MSC70)
echo   4. Build ALL available
echo   5. Exit
echo.
CHOICE /C:12345 /N Pick a build [1-5]:
if errorlevel 5 goto END
if errorlevel 4 goto ALL
if errorlevel 3 goto KMS
if errorlevel 2 goto KIT
if errorlevel 1 goto KBC
:KBC
call C:\BUILD\SCRIPTS\BLDKBC.BAT
goto MENU
:KIT
call C:\BUILD\SCRIPTS\BLDKIT.BAT
goto MENU
:KMS
call C:\BUILD\SCRIPTS\BLDKMS.BAT
goto MENU
:ALL
call C:\BUILD\SCRIPTS\BLDKBC.BAT
call C:\BUILD\SCRIPTS\BLDKIT.BAT
call C:\BUILD\SCRIPTS\BLDKMS.BAT
goto MENU
:END
echo Done.
