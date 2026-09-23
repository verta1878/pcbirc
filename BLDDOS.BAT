@echo off
rem ============================================================================
rem  BLDDOS.BAT -- PCBoard 15.3 PWA, Borland C++ 3.1, DOS
rem  Rewritten 2026-09-22 for the repointed makefiles.
rem
rem  THE BUILD DRIVE IS THE REPO.  Mount the repo folder as a drive root:
rem        mount C C:\path\to\pcbircrevival        (see DOSBOX.CFG)
rem  Then \OUT, \PCB153, \TOOLKIT and \PCBCBASE are all inside the repo and
rem  the repo folder can be called anything.  This script stops if \APPLY.txt
rem  is missing, because that means the wrong folder is mounted.
rem
rem  It drives Clark's makefiles; it does not compile anything itself.
rem  Each program is built with  MAKE -f<PROG>.MAK  in its own folder and the
rem  .EXE is copied to \OUT\PWA153.
rem
rem  Usage:  BLDDOS [target]
rem     ALL (default) | CLEAN | PCBOARD | PPLC | PCBSETUP | PCBSM | MKPCBTXT
rem     MAKEIDX | USERNET | FIDOUTIL | UUIN | UUOUT | UUUTIL | UUXFER
rem
rem  Compiler: set BC31PATH first, or leave it and \BC31 then \B\C31 are tried.
rem  DOSBox shows no scrollback worth reading, so keep a log:
rem        BLDDOS ALL > \OUT\BUILD.LOG
rem  Written without CALL :label so it runs under COMMAND.COM as well as cmd.
rem  Named 8.3 on purpose: DOS cannot call a file named BUILD_DOS.BAT (9 chars).
rem ============================================================================

if not exist \APPLY.txt goto NOROOT

rem --- compiler -------------------------------------------------------------
if not "%BC31PATH%"=="" goto GOTBC
if exist \BC31\BIN\BCC.EXE set BC31PATH=\BC31
if exist \B\C31\BIN\BCC.EXE set BC31PATH=\B\C31
:GOTBC
if "%BC31PATH%"=="" goto NOBC
if not exist %BC31PATH%\BIN\BCC.EXE goto NOBC

set BCROOT=%BC31PATH%
set BCPGM=%BC31PATH%\BIN
set INCLUDE=%BC31PATH%\INCLUDE
set LIBPATH=%BC31PATH%\LIB
set COMPILER=%BCPGM%\BCC.EXE
set LINKER=%BCPGM%\TLINK.EXE
set TLIB=%BCPGM%\TLIB.EXE
set MAKE=%BCPGM%\MAKE.EXE
set TASM=%BCPGM%\TASM.EXE
set ASMROOT=%BCPGM%
set BCCOMPILER=bc31
set BC31=yes
set TC30=
set BC50=
set DEBUG=0
set TD=0
set LIBSDIR=\PCBCBASE
if not exist \OUT\NUL mkdir \OUT
if not exist \OUT\PWA153\NUL mkdir \OUT\PWA153

if "%1"=="" goto ALL
if "%1"=="ALL" goto ALL
if "%1"=="all" goto ALL
if "%1"=="CLEAN" goto CLEANALL
if "%1"=="clean" goto CLEANALL
goto ONE

rem --- run every target, by calling this file once per target ---------------
:ALL
echo.
echo  PCBoard 15.3 PWA build -- BC 3.1 at %BC31PATH%, output \OUT\PWA153
echo.
call \BLDDOS.BAT PCBOARD
call \BLDDOS.BAT PPLC
call \BLDDOS.BAT PCBSETUP
call \BLDDOS.BAT PCBSM
call \BLDDOS.BAT MKPCBTXT
call \BLDDOS.BAT MAKEIDX
call \BLDDOS.BAT USERNET
call \BLDDOS.BAT FIDOUTIL
call \BLDDOS.BAT UUIN
call \BLDDOS.BAT UUOUT
call \BLDDOS.BAT UUUTIL
call \BLDDOS.BAT UUXFER
echo.
echo  Build run finished.  Binaries in \OUT\PWA153:
dir \OUT\PWA153\*.EXE
goto END

:CLEANALL
set BCLEAN=1
call \BLDDOS.BAT PCBOARD
call \BLDDOS.BAT PPLC
call \BLDDOS.BAT PCBSETUP
call \BLDDOS.BAT PCBSM
call \BLDDOS.BAT MKPCBTXT
call \BLDDOS.BAT MAKEIDX
call \BLDDOS.BAT USERNET
call \BLDDOS.BAT FIDOUTIL
call \BLDDOS.BAT UUIN
call \BLDDOS.BAT UUOUT
call \BLDDOS.BAT UUUTIL
call \BLDDOS.BAT UUXFER
set BCLEAN=
echo.
echo  CLEAN done.  \OUT\PWA153 was not touched.
goto END

rem --- one target -----------------------------------------------------------
:ONE
set BDIR=
set BMAK=
set BOBJ=
if "%1"=="PCBOARD"  set BDIR=153
if "%1"=="PCBOARD"  set BMAK=PCBOARD.MAK
if "%1"=="PCBOARD"  set BOBJ=OBJ\BC31
if "%1"=="PPLC"     set BDIR=153
if "%1"=="PPLC"     set BMAK=PPLC.MAK
if "%1"=="PPLC"     set BOBJ=OBJ\PPL
if "%1"=="PCBSETUP" set BDIR=SOURCE\UTIL\PCBSETUP
if "%1"=="PCBSETUP" set BMAK=PCBSETUP.MAK
if "%1"=="PCBSM"    set BDIR=SOURCE\UTIL\PCBSM
if "%1"=="PCBSM"    set BMAK=PCBSM.MAK
if "%1"=="MKPCBTXT" set BDIR=SOURCE\UTIL\PCBTEXT
if "%1"=="MKPCBTXT" set BMAK=MKPCBTXT.MAK
if "%1"=="MAKEIDX"  set BDIR=SOURCE\MISC\IDX
if "%1"=="MAKEIDX"  set BMAK=MAKEIDX.MAK
if "%1"=="USERNET"  set BDIR=SOURCE\MISC\USERNET
if "%1"=="USERNET"  set BMAK=USERNET.MAK
if "%1"=="FIDOUTIL" set BDIR=SOURCE\MISC\FIDOUTIL
if "%1"=="FIDOUTIL" set BMAK=FIDOUTIL.MAK
if "%1"=="UUIN"     set BDIR=SOURCE\UUCP\UUIN
if "%1"=="UUIN"     set BMAK=UUIN.MAK
if "%1"=="UUOUT"    set BDIR=SOURCE\UUCP\UUOUT
if "%1"=="UUOUT"    set BMAK=UUOUT.MAK
if "%1"=="UUUTIL"   set BDIR=SOURCE\UUCP\UUUTIL
if "%1"=="UUUTIL"   set BMAK=UUUTIL.MAK
if "%1"=="UUXFER"   set BDIR=SOURCE\UUCP\UUXFER
if "%1"=="UUXFER"   set BMAK=UUXFER.MAK
if "%BDIR%"=="" goto BADTGT
if "%BOBJ%"=="" set BOBJ=BC31

cd \PCB153\%BDIR%
if not exist %BMAK% goto NOMAK
if "%BCLEAN%"=="1" goto DOCLEAN

echo --- %1
if "%BOBJ%"=="OBJ\BC31" if not exist OBJ\NUL mkdir OBJ
if "%BOBJ%"=="OBJ\PPL" if not exist OBJ\NUL mkdir OBJ
if not exist %BOBJ%\NUL mkdir %BOBJ%
set PROGNAME=%1
%MAKE% -f%BMAK%
set PROGNAME=
if exist %BOBJ%\%1.EXE copy %BOBJ%\%1.EXE \OUT\PWA153\%1.EXE > NUL
if exist %BOBJ%\%1.EXE echo     built  \OUT\PWA153\%1.EXE
if not exist %BOBJ%\%1.EXE echo     NOT BUILT  %1.EXE -- see the make output
if exist %BOBJ%\PCBOARDM.EXE copy %BOBJ%\PCBOARDM.EXE \OUT\PWA153\PCBOARDM.EXE > NUL
if exist %BOBJ%\PCBOARDM.EXE echo     built  \OUT\PWA153\PCBOARDM.EXE
cd \
goto END

:DOCLEAN
echo --- CLEAN %1
set PROGNAME=%1
%MAKE% -f%BMAK% CLEAN
set PROGNAME=
cd \
goto END

:NOMAK
echo     skipped %1 -- %BMAK% not found in \PCB153\%BDIR%
cd \
goto END

:BADTGT
echo Unknown target "%1".
echo Targets: ALL CLEAN PCBOARD PPLC PCBSETUP PCBSM MKPCBTXT MAKEIDX USERNET
echo          FIDOUTIL UUIN UUOUT UUUTIL UUXFER
goto END

:NOROOT
echo ERROR: \APPLY.txt not found.
echo        The repo root must be the root of the build drive, e.g.
echo            mount C C:\path\to\pcbircrevival
echo        so \OUT, \PCB153 and \TOOLKIT are inside the repo.
goto END

:NOBC
echo ERROR: Borland C++ 3.1 not found (BIN\BCC.EXE under \BC31 or \B\C31).
echo        Set it first:   set BC31PATH=\BC31
goto END

:END
