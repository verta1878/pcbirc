@echo off
rem ============================================================================
rem  bldos2.cmd -- pcboard os/2 build, borland c++ for os/2 (bcos2)
rem  rewritten 2026-09-22 for the repointed makefiles.
rem
rem  the build drive is the repo, same rule as the dos build: the repo folder
rem  is the root of the drive this runs from, so \out, \pcb153, \toolkit and
rem  \pcbcbase are inside the repo whatever the repo folder is called.  this
rem  script stops if \apply.txt is missing.
rem
rem  named 8.3 on purpose (build_os2 is 9 characters, which a fat volume
rem  cannot hold).
rem
rem  targets:  pcboard2   \out\pwa153\pcboard2.exe   from 153\pcboard2.mak
rem            usernet2   \out\pwa153\usernet2.exe   from source\misc\usernet
rem            clean      make clean for both
rem
rem  borland c++ for os/2 is 32-bit flat: one library set, no memory models.
rem  set bcroot if the compiler is not at \bcos2.  pcbcp additionally wants
rem  the ibm os/2 developer's toolkit 2.1; set tkroot if it is not at
rem  \toolkt21.  that kit is not in the repo -- see apply.txt.
rem ============================================================================

if not exist \apply.txt goto noroot

if "%bcroot%"=="" set bcroot=\bcos2
if not exist %bcroot%\bin\bcc.exe goto nobcos2

set bcpgm=%bcroot%\bin
set include=%bcroot%\include
set libpath=%bcroot%\lib
set compiler=%bcpgm%\bcc.exe
set linker=%bcpgm%\tlink.exe
set tlib=%bcpgm%\tlib.exe
set make=%bcpgm%\make.exe
set bccompiler=bcos2
set libsdir=\pcbcbase
set debug=0
set td=0

if not exist \out\nul mkdir \out
if not exist \out\pwa153\nul mkdir \out\pwa153

if "%1"=="clean" goto cleanall
if "%1"=="clean" goto cleanall
if "%1"=="usernet2" goto usernet2
if "%1"=="pcboard2" goto pcboard2
if "%1"=="" goto pcboard2
goto badtgt

:pcboard2
echo --- pcboard2 (os/2)
cd \pcb153
if not exist obj\nul mkdir obj
if not exist obj\bcos2\nul mkdir obj\bcos2
%make% -f153\pcboard2.mak
if exist obj\bcos2\pcboard2.exe copy obj\bcos2\pcboard2.exe \out\pwa153\pcboard2.exe
if exist obj\bcos2\pcboard2.exe echo     built  \out\pwa153\pcboard2.exe
if not exist obj\bcos2\pcboard2.exe echo     not built -- see the make output
cd \
if "%1"=="" goto usernet2
goto end

:usernet2
echo --- usernet2 (os/2)
cd \pcb153\source\misc\usernet
if not exist bcos2\nul mkdir bcos2
set progname=usernet2
%make% -fusernet2.mak
set progname=
if exist bcos2\usernet2.exe copy bcos2\usernet2.exe \out\pwa153\usernet2.exe
if exist bcos2\usernet2.exe echo     built  \out\pwa153\usernet2.exe
if not exist bcos2\usernet2.exe echo     not built -- see the make output
cd \
goto end

:cleanall
cd \pcb153
%make% -f153\pcboard2.mak clean
cd \pcb153\source\misc\usernet
%make% -fusernet2.mak clean
cd \
echo  clean done.  \out\pwa153 was not touched.
goto end

:badtgt
echo unknown target "%1".  targets: pcboard2  usernet2  clean
goto end

:noroot
echo error: \apply.txt not found.  the repo root must be the root of the
echo        drive this runs from, so \out and \pcb153 are inside the repo.
goto end

:nobcos2
echo error: borland c++ for os/2 not found at %bcroot%\bin\bcc.exe
echo        set bcroot first, e.g.   set bcroot=\bcos2
echo.
echo note: pcboard2 also needs the codebase os/2 library
echo       (%libsdir%\codebase\os2bor1\b4.lib) and lxbfix, neither of which
echo       is in the repo yet.  see apply.txt.
goto end

:end
