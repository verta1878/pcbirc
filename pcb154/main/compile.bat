@echo off
set p1=
set p2=
set p3=
set p4=
set p5=
set p6=
set p7=
set p8=
set p9=

    set p1=-dcomm
    set p2=-dstats
rem set p3=-ddebug
rem set p4=-dtd
    set p5=-dmp
    set p6=-d386
    set p7=-ddbase
rem set p8=-dcommdrv
    set p9=-dfido
    set nodes=25

rem set up the bc++ for dos environment
call bcdos.bat

rem  as you move from one compiler to another, the pcboard.cfg file needs to
rem  be rebuilt because it contains some compiler-specific information within
rem  it.  to facilitate this, a file called "used####" will be created in
rem  the 153 directory which will look like "usedbc31" or "usedtc30" to
rem  indicate which compiler was last used.  if you are now compiling with
rem  a different compiler, it will delete the used* file and create a new
rem  one and, in the process, it will delete the pcboard.cfg file and let the
rem  pcboard.mak file create a new one.

if exist 153\libsbc31.386 del 153\libsbc31.386
if exist 153\pcboard.cfg del 153\pcboard.cfg
if exist 153\used%bccompiler% goto continue
echo now using %bccompiler% to compile pcboard. > 153\used%bccompiler%

:continue
rem cls
echo performing a "make" on pcboard...  making the /%nodes% model

if exist bcc.res del bcc.res
%make% -f153\pcboard.mak -dnumnodes=pcb_maxnodes=%nodes% %p1% %p2% %p3% %p4% %p5% %p6% %p7% %p8% %p9% > errors
rem list errors

if exist obj\bc31\pcboardm.exe attrib +r obj\bc31\pcboardm.exe

set p1=
set p2=
set p3=
set p4=
set p5=
set p6=
set p7=
set p8=
set p9=
set nodes=
