@echo off
cd source\country
make
cd ..\dos
make
cd ..\doscls
make
cd ..\misc
make
cd ..\pcb
make
cd ..\screen
make
cd ..\system
make
cd ..\toolkit
make
cd ..\..\bcdos\bc31
del *.bak
if exist *.386 del *.386
ren *.lib *.386
cd ..\..
