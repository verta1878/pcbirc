@echo off
if exist lint.tmp del lint.tmp
call d:\lint\lset
call d:\lint\os2lin.cmd 153\pcboard2.lnt
