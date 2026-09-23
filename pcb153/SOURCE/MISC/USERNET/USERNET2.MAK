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
#	USERNET2.MAK - Makefile for project $(ROOT)\PROJ\USERNET\usernet2.prj
#		Created on 08/11/96 at 23:23
#
#=============================================================

.AUTODEPEND

.PATH.obj = BCOS2

#=============================================================
#		Translator Definitions
#=============================================================
!ifndef ROOT
ROOT     = \OUT
!endif
!ifndef BRANCH
BRANCH   = PWA153
!endif
!ifndef CVER
CVER     = $(BCCOMPILER)
!endif
!ifndef SRC
SRC      = \PCB153
!endif
!ifndef TKIT
TKIT     = \TOOLKIT\PWA153
!endif
!ifndef LIBSDIR
LIBSDIR  = \PCBCBASE
!endif
OUTDIR   = $(ROOT)\$(BRANCH)
SDK      = $(OUTDIR)\SDK\$(CVER)
SDKLIB   = $(SDK)\LIB
SDKOBJ   = $(SDK)\OBJ
!ifndef BCROOT
BCROOT   = \BCOS2
!endif
CC = bcc +USERNET2.CFG
TASM = tasm.exe
TLIB = tlib.exe
TLINK = tlink
RC = brcc.exe
RB = rc.exe
LIBPATH = $(BCROOT)\LIB
INCLUDEPATH = $(BCROOT)\INCLUDE;$(TKIT)\H


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
 $(OUTDIR)\SDK\BCOS2\LIB\system.lib \
 $(OUTDIR)\SDK\BCOS2\LIB\screen.lib \
 $(OUTDIR)\SDK\BCOS2\LIB\misc.lib \
 $(OUTDIR)\SDK\BCOS2\LIB\dos.lib \
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
$(OUTDIR)\SDK\BCOS2\LIB\system.lib+
$(OUTDIR)\SDK\BCOS2\LIB\screen.lib+
$(OUTDIR)\SDK\BCOS2\LIB\misc.lib+
$(OUTDIR)\SDK\BCOS2\LIB\dos.lib+
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


#=============================================================
#       CLEAN - remove everything this makefile produces
#=============================================================

CLEAN:
  -if exist BCOS2\*.obj del BCOS2\*.obj
  -if exist BCOS2\*.map del BCOS2\*.map
  -if exist BCOS2\*.res del BCOS2\*.res
  -if exist BCOS2\USERNET2.EXE del BCOS2\USERNET2.EXE
