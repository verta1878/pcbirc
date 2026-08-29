@echo off
certutil -addstore TrustedPublisher cytest.cer
certutil -addstore Root cytest.cer
signtool sign /v /f cytest.pfx ..\cyport.sys
signtool sign /v /f cytest.pfx ..\cyinstall.exe
signtool sign /v /f cytest.pfx ..\cyloopback.exe
signtool sign /v /f cytest.pfx ..\cystress.exe
bcdedit -set TESTSIGNING ON
echo DONE. Reboot required.
pause
