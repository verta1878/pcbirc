if %1==COMPRESS pkzip -ex -m %2 @%4
if %1==EXTRACT  pkunzip -o %2 %3
