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
#	USERNET2.MAK - Makefile for project D:\PROJ\USERNET\usernet2.prj
#		Created on 08/11/96 at 23:23
#
#=============================================================

.AUTODEPEND

# ROOT/BCROOT added 2026-09-17. This makefile declared neither and
# instead carried five hardcoded D:\ paths from the machine it was
# written on -- which is why the v0.3.2 ROOT pass skipped it.
!ifndef ROOT
ROOT    = \PWA153
!endif
!ifndef BCROOT
BCROOT  = \BCOS2
!endif
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\bcos2\lib

.PATH.obj = BCOS2

#=============================================================
#		Translator Definitions
#=============================================================
CC = bcc +USERNET2.CFG
TASM = tasm.exe
TLIB = tlib.exe
TLINK = tlink
RC = brcc.exe
RB = rc.exe
LIBPATH = $(BCROOT)\LIB
INCLUDEPATH = $(BCROOT)\INCLUDE;$(LIBROOT)\H


#=============================================================
#		Implicit Rules
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
#		List Macros
#=============================================================


EXE_DEPENDENCIES =  \
 ..\lib\bcos2\system.lib \
 ..\lib\bcos2\screen.lib \
 ..\lib\bcos2\misc.lib \
 ..\lib\bcos2\dos.lib \
 usernet.obj

#=============================================================
#		Explicit Rules
#=============================================================
bcos2\usernet2.exe: usernet2.cfg $(EXE_DEPENDENCIES)
  $(TLINK) /B:0x10000 /x /Toe /ap /L$(LIBPATH) @&&|
$(BCROOT)\LIB\C02.OBJ+
bcos2\usernet.obj
bcos2\usernet2
		# no map file
$(LIBDIR)\system.lib+
$(LIBDIR)\screen.lib+
$(LIBDIR)\misc.lib+
$(LIBDIR)\dos.lib+
$(BCROOT)\LIB\C2.LIB+
$(BCROOT)\LIB\OS2.LIB

|


#=============================================================
#		Individual File Dependencies
#=============================================================
usernet.obj: usernet2.cfg usernet.c 

#=============================================================
#		Compiler Configuration File
#=============================================================
usernet2.cfg: usernet2.mak
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
-nbcos2
-P
-vi
-d
-k-
-O
-v
-w
-C
-K
-a
| usernet2.cfg


