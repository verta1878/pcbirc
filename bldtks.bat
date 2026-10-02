@echo off
set bc31path=\bc31
set path=\bc31\bin;%path%
echo --- small-model category libs --- > \out\bldtks.log

cd \toolkit\pwa153\source\dos
echo building dos_s... >> \out\bldtks.log
make -ftklib.mak -Dmdl=s -Dmodel=small -Dcfgname=tks >> \out\bldtks.log
echo dos_s done >> \out\bldtks.log

cd \toolkit\pwa153\source\misc
echo building misc_s... >> \out\bldtks.log
make -ftklib.mak -Dmdl=s -Dmodel=small -Dcfgname=tks >> \out\bldtks.log
echo misc_s done >> \out\bldtks.log

cd \toolkit\pwa153\source\screen
echo building screen_s... >> \out\bldtks.log
make -ftklib.mak -Dmdl=s -Dmodel=small -Dcfgname=tks >> \out\bldtks.log
echo screen_s done >> \out\bldtks.log

cd \toolkit\pwa153\source\system
echo building system_s... >> \out\bldtks.log
make -ftklib.mak -Dmdl=s -Dmodel=small -Dcfgname=tks >> \out\bldtks.log
echo system_s done >> \out\bldtks.log

echo --- results --- >> \out\bldtks.log
dir \out\pwa153\sdk\bc31\lib\*_s.lib >> \out\bldtks.log
dir \out\pwa153\sdk\bc31\obj\dos\small\showerr2.obj >> \out\bldtks.log
echo all done >> \out\bldtks.log
