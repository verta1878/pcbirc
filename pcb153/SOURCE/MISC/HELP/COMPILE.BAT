@echo off
call \proj\bcdos.bat

%compiler% -I%include%;\PROJ\LIB\H -L%bcroot%\lib -n%bccompiler% -P makehelp.c
