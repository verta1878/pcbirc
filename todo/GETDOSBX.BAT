@echo off
rem ============================================================================
rem  Download DOSBox-X for Windows
rem  Run this on Windows to fetch the portable DOSBox-X zip
rem ============================================================================
echo.
echo  DOSBox-X is required to compile PCBoard 15.3 with Borland C++ 3.1.
echo.
echo  Download the Windows portable ZIP from:
echo    https://github.com/joncampbell123/dosbox-x/releases
echo.
echo  Look for: dosbox-x-mingw-win64-YYYYMMDD.zip
echo.
echo  Extract it to this directory, then run:
echo    dosbox-x.exe -conf DOSBOX.CFG
echo.
echo  On Linux:
echo    apt install dosbox-x
echo    dosbox-x -conf DOSBOX.CFG
echo.
echo  On macOS:
echo    brew install dosbox-x
echo    dosbox-x -conf DOSBOX.CFG
echo.
pause
