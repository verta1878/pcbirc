@echo off
SET P1=
SET P2=
SET P3=
SET P4=
SET P5=
SET P6=
SET P7=
SET P8=
SET P9=

    SET P1=-DCOMM
    SET P2=-DSTATS
rem SET P3=-DDEBUG
rem SET P4=-DTD
rem SET P5=-DMP
    SET P6=-D386
rem SET P7=-DDBASE
rem SET P8=-DCOMMDRV
rem SET P9=-DFIDO
    SET NODES=250

rem set up the BC++ for DOS environment
call \proj\bcdos.bat

rem  As you move from one compiler to another, the PCBOARD.CFG file needs to
rem  be rebuilt because it contains some compiler-specific information within
rem  it.  To facilitate this, a file called "USED####" will be created in
rem  the 153 directory which will look like "USEDBC31" or "USEDTC30" to
rem  indicate which compiler was last used.  If you are now compiling with
rem  a different compiler, it will delete the USED* file and create a new
rem  one and, in the process, it will delete the PCBOARD.CFG file and let the
rem  PCBOARD.MAK file create a new one.

if exist 153\used%bccompiler% goto continue
del 153\used*
del 153\pcboard.cfg
echo Now using %bccompiler% to compile PCBoard. > 153\used%bccompiler%

:continue
cls
echo Performing a "MAKE" on PCBoard...  making the /%NODES% model

if exist bcc.res del bcc.res
%MAKE% -f153\pcboard.mak -DNUMNODES=PCB_MAXNODES=%NODES% %P1% %P2% %P3% %P4% %P5% %P6% %P7% %P8% %P9% > errors
list errors

SET P1=
SET P2=
SET P3=
SET P4=
SET P5=
SET P6=
SET P7=
SET P8=
SET P9=
SET NODES=
