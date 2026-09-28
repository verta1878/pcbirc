@echo off
call \proj\bcdos.bat

%compiler% -i%include%;\proj\lib\h -l%bcroot%\lib -n%bccompiler% -p makehelp.c
