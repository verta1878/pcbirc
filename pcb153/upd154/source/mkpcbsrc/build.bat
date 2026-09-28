@echo off
call c:\pcbsrc.bat
del build.log >nul 2>nul
del &1 >nul 2>nul
echo building mkpcbtxt.exe (re from 15.4b) >build.log
echo. >>build.log
%bc31path%\bin\bcc.exe -ml -c -i. -i%bc31path%\include mkpcb.c >bcc.out
type bcc.out >>build.log
echo. >>build.log
if not exist mkpcb.obj echo compile fail >>build.log
if exist mkpcb.obj echo compile ok >>build.log
if not exist mkpcb.obj goto end
echo. >>build.log
echo link step: >>build.log
%bc31path%\bin\tlink.exe /l%bc31path%\lib %bc31path%\lib\c0l mkpcb.obj, mkpcbtxt.exe, mkpcbtxt.map, cl.lib >lnk.out
type lnk.out >>build.log
if exist mkpcbtxt.exe echo link ok - mkpcbtxt.exe built >>build.log
if not exist mkpcbtxt.exe echo link fail >>build.log
:end
del bcc.out >nul 2>nul
del lnk.out >nul 2>nul
