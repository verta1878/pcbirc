@echo off
rem pcbis_initv.bat — pcboard first-time setup (windows)
rem part of pcbrevival (gpl v3.0)

if "%pcbis_root%"=="" set pcbis_root=%userprofile%\pcboard
if "%pcbis_port%"=="" set pcbis_port=23

echo ╔══════════════════════════════════════════════════╗
echo ║  pcboard 15.4 installation system (pcbis)       ║
echo ║  part of pcbrevival                              ║
echo ╚══════════════════════════════════════════════════╝
echo.
echo installing to: %pcbis_root%
echo telnet port:   %pcbis_port%
echo.

rem create directory structure
mkdir "%pcbis_root%" 2>nul
mkdir "%pcbis_root%\bin" 2>nul
mkdir "%pcbis_root%\data" 2>nul
mkdir "%pcbis_root%\fossil" 2>nul
mkdir "%pcbis_root%\work" 2>nul
mkdir "%pcbis_root%\logs" 2>nul
mkdir "%pcbis_root%\nodes" 2>nul
mkdir "%pcbis_root%\nodes\node1" 2>nul
mkdir "%pcbis_root%\netmodem" 2>nul

echo [1/4] directory structure created

rem create dosbox config
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
echo mount c "%pcbis_root%"
echo c:
echo cd bin
echo pcboard.exe /n:1
echo exit
) > "%pcbis_root%\dosbox.conf"
echo [2/4] dosbox config generated

rem create pcbis.cfg
(
echo # pcbis.cfg — pcboard installation system configuration
echo listen_port=%pcbis_port%
echo forward_port=123
echo fossil_mode=true
echo baud_rate=115200
echo nodes=1
echo log_file=%pcbis_root%\logs\pcbis-netmodem.log
) > "%pcbis_root%\pcbis.cfg"
echo [3/4] configuration created

rem create minimal welcome
(
echo @cls@@poff@
echo @x0fpcboard 15.4 bbs@x07
echo @x0bpowered by pcbrevival@x07
echo.
echo welcome to pcboard!
echo type your name at the login prompt, or new if you're a new user.
echo.
echo @pon@
) > "%pcbis_root%\data\welcome"
echo [4/4] welcome screen created

echo.
echo installation complete!
echo.
echo next steps:
echo   1. copy pcboard binaries to %pcbis_root%\bin\
echo   2. copy your pcboard.dat to %pcbis_root%\data\
echo   3. install netmodem2irc to %pcbis_root%\netmodem\
echo   4. run: pcbis_ui.exe    (to configure)
echo   5. run: pcbis_startup.bat (to start the bbs)
