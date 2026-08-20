#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* The information in this module is proprietary software belonging to       */
#* Clark Development Company and is part of the PCBoard source code library. */
#* You are granted the right to use this information for the building of any */
#* of the PCBoard products you have licensed.  Any other usage is forbidden  */
#* without prior written consent from Clark Development Company, Inc.        */
#*                                                                           */
#* Be sure to read the source code license agreement before utilizing any    */
#* of the source code found herein.                                          */
#*                                                                           */
#* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#=============================================================
#
#       PCBMONI2.MAK - Makefile for project D:\PROJ\PCBMONI\pcbmoni2.prj
#               Created on 08/11/96 at 23:20
#
#=============================================================

.AUTODEPEND

.PATH.obj = \PROJ\PCBMONI\OS2

#=============================================================
#               Translator Definitions
#=============================================================
CC = bcc +PCBMONI2.CFG
TASM = tasm.exe
TLIB = tlib.exe
TLINK = tlink
RC = brcc.exe
RB = rc.exe
LIBPATH = D:\BCOS2\LIB
INCLUDEPATH = D:\BCOS2\INCLUDE;\proj\lib\h


#=============================================================
#               Implicit Rules
#=============================================================
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

.asm.obj:
  $(TASM) -Mx $*.asm,$*.obj

.rc.res:
  $(RC) -r $*.rc

#=============================================================
#               List Macros
#=============================================================


EXE_DEPENDENCIES =  \
 ..\lib\bcos2\country.lib \
 ..\lib\bcos2\misc.lib \
 ..\lib\bcos2\scrnio.lib \
 ..\lib\bcos2\screen.lib \
 ..\lib\bcos2\dos.lib \
 ..\lib\bcos2\system.lib \
 scrninpt.obj \
 pcbmoni.obj

#=============================================================
#               Explicit Rules
#=============================================================
\proj\pcbmoni\os2\pcbmoni2.exe: pcbmoni2.cfg $(EXE_DEPENDENCIES)
  $(TLINK) /B:0x10000 /Toe /ap /L$(LIBPATH) @&&|
D:\BCOS2\LIB\C02.OBJ+
\proj\pcbmoni\os2\scrninpt.obj+
\proj\pcbmoni\os2\pcbmoni.obj
\proj\pcbmoni\os2\pcbmoni2,\proj\pcbmoni\os2\pcbmoni2
..\lib\bcos2\country.lib+
..\lib\bcos2\misc.lib+
..\lib\bcos2\scrnio.lib+
..\lib\bcos2\screen.lib+
..\lib\bcos2\dos.lib+
..\lib\bcos2\system.lib+
D:\BCOS2\LIB\C2MT.LIB+
D:\BCOS2\LIB\OS2.LIB

|


#=============================================================
#               Individual File Dependencies
#=============================================================
scrninpt.obj: pcbmoni2.cfg ..\lib\source\scrnio\scrninpt.c
        $(CC) -c ..\lib\source\scrnio\scrninpt.c

pcbmoni.obj: pcbmoni2.cfg pcbmoni.c

#=============================================================
#               Compiler Configuration File
#=============================================================
pcbmoni2.cfg: pcbmoni2.mak
  copy &&|
-RT-
-xd-
-x-
-R
-Oz
-Ob
-Oe
-Oc
-L$(LIBPATH)
-I$(INCLUDEPATH)
-n\proj\pcbmoni\os2
-P
-vi
-sm
-d
-k-
-O
-Ot
-v
-w
-C
-K
-a
-D_FARDATA_
| pcbmoni2.cfg


