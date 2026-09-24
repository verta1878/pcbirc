@echo off

rem NOTE:  If you change P1 or P2, then do a TOUCH PCBCP.MAK command

rem set P1=-DDEBUG
rem set P2=-DVALIDATOR

set BCROOT=D:\BCOS2
set COMPILER=%BCROOT%\BIN\BCC.EXE
set LINKER=%BCROOT%\BIN\TLINK.EXE
set INCLUDE=%BCROOT%\INCLUDE
set LIBPATH=%BCROOT%\LIB
set BRCC=%BCROOT%\BIN\BRCC.EXE
set IPFCOMP=%BCROOT%\BIN\IPFC.EXE

make -f 1522\pcbcp.mak %P1% %P2%

set P1=
set P2=
