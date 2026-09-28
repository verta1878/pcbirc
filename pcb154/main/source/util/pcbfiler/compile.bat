@echo off
call \proj\bcdos.bat

set progname=pcbfiler

rem  as you move from one compiler to another, the .cfg file needs to
rem  be rebuilt because it contains some compiler-specific information within
rem  it.  to facilitate this, a file called "used####" will be created in
rem  the current directory which will look like "usedbc31" or "usedtc30" to
rem  indicate which compiler was last used.  if you are now compiling with
rem  a different compiler, it will delete the used* file and create a new
rem  one and, in the process, it will delete the .cfg file and let the
rem  .mak file create a new one.

if exist used%bccompiler% goto continue
del used*
del %progname%.cfg
echo now using %bccompiler%. > used%bccompiler%

:continue

%make% -f%progname%.mak > errors
list errors

set progname=
