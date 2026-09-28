@echo off
    set p1=250
rem set p2=-dtd
rem set p3=-ddebug

rem set up the bc++ for os/2 environment variables
call \proj\bcos2.cmd

if exist obj\bcos2\compiler.res del obj\bcos2\compiler.res
if exist obj\bcos2\linker.res   del obj\bcos2\linker.res

cls
set nodes=%p1%
echo performing a "make" on pcboard-for-os/2...  making the /%p1% model
%make% -f153\pcboard2.mak -dnumnodes=pcb_maxnodes=%p1% %p2% %p3%

set p1=
set p2=
set p3=
