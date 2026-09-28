@echo off
call \proj\bcdos.bat
echo Y | del %bccompiler%\*.* > nul
