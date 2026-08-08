@echo off
rem ====================================================================
rem build.cmd — Build pcbbinkp with OpenWatcom
rem ====================================================================
rem Targets: OS/2 (default), DOS4G, Windows NT
rem Usage: build [os2|dos|nt]
rem ====================================================================

set WATCOM=C:\WATCOM
set PATH=%WATCOM%\BINW;%WATCOM%\BINP;%PATH%
set INCLUDE=%WATCOM%\H;%WATCOM%\H\OS2
set CFLAGS=-5r -oxs -zq -w4

if "%1"=="" goto os2
if "%1"=="os2" goto os2
if "%1"=="dos" goto dos
if "%1"=="nt"  goto nt
echo Unknown target: %1
goto end

:os2
echo Building pcbbinkp for OS/2...
set CFLAGS=%CFLAGS% -bt=os2
wcc386 %CFLAGS% pcbbinkp.c
wcc386 %CFLAGS% binkp.c
wcc386 %CFLAGS% binkpauth.c
wcc386 %CFLAGS% bso.c
wcc386 %CFLAGS% md5.c
wlink system os2v2 name PCBBINKP.EXE file pcbbinkp,binkp,binkpauth,bso,md5 library so32dll,tcp32dll
goto done

:dos
echo Building pcbbinkp for DOS (DOS4G)...
set CFLAGS=%CFLAGS% -bt=dos
wcc386 %CFLAGS% pcbbinkp.c
wcc386 %CFLAGS% binkp.c
wcc386 %CFLAGS% binkpauth.c
wcc386 %CFLAGS% bso.c
wcc386 %CFLAGS% md5.c
wlink system dos4g name PCBBINKP.EXE file pcbbinkp,binkp,binkpauth,bso,md5
goto done

:nt
echo Building pcbbinkp for Windows NT...
set CFLAGS=%CFLAGS% -bt=nt
wcc386 %CFLAGS% pcbbinkp.c
wcc386 %CFLAGS% binkp.c
wcc386 %CFLAGS% binkpauth.c
wcc386 %CFLAGS% bso.c
wcc386 %CFLAGS% md5.c
wlink system nt name PCBBINKP.EXE file pcbbinkp,binkp,binkpauth,bso,md5 library ws2_32
goto done

:done
echo Build complete.
dir PCBBINKP.EXE

:end
