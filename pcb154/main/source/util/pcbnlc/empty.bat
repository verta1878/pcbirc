@echo off
call \proj\bcdos.bat
echo y | del %bccompiler%\*.* > nul
