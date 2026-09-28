@echo off
call \proj\bcdos.bat

%make% -fpcbmoni.mak > errors
list errors
