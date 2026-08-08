@echo off
REM pcbis_startup.bat — Start PCBoard BBS + netmodem2irc (Windows)
REM Part of pcbrevival (GPL v3.0)

if "%PCBIS_ROOT%"=="" set PCBIS_ROOT=%USERPROFILE%\pcboard

echo [%date% %time%] pcbis_startup: beginning >> "%PCBIS_ROOT%\logs\pcbis.log"

REM Check prerequisites
if not exist "%PCBIS_ROOT%\dosbox.conf" (
    echo ERROR: Not initialized. Run pcbis_initv first.
    exit /b 1
)

REM Create logs dir if needed
if not exist "%PCBIS_ROOT%\logs" mkdir "%PCBIS_ROOT%\logs"

REM Start netmodem2irc
if exist "%PCBIS_ROOT%\netmodem\NMServer.exe" (
    echo Starting NMServer.exe...
    start /b "netmodem2irc" "%PCBIS_ROOT%\netmodem\NMServer.exe" > "%PCBIS_ROOT%\logs\pcbis-netmodem.log" 2>&1
    timeout /t 2 /nobreak > nul
) else (
    echo WARNING: netmodem2irc not found at %PCBIS_ROOT%\netmodem\NMServer.exe
    echo          Install netmodem2irc for telnet access.
)

REM Start DOSBox with PCBoard
echo Starting DOSBox + PCBoard...
start "PCBoard" dosbox -conf "%PCBIS_ROOT%\dosbox.conf"

echo [%date% %time%] pcbis_startup: complete >> "%PCBIS_ROOT%\logs\pcbis.log"
echo.
echo PCBoard is running.
echo   Telnet: telnet localhost 23
echo   Logs:   %PCBIS_ROOT%\logs\
echo   Stop:   pcbis_shutdown.bat
