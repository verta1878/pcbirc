@echo off
REM ============================================================
REM  BUILD.BAT - PCBoard SDK top-level dispatcher
REM  Everything starts from MAIN\. This is the entry point.
REM
REM  Usage:  BUILD <target>
REM  See:    BUILD help  (or:  BUILD /?  /  BUILD)
REM
REM  Runs under real DOS + 386MAX (for MSC7 DPMI) or under
REM  DOSBox-X BOOT mode. Called from AUTOEXEC.BAT on the golden
REM  build image, or run manually inside DOSBox.
REM
REM  Errorlevel: 0 = ok, non-zero = failure from called script.
REM ============================================================
setlocal
if "%1"=="" goto HELP
if /I "%1"=="help"      goto HELP
if /I "%1"=="/?"        goto HELP
if /I "%1"=="-h"        goto HELP

REM --- per-compiler (build pwa153 with one compiler) ---
if /I "%1"=="borland"   goto BORLAND
if /I "%1"=="bldkbc"    goto BORLAND
if /I "%1"=="turbo"     goto TURBO
if /I "%1"=="bldkit"    goto TURBO
if /I "%1"=="msc"       goto MSC
if /I "%1"=="mvc"       goto MSC
if /I "%1"=="bldkms"    goto MSC

REM --- per-version (build all compilers for one version) ---
if /I "%1"=="pwa1522"    goto PWA1522
if /I "%1"=="pwa153"    goto PWA153
if /I "%1"=="pwa154"    goto PWA154
if /I "%1"=="delta154"  goto DELTA154
if /I "%1"=="irc1541"   goto IRC1541

REM --- meta ---
if /I "%1"=="all"       goto ALL
if /I "%1"=="clean"     goto CLEAN
if /I "%1"=="mrproper"  goto MRPROPER
if /I "%1"=="status"    goto STATUS

echo Unknown target: %1
echo.
goto HELP

REM ============================================================
:HELP
echo.
echo  BUILD.BAT - PCBoard SDK top-level dispatcher
echo.
echo  Usage:  BUILD ^<target^>
echo.
echo  Per-compiler (pwa153):
echo    borland    Borland C++ 3.1  -^> PCBKBC{S,M,C,L}.LIB
echo    turbo      Turbo C 2.01     -^> PCBKIT{S,M,C,L}.LIB
echo    msc        Microsoft C 7.0  -^> PCBKMS{S,M,C,L}.LIB   (needs 386MAX)
echo.
echo  Per-version (all compilers):
echo    pwa1522    [STUB] source not found  (see toolkit/PWA1522-FUTURE.md)
echo    pwa153     clean + borland + turbo + msc
echo    pwa154     [STUB] build-fix pending (STATS OBJ, headers)
echo    delta154   [STUB] toolkit update pending; output: OUT/delta154/
echo    irc1541    [STUB] openwatcom2irc path (see verta1878/ow2irc)
echo.
echo  Meta:
echo    all        clean, then pwa153
echo    clean      delete OBJs under OUT\LIB\*\OBJ\  (keep .LIBs)
echo    mrproper   delete OBJs and .LIBs (keep dir tree)
echo    status     list .LIB files present
echo    help       this list
echo.
echo  Aliases: bldkbc=borland  bldkit=turbo  bldkms=msc  mvc=msc
echo.
endlocal & exit /b 0

REM ============================================================
:BORLAND
echo === BUILD: Borland C++ 3.1 (PCBKBC, pwa153) ===
call C:\BUILD\SCRIPTS\BLDKBC.BAT
if errorlevel 1 goto FAIL
goto DONE

:TURBO
echo === BUILD: Turbo C 2.01 (PCBKIT, pwa153) ===
call C:\BUILD\SCRIPTS\BLDKIT.BAT
if errorlevel 1 goto FAIL
goto DONE

:MSC
echo === BUILD: Microsoft C 7.0 (PCBKMS, pwa153) ===
call C:\BUILD\SCRIPTS\BLDKMS.BAT
if errorlevel 1 goto FAIL
goto DONE

REM ============================================================
:PWA153
echo === BUILD: pwa153 - all three compilers ===
call C:\BUILD\SCRIPTS\BUILD.BAT clean
if errorlevel 1 goto FAIL
call C:\BUILD\SCRIPTS\BUILD.BAT borland
if errorlevel 1 goto FAIL
call C:\BUILD\SCRIPTS\BUILD.BAT turbo
if errorlevel 1 goto FAIL
call C:\BUILD\SCRIPTS\BUILD.BAT msc
if errorlevel 1 goto FAIL
goto DONE

:PWA1522
echo === STUB: pwa1522 not yet buildable ===
echo Source for the pre-15.3 toolkit has not been located.
echo See:  toolkit\PWA1522-FUTURE.md
endlocal & exit /b 2

:PWA154
echo === STUB: pwa154 build-fix pending ===
echo Needs STATS OBJ + a few header resolutions before it will build.
echo Framework in place: toolkit\pwa154\ + pcb153\upd154\ + OUT\pwa153\upd154\
endlocal & exit /b 2

:DELTA154
echo === STUB: delta154 toolkit update pending ===
echo delta154 toolkit needs updating before compile is possible.
echo When ready, output goes to OUT\delta154\ (its own tree).
endlocal & exit /b 2

:IRC1541
echo === STUB: irc1541 uses openwatcom2irc (external) ===
echo Build with verta1878/ow2irc, not this dispatcher.
endlocal & exit /b 2

REM ============================================================
:ALL
echo === BUILD: all (clean + pwa153) ===
call C:\BUILD\SCRIPTS\BUILD.BAT pwa153
if errorlevel 1 goto FAIL
goto DONE

:CLEAN
echo === CLEAN: removing OBJs (keeping .LIBs) ===
if exist C:\OUT\LIB\PWA153\bc31\OBJ\small\*.OBJ    del /Q C:\OUT\LIB\PWA153\bc31\OBJ\small\*.OBJ
if exist C:\OUT\LIB\PWA153\bc31\OBJ\medium\*.OBJ   del /Q C:\OUT\LIB\PWA153\bc31\OBJ\medium\*.OBJ
if exist C:\OUT\LIB\PWA153\bc31\OBJ\compact\*.OBJ  del /Q C:\OUT\LIB\PWA153\bc31\OBJ\compact\*.OBJ
if exist C:\OUT\LIB\PWA153\bc31\OBJ\large\*.OBJ    del /Q C:\OUT\LIB\PWA153\bc31\OBJ\large\*.OBJ
if exist C:\OUT\LIB\PWA153\tc201\OBJ\small\*.OBJ    del /Q C:\OUT\LIB\PWA153\tc201\OBJ\small\*.OBJ
if exist C:\OUT\LIB\PWA153\tc201\OBJ\medium\*.OBJ   del /Q C:\OUT\LIB\PWA153\tc201\OBJ\medium\*.OBJ
if exist C:\OUT\LIB\PWA153\tc201\OBJ\compact\*.OBJ  del /Q C:\OUT\LIB\PWA153\tc201\OBJ\compact\*.OBJ
if exist C:\OUT\LIB\PWA153\tc201\OBJ\large\*.OBJ    del /Q C:\OUT\LIB\PWA153\tc201\OBJ\large\*.OBJ
if exist C:\OUT\LIB\PWA153\msc70\OBJ\small\*.OBJ    del /Q C:\OUT\LIB\PWA153\msc70\OBJ\small\*.OBJ
if exist C:\OUT\LIB\PWA153\msc70\OBJ\medium\*.OBJ   del /Q C:\OUT\LIB\PWA153\msc70\OBJ\medium\*.OBJ
if exist C:\OUT\LIB\PWA153\msc70\OBJ\compact\*.OBJ  del /Q C:\OUT\LIB\PWA153\msc70\OBJ\compact\*.OBJ
if exist C:\OUT\LIB\PWA153\msc70\OBJ\large\*.OBJ    del /Q C:\OUT\LIB\PWA153\msc70\OBJ\large\*.OBJ
echo Clean done.
goto DONE

:MRPROPER
echo === MRPROPER: removing OBJs AND .LIBs ===
call C:\BUILD\SCRIPTS\BUILD.BAT clean
if exist C:\OUT\LIB\PWA153\bc31\*.LIB     del /Q C:\OUT\LIB\PWA153\bc31\*.LIB
if exist C:\OUT\LIB\PWA153\tc201\*.LIB     del /Q C:\OUT\LIB\PWA153\tc201\*.LIB
if exist C:\OUT\LIB\PWA153\msc70\*.LIB     del /Q C:\OUT\LIB\PWA153\msc70\*.LIB
echo Mrproper done.
goto DONE

:STATUS
echo === STATUS: .LIB files present ===
echo.
echo --- pwa153 ---
if exist C:\OUT\LIB\PWA153\bc31\*.LIB dir /B C:\OUT\LIB\PWA153\bc31\*.LIB
if exist C:\OUT\LIB\PWA153\tc201\*.LIB dir /B C:\OUT\LIB\PWA153\tc201\*.LIB
if exist C:\OUT\LIB\PWA153\msc70\*.LIB dir /B C:\OUT\LIB\PWA153\msc70\*.LIB
echo.
echo (pwa1522 pwa154 delta154 irc1541 not yet buildable - see BUILD help)
goto DONE

REM ============================================================
:FAIL
echo.
echo *** BUILD FAILED (target: %1, errorlevel %errorlevel%) ***
endlocal & exit /b 1

:DONE
echo.
echo BUILD ok (target: %1)
endlocal & exit /b 0
