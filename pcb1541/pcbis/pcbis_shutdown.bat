@echo off
rem pcbis_shutdown.bat — stop pcboard bbs + netmodem2irc (windows)
rem part of pcbrevival (gpl v3.0)

if "%pcbis_root%"=="" set pcbis_root=%userprofile%\pcboard

echo [%date% %time%] pcbis_shutdown: beginning >> "%pcbis_root%\logs\pcbis.log"

rem stop dosbox
taskkill /im dosbox.exe /f 2>nul
if %errorlevel%==0 echo dosbox stopped.

rem stop netmodem2irc
taskkill /im nmserver.exe /f 2>nul
if %errorlevel%==0 echo netmodem2irc stopped.

echo [%date% %time%] pcbis_shutdown: complete >> "%pcbis_root%\logs\pcbis.log"
echo pcboard stopped.
