@echo off
rem ============================================================================
rem  blddos.bat -- pcboard 15.3 pwa, borland c++ 3.1, dos
rem  rewritten 2026-09-22 for the repointed makefiles.
rem
rem  the build drive is the repo.  mount the repo folder as a drive root:
rem        mount c c:\path\to\pcbircrevival        (see dosbox.cfg)
rem  then \out, \pcb153, \toolkit and \pcbcbase are all inside the repo and
rem  the repo folder can be called anything.  this script stops if \apply.txt
rem  is missing, because that means the wrong folder is mounted.
rem
rem  it drives clark's makefiles; it does not compile anything itself.
rem  each program is built with  make -f<prog>.mak  in its own folder and the
rem  .exe is copied to \out\pwa153.
rem
rem  usage:  blddos [target]
rem     all (default) | clean | pcboard | pplc | pcbsetup | pcbsm | mkpcbtxt
rem     makeidx | usernet | fidoutil | uuin | uuout | uuutil | uuxfer
rem     commdrv
rem
rem  compiler: set bc31path first, or leave it and \bc31 then \b\c31 are tried.
rem  dosbox shows no scrollback worth reading, so keep a log:
rem        blddos all > \out\build.log
rem  written without call :label so it runs under command.com as well as cmd.
rem  named 8.3 on purpose: dos cannot call a file named build_dos.bat (9 chars).
rem ============================================================================

if not exist \apply.txt goto noroot

rem --- compiler -------------------------------------------------------------
if not "%bc31path%"=="" goto gotbc
if exist \bc31\bin\bcc.exe set bc31path=\bc31
if exist \b\c31\bin\bcc.exe set bc31path=\b\c31
:gotbc
if "%bc31path%"=="" goto nobc
if not exist %bc31path%\bin\bcc.exe goto nobc

set bcroot=%bc31path%
set bcpgm=%bc31path%\bin
set include=%bc31path%\include
set libpath=%bc31path%\lib
set compiler=%bcpgm%\bcc.exe
set linker=%bcpgm%\tlink.exe
set tlib=%bcpgm%\tlib.exe
set make=%bcpgm%\make.exe
set tasm=%bcpgm%\tasm.exe
set asmroot=%bcpgm%
set bccompiler=bc31
set bc31=yes
set tc30=
set bc50=
set debug=
set td=
set libsdir=\pcbcbase
set path=%bc31path%\bin;%path%
if not exist \out\nul mkdir \out
if not exist \out\pwa153\nul mkdir \out\pwa153

if "%1"=="" goto all
if "%1"=="all" goto all
if "%1"=="all" goto all
if "%1"=="clean" goto cleanall
if "%1"=="clean" goto cleanall
goto one

rem --- run every target, by calling this file once per target ---------------
:all
echo.
echo  pcboard 15.3 pwa build -- bc 3.1 at %bc31path%, output \out\pwa153
echo.
call \blddos.bat commdrv
call \blddos.bat pcboard
call \blddos.bat pplc
call \blddos.bat pcbsetup
call \blddos.bat pcbsm
call \blddos.bat mkpcbtxt
call \blddos.bat makeidx
call \blddos.bat usernet
call \blddos.bat fidoutil
call \blddos.bat uuin
call \blddos.bat uuout
call \blddos.bat uuutil
call \blddos.bat uuxfer
echo.
echo  build run finished.  binaries in \out\pwa153:
dir \out\pwa153\*.exe
goto end

:cleanall
set bclean=1
call \blddos.bat commdrv
call \blddos.bat pcboard
call \blddos.bat pplc
call \blddos.bat pcbsetup
call \blddos.bat pcbsm
call \blddos.bat mkpcbtxt
call \blddos.bat makeidx
call \blddos.bat usernet
call \blddos.bat fidoutil
call \blddos.bat uuin
call \blddos.bat uuout
call \blddos.bat uuutil
call \blddos.bat uuxfer
set bclean=
echo.
echo  clean done.  \out\pwa153 was not touched.
goto end

rem --- one target -----------------------------------------------------------
rem  bdir  = subdirectory under \pcb153 that the mak lives in
rem  bcdto = directory to cd into before running make
rem  bmakp = path to the mak file relative to bcdto
rem
rem  for most targets bcdto = \pcb153\bdir and bmakp = the mak filename.
rem  for pcboard/pplc/commdrv whose mak lives in 153\ but whose source\
rem  paths are relative to \pcb153, bcdto = \pcb153 and bmakp = 153\mak.
rem  this matches clark's original compile.bat which ran make -f153\pcboard.mak
rem  from \pcb153, not from \pcb153\153.
:one
set bdir=
set bmak=
set bobj=
set bcdto=
set bmakp=
if "%1"=="pcboard"  set bdir=153
if "%1"=="pcboard"  set bmak=pcboard.mak
if "%1"=="pcboard"  set bobj=obj\bc31
if "%1"=="pcboard"  set bcdto=\pcb153
if "%1"=="pcboard"  set bmakp=153\pcboard.mak
if "%1"=="pplc"     set bdir=153
if "%1"=="pplc"     set bmak=pplc.mak
if "%1"=="pplc"     set bobj=obj\ppl
if "%1"=="pplc"     set bcdto=\pcb153
if "%1"=="pplc"     set bmakp=153\pplc.mak
if "%1"=="pcbsetup" set bdir=source\util\pcbsetup
if "%1"=="pcbsetup" set bmak=pcbsetup.mak
if "%1"=="pcbsm"    set bdir=source\util\pcbsm
if "%1"=="pcbsm"    set bmak=pcbsm.mak
if "%1"=="mkpcbtxt" set bdir=source\util\pcbtext
if "%1"=="mkpcbtxt" set bmak=mkpcbtxt.mak
if "%1"=="makeidx"  set bdir=source\misc\idx
if "%1"=="makeidx"  set bmak=makeidx.mak
if "%1"=="usernet"  set bdir=source\misc\usernet
if "%1"=="usernet"  set bmak=usernet.mak
if "%1"=="fidoutil" set bdir=source\misc\fidoutil
if "%1"=="fidoutil" set bmak=fidoutil.mak
if "%1"=="uuin"     set bdir=source\uucp\uuin
if "%1"=="uuin"     set bmak=uuin.mak
if "%1"=="uuout"    set bdir=source\uucp\uuout
if "%1"=="uuout"    set bmak=uuout.mak
if "%1"=="uuutil"   set bdir=source\uucp\uuutil
if "%1"=="uuutil"   set bmak=uuutil.mak
if "%1"=="uuxfer"   set bdir=source\uucp\uuxfer
if "%1"=="uuxfer"   set bmak=uuxfer.mak
if "%1"=="commdrv"  set bdir=153
if "%1"=="commdrv"  set bmak=commdrv.mak
if "%1"=="commdrv"  set bcdto=\
if "%1"=="commdrv"  set bmakp=pcb153\153\commdrv.mak
if "%1"=="pcboard"  set bmakeopt=-dcommdrv -dcomm -dstats -dmp -d386 -ddbase -dfido -dnumnodes=pcb_maxnodes=25
if "%bdir%"=="" goto badtgt
if "%bobj%"=="" set bobj=bc31
rem default: cd into the bdir and run bmak from there (works for co-located maks)
if "%bcdto%"=="" set bcdto=\pcb153\%bdir%
if "%bmakp%"=="" set bmakp=%bmak%

cd %bcdto%
if not exist %bmakp% goto nomak
if "%bclean%"=="1" goto doclean

echo --- %1
if exist bcc.res del bcc.res
rem force cfg regeneration with our paths (clark's compile.bat did this too)
if "%1"=="pcboard" if exist 153\pcboard.cfg del 153\pcboard.cfg
if "%1"=="pplc" if exist 153\pplc.cfg del 153\pplc.cfg
if "%bobj%"=="obj\bc31" if not exist obj\nul mkdir obj
if "%bobj%"=="obj\ppl" if not exist obj\nul mkdir obj
if not exist %bobj%\nul mkdir %bobj%
set progname=%1
%make% -f%bmakp% %bmakeopt%
set progname=
set bmakeopt=
if exist %bobj%\%1.exe copy %bobj%\%1.exe \out\pwa153\%1.exe > nul
if exist %bobj%\%1.exe echo     built  \out\pwa153\%1.exe
if not exist %bobj%\%1.exe echo     not built  %1.exe -- see the make output
if exist %bobj%\pcboardm.exe copy %bobj%\pcboardm.exe \out\pwa153\pcboardm.exe > nul
if exist %bobj%\pcboardm.exe echo     built  \out\pwa153\pcboardm.exe
if exist %bobj%\pcboardm.exe copy %bobj%\pcboardm.exe \out\pwa153\pcboard.exe > nul
if exist %bobj%\pcboardm.exe echo     built  \out\pwa153\pcboard.exe
cd \
goto end

:doclean
echo --- clean %1
set progname=%1
%make% -f%bmakp% clean
set progname=
cd \
goto end

:nomak
echo     skipped %1 -- %bmakp% not found (cd was %bcdto%)
cd \
goto end

:badtgt
echo unknown target "%1".
echo targets: all clean pcboard pplc pcbsetup pcbsm mkpcbtxt makeidx usernet
echo          fidoutil uuin uuout uuutil uuxfer commdrv
goto end

:noroot
echo error: \apply.txt not found.
echo        the repo root must be the root of the build drive, e.g.
echo            mount c c:\path\to\pcbircrevival
echo        so \out, \pcb153 and \toolkit are inside the repo.
goto end

:nobc
echo error: borland c++ 3.1 not found (bin\bcc.exe under \bc31 or \b\c31).
echo        set it first:   set bc31path=\bc31
goto end

:end
