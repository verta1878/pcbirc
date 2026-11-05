@echo off
c:
cd \pcbcbase\codebase\source
set PATH=c:\bc31\bin;%PATH%
echo ===== Building c4base.lib with BCC 3.1 =====
echo Compiling b4block.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source b4block.c
if errorlevel 1 goto err
echo Compiling c4.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source c4.c
if errorlevel 1 goto err
echo Compiling c4bcd.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source c4bcd.c
if errorlevel 1 goto err
echo Compiling c4code.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source c4code.c
if errorlevel 1 goto err
echo Compiling c4const.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source c4const.c
if errorlevel 1 goto err
echo Compiling d4append.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4append.c
if errorlevel 1 goto err
echo Compiling d4close.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4close.c
if errorlevel 1 goto err
echo Compiling d4create.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4create.c
if errorlevel 1 goto err
echo Compiling d4data.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4data.c
if errorlevel 1 goto err
echo Compiling d4date.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4date.c
if errorlevel 1 goto err
echo Compiling d4field.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4field.c
if errorlevel 1 goto err
echo Compiling d4flush.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4flush.c
if errorlevel 1 goto err
echo Compiling d4fresh.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4fresh.c
if errorlevel 1 goto err
echo Compiling d4go.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4go.c
if errorlevel 1 goto err
echo Compiling d4index.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4index.c
if errorlevel 1 goto err
echo Compiling d4lock.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4lock.c
if errorlevel 1 goto err
echo Compiling d4open.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4open.c
if errorlevel 1 goto err
echo Compiling d4opt.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4opt.c
if errorlevel 1 goto err
echo Compiling d4pack.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4pack.c
if errorlevel 1 goto err
echo Compiling d4positi.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4positi.c
if errorlevel 1 goto err
echo Compiling d4seek.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4seek.c
if errorlevel 1 goto err
echo Compiling d4skip.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4skip.c
if errorlevel 1 goto err
echo Compiling d4tag.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4tag.c
if errorlevel 1 goto err
echo Compiling d4unlock.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4unlock.c
if errorlevel 1 goto err
echo Compiling d4write.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4write.c
if errorlevel 1 goto err
echo Compiling d4zap.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source d4zap.c
if errorlevel 1 goto err
echo Compiling e4calc.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source e4calc.c
if errorlevel 1 goto err
echo Compiling e4error.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source e4error.c
if errorlevel 1 goto err
echo Compiling e4expr.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source e4expr.c
if errorlevel 1 goto err
echo Compiling e4functi.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source e4functi.c
if errorlevel 1 goto err
echo Compiling e4not_s.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source e4not_s.c
if errorlevel 1 goto err
echo Compiling e4parse.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source e4parse.c
if errorlevel 1 goto err
echo Compiling f4ass_f.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4ass_f.c
if errorlevel 1 goto err
echo Compiling f4char.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4char.c
if errorlevel 1 goto err
echo Compiling f4close.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4close.c
if errorlevel 1 goto err
echo Compiling f4create.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4create.c
if errorlevel 1 goto err
echo Compiling f4double.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4double.c
if errorlevel 1 goto err
echo Compiling f4field.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4field.c
if errorlevel 1 goto err
echo Compiling f4file.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4file.c
if errorlevel 1 goto err
echo Compiling f4filese.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4filese.c
if errorlevel 1 goto err
echo Compiling f4flag.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4flag.c
if errorlevel 1 goto err
echo Compiling f4flush.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4flush.c
if errorlevel 1 goto err
echo Compiling f4info.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4info.c
if errorlevel 1 goto err
echo Compiling f4int.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4int.c
if errorlevel 1 goto err
echo Compiling f4lock.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4lock.c
if errorlevel 1 goto err
echo Compiling f4long.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4long.c
if errorlevel 1 goto err
echo Compiling f4memo.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4memo.c
if errorlevel 1 goto err
echo Compiling f4open.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4open.c
if errorlevel 1 goto err
echo Compiling f4opt.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4opt.c
if errorlevel 1 goto err
echo Compiling f4ptr.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4ptr.c
if errorlevel 1 goto err
echo Compiling f4str.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4str.c
if errorlevel 1 goto err
echo Compiling f4temp.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4temp.c
if errorlevel 1 goto err
echo Compiling f4true.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4true.c
if errorlevel 1 goto err
echo Compiling f4write.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source f4write.c
if errorlevel 1 goto err
echo Compiling i4add.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4add.c
if errorlevel 1 goto err
echo Compiling i4addtag.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4addtag.c
if errorlevel 1 goto err
echo Compiling i4check.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4check.c
if errorlevel 1 goto err
echo Compiling i4create.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4create.c
if errorlevel 1 goto err
echo Compiling i4dump.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4dump.c
if errorlevel 1 goto err
echo Compiling i4index.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4index.c
if errorlevel 1 goto err
echo Compiling i4info.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4info.c
if errorlevel 1 goto err
echo Compiling i4init.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4init.c
if errorlevel 1 goto err
echo Compiling i4key.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4key.c
if errorlevel 1 goto err
echo Compiling i4lock.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4lock.c
if errorlevel 1 goto err
echo Compiling i4ntag.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4ntag.c
if errorlevel 1 goto err
echo Compiling i4positi.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4positi.c
if errorlevel 1 goto err
echo Compiling i4remove.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4remove.c
if errorlevel 1 goto err
echo Compiling i4tag.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source i4tag.c
if errorlevel 1 goto err
echo Compiling l4link.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source l4link.c
if errorlevel 1 goto err
echo Compiling l4lock_c.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source l4lock_c.c
if errorlevel 1 goto err
echo Compiling m4check.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source m4check.c
if errorlevel 1 goto err
echo Compiling m4create.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source m4create.c
if errorlevel 1 goto err
echo Compiling m4file.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source m4file.c
if errorlevel 1 goto err
echo Compiling m4map.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source m4map.c
if errorlevel 1 goto err
echo Compiling m4memo.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source m4memo.c
if errorlevel 1 goto err
echo Compiling m4memory.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source m4memory.c
if errorlevel 1 goto err
echo Compiling mem.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source mem.c
if errorlevel 1 goto err
echo Compiling o4opt.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source o4opt.c
if errorlevel 1 goto err
echo Compiling r4code.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4code.c
if errorlevel 1 goto err
echo Compiling r4driver.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4driver.c
if errorlevel 1 goto err
echo Compiling r4group.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4group.c
if errorlevel 1 goto err
echo Compiling r4log.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4log.c
if errorlevel 1 goto err
echo Compiling r4object.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4object.c
if errorlevel 1 goto err
echo Compiling r4reinde.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4reinde.c
if errorlevel 1 goto err
echo Compiling r4reindx.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4reindx.c
if errorlevel 1 goto err
echo Compiling r4relate.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4relate.c
if errorlevel 1 goto err
echo Compiling r4report.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4report.c
if errorlevel 1 goto err
echo Compiling r4save.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4save.c
if errorlevel 1 goto err
echo Compiling r4save_m.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4save_m.c
if errorlevel 1 goto err
echo Compiling r4styles.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4styles.c
if errorlevel 1 goto err
echo Compiling r4text.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4text.c
if errorlevel 1 goto err
echo Compiling r4total.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source r4total.c
if errorlevel 1 goto err
echo Compiling s4init.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source s4init.c
if errorlevel 1 goto err
echo Compiling s4initfr.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source s4initfr.c
if errorlevel 1 goto err
echo Compiling s4next.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source s4next.c
if errorlevel 1 goto err
echo Compiling s4quick.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source s4quick.c
if errorlevel 1 goto err
echo Compiling s4sort.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source s4sort.c
if errorlevel 1 goto err
echo Compiling u4name.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source u4name.c
if errorlevel 1 goto err
echo Compiling u4util.c
bcc -c -ml -Ic:\bc31\include -Ic:\pcbcbase\codebase\source u4util.c
if errorlevel 1 goto err
echo ===== All files compiled, building library =====
cd \pcbcbase\codebase\bor31
del c4base.lib >nul 2>nul
tlib c4base.lib +\pcbcbase\codebase\source\b4block.obj &
+\pcbcbase\codebase\source\c4.obj &
+\pcbcbase\codebase\source\c4bcd.obj &
+\pcbcbase\codebase\source\c4code.obj &
+\pcbcbase\codebase\source\c4const.obj &
+\pcbcbase\codebase\source\d4append.obj &
+\pcbcbase\codebase\source\d4close.obj &
+\pcbcbase\codebase\source\d4create.obj &
+\pcbcbase\codebase\source\d4data.obj &
+\pcbcbase\codebase\source\d4date.obj &
+\pcbcbase\codebase\source\d4field.obj &
+\pcbcbase\codebase\source\d4flush.obj &
+\pcbcbase\codebase\source\d4fresh.obj &
+\pcbcbase\codebase\source\d4go.obj &
+\pcbcbase\codebase\source\d4index.obj &
+\pcbcbase\codebase\source\d4lock.obj &
+\pcbcbase\codebase\source\d4open.obj &
+\pcbcbase\codebase\source\d4opt.obj &
+\pcbcbase\codebase\source\d4pack.obj &
+\pcbcbase\codebase\source\d4positi.obj &
+\pcbcbase\codebase\source\d4seek.obj &
+\pcbcbase\codebase\source\d4skip.obj &
+\pcbcbase\codebase\source\d4tag.obj &
+\pcbcbase\codebase\source\d4unlock.obj &
+\pcbcbase\codebase\source\d4write.obj &
+\pcbcbase\codebase\source\d4zap.obj &
+\pcbcbase\codebase\source\e4calc.obj &
+\pcbcbase\codebase\source\e4error.obj &
+\pcbcbase\codebase\source\e4expr.obj &
+\pcbcbase\codebase\source\e4functi.obj &
+\pcbcbase\codebase\source\e4not_s.obj &
+\pcbcbase\codebase\source\e4parse.obj &
+\pcbcbase\codebase\source\f4ass_f.obj &
+\pcbcbase\codebase\source\f4char.obj &
+\pcbcbase\codebase\source\f4close.obj &
+\pcbcbase\codebase\source\f4create.obj &
+\pcbcbase\codebase\source\f4double.obj &
+\pcbcbase\codebase\source\f4field.obj &
+\pcbcbase\codebase\source\f4file.obj &
+\pcbcbase\codebase\source\f4filese.obj &
+\pcbcbase\codebase\source\f4flag.obj &
+\pcbcbase\codebase\source\f4flush.obj &
+\pcbcbase\codebase\source\f4info.obj &
+\pcbcbase\codebase\source\f4int.obj &
+\pcbcbase\codebase\source\f4lock.obj &
+\pcbcbase\codebase\source\f4long.obj &
+\pcbcbase\codebase\source\f4memo.obj &
+\pcbcbase\codebase\source\f4open.obj &
+\pcbcbase\codebase\source\f4opt.obj &
+\pcbcbase\codebase\source\f4ptr.obj &
+\pcbcbase\codebase\source\f4str.obj &
+\pcbcbase\codebase\source\f4temp.obj &
+\pcbcbase\codebase\source\f4true.obj &
+\pcbcbase\codebase\source\f4write.obj &
+\pcbcbase\codebase\source\i4add.obj &
+\pcbcbase\codebase\source\i4addtag.obj &
+\pcbcbase\codebase\source\i4check.obj &
+\pcbcbase\codebase\source\i4create.obj &
+\pcbcbase\codebase\source\i4dump.obj &
+\pcbcbase\codebase\source\i4index.obj &
+\pcbcbase\codebase\source\i4info.obj &
+\pcbcbase\codebase\source\i4init.obj &
+\pcbcbase\codebase\source\i4key.obj &
+\pcbcbase\codebase\source\i4lock.obj &
+\pcbcbase\codebase\source\i4ntag.obj &
+\pcbcbase\codebase\source\i4positi.obj &
+\pcbcbase\codebase\source\i4remove.obj &
+\pcbcbase\codebase\source\i4tag.obj &
+\pcbcbase\codebase\source\l4link.obj &
+\pcbcbase\codebase\source\l4lock_c.obj &
+\pcbcbase\codebase\source\m4check.obj &
+\pcbcbase\codebase\source\m4create.obj &
+\pcbcbase\codebase\source\m4file.obj &
+\pcbcbase\codebase\source\m4map.obj &
+\pcbcbase\codebase\source\m4memo.obj &
+\pcbcbase\codebase\source\m4memory.obj &
+\pcbcbase\codebase\source\mem.obj &
+\pcbcbase\codebase\source\o4opt.obj &
+\pcbcbase\codebase\source\r4code.obj &
+\pcbcbase\codebase\source\r4driver.obj &
+\pcbcbase\codebase\source\r4group.obj &
+\pcbcbase\codebase\source\r4log.obj &
+\pcbcbase\codebase\source\r4object.obj &
+\pcbcbase\codebase\source\r4reinde.obj &
+\pcbcbase\codebase\source\r4reindx.obj &
+\pcbcbase\codebase\source\r4relate.obj &
+\pcbcbase\codebase\source\r4report.obj &
+\pcbcbase\codebase\source\r4save.obj &
+\pcbcbase\codebase\source\r4save_m.obj &
+\pcbcbase\codebase\source\r4styles.obj &
+\pcbcbase\codebase\source\r4text.obj &
+\pcbcbase\codebase\source\r4total.obj &
+\pcbcbase\codebase\source\s4init.obj &
+\pcbcbase\codebase\source\s4initfr.obj &
+\pcbcbase\codebase\source\s4next.obj &
+\pcbcbase\codebase\source\s4quick.obj &
+\pcbcbase\codebase\source\s4sort.obj &
+\pcbcbase\codebase\source\u4name.obj &
+\pcbcbase\codebase\source\u4util.obj
if errorlevel 1 goto err
echo ===== c4base.lib BUILT SUCCESSFULLY =====
goto done
:err
echo !!!!! ERROR OCCURRED !!!!!
:done
