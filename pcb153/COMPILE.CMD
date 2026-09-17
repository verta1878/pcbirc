@echo off
    set p1=250
rem set p2=-DTD
rem set p3=-DDEBUG

rem set up the BC++ for OS/2 environment variables
call \proj\bcos2.cmd

if exist obj\bcos2\compiler.res del obj\bcos2\compiler.res
if exist obj\bcos2\linker.res   del obj\bcos2\linker.res

cls
set nodes=%p1%
echo Performing a "MAKE" on PCBoard-for-OS/2...  making the /%p1% model
%MAKE% -f153\pcboard2.mak -DNUMNODES=PCB_MAXNODES=%p1% %p2% %p3%

set p1=
set p2=
set p3=
