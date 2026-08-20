@echo off
call \proj\bcdos.bat
%bccompiler%\waitfile %1 %2 %3
