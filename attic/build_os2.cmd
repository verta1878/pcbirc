@echo off
rem ============================================================================
rem  BUILD_OS2.CMD -- replaced 2026-09-22.  The OS/2 build is now BLDOS2.CMD.
rem
rem  Why the rename: "BUILD_OS2" is 9 characters, which a FAT volume cannot
rem  hold; the OS/2 build has to work on FAT as well as HPFS.
rem
rem  Use:    BLDOS2 [PCBOARD2|USERNET2|CLEAN]
rem
rem  The old contents assumed Clark's \PROJ\PCB layout and wrote to \OUT\OS2.
rem  BLDOS2.CMD uses this repo's layout and writes to \OUT\PWA153.
rem ============================================================================
echo.
echo  BUILD_OS2.CMD has been replaced by BLDOS2.CMD.
echo  Run:  BLDOS2 PCBOARD2
echo.
