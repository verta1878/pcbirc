@echo off

rem note:  if you change p1 or p2, then do a touch pcbcp.mak command

rem set p1=-ddebug
rem set p2=-dvalidator

set bcroot=d:\bcos2
set compiler=%bcroot%\bin\bcc.exe
set linker=%bcroot%\bin\tlink.exe
set include=%bcroot%\include
set libpath=%bcroot%\lib
set brcc=%bcroot%\bin\brcc.exe
set ipfcomp=%bcroot%\bin\ipfc.exe

make -f 1522\pcbcp.mak %p1% %p2%

set p1=
set p2=
