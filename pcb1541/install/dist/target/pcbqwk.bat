if %1==compress pkzip -ex -m %2 @%4
if %1==extract  pkunzip -o %2 %3
