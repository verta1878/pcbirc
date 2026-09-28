@echo off
echo compiling 4 streambuf source files...
\bc31\bin\bcc +\pcb153\source\uucp\stb.cfg -n\pcb153\source\uucp\bc31 \pcb153\source\uucp\stbdsgtn.cpp
if errorlevel 1 goto fail
echo stbdsgtn ok
\bc31\bin\bcc +\pcb153\source\uucp\stb.cfg -n\pcb153\source\uucp\bc31 \pcb153\source\uucp\stbdsptn.cpp
if errorlevel 1 goto fail
echo stbdsptn ok
\bc31\bin\bcc +\pcb153\source\uucp\stb.cfg -n\pcb153\source\uucp\bc31 \pcb153\source\uucp\stbsgetn.cpp
if errorlevel 1 goto fail
echo stbsgetn ok
\bc31\bin\bcc +\pcb153\source\uucp\stb.cfg -n\pcb153\source\uucp\bc31 \pcb153\source\uucp\stbsputn.cpp
if errorlevel 1 goto fail
echo stbsputn ok
echo all 4 streambuf files compiled
echo done > \tmp\stbdone.txt
goto end
:fail
echo compile failed
echo fail > \tmp\stbdone.txt
:end
