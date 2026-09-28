@echo off
rem ---------------------------------------------------------------
rem BLDTKOS2.CMD - build the PCBoard toolkit category libraries
rem                for OS/2 (OpenWatcom 2.0, flat model)
rem
rem   BLDTKOS2              build all ten libraries
rem   BLDTKOS2 CLEAN        delete the objects and the libraries
rem   BLDTKOS2 <CATEGORY>   one of COUNTRY DOS DOSCLS MISC PCB
rem                         SCREEN SCRNIO SYSTEM TOOLKIT VMDATA
rem
rem Each category is built with MAKE -fTKLIBOS2.MAK in its own folder.
rem If TKLIBOS2.MAK does not exist yet, the script stops with a message.
rem
rem Output:  \OUT\DELTA154\SDK\OW2\LIB\*.LIB
rem Objects: \OUT\DELTA154\SDK\OW2\OBJ\<category>
rem
rem Run it from the repo root.  The repo folder is mounted as the
rem drive root, so \OUT, \TOOLKIT and \OW2 are all inside the repo.
rem
rem Compiler: OpenWatcom 2.0.  %WATCOM% if set, else \WATCOM.
rem Log it with:  BLDTKOS2 > \OUT\TKOS2.LOG
rem
rem No CALL :label anywhere - this has to run under COMMAND.COM.
rem ---------------------------------------------------------------

if not exist \APPLY.txt goto NOROOT

set TKWC=%WATCOM%
if not "%TKWC%"=="" goto GOTWC
if exist \WATCOM\BINW\WCC386.EXE set TKWC=\WATCOM
if "%TKWC%"=="" goto NOWCC
:GOTWC
set PATH=%TKWC%\BINW;%PATH%
set INCLUDE=%TKWC%\H;%TKWC%\H\OS2

rem --- create output directories if missing
if not exist \OUT\NUL mkdir \OUT
if not exist \OUT\DELTA154\NUL mkdir \OUT\DELTA154
if not exist \OUT\DELTA154\SDK\NUL mkdir \OUT\DELTA154\SDK
if not exist \OUT\DELTA154\SDK\OW2\NUL mkdir \OUT\DELTA154\SDK\OW2
if not exist \OUT\DELTA154\SDK\OW2\LIB\NUL mkdir \OUT\DELTA154\SDK\OW2\LIB
if not exist \OUT\DELTA154\SDK\OW2\OBJ\NUL mkdir \OUT\DELTA154\SDK\OW2\OBJ

set TKACT=all
if "%1"=="CLEAN" set TKACT=CLEAN
if "%1"=="clean" set TKACT=CLEAN

if "%1"=="" goto ALL
if "%TKACT%"=="CLEAN" goto ALL
set TKONE=%1
goto ONE

:ONE
if not exist \TOOLKIT\DELTA154\SOURCE\%TKONE%\TKLIBOS2.MAK goto NOMAK
cd \TOOLKIT\DELTA154\SOURCE\%TKONE%
echo === %TKONE% (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \
goto DONE

:ALL
cd \TOOLKIT\DELTA154\SOURCE\COUNTRY
echo === COUNTRY (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\DOS
echo === DOS (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\DOSCLS
echo === DOSCLS (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\MISC
echo === MISC (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\PCB
echo === PCB (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\SCREEN
echo === SCREEN (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\SCRNIO
echo === SCRNIO (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\SYSTEM
echo === SYSTEM (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\TOOLKIT
echo === TOOLKIT (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \TOOLKIT\DELTA154\SOURCE\VMDATA
echo === VMDATA (OS/2) ===
MAKE -fTKLIBOS2.MAK %TKACT%
cd \
goto DONE

:NOMAK
echo ERROR: TKLIBOS2.MAK not found in \TOOLKIT\DELTA154\SOURCE\%TKONE%
echo        Each category needs a TKLIBOS2.MAK for the OS/2 build.
echo        Copy TKLIB.MAK and change the compiler flags to OW2 flat model.
goto END

:NOWCC
echo ERROR: OpenWatcom 2.0 not found.
echo        Looked for %%WATCOM%%, then \WATCOM\BINW\WCC386.EXE.
goto END

:NOROOT
echo ERROR: run BLDTKOS2 from the repo root (\APPLY.txt was not found there).
goto END

:DONE
echo.
echo Libraries are in \OUT\DELTA154\SDK\OW2\LIB

:END
set TKWC=
set TKACT=
set TKONE=
