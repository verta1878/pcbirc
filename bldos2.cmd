@echo off
rem ============================================================================
rem  BLDOS2.CMD -- PCBoard OS/2 build, Borland C++ for OS/2 (BCOS2)
rem  Rewritten 2026-09-22 for the repointed makefiles.
rem
rem  THE BUILD DRIVE IS THE REPO, same rule as the DOS build: the repo folder
rem  is the root of the drive this runs from, so \OUT, \PCB153, \TOOLKIT and
rem  \PCBCBASE are inside the repo whatever the repo folder is called.  This
rem  script stops if \APPLY.txt is missing.
rem
rem  Named 8.3 on purpose (BUILD_OS2 is 9 characters, which a FAT volume
rem  cannot hold).
rem
rem  Targets:  PCBOARD2   \OUT\PWA153\PCBOARD2.EXE   from 153\PCBOARD2.MAK
rem            USERNET2   \OUT\PWA153\USERNET2.EXE   from SOURCE\MISC\USERNET
rem            CLEAN      make CLEAN for both
rem
rem  Borland C++ for OS/2 is 32-bit flat: ONE library set, no memory models.
rem  Set BCROOT if the compiler is not at \BCOS2.  PCBCP additionally wants
rem  the IBM OS/2 Developer's Toolkit 2.1; set TKROOT if it is not at
rem  \TOOLKT21.  That kit is not in the repo -- see APPLY.txt.
rem ============================================================================

if not exist \APPLY.txt goto NOROOT

if "%BCROOT%"=="" set BCROOT=\BCOS2
if not exist %BCROOT%\BIN\BCC.EXE goto NOBCOS2

set BCPGM=%BCROOT%\BIN
set INCLUDE=%BCROOT%\INCLUDE
set LIBPATH=%BCROOT%\LIB
set COMPILER=%BCPGM%\BCC.EXE
set LINKER=%BCPGM%\TLINK.EXE
set TLIB=%BCPGM%\TLIB.EXE
set MAKE=%BCPGM%\MAKE.EXE
set BCCOMPILER=bcos2
set LIBSDIR=\PCBCBASE
set DEBUG=0
set TD=0

if not exist \OUT\NUL mkdir \OUT
if not exist \OUT\PWA153\NUL mkdir \OUT\PWA153

if "%1"=="CLEAN" goto CLEANALL
if "%1"=="clean" goto CLEANALL
if "%1"=="USERNET2" goto USERNET2
if "%1"=="PCBOARD2" goto PCBOARD2
if "%1"=="" goto PCBOARD2
goto BADTGT

:PCBOARD2
echo --- PCBOARD2 (OS/2)
cd \PCB153
if not exist OBJ\NUL mkdir OBJ
if not exist OBJ\BCOS2\NUL mkdir OBJ\BCOS2
%MAKE% -f153\PCBOARD2.MAK
if exist OBJ\BCOS2\PCBOARD2.EXE copy OBJ\BCOS2\PCBOARD2.EXE \OUT\PWA153\PCBOARD2.EXE
if exist OBJ\BCOS2\PCBOARD2.EXE echo     built  \OUT\PWA153\PCBOARD2.EXE
if not exist OBJ\BCOS2\PCBOARD2.EXE echo     NOT BUILT -- see the make output
cd \
if "%1"=="" goto USERNET2
goto END

:USERNET2
echo --- USERNET2 (OS/2)
cd \PCB153\SOURCE\MISC\USERNET
if not exist BCOS2\NUL mkdir BCOS2
set PROGNAME=USERNET2
%MAKE% -fUSERNET2.MAK
set PROGNAME=
if exist BCOS2\USERNET2.EXE copy BCOS2\USERNET2.EXE \OUT\PWA153\USERNET2.EXE
if exist BCOS2\USERNET2.EXE echo     built  \OUT\PWA153\USERNET2.EXE
if not exist BCOS2\USERNET2.EXE echo     NOT BUILT -- see the make output
cd \
goto END

:CLEANALL
cd \PCB153
%MAKE% -f153\PCBOARD2.MAK CLEAN
cd \PCB153\SOURCE\MISC\USERNET
%MAKE% -fUSERNET2.MAK CLEAN
cd \
echo  CLEAN done.  \OUT\PWA153 was not touched.
goto END

:BADTGT
echo Unknown target "%1".  Targets: PCBOARD2  USERNET2  CLEAN
goto END

:NOROOT
echo ERROR: \APPLY.txt not found.  The repo root must be the root of the
echo        drive this runs from, so \OUT and \PCB153 are inside the repo.
goto END

:NOBCOS2
echo ERROR: Borland C++ for OS/2 not found at %BCROOT%\BIN\BCC.EXE
echo        Set BCROOT first, e.g.   set BCROOT=\BCOS2
echo.
echo NOTE: PCBOARD2 also needs the CodeBase OS/2 library
echo       (%LIBSDIR%\CODEBASE\OS2BOR1\B4.LIB) and LXBFIX, neither of which
echo       is in the repo yet.  See APPLY.txt.
goto END

:END
