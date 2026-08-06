@echo off
call \proj\bcdos.bat
del 153\pcboard.cfg
del 153\lib*.*
echo Y | del obj\%bccompiler%\*.* > nul
