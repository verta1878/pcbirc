#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* the information in this module is proprietary software belonging to       */
#* clark development company and is part of the pcboard source code library. */
#* you are granted the right to use this information for the building of any */
#* of the pcboard products you have licensed.  any other usage is forbidden  */
#* without prior written consent from clark development company, inc.        */
#*                                                                           */
#* be sure to read the source code license agreement before utilizing any    */
#* of the source code found herein.                                          */
#*                                                                           */
#* copyright (c) 1996  clark development company, inc.  all rights reserved. */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#=============================================================
#
#	usernet2.mak - makefile for project d:\proj\usernet\usernet2.prj
#		created on 08/11/96 at 23:23
#
#=============================================================

.autodepend

!ifndef root
root     = \out
!endif
!ifndef bcroot
bcroot   = \bcos2
!endif

.path.obj = bcos2

#=============================================================
#		translator definitions
#=============================================================
cc = bcc +usernet2.cfg
tasm = tasm.exe
tlib = tlib.exe
tlink = tlink
rc = brcc.exe
rb = rc.exe
libpath = $(bcroot)\lib
includepath = $(bcroot)\include;$(root)\lib\h


#=============================================================
#		implicit rules
#=============================================================
.c.obj:
  $(cc) -c {$< }

.cpp.obj:
  $(cc) -c {$< }

.asm.obj:
  $(tasm) -mx $*.asm,$*.obj

.rc.res:
  $(rc) -r $*.rc

#=============================================================
#		list macros
#=============================================================


exe_dependencies =  \
 ..\lib\bcos2\system.lib \
 ..\lib\bcos2\screen.lib \
 ..\lib\bcos2\misc.lib \
 ..\lib\bcos2\dos.lib \
 usernet.obj

#=============================================================
#		explicit rules
#=============================================================
bcos2\usernet2.exe: usernet2.cfg $(exe_dependencies)
  $(tlink) /b:0x10000 /x /toe /ap /l$(libpath) @&&|
$(bcroot)\lib\c02.obj+
bcos2\usernet.obj
bcos2\usernet2
		# no map file
..\lib\bcos2\system.lib+
..\lib\bcos2\screen.lib+
..\lib\bcos2\misc.lib+
..\lib\bcos2\dos.lib+
$(bcroot)\lib\c2.lib+
$(bcroot)\lib\os2.lib

|


#=============================================================
#		individual file dependencies
#=============================================================
usernet.obj: usernet2.cfg usernet.c 

#=============================================================
#		compiler configuration file
#=============================================================
usernet2.cfg: usernet2.mak
  copy &&|
-rt-
-xd-
-x-
-r
-oz
-ob
-oe
-oc
-l$(libpath)
-i$(includepath)
-nbcos2
-p
-vi
-d
-k-
-o
-v
-w
-c
-k
-a
| usernet2.cfg


