@echo off
REM ============================================================
REM  BLDINS.BAT - install v1.11+ byte-exact INSTALL.EXE build
REM  Scoped to pcb1541/install/ (NOT the pwa153 SDK matrix).
REM
REM  Compiler: Borland C++ 3.1 (from devtools/BC31.zip)
REM  Linker:   TLINK 5.1 (writes NE linker bytes 05 0A = 5.10)
REM  Libs:     BC 3.1 standard + API.LIB from OS/2 SDK 1.03
REM  Target:   NE Family API .EXE - byte-exact match to Clark's
REM            reference INSTALL.EXE (338,548 B, linker 5.10)
REM
REM  Assumes DOSBox-X mount conventions:
REM    C:\BC31\        Borland C++ 3.1 tree
REM    C:\OS2SDK\      OS/2 SDK 1.03 (needs LIB\API.LIB, INCLUDE\)
REM    C:\PCBIRC\      the repo root
REM    C:\OUT\install\ output tree (auto-created)
REM
REM  Run manually from DOSBox-X, or add as a target to your local
REM  build-image AUTOEXEC.BAT. Does NOT dispatch through
REM  MAIN\build\scripts\BUILD.BAT - that's for the pwa153 SDK.
REM ============================================================
echo.
echo Building install v1.11 (byte-exact INSTALL.EXE reconstruction)
echo Compiler: BC 3.1  Linker: TLINK 5.1  Target: NE Family API
echo.
set PATH=C:\BC31\BIN;%PATH%
if not exist C:\OUT\install md C:\OUT\install
cd C:\PCBIRC\pcb1541\install\src

echo   [1/2] Compile install-1011.c -^> install-1011.OBJ
BCC.EXE -c -ml -IC:\BC31\INCLUDE -IC:\OS2SDK\INCLUDE -oC:\OUT\install\install-1011.OBJ install-1011.c
if errorlevel 1 goto :FAIL

echo   [2/2] Link install-1011.OBJ -^> INSTALL.EXE (NE Family API, OS/2 target)
TLINK.EXE /Toe C:\BC31\LIB\C0FL.OBJ C:\OUT\install\install-1011.OBJ, C:\OUT\install\INSTALL.EXE, C:\OUT\install\INSTALL.MAP, C:\BC31\LIB\CL.LIB C:\OS2SDK\LIB\API.LIB
if errorlevel 1 goto :FAIL

echo.
echo Build OK. Output: C:\OUT\install\INSTALL.EXE
echo Verify linker bytes: 05 0A (=5.10) at offset (LE32 from 0x3C) + 2
goto :EOF

:FAIL
echo BUILD FAILED
exit /b 1
