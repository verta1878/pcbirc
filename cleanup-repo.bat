@echo off
rem ============================================================
rem  fpc264irc repo cleanup — removes build artifacts
rem  Run from the fpc264irc repo root directory
rem  2026-10-01 — byte (program discovery)
rem ============================================================

echo fpc264irc repo cleanup
echo =====================
echo.

rem --- Build temp files in repo root ---
echo Removing build temp files...
if exist ppas.sh    del ppas.sh
if exist link.res   del link.res
if exist script.res del script.res
echo   ppas.sh, link.res, script.res

rem --- .o build artifacts in src/ (NOT sdk/emx — those are SDK objects) ---
echo.
echo Removing .o build artifacts from src/...
if exist src\packages\usb\src\usbcore.o   del src\packages\usb\src\usbcore.o
if exist src\packages\usb\src\usbhub.o    del src\packages\usb\src\usbhub.o
if exist src\packages\usb\src\usbmsd.o    del src\packages\usb\src\usbmsd.o
if exist src\packages\usb\src\usbtrans.o  del src\packages\usb\src\usbtrans.o
if exist src\rtl\usb\libusb.o             del src\rtl\usb\libusb.o
if exist src\rtl\usb\usbserial.o          del src\rtl\usb\usbserial.o
echo   6 files from src/packages/usb and src/rtl/usb

rem --- installer build artifact ---
echo.
echo Removing installer build artifact...
if exist installer\install.o del installer\install.o
echo   installer/install.o

rem --- Stale units in src/ (BUG-006 safety sweep) ---
echo.
echo Checking for stale PPUs in src/rtl/units and src/packages/*/units...
if exist src\rtl\units\*.ppu (
    echo   WARNING: stale PPUs found in src\rtl\units — removing
    del /q src\rtl\units\*.ppu
    del /q src\rtl\units\*.o 2>nul
) else (
    echo   clean
)

rem --- .s assembly temp files anywhere outside bin/ ---
echo.
echo Checking for .s assembly temps outside bin/...
for /r %%f in (*.s) do (
    echo %%f | findstr /v /i "\\bin\\" >nul && (
        echo %%f | findstr /v /i "\\.git\\" >nul && (
            echo %%f | findstr /v /i "\\sdk\\" >nul && (
                echo   removing %%f
                del "%%f"
            )
        )
    )
)

rem --- NOT cleaned (intentional) ---
echo.
echo Kept (not build artifacts):
echo   sdk/emx/lib/*.o              — OS/2 EMX SDK objects (needed for cross-compile)
echo   bin/**/*.o                    — compiled unit objects (shipped with compiler)
echo   bin/**/*.ppu                  — compiled units (shipped with compiler)
echo   attic/                        — archived originals, never delete
echo.
echo Note: i386-darwin graph.ppu and pthreads.ppu have no matching .o files.
echo   They were cross-compiled with -s (skip assembler) on a Linux host.
echo   This is normal for PPU-only cross-compile targets.

echo.
echo Done. Review with: git status
echo Remember: never commit .o or .s files from src/ — only .ppu, .rst, .a and source.
