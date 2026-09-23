@echo off
rem ============================================================================
rem  BUILD_DOS.BAT -- replaced 2026-09-22.  The DOS build is now BLDDOS.BAT.
rem
rem  Why the rename: "BUILD_DOS" is 9 characters.  DOS cannot call a file with
rem  a 9-character name, so this file was unusable from the environment it was
rem  written for -- COMMAND.COM reports "Bad command or filename".
rem
rem  Use:    BLDDOS [ALL|CLEAN|PCBOARD|PPLC|PCBSETUP|PCBSM|MKPCBTXT|MAKEIDX|
rem                  USERNET|FIDOUTIL|UUIN|UUOUT|UUUTIL|UUXFER]
rem
rem  The old contents built by calling BCC directly against a staged build root
rem  (\PCBSRC, \B\C31, \PCB154\PCB153\...) and wrote to \OUT\BIN.  None of
rem  those paths exist in this repo any more.  BLDDOS.BAT drives Clark's
rem  makefiles instead and writes to \OUT\PWA153.
rem ============================================================================
echo.
echo  BUILD_DOS.BAT has been replaced by BLDDOS.BAT (8.3 name, DOS can call it).
echo  Run:  BLDDOS ALL
echo.
