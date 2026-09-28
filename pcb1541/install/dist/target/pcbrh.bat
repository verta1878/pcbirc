echo off
if exist pcberr.fil del pcberr.fil
if exist %dszlog% del %dszlog%
hslink -hs -a -p%1 -e%5 -u%3
