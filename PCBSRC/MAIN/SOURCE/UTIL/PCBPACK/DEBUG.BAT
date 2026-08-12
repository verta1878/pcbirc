@echo off
call \proj\bcdos.bat
set memcheck=yes
td %bccompiler%\pcbpack /area:0 /killdups /crc:90
