@echo off
cd uuin
call compile.bat
cd ..\uuout
call compile.bat
cd ..\uuutil
call compile.bat
cd ..\uuxfer
call compile.bat
cd ..
