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
#       pcbmoni2.mak - makefile for project d:\proj\pcbmoni\pcbmoni2.prj
#               created on 08/11/96 at 23:20
#
#=============================================================

.autodepend

!ifndef root
root     = \out
!endif
!ifndef bcroot
bcroot   = \bcos2
!endif

.path.obj = $(root)\pcbmoni\os2

#=============================================================
#               translator definitions
#=============================================================
cc = bcc +pcbmoni2.cfg
tasm = tasm.exe
tlib = tlib.exe
tlink = tlink
rc = brcc.exe
rb = rc.exe
libpath = $(bcroot)\lib
includepath = $(bcroot)\include;$(root)\lib\h


#=============================================================
#               implicit rules
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
#               list macros
#=============================================================


exe_dependencies =  \
 ..\lib\bcos2\country.lib \
 ..\lib\bcos2\misc.lib \
 ..\lib\bcos2\scrnio.lib \
 ..\lib\bcos2\screen.lib \
 ..\lib\bcos2\dos.lib \
 ..\lib\bcos2\system.lib \
 scrninpt.obj \
 pcbmoni.obj

#=============================================================
#               explicit rules
#=============================================================
$(root)\pcbmoni\os2\pcbmoni2.exe: pcbmoni2.cfg $(exe_dependencies)
  $(tlink) /b:0x10000 /toe /ap /l$(libpath) @&&|
$(bcroot)\lib\c02.obj+
$(root)\pcbmoni\os2\scrninpt.obj+
$(root)\pcbmoni\os2\pcbmoni.obj
$(root)\pcbmoni\os2\pcbmoni2,$(root)\pcbmoni\os2\pcbmoni2
..\lib\bcos2\country.lib+
..\lib\bcos2\misc.lib+
..\lib\bcos2\scrnio.lib+
..\lib\bcos2\screen.lib+
..\lib\bcos2\dos.lib+
..\lib\bcos2\system.lib+
$(bcroot)\lib\c2mt.lib+
$(bcroot)\lib\os2.lib

|


#=============================================================
#               individual file dependencies
#=============================================================
scrninpt.obj: pcbmoni2.cfg ..\lib\source\scrnio\scrninpt.c
        $(cc) -c ..\lib\source\scrnio\scrninpt.c

pcbmoni.obj: pcbmoni2.cfg pcbmoni.c

#=============================================================
#               compiler configuration file
#=============================================================
pcbmoni2.cfg: pcbmoni2.mak
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
-n\proj\pcbmoni\os2
-p
-vi
-sm
-d
-k-
-o
-ot
-v
-w
-c
-k
-a
-d_fardata_
| pcbmoni2.cfg


