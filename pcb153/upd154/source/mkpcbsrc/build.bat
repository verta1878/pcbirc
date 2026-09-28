@echo off
call c:\pcbsrc.bat
del BUILD.LOG >nul 2>nul
del &1 >nul 2>nul
echo Building MKPCBTXT.EXE (RE from 15.4b) >BUILD.LOG
echo. >>BUILD.LOG
%bc31path%\bin\bcc.exe -ml -c -I. -I%bc31path%\include MKPCB.C >BCC.OUT
type BCC.OUT >>BUILD.LOG
echo. >>BUILD.LOG
if not exist MKPCB.OBJ echo COMPILE FAIL >>BUILD.LOG
if exist MKPCB.OBJ echo Compile OK >>BUILD.LOG
if not exist MKPCB.OBJ goto end
echo. >>BUILD.LOG
echo Link step: >>BUILD.LOG
%bc31path%\bin\tlink.exe /L%bc31path%\lib %bc31path%\lib\c0l MKPCB.OBJ, MKPCBTXT.EXE, MKPCBTXT.MAP, cl.lib >LNK.OUT
type LNK.OUT >>BUILD.LOG
if exist MKPCBTXT.EXE echo Link OK - MKPCBTXT.EXE built >>BUILD.LOG
if not exist MKPCBTXT.EXE echo LINK FAIL >>BUILD.LOG
:end
del BCC.OUT >nul 2>nul
del LNK.OUT >nul 2>nul
