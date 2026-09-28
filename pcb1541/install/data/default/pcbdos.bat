@echo off
rem this batch file is created by pcboard for os/2 to run dos programs while
rem passing the various pcboard environment variables to the vdm.
set pcbdrive=d:
set pcbdir=\proj\pcb\test
set pcbdat=d:\proj\pcb\test\pcboard.dat
set pcbos2=y
set pcbhandle=0
set dszlog=pcbdsz.log
pcbtitle.com
call %1 %2 %3 %4 %5 %6 %7 %8 %9
