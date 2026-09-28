@echo off
rem pcbtest.bat — pcboard upload file test script
rem called by pcboard after each file upload
rem
rem %1 = full path to uploaded file
rem %2 = upload, attach, or test
rem %3 = upload description file path
rem %4 = original filename
rem
rem exit: creates pcbfail.txt if file fails
rem       creates pcbpass.txt if file passes

pcbpscan %1 %2 %3 %4
if errorlevel 1 echo file failed testing > pcbfail.txt
