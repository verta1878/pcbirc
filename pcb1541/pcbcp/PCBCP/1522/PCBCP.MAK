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
#       PCBCP.MAK - Makefile for project D:\bcos2\examples\test\PCBCP.PRJ
#               Created on 05/25/95 at 10:50
#
#=============================================================

.SILENT
.AUTODEPEND

#=============================================================
#               Translator Definitions
#=============================================================

VERSION = 1522
ROOT    = \PROJ
PROJ    = $(ROOT)\PCBCP
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\BCOS2
SOURCE  = SOURCE
HELP    = $(PROJ)\HELP
OBJ     = $(PROJ)\OBJ
CFG     = $(VERSION)\PCBCP.CFG
MAK     = $(VERSION)\PCBCP.MAK

!if $d(DEBUG)
  DEBUG = -v
!else
  DEBUG =
!endif

!if $d(VALIDATOR)
  CC = $(COMPILER) +$(CFG) -DVALIDATOR
!else
  CC = $(COMPILER) +$(CFG)
!endif

INCLUDEPATH = $(INCLUDE);$(SOURCE);$(LIBROOT)\H


.PATH.c   = $(SOURCE)
.PATH.obj = $(OBJ)

#=============================================================
#               Implicit Rules
#=============================================================
.c.obj:
  $(CC) -c {$< }


#=============================================================
#               List Macros
#=============================================================
LINK_EXCLUDE =  \
 $(OBJ)\main.res

LINK_INCLUDE =  \
 $(LIBDIR)\system.lib \
 $(LIBDIR)\misc.lib \
 $(LIBDIR)\screen.lib \
 $(LIBDIR)\dos.lib \
 $(SOURCE)\pcbcp.def \
 $(OBJ)\user.obj \
 $(OBJ)\thrd.obj \
 $(OBJ)\pnt.obj \
 $(OBJ)\main.obj \
 $(OBJ)\help.obj \
 $(OBJ)\init.obj \
 $(OBJ)\file.obj \
 $(OBJ)\dlg.obj

#=============================================================
#               Explicit Rules
#=============================================================

$(OBJ)\done: $(OBJ)\pcbcp.exe $(OBJ)\pcbcp.hlp
      echo all done > $(OBJ)\done

$(OBJ)\pcbcp.exe: $(CFG) $(LINK_INCLUDE) $(LINK_EXCLUDE)
  $(LINKER) $(DEBUG) /B:0x10000 /x /Toe /aa /L$(LIBPATH) @&&|
$(LIBPATH)\C02.OBJ+
$(OBJ)\user.obj+
$(OBJ)\thrd.obj+
$(OBJ)\pnt.obj+
$(OBJ)\main.obj+
$(OBJ)\help.obj+
$(OBJ)\init.obj+
$(OBJ)\file.obj+
$(OBJ)\dlg.obj
$(OBJ)\pcbcp.exe
$(OBJ)\pcbcp.map
$(LIBDIR)\system.lib+
$(LIBDIR)\misc.lib+
$(LIBDIR)\screen.lib+
$(LIBDIR)\dos.lib+
!if $d(VALIDATOR)
  d:\toolkt21\os2lib\validatr.lib+
!endif
$(LIBPATH)\C2MT.LIB+
$(LIBPATH)\OS2.LIB
$(SOURCE)\pcbcp.def
|
  rc.exe $(OBJ)\main.res $(OBJ)\pcbcp.exe

#=============================================================
#               Individual File Dependencies
#=============================================================
$(OBJ)\pcbcp.hlp: $(HELP)\pcbcp.ipf $(HELP)\dlg.ipf $(HELP)\edit.ipf $(HELP)\file.ipf $(HELP)\help.ipf $(HELP)\menu.ipf $(HELP)\action.ipf $(HELP)\option.ipf
         cd help
         $(IPFCOMP) pcbcp.ipf
         cd ..

$(OBJ)\main.res: $(SOURCE)\main.rc $(SOURCE)\help.rc
         $(BRCC) -R -I$(INCLUDEPATH) -FO $(OBJ)\main.res $(SOURCE)\main.rc

USER.obj: USER.C

THRD.obj: THRD.C

PNT.obj:  PNT.C

MAIN.obj: MAIN.C

HELP.obj: HELP.C

INIT.obj: INIT.C

FILE.obj: FILE.C

DLG.obj: DLG.C

#=============================================================
#               Compiler Configuration File
#=============================================================
$(CFG): $(MAK)
  copy &&|
-RT-
-xd-
-x-
-Oz
-Ob
-Oe
-Oc
-DBACKGROUND_THREAD
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
-w
-C
-K
-a4
-5
$(DEBUG)
| $(CFG)


