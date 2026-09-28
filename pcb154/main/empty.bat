@echo off
call \proj\bcdos.bat
del 153\pcboard.cfg
del 153\lib*.*
echo y | del obj\%bccompiler%\*.* > nul
