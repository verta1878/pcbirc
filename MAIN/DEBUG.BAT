@echo off
call \proj\bcdos.bat
set pcb=%1
td obj\%bccompiler%\pcboard.exe /file:\proj\pcb\test\pcboard.dat
