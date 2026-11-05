@echo off
rem ============================================================================
rem  bldcb4.bat — build c4base.lib from codebase 4 source (99 .c files)
rem  compiler: borland c++ 3.1, large model
rem  output:   \pcbcbase\codebase\bor31\c4base.lib
rem ============================================================================
c:
set bc31path=\bc31
set cbsrc=\pcbcbase\codebase\source
set cbobj=\tmp\cbobj
set cbout=\pcbcbase\codebase\bor31
set bcc=%bc31path%\bin\bcc.exe
set tlib=%bc31path%\bin\tlib.exe

rem clean
if exist %cbobj%\nul del %cbobj%\*.obj > nul 2>&1
if not exist %cbobj%\nul mkdir %cbobj%

rem compile flags: large model, no warnings-as-errors, codebase source as include
set cflags=-ml -c -w -i%bc31path%\include -i%cbsrc% -n%cbobj%

echo compiling codebase source (99 files)...

%bcc% %cflags% %cbsrc%\b4block.c
%bcc% %cflags% %cbsrc%\c4.c
%bcc% %cflags% %cbsrc%\c4bcd.c
%bcc% %cflags% %cbsrc%\c4code.c
%bcc% %cflags% %cbsrc%\c4const.c
%bcc% %cflags% %cbsrc%\d4append.c
%bcc% %cflags% %cbsrc%\d4close.c
%bcc% %cflags% %cbsrc%\d4create.c
%bcc% %cflags% %cbsrc%\d4data.c
%bcc% %cflags% %cbsrc%\d4date.c
%bcc% %cflags% %cbsrc%\d4field.c
%bcc% %cflags% %cbsrc%\d4flush.c
%bcc% %cflags% %cbsrc%\d4fresh.c
%bcc% %cflags% %cbsrc%\d4go.c
%bcc% %cflags% %cbsrc%\d4index.c
%bcc% %cflags% %cbsrc%\d4lock.c
%bcc% %cflags% %cbsrc%\d4open.c
%bcc% %cflags% %cbsrc%\d4opt.c
%bcc% %cflags% %cbsrc%\d4pack.c
%bcc% %cflags% %cbsrc%\d4positi.c
%bcc% %cflags% %cbsrc%\d4seek.c
%bcc% %cflags% %cbsrc%\d4skip.c
%bcc% %cflags% %cbsrc%\d4tag.c
%bcc% %cflags% %cbsrc%\d4unlock.c
%bcc% %cflags% %cbsrc%\d4write.c
%bcc% %cflags% %cbsrc%\d4zap.c
%bcc% %cflags% %cbsrc%\e4calc.c
%bcc% %cflags% %cbsrc%\e4error.c
%bcc% %cflags% %cbsrc%\e4expr.c
%bcc% %cflags% %cbsrc%\e4functi.c
%bcc% %cflags% %cbsrc%\e4not_s.c
%bcc% %cflags% %cbsrc%\e4parse.c
%bcc% %cflags% %cbsrc%\f4ass_f.c
%bcc% %cflags% %cbsrc%\f4char.c
%bcc% %cflags% %cbsrc%\f4close.c
%bcc% %cflags% %cbsrc%\f4create.c
%bcc% %cflags% %cbsrc%\f4double.c
%bcc% %cflags% %cbsrc%\f4field.c
%bcc% %cflags% %cbsrc%\f4file.c
%bcc% %cflags% %cbsrc%\f4filese.c
%bcc% %cflags% %cbsrc%\f4flag.c
%bcc% %cflags% %cbsrc%\f4flush.c
%bcc% %cflags% %cbsrc%\f4info.c
%bcc% %cflags% %cbsrc%\f4int.c
%bcc% %cflags% %cbsrc%\f4lock.c
%bcc% %cflags% %cbsrc%\f4long.c
%bcc% %cflags% %cbsrc%\f4memo.c
%bcc% %cflags% %cbsrc%\f4open.c
%bcc% %cflags% %cbsrc%\f4opt.c
%bcc% %cflags% %cbsrc%\f4ptr.c
%bcc% %cflags% %cbsrc%\f4str.c
%bcc% %cflags% %cbsrc%\f4temp.c
%bcc% %cflags% %cbsrc%\f4true.c
%bcc% %cflags% %cbsrc%\f4write.c
%bcc% %cflags% %cbsrc%\i4add.c
%bcc% %cflags% %cbsrc%\i4addtag.c
%bcc% %cflags% %cbsrc%\i4check.c
%bcc% %cflags% %cbsrc%\i4create.c
%bcc% %cflags% %cbsrc%\i4dump.c
%bcc% %cflags% %cbsrc%\i4index.c
%bcc% %cflags% %cbsrc%\i4info.c
%bcc% %cflags% %cbsrc%\i4init.c
%bcc% %cflags% %cbsrc%\i4key.c
%bcc% %cflags% %cbsrc%\i4lock.c
%bcc% %cflags% %cbsrc%\i4ntag.c
%bcc% %cflags% %cbsrc%\i4positi.c
%bcc% %cflags% %cbsrc%\i4remove.c
%bcc% %cflags% %cbsrc%\i4tag.c
%bcc% %cflags% %cbsrc%\l4link.c
%bcc% %cflags% %cbsrc%\l4lock_c.c
%bcc% %cflags% %cbsrc%\m4check.c
%bcc% %cflags% %cbsrc%\m4create.c
%bcc% %cflags% %cbsrc%\m4file.c
%bcc% %cflags% %cbsrc%\m4map.c
%bcc% %cflags% %cbsrc%\m4memo.c
%bcc% %cflags% %cbsrc%\m4memory.c
%bcc% %cflags% %cbsrc%\mem.c
%bcc% %cflags% %cbsrc%\o4opt.c
%bcc% %cflags% %cbsrc%\r4code.c
%bcc% %cflags% %cbsrc%\r4driver.c
%bcc% %cflags% %cbsrc%\r4group.c
%bcc% %cflags% %cbsrc%\r4log.c
%bcc% %cflags% %cbsrc%\r4object.c
%bcc% %cflags% %cbsrc%\r4reinde.c
%bcc% %cflags% %cbsrc%\r4reindx.c
%bcc% %cflags% %cbsrc%\r4relate.c
%bcc% %cflags% %cbsrc%\r4report.c
%bcc% %cflags% %cbsrc%\r4save.c
%bcc% %cflags% %cbsrc%\r4save_m.c
%bcc% %cflags% %cbsrc%\r4styles.c
%bcc% %cflags% %cbsrc%\r4text.c
%bcc% %cflags% %cbsrc%\r4total.c
%bcc% %cflags% %cbsrc%\s4init.c
%bcc% %cflags% %cbsrc%\s4initfr.c
%bcc% %cflags% %cbsrc%\s4next.c
%bcc% %cflags% %cbsrc%\s4quick.c
%bcc% %cflags% %cbsrc%\s4sort.c
%bcc% %cflags% %cbsrc%\u4name.c
%bcc% %cflags% %cbsrc%\u4util.c

echo.
echo compile phase done. counting objs...
dir %cbobj%\*.obj > \tmp\c4objs.txt

rem build the library with tlib using one-at-a-time adds (dos cmd line limit)
if not exist %cbout%\nul mkdir %cbout%
if exist %cbout%\c4base.lib del %cbout%\c4base.lib
if exist %cbout%\c4base.lib del %cbout%\c4base.lib
if exist %cbout%\c4base.bak del %cbout%\c4base.bak

echo building c4base.lib (adding modules one at a time)...
%tlib% %cbout%\c4base.lib +%cbobj%\b4block.obj
%tlib% %cbout%\c4base.lib +%cbobj%\c4.obj
%tlib% %cbout%\c4base.lib +%cbobj%\c4bcd.obj
%tlib% %cbout%\c4base.lib +%cbobj%\c4code.obj
%tlib% %cbout%\c4base.lib +%cbobj%\c4const.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4append.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4close.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4create.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4data.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4date.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4field.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4flush.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4fresh.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4go.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4index.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4lock.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4open.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4opt.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4pack.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4positi.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4seek.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4skip.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4tag.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4unlock.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4write.obj
%tlib% %cbout%\c4base.lib +%cbobj%\d4zap.obj
%tlib% %cbout%\c4base.lib +%cbobj%\e4calc.obj
%tlib% %cbout%\c4base.lib +%cbobj%\e4error.obj
%tlib% %cbout%\c4base.lib +%cbobj%\e4expr.obj
%tlib% %cbout%\c4base.lib +%cbobj%\e4functi.obj
%tlib% %cbout%\c4base.lib +%cbobj%\e4not_s.obj
%tlib% %cbout%\c4base.lib +%cbobj%\e4parse.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4ass_f.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4char.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4close.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4create.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4double.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4field.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4file.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4filese.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4flag.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4flush.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4info.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4int.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4lock.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4long.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4memo.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4open.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4opt.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4ptr.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4str.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4temp.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4true.obj
%tlib% %cbout%\c4base.lib +%cbobj%\f4write.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4add.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4addtag.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4check.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4create.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4dump.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4index.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4info.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4init.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4key.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4lock.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4ntag.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4positi.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4remove.obj
%tlib% %cbout%\c4base.lib +%cbobj%\i4tag.obj
%tlib% %cbout%\c4base.lib +%cbobj%\l4link.obj
%tlib% %cbout%\c4base.lib +%cbobj%\l4lock_c.obj
%tlib% %cbout%\c4base.lib +%cbobj%\m4check.obj
%tlib% %cbout%\c4base.lib +%cbobj%\m4create.obj
%tlib% %cbout%\c4base.lib +%cbobj%\m4file.obj
%tlib% %cbout%\c4base.lib +%cbobj%\m4map.obj
%tlib% %cbout%\c4base.lib +%cbobj%\m4memo.obj
%tlib% %cbout%\c4base.lib +%cbobj%\m4memory.obj
%tlib% %cbout%\c4base.lib +%cbobj%\mem.obj
%tlib% %cbout%\c4base.lib +%cbobj%\o4opt.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4code.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4driver.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4group.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4log.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4object.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4reinde.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4reindx.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4relate.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4report.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4save.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4save_m.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4styles.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4text.obj
%tlib% %cbout%\c4base.lib +%cbobj%\r4total.obj
%tlib% %cbout%\c4base.lib +%cbobj%\s4init.obj
%tlib% %cbout%\c4base.lib +%cbobj%\s4initfr.obj
%tlib% %cbout%\c4base.lib +%cbobj%\s4next.obj
%tlib% %cbout%\c4base.lib +%cbobj%\s4quick.obj
%tlib% %cbout%\c4base.lib +%cbobj%\s4sort.obj
%tlib% %cbout%\c4base.lib +%cbobj%\u4name.obj
%tlib% %cbout%\c4base.lib +%cbobj%\u4util.obj

echo.
if exist %cbout%\c4base.lib echo success: c4base.lib built
if exist %cbout%\c4base.lib dir %cbout%\c4base.lib
if not exist %cbout%\c4base.lib echo failed: c4base.lib not created

echo done > \tmp\c4done.txt
