@echo off
call \proj\bcdos.bat
set memcheck=yes
rem copy usernet.xxx d:\pcb\main\usernet.xxx
%bccompiler%\pcbmoni d:\pcb\main\usernet.xxx
