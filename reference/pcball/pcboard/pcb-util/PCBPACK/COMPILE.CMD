@echo off
call \proj\bcos2.cmd

rem set BCROOT=D:\BCOS2
rem set COMPILER=%BCROOT%\BIN\BCC.EXE
rem set LINKER=%BCROOT%\BIN\TLINK.EXE
rem set INCLUDE=%BCROOT%\INCLUDE

make -f153\pcbpack2.mak > errors

list errors
