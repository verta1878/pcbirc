@echo off
call \proj\bcdos.bat

set PROGNAME=UUOUT

rem  As you move from one compiler to another, the .CFG file needs to
rem  be rebuilt because it contains some compiler-specific information within
rem  it.  To facilitate this, a file called "USED####" will be created in
rem  the current directory which will look like "USEDBC31" or "USEDTC30" to
rem  indicate which compiler was last used.  If you are now compiling with
rem  a different compiler, it will delete the USED* file and create a new
rem  one and, in the process, it will delete the .CFG file and let the
rem  .MAK file create a new one.

if exist used%bccompiler% goto continue
del used*
del %PROGNAME%.cfg
echo Now using %bccompiler%. > used%bccompiler%

:continue

%MAKE% -f%PROGNAME%.mak > errors
list errors

set PROGNAME=
