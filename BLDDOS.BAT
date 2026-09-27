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
rem     COMMDRV
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
set DEBUG=
set TD=
set LIBSDIR=\PCBCBASE
set PATH=%BC31PATH%\BIN;%PATH%
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
call \BLDDOS.BAT COMMDRV
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
call \BLDDOS.BAT COMMDRV
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
rem  BDIR  = subdirectory under \PCB153 that the MAK lives in
rem  BCDTO = directory to cd into before running MAKE
rem  BMAKP = path to the MAK file relative to BCDTO
rem
rem  For most targets BCDTO = \PCB153\BDIR and BMAKP = the MAK filename.
rem  For PCBOARD/PPLC/COMMDRV whose MAK lives in 153\ but whose SOURCE\
rem  paths are relative to \PCB153, BCDTO = \PCB153 and BMAKP = 153\MAK.
rem  This matches Clark's original COMPILE.BAT which ran MAKE -f153\pcboard.mak
rem  from \PCB153, NOT from \PCB153\153.
:ONE
set BDIR=
set BMAK=
set BOBJ=
set BCDTO=
set BMAKP=
if "%1"=="PCBOARD"  set BDIR=153
if "%1"=="PCBOARD"  set BMAK=PCBOARD.MAK
if "%1"=="PCBOARD"  set BOBJ=OBJ\BC31
if "%1"=="PCBOARD"  set BCDTO=\PCB153
if "%1"=="PCBOARD"  set BMAKP=153\PCBOARD.MAK
if "%1"=="PPLC"     set BDIR=153
if "%1"=="PPLC"     set BMAK=PPLC.MAK
if "%1"=="PPLC"     set BOBJ=OBJ\PPL
if "%1"=="PPLC"     set BCDTO=\PCB153
if "%1"=="PPLC"     set BMAKP=153\PPLC.MAK
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
if "%1"=="COMMDRV"  set BDIR=153
if "%1"=="COMMDRV"  set BMAK=COMMDRV.MAK
if "%1"=="COMMDRV"  set BCDTO=\
if "%1"=="COMMDRV"  set BMAKP=PCB153\153\COMMDRV.MAK
if "%1"=="PCBOARD"  set BMAKEOPT=-DCOMMDRV -DCOMM -DSTATS -DMP -D386 -DDBASE -DFIDO -DNUMNODES=PCB_MAXNODES=25
if "%BDIR%"=="" goto BADTGT
if "%BOBJ%"=="" set BOBJ=BC31
rem Default: cd into the BDIR and run BMAK from there (works for co-located MAKs)
if "%BCDTO%"=="" set BCDTO=\PCB153\%BDIR%
if "%BMAKP%"=="" set BMAKP=%BMAK%

cd %BCDTO%
if not exist %BMAKP% goto NOMAK
if "%BCLEAN%"=="1" goto DOCLEAN

echo --- %1
if exist bcc.res del bcc.res
rem Force CFG regeneration with our paths (Clark's COMPILE.BAT did this too)
if "%1"=="PCBOARD" if exist 153\PCBOARD.CFG del 153\PCBOARD.CFG
if "%1"=="PPLC" if exist 153\PPLC.CFG del 153\PPLC.CFG
if "%BOBJ%"=="OBJ\BC31" if not exist OBJ\NUL mkdir OBJ
if "%BOBJ%"=="OBJ\PPL" if not exist OBJ\NUL mkdir OBJ
if not exist %BOBJ%\NUL mkdir %BOBJ%
set PROGNAME=%1
%MAKE% -f%BMAKP% %BMAKEOPT%
set PROGNAME=
set BMAKEOPT=
if exist %BOBJ%\%1.EXE copy %BOBJ%\%1.EXE \OUT\PWA153\%1.EXE > NUL
if exist %BOBJ%\%1.EXE echo     built  \OUT\PWA153\%1.EXE
if not exist %BOBJ%\%1.EXE echo     NOT BUILT  %1.EXE -- see the make output
if exist %BOBJ%\PCBOARDM.EXE copy %BOBJ%\PCBOARDM.EXE \OUT\PWA153\PCBOARDM.EXE > NUL
if exist %BOBJ%\PCBOARDM.EXE echo     built  \OUT\PWA153\PCBOARDM.EXE
if exist %BOBJ%\PCBOARDM.EXE copy %BOBJ%\PCBOARDM.EXE \OUT\PWA153\PCBOARD.EXE > NUL
if exist %BOBJ%\PCBOARDM.EXE echo     built  \OUT\PWA153\PCBOARD.EXE
cd \
goto END

:DOCLEAN
echo --- CLEAN %1
set PROGNAME=%1
%MAKE% -f%BMAKP% CLEAN
set PROGNAME=
cd \
goto END

:NOMAK
echo     skipped %1 -- %BMAKP% not found (cd was %BCDTO%)
cd \
goto END

:BADTGT
echo Unknown target "%1".
echo Targets: ALL CLEAN PCBOARD PPLC PCBSETUP PCBSM MKPCBTXT MAKEIDX USERNET
echo          FIDOUTIL UUIN UUOUT UUUTIL UUXFER COMMDRV
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
