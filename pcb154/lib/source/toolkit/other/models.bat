@echo off
tasm /w2 /mx /t /d__s__ %1,small\%1
tasm /w2 /mx /t /d__c__ %1,compact\%1
tasm /w2 /mx /t /d__m__ %1,medium\%1
tasm /w2 /mx /t /d__l__ %1,large\%1
