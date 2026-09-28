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
#       PCBPACK.MAK - Makefile for PCBPack-for-OS/2
#
#=============================================================
#
#  NOTE:  To use this makefile, the following environment variables must
#         be defined.  The actual values can be different, but the variable
#         names themselves are used by the makefile.
#
#         set BCROOT=D:\BCOS2
#         set COMPILER=%BCROOT%\BCC.EXE
#         set LINKER=%BCROOT%\TLINK.EXE
#         set INCLUDE=%BCROOT%\INCLUDE
#
#=============================================================

.SILENT
.AUTODEPEND

VERSION = 153
ROOT    = \PROJ
PROJ    = $(ROOT)\PCBPACK2
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\BCOS2
SOURCE  = SOURCE
OBJ     = BCOS2

CFG     = $(VERSION)\PCBPACK2.CFG
MAK     = $(VERSION)\PCBPACK2.MAK

INCLUDEPATH = $(INCLUDE);$(SOURCE);$(LIBROOT)\H
LIBPATH     = $(BCROOT)\LIB

CC = $(COMPILER) +$(CFG)

#=============================================================

.PATH.obj = $(OBJ)
.PATH.c   = $(SOURCE)

#=============================================================
#               Implicit Rules
#=============================================================

.c.obj:
  $(CC) -c {$< }

#=============================================================
#               List Macros
#=============================================================


EXE_DEPENDENCIES =  \
 $(LIBDIR)\scrnio.lib \
 $(LIBDIR)\pcb.lib \
 $(LIBDIR)\country.lib \
 $(LIBDIR)\misc.lib \
 $(LIBDIR)\screen.lib \
 $(LIBDIR)\dos.lib \
 $(LIBDIR)\system.lib \
 showerr.obj \
 report.obj \
 repair.obj \
 pcbpack.obj \
 packconf.obj \
 logs.obj \
 index.obj \
 help.obj \
 crcblk.obj \
 addext.obj

#=============================================================
#               Explicit Rules
#=============================================================
$(OBJ)\pcbpack2.exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /B:0x10000 /x /Toe /ap /L$(LIBPATH) @&&|
$(BCROOT)\LIB\C02.OBJ+
$(OBJ)\showerr.obj+
$(OBJ)\report.obj+
$(OBJ)\repair.obj+
$(OBJ)\pcbpack.obj+
$(OBJ)\packconf.obj+
$(OBJ)\logs.obj+
$(OBJ)\index.obj+
$(OBJ)\help.obj+
$(OBJ)\crcblk.obj+
$(OBJ)\addext.obj
$(OBJ)\pcbpack2
                # no map file
$(LIBDIR)\scrnio.lib+
$(LIBDIR)\pcb.lib+
$(LIBDIR)\country.lib+
$(LIBDIR)\misc.lib+
$(LIBDIR)\screen.lib+
$(LIBDIR)\dos.lib+
$(LIBDIR)\system.lib+
$(BCROOT)\LIB\C2MT.LIB+
$(BCROOT)\LIB\OS2.LIB

|


#=============================================================
#               Individual File Dependencies
#=============================================================
showerr.obj:  showerr.c

report.obj:   report.c

repair.obj:   repair.c

pcbpack.obj:  pcbpack.c

packconf.obj: packconf.c

logs.obj:     logs.c

index.obj:    index.c

help.obj:     help.c

crcblk.obj:   crcblk.c

addext.obj:   addext.c

#=============================================================
#               Compiler Configuration File
#=============================================================
$(CFG): $(MAK)
  copy &&|
-RT-
-xd-
-x-
-R
-Oz
-Ob
-Oe
-Oc
-DALLOCMODE
-DBLOCKMODE
-DCHECKDUPS
-DPACKATTACH
-DPACKGAPS
-DREFNUM
-DRENUMBER
-DREPORT
-DUSECONFIG
-DWARN
-L$(LIBPATH)
-I$(INCLUDEPATH)
-n$(OBJ)
-P
-vi
-sm
-d
-k-
-O
-Ot
-v
-C
-K
-a
-5
-D_FARDATA_
| $(CFG)
