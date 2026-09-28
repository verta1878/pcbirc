@echo off
rem PCBTEST.BAT — PCBoard upload file test script
rem Called by PCBoard after each file upload
rem
rem %1 = full path to uploaded file
rem %2 = UPLOAD, ATTACH, or TEST
rem %3 = upload description file path
rem %4 = original filename
rem
rem Exit: creates PCBFAIL.TXT if file fails
rem       creates PCBPASS.TXT if file passes

pcbpscan %1 %2 %3 %4
if errorlevel 1 echo File failed testing > PCBFAIL.TXT
