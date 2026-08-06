@echo off
call \proj\bcdos.bat
set pcb=%1
cd test
\proj\pcb\obj\%bccompiler%\pcboard.exe
cd ..
set pcb=
