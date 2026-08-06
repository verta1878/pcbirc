@echo off
if not "%DJGPP%" == "" goto run
echo need DJGPP environment
goto exit
:run
rm -f main/bcc.res
rm -f main/copt.res
rm -f main/errors
rm -f main/errors.ppl
rm -f main/input.res
rm -f main/153/libsbc31.386
rm -f main/153/pcboard.cfg
rm -f main/153/usedbc31
find lib/bcdos/bc31 -type f -iregex ".*\.\(bak\|lib\|386\)" | xargs -r rm
find . -type f -iregex ".*\.\(exe\|obj\)" | xargs -r rm
:exit
