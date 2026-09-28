@echo off
:top
%pcbdrive%
cd %pcbdir%
if exist remote.bat ren remote.bat remote.sys
if exist door.bat   del door.bat
if exist endpcb     del endpcb
pcboardm /file:%pcbdat%
if exist remote.bat call remote.bat
if exist door.bat   call door.bat
if exist event.bat  call event.bat
if not exist endpcb goto top
:end
