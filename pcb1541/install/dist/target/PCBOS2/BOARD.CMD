@echo off
:top
%pcbdrive%
cd %pcbdir%
if exist remote.cmd REN remote.cmd remote.sys
if exist endpcb     DEL endpcb
pcboard2 /file:%pcbdat%
if exist remote.cmd CALL remote.cmd
if exist event.cmd  CALL event.cmd
if exist event.bat  CALL event.bat
if NOT exist endpcb GOTO top
:end
