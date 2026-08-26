@echo off
tasm /w2 /mx /t /D__s__ %1,small\%1
tasm /w2 /mx /t /D__c__ %1,compact\%1
tasm /w2 /mx /t /D__m__ %1,medium\%1
tasm /w2 /mx /t /D__l__ %1,large\%1
