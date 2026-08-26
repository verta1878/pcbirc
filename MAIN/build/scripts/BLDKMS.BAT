@echo off
REM ============================================================
REM  BLDKMS.BAT - build PCBKMS (Microsoft C 7.0 family)
REM ============================================================
echo.
echo PCBKMS (Microsoft C 7.0) - two host routes:
echo.
echo   The MSC 7.0 compiler is in C:\MSC70. It TARGETS 16-bit
echo   (8086) code in all four memory models - exactly what
echo   PCBKMS needs. The catch is the compiler HOST:
echo.
echo   Route A - DOS host (C:\MSC70\BIN):
echo     CL - C13216/C23216/C33216 need a DPMI host. Under
echo     plain DOS/DOSBox this fails (R6901). Install 386-Max
echo     (or run under Windows) to provide DPMI, then this
echo     script can drive CL like BLDKBC/BLDKIT do.
echo.
echo   Route B - OS/2 host (C:\MSC70\OS2\BINP) - NO DPMI:
echo     C11616/C21616/C31616 run natively under OS/2 in real
echo     16-bit mode. Run this build under OS/2 (or OS/2
echo     emulation) with C:\MSC70\OS2\BINP on the PATH and
echo     INCLUDE/LIB pointing at C:\MSC70\INCLUDE and
echo     C:\MSC70\LIB. This is the preferred route.
echo.
echo   Scaffolding is ready: 119-module manifest, obj\msc70
echo   dirs, and MS??.RSP response files. Once a working host
echo   is in place, PCBKMS_S/M/C/L.LIB build the same way as
echo   KBC and KIT (compile 119 x 4 models, then MKLIB).
echo.
echo   See C:\MSC70\README.md for full details.
echo.
