echo off
if exist pcberr.fil del pcberr.fil
BIMODEM /L %1 /B %5 /E0 /R %6
if errorlevel 1 goto bad
goto end
:bad
copy pcberr.old pcberr.fil
:end
