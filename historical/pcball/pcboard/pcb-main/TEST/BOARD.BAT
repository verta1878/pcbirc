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
if exist remote.bat CALL remote
if exist door.bat   CALL door
if exist event.bat  CALL event
if NOT exist endpcb GOTO top
:end
