@echo off
rem ============================================================
rem  bldins.bat - install v1.11+ byte-exact install.exe build
rem  scoped to pcb1541/install/ (not the pwa153 sdk matrix).
rem
rem  compiler: borland c++ 3.1 (from devtools/bc31.zip)
rem  linker:   tlink 5.1 (writes ne linker bytes 05 0a = 5.10)
rem  libs:     bc 3.1 standard + api.lib from os/2 sdk 1.03
rem  target:   ne family api .exe - byte-exact match to clark's
rem            reference install.exe (338,548 b, linker 5.10)
rem
rem  assumes dosbox-x mount conventions:
rem    c:\bc31\        borland c++ 3.1 tree
rem    c:\os2sdk\      os/2 sdk 1.03 (needs lib\api.lib, include\)
rem    c:\pcbirc\      the repo root
rem    c:\out\install\ output tree (auto-created)
rem
rem  run manually from dosbox-x, or add as a target to your local
rem  build-image autoexec.bat. does not dispatch through
rem  main\build\scripts\build.bat - that's for the pwa153 sdk.
rem ============================================================
echo.
echo building install v1.11 (byte-exact install.exe reconstruction)
echo compiler: bc 3.1  linker: tlink 5.1  target: ne family api
echo.
set path=c:\bc31\bin;%path%
if not exist c:\out\install md c:\out\install
cd c:\pcbirc\pcb1541\install\src

echo   [1/2] compile install-1011.c -^> install-1011.obj
bcc.exe -c -ml -ic:\bc31\include -ic:\os2sdk\include -oc:\out\install\install-1011.obj install-1011.c
if errorlevel 1 goto :fail

echo   [2/2] link install-1011.obj -^> install.exe (ne family api, os/2 target)
tlink.exe /toe c:\bc31\lib\c0fl.obj c:\out\install\install-1011.obj, c:\out\install\install.exe, c:\out\install\install.map, c:\bc31\lib\cl.lib c:\bc31\lib\api.lib c:\os2sdk\lib\api.lib
if errorlevel 1 goto :fail

echo.
echo build ok. output: c:\out\install\install.exe
echo verify linker bytes: 05 0a (=5.10) at offset (le32 from 0x3c) + 2
goto :eof

:fail
echo build failed
exit /b 1
