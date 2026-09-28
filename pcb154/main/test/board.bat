echo off
call \proj\bcdos.bat

:top
d:
cd \proj\pcb\test
set pcb=%1
set dszlog=pcbdsz.log
if exist remote.bat ren remote.bat remote.sys
if exist door.bat   del door.bat
if exist event.bat  del event.bat
if exist endpcb     del endpcb
\proj\pcb\obj\%bccompiler%\pcboard.exe
if exist remote.bat call remote
if exist door.bat   call door
if exist event.bat  call event
if not exist endpcb goto top
:end
