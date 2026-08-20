@echo off
call \proj\bcdos.bat

%MAKE% -fpcbmoni.mak > errors
list errors
