@echo off
rem This batch file is created by PCBoard for OS/2 to run DOS programs while
rem passing the various PCBoard environment variables to the VDM.
SET PCBDRIVE=D:
SET PCBDIR=\PROJ\PCB\TEST
SET PCBDAT=D:\PROJ\PCB\TEST\PCBOARD.DAT
SET PCBOS2=Y
SET PCBHANDLE=0
SET DSZLOG=PCBDSZ.LOG
pcbtitle.com
call %1 %2 %3 %4 %5 %6 %7 %8 %9
