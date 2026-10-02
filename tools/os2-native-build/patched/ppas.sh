#!/bin/sh
DoExitAsm ()
{ echo "An error occurred while assembling $1"; exit 1; }
DoExitLink ()
{ echo "An error occurred while linking $1"; exit 1; }
echo Assembling unixcp
/usr/bin/as -o /home/claude/os2build/units/unixcp.o  /home/claude/os2build/units/unixcp.s
if [ $? != 0 ]; then DoExitAsm unixcp; fi
rm /home/claude/os2build/units/unixcp.s
