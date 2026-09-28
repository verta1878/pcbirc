echo off
if exist pcberr.fil del pcberr.fil
bimodem /l %1 /b %5 /e0 /r %6
if errorlevel 1 goto bad
goto end
:bad
copy pcberr.old pcberr.fil
:end
