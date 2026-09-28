@echo off
rem ---------------------------------------------------------------
rem bldtkos2.cmd - build the pcboard toolkit category libraries
rem                for os/2 (openwatcom 2.0, flat model)
rem
rem   bldtkos2              build all ten libraries
rem   bldtkos2 clean        delete the objects and the libraries
rem   bldtkos2 <category>   one of country dos doscls misc pcb
rem                         screen scrnio system toolkit vmdata
rem
rem each category is built with make -ftklibos2.mak in its own folder.
rem if tklibos2.mak does not exist yet, the script stops with a message.
rem
rem output:  \out\delta154\sdk\ow2\lib\*.lib
rem objects: \out\delta154\sdk\ow2\obj\<category>
rem
rem run it from the repo root.  the repo folder is mounted as the
rem drive root, so \out, \toolkit and \ow2 are all inside the repo.
rem
rem compiler: openwatcom 2.0.  %watcom% if set, else \watcom.
rem log it with:  bldtkos2 > \out\tkos2.log
rem
rem no call :label anywhere - this has to run under command.com.
rem ---------------------------------------------------------------

if not exist \apply.txt goto noroot

set tkwc=%watcom%
if not "%tkwc%"=="" goto gotwc
if exist \watcom\binw\wcc386.exe set tkwc=\watcom
if "%tkwc%"=="" goto nowcc
:gotwc
set path=%tkwc%\binw;%path%
set include=%tkwc%\h;%tkwc%\h\os2

rem --- create output directories if missing
if not exist \out\nul mkdir \out
if not exist \out\delta154\nul mkdir \out\delta154
if not exist \out\delta154\sdk\nul mkdir \out\delta154\sdk
if not exist \out\delta154\sdk\ow2\nul mkdir \out\delta154\sdk\ow2
if not exist \out\delta154\sdk\ow2\lib\nul mkdir \out\delta154\sdk\ow2\lib
if not exist \out\delta154\sdk\ow2\obj\nul mkdir \out\delta154\sdk\ow2\obj

set tkact=all
if "%1"=="clean" set tkact=clean
if "%1"=="clean" set tkact=clean

if "%1"=="" goto all
if "%tkact%"=="clean" goto all
set tkone=%1
goto one

:one
if not exist \toolkit\delta154\source\%tkone%\tklibos2.mak goto nomak
cd \toolkit\delta154\source\%tkone%
echo === %tkone% (os/2) ===
make -ftklibos2.mak %tkact%
cd \
goto done

:all
cd \toolkit\delta154\source\country
echo === country (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\dos
echo === dos (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\doscls
echo === doscls (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\misc
echo === misc (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\pcb
echo === pcb (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\screen
echo === screen (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\scrnio
echo === scrnio (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\system
echo === system (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\toolkit
echo === toolkit (os/2) ===
make -ftklibos2.mak %tkact%
cd \toolkit\delta154\source\vmdata
echo === vmdata (os/2) ===
make -ftklibos2.mak %tkact%
cd \
goto done

:nomak
echo error: tklibos2.mak not found in \toolkit\delta154\source\%tkone%
echo        each category needs a tklibos2.mak for the os/2 build.
echo        copy tklib.mak and change the compiler flags to ow2 flat model.
goto end

:nowcc
echo error: openwatcom 2.0 not found.
echo        looked for %%watcom%%, then \watcom\binw\wcc386.exe.
goto end

:noroot
echo error: run bldtkos2 from the repo root (\apply.txt was not found there).
goto end

:done
echo.
echo libraries are in \out\delta154\sdk\ow2\lib

:end
set tkwc=
set tkact=
set tkone=
