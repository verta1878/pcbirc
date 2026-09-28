@echo off
rem pcbis_startup.bat — start pcboard bbs + netmodem2irc (windows)
rem part of pcbrevival (gpl v3.0)

if "%pcbis_root%"=="" set pcbis_root=%userprofile%\pcboard

echo [%date% %time%] pcbis_startup: beginning >> "%pcbis_root%\logs\pcbis.log"

rem check prerequisites
if not exist "%pcbis_root%\dosbox.conf" (
    echo error: not initialized. run pcbis_initv first.
    exit /b 1
)

rem create logs dir if needed
if not exist "%pcbis_root%\logs" mkdir "%pcbis_root%\logs"

rem start netmodem2irc
if exist "%pcbis_root%\netmodem\nmserver.exe" (
    echo starting nmserver.exe...
    start /b "netmodem2irc" "%pcbis_root%\netmodem\nmserver.exe" > "%pcbis_root%\logs\pcbis-netmodem.log" 2>&1
    timeout /t 2 /nobreak > nul
) else (
    echo warning: netmodem2irc not found at %pcbis_root%\netmodem\nmserver.exe
    echo          install netmodem2irc for telnet access.
)

rem start dosbox with pcboard
echo starting dosbox + pcboard...
start "pcboard" dosbox -conf "%pcbis_root%\dosbox.conf"

echo [%date% %time%] pcbis_startup: complete >> "%pcbis_root%\logs\pcbis.log"
echo.
echo pcboard is running.
echo   telnet: telnet localhost 23
echo   logs:   %pcbis_root%\logs\
echo   stop:   pcbis_shutdown.bat
