@echo off

rem   Define P1=-DCOMMDRV if COMM-DRV support is desired, otherwise FOSSIL
rem   support will be used instead.  (COMM-DRV also includes fossil)
rem   NOTE: For COMM-DRV support to work, you must have purchased COMM-DRV
rem   and must have the files in \LIBS\COMMDRV.

SET P1=
SET P2=
SET P3=
rem SET P1=-DCOMMDRV
rem SET P2=-DDEBUG
rem SET P3=-DTD

call \proj\bcdos.bat
if exist %bccompiler%\done del %bccompiler%\done

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

echo Performing a "MAKE" on ZMSEND and ZMRECV
if exist obj\bcc.res del obj\bcc.res
make -fzmodem.mak %P1% %P2% %P3% > errors
if exist obj\done del obj\done
list errors

SET P1=
SET P2=
SET P3=
