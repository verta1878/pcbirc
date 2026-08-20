@echo off
REM pcbis_initv.bat — PCBoard first-time setup (Windows)
REM Part of pcbrevival (GPL v3.0)

if "%PCBIS_ROOT%"=="" set PCBIS_ROOT=%USERPROFILE%\pcboard
if "%PCBIS_PORT%"=="" set PCBIS_PORT=23

echo ╔══════════════════════════════════════════════════╗
echo ║  PCBoard 15.4 Installation System (pcbis)       ║
echo ║  Part of pcbrevival                              ║
echo ╚══════════════════════════════════════════════════╝
echo.
echo Installing to: %PCBIS_ROOT%
echo Telnet port:   %PCBIS_PORT%
echo.

REM Create directory structure
mkdir "%PCBIS_ROOT%" 2>nul
mkdir "%PCBIS_ROOT%\bin" 2>nul
mkdir "%PCBIS_ROOT%\data" 2>nul
mkdir "%PCBIS_ROOT%\fossil" 2>nul
mkdir "%PCBIS_ROOT%\work" 2>nul
mkdir "%PCBIS_ROOT%\logs" 2>nul
mkdir "%PCBIS_ROOT%\nodes" 2>nul
mkdir "%PCBIS_ROOT%\nodes\node1" 2>nul
mkdir "%PCBIS_ROOT%\netmodem" 2>nul

echo [1/4] Directory structure created

REM Create DOSBox config
(
echo [sdl]
echo output=surface
echo fullscreen=false
echo.
echo [cpu]
echo cycles=max
echo.
echo [serial]
echo serial1=nullmodem server:localhost port:123
echo.
echo [autoexec]
echo @echo off
echo mount C "%PCBIS_ROOT%"
echo C:
echo cd bin
echo PCBOARD.EXE /N:1
echo exit
) > "%PCBIS_ROOT%\dosbox.conf"
echo [2/4] DOSBox config generated

REM Create pcbis.cfg
(
echo # pcbis.cfg — PCBoard Installation System configuration
echo listen_port=%PCBIS_PORT%
echo forward_port=123
echo fossil_mode=true
echo baud_rate=115200
echo nodes=1
echo log_file=%PCBIS_ROOT%\logs\pcbis-netmodem.log
) > "%PCBIS_ROOT%\pcbis.cfg"
echo [3/4] Configuration created

REM Create minimal WELCOME
(
echo @CLS@@POFF@
echo @X0FPCBOARD 15.4 BBS@X07
echo @X0Bpowered by pcbrevival@X07
echo.
echo Welcome to PCBoard!
echo Type your name at the login prompt, or NEW if you're a new user.
echo.
echo @PON@
) > "%PCBIS_ROOT%\data\WELCOME"
echo [4/4] WELCOME screen created

echo.
echo Installation complete!
echo.
echo Next steps:
echo   1. Copy PCBoard binaries to %PCBIS_ROOT%\bin\
echo   2. Copy your PCBOARD.DAT to %PCBIS_ROOT%\data\
echo   3. Install netmodem2irc to %PCBIS_ROOT%\netmodem\
echo   4. Run: pcbis_ui.exe    (to configure)
echo   5. Run: pcbis_startup.bat (to start the BBS)
