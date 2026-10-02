@echo off
rem ---------------------------------------------------------------
rem bldtk.bat - build the pcboard toolkit category libraries
rem
rem   bldtk              build all ten libraries (bc31)
rem   bldtk clean        delete the objects and the libraries (bc31)
rem   bldtk <category>   one of country dos doscls misc pcb
rem                      screen scrnio system toolkit vmdata
rem   bldtk tc201        build all ten libraries (turbo c 2.01)
rem   bldtk tc201 clean  clean tc201 output
rem   bldtk tc201 <cat>  one category with tc201
rem
rem each category is built with make -ftklib.mak in its own folder.
rem clark's original makefile is left in place, untouched and unused.
rem
rem output:  \out\pwa153\sdk\<cver>\lib\*.lib
rem objects: \out\pwa153\sdk\<cver>\obj\<category>
rem
rem run it from the repo root.  the repo folder is mounted as the
rem drive root, so \out, \toolkit and \bc31 are all inside the repo
rem whatever the repo folder happens to be called.
rem
rem compiler: bc31 (default) or tc201 (first argument).
rem bc31 is always needed for make and tasm even when compiling
rem with tc201.
rem bc31 path: %bc31path% if set, else \bc31, else \b\c31.
rem tc201 path: %tc201path% if set, else \tc201, else \t\c201.
rem log it with:  bldtk > \out\toolkit.log
rem
rem no call :label anywhere - this has to run under command.com.
rem ---------------------------------------------------------------

if not exist \apply.txt goto noroot

set tkbc=%bc31path%
if not "%tkbc%"=="" goto gotbc
if exist \bc31\bin\bcc.exe set tkbc=\bc31
if not "%tkbc%"=="" goto gotbc
if exist \b\c31\bin\bcc.exe set tkbc=\b\c31
if "%tkbc%"=="" goto nobcc
:gotbc
set path=%tkbc%\bin;%path%

rem --- tc201 compiler selection ---
set tkccarg=
if not "%1"=="tc201" goto setact
set tktc=%tc201path%
if not "%tktc%"=="" goto gottc
if exist \tc201\bin\tcc.exe set tktc=\tc201
if not "%tktc%"=="" goto gottc
if exist \t\c201\bin\tcc.exe set tktc=\t\c201
if "%tktc%"=="" goto notcc
:gottc
set path=%tktc%\bin;%path%
rem tkcc overrides the compiler command in tklib.mak (bcc -> tcc)
rem cfgflag auto-derives from tkcc in tklib.mak (no override needed)
set tkccarg=-Dtkcc=tcc -Dcver=tc201
shift

:setact
set tkact=all
if "%1"=="clean" set tkact=clean
if "%1"=="clean" set tkact=clean

if "%1"=="" goto all
if "%tkact%"=="clean" goto all
set tkone=%1
goto one

:one
if not exist \toolkit\pwa153\source\%tkone%\tklib.mak goto badcat
cd \toolkit\pwa153\source\%tkone%
echo === %tkone% ===
make -ftklib.mak %tkccarg% %tkact%
cd \
goto done

:all
cd \toolkit\pwa153\source\country
echo === country ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\dos
echo === dos ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\doscls
echo === doscls ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\misc
echo === misc ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\pcb
echo === pcb ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\screen
echo === screen ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\scrnio
echo === scrnio ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\system
echo === system ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\toolkit
echo === toolkit ===
make -ftklib.mak %tkccarg% %tkact%
cd \toolkit\pwa153\source\vmdata
echo === vmdata ===
make -ftklib.mak %tkccarg% %tkact%
cd \
goto done

:badcat
echo error: no such category "%tkone%".
echo        country dos doscls misc pcb screen scrnio system toolkit vmdata
goto end

:notcc
echo error: turbo c 2.01 not found.
echo        looked for %%tc201path%%, then \tc201\bin\tcc.exe, then \t\c201\bin\tcc.exe.
goto end

:nobcc
echo error: borland c++ 3.1 not found.
echo        looked for %%bc31path%%, then \bc31\bin\bcc.exe, then \b\c31\bin\bcc.exe.
goto end

:noroot
echo error: run bldtk from the repo root (\apply.txt was not found there).
goto end

:done
echo.
if "%tkccarg%"=="" echo libraries are in \out\pwa153\sdk\bc31\lib
if not "%tkccarg%"=="" echo libraries are in \out\pwa153\sdk\tc201\lib

:end
set tkbc=
set tktc=
set tkact=
set tkone=
set tkccarg=
set tkcver=
