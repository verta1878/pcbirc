@echo off

rem set any common variables here
set dszlog=pcbdsz.log
set node=%PCBNODE%
set port=%PCBPORT%

rem now create variables that the PCBoard and the board batch file will use
set pcbdat=c:\pcb\pcboard.dat
set pcb=/NODE:%node% /PORT%port% /OS

prompt [NODE %node% - OS/2] $p$g

rem change to the node directory and delete the pcboard.sys file so that if
rem the system is being started up after a reboot it will not assume that a
rem caller is already online
%pcbdrive%
cd %pcbdir%
del pcboard.sys
board.cmd
