@echo off
testfile %1 arc zip pak txt
if errorlevel == 98 goto error
if errorlevel == 4 goto txt
if errorlevel == 3 goto pak
if errorlevel == 2 goto zip
if errorlevel == 1 goto arc
goto end

:txt
copy %1 pcbview.txt
goto end

:arc
viewarch %1
goto end

:zip
viewzip %1
goto end

:pak
pak v %1 > pcbview.txt
goto end

:error
echo. > pcbview.txt
echo @x0csorry, @first@, this file is not viewable. >>pcbview.txt
goto end

:end
echo. >> pcbview.txt
echo @wait@ >> pcbview.txt
