@echo off
call \proj\bcdos.bat
set memcheck=yes
%bccompiler%\pcbsm %1 %2 %3
