@echo off
REM pcbis_shutdown.bat — Stop PCBoard BBS + netmodem2irc (Windows)
REM Part of pcbrevival (GPL v3.0)

if "%PCBIS_ROOT%"=="" set PCBIS_ROOT=%USERPROFILE%\pcboard

echo [%date% %time%] pcbis_shutdown: beginning >> "%PCBIS_ROOT%\logs\pcbis.log"

REM Stop DOSBox
taskkill /IM dosbox.exe /F 2>nul
if %errorlevel%==0 echo DOSBox stopped.

REM Stop netmodem2irc
taskkill /IM NMServer.exe /F 2>nul
if %errorlevel%==0 echo netmodem2irc stopped.

echo [%date% %time%] pcbis_shutdown: complete >> "%PCBIS_ROOT%\logs\pcbis.log"
echo PCBoard stopped.
