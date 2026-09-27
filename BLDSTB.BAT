@echo off
echo Compiling 4 streambuf source files...
\BC31\BIN\BCC +\PCB153\SOURCE\UUCP\STB.CFG -n\PCB153\SOURCE\UUCP\BC31 \PCB153\SOURCE\UUCP\stbdsgtn.cpp
if errorlevel 1 goto fail
echo stbdsgtn OK
\BC31\BIN\BCC +\PCB153\SOURCE\UUCP\STB.CFG -n\PCB153\SOURCE\UUCP\BC31 \PCB153\SOURCE\UUCP\stbdsptn.cpp
if errorlevel 1 goto fail
echo stbdsptn OK
\BC31\BIN\BCC +\PCB153\SOURCE\UUCP\STB.CFG -n\PCB153\SOURCE\UUCP\BC31 \PCB153\SOURCE\UUCP\stbsgetn.cpp
if errorlevel 1 goto fail
echo stbsgetn OK
\BC31\BIN\BCC +\PCB153\SOURCE\UUCP\STB.CFG -n\PCB153\SOURCE\UUCP\BC31 \PCB153\SOURCE\UUCP\stbsputn.cpp
if errorlevel 1 goto fail
echo stbsputn OK
echo ALL 4 STREAMBUF FILES COMPILED
echo DONE > \TMP\STBDONE.TXT
goto end
:fail
echo COMPILE FAILED
echo FAIL > \TMP\STBDONE.TXT
:end
