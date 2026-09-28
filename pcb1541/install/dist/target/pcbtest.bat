@echo off

echo þ checking for included file_id.diz for description
if %2 == upload pcbdesc %1 %3

testfile %1 arc zip
if errorlevel == 98 goto end
if errorlevel == 2 goto zip
if errorlevel == 1 goto arc
goto end

:zip
echo þ testing zip file integrity
pkunzip -t %1 > pcbfail.txt
if errorlevel == 1 goto end
del pcbfail.txt
goto end

:arc
echo þ testing arc file integrity
pkxarc -t %1 > pcbfail.txt
if errorlevel == 1 goto end
del pcbfail.txt
goto end

:end
