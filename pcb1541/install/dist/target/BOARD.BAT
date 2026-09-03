@echo off
:top
%pcbdrive%
cd %pcbdir%
if exist remote.bat REN remote.bat remote.sys
if exist door.bat   DEL door.bat
if exist endpcb     DEL endpcb
pcboardm /file:%pcbdat%
if exist remote.bat CALL remote.bat
if exist door.bat   CALL door.bat
if exist event.bat  CALL event.bat
if NOT exist endpcb GOTO top
:end
