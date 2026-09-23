@ECHO OFF
REM ---------------------------------------------------------------
REM BLDTK.BAT - build the PCBoard toolkit category libraries
REM
REM   BLDTK              build all nine categories
REM   BLDTK CLEAN        delete the objects and the libraries
REM   BLDTK <CATEGORY>   one of COUNTRY DOS DOSCLS MISC PCB
REM                      SCREEN SCRNIO SYSTEM TOOLKIT
REM
REM Each category is built with MAKE -fTKLIB.MAK in its own folder.
REM Clark's original MAKEFILE is left in place, untouched and unused.
REM
REM Output:  \OUT\PWA153\SDK\BC31\LIB\*.LIB
REM Objects: \OUT\PWA153\SDK\BC31\OBJ\<category>
REM
REM Run it from the repo root.  The repo folder is mounted as the
REM drive root, so \OUT, \TOOLKIT and \BC31 are all inside the repo
REM whatever the repo folder happens to be called.
REM
REM Compiler: %BC31PATH% if set, else \BC31, else \B\C31.
REM Log it with:  BLDTK > \OUT\TOOLKIT.LOG
REM
REM No CALL :label anywhere - this has to run under COMMAND.COM.
REM ---------------------------------------------------------------

IF NOT EXIST \APPLY.txt GOTO NOROOT

SET TKBC=%BC31PATH%
IF NOT "%TKBC%"=="" GOTO GOTBC
IF EXIST \BC31\BIN\BCC.EXE SET TKBC=\BC31
IF NOT "%TKBC%"=="" GOTO GOTBC
IF EXIST \B\C31\BIN\BCC.EXE SET TKBC=\B\C31
IF "%TKBC%"=="" GOTO NOBCC
:GOTBC
SET PATH=%TKBC%\BIN;%PATH%

SET TKACT=all
IF "%1"=="CLEAN" SET TKACT=CLEAN
IF "%1"=="clean" SET TKACT=CLEAN

IF "%1"=="" GOTO ALL
IF "%TKACT%"=="CLEAN" GOTO ALL
SET TKONE=%1
GOTO ONE

:ONE
IF NOT EXIST \TOOLKIT\PWA153\SOURCE\%TKONE%\TKLIB.MAK GOTO BADCAT
CD \TOOLKIT\PWA153\SOURCE\%TKONE%
ECHO === %TKONE% ===
MAKE -fTKLIB.MAK %TKACT%
CD \
GOTO DONE

:ALL
CD \TOOLKIT\PWA153\SOURCE\COUNTRY
ECHO === COUNTRY ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\DOS
ECHO === DOS ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\DOSCLS
ECHO === DOSCLS ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\MISC
ECHO === MISC ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\PCB
ECHO === PCB ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\SCREEN
ECHO === SCREEN ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\SCRNIO
ECHO === SCRNIO ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\SYSTEM
ECHO === SYSTEM ===
MAKE -fTKLIB.MAK %TKACT%
CD \TOOLKIT\PWA153\SOURCE\TOOLKIT
ECHO === TOOLKIT ===
MAKE -fTKLIB.MAK %TKACT%
CD \
GOTO DONE

:BADCAT
ECHO ERROR: no such category "%TKONE%".
ECHO        COUNTRY DOS DOSCLS MISC PCB SCREEN SCRNIO SYSTEM TOOLKIT
GOTO END

:NOBCC
ECHO ERROR: Borland C++ 3.1 not found.
ECHO        Looked for %%BC31PATH%%, then \BC31\BIN\BCC.EXE, then \B\C31\BIN\BCC.EXE.
GOTO END

:NOROOT
ECHO ERROR: run BLDTK from the repo root (\APPLY.txt was not found there).
GOTO END

:DONE
ECHO.
ECHO Libraries are in \OUT\PWA153\SDK\BC31\LIB

:END
SET TKBC=
SET TKACT=
SET TKONE=
