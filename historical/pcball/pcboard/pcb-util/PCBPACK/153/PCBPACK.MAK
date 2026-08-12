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
#       PCBPACK.MAK - Makefile for PCBPack-for-DOS
#
#=============================================================

.SILENT
.AUTODEPEND

VERSION = 153
ROOT    = \PROJ
PROJ    = $(ROOT)\PCBPACK
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\BCDOS\$(BCCOMPILER)
SOURCE  = SOURCE
OBJ     = $(BCCOMPILER)

CFG     = $(VERSION)\PCBPACK.CFG
MAK     = $(VERSION)\PCBPACK.MAK

INCLUDEPATH = $(INCLUDE);$(SOURCE);$(LIBROOT)\H

#   BC50 runs the _TEXT segment past 64K if compact model is used, therefore
#   we muse use large model with BC50.
!if $d(BC50)
MDL = l
!else
MDL = c
!endif

CODEOPT=
LINKOPT=-x

!if $(TD)
CODEOPT=$(CODEOPT) -v
LINKOPT=$(LINKOPT) -v
!endif

!if $(DEBUG)
CODEOPT=$(CODEOPT) -DDEBUG
!endif

COPT = -c

!if $d(BC50)
#leave out -Oe due to a bug in Borland C 4.0 thru 5.0
COPT = $(COPT) -Obglmptv
!elif $d(TC30)
#leave out all of the extra -Oxxx switches for TC 3.0 because they aren't valid
!elif $d(BC31)
COPT = $(COPT) -Oebglmptv
!endif

ASMOPT =  /mx /t /D__$(MDL)__                   # Assembler options

.PATH.obj = $(OBJ)
.PATH.asm = $(SOURCE)
.PATH.c   = $(SOURCE)

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) { $(.PATH.c)\$. }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&;

$(OBJ)\pcbpack.exe:  $(CFG) \
  addext.obj                  \
  crcblk.obj                  \
  help.obj                    \
  index.obj                   \
  logs.obj                    \
  packconf.obj                \
  pcbpack.obj                 \
  repair.obj                  \
  report.obj                  \
  showerr.obj                 \
  timer.obj                   \
  $(LIBDIR)\pcb_$(MDL).lib    \
  $(LIBDIR)\dos_$(MDL).lib    \
  $(LIBDIR)\misc_$(MDL).lib   \
  $(LIBDIR)\screen_$(MDL).lib \
  $(LIBDIR)\scrnio_$(MDL).lib \
  $(LIBDIR)\system_$(MDL).lib \
  $(LIBDIR)\country$(MDL).lib

  $(LINKER) $(LINKOPT) $(BCROOT)\lib\c0$(MDL)+@&&|
$(OBJ)\addext.obj+
$(OBJ)\crcblk.obj+
$(OBJ)\help.obj+
$(OBJ)\index.obj+
$(OBJ)\logs.obj+
$(OBJ)\packconf.obj+
$(OBJ)\pcbpack.obj+
$(OBJ)\repair.obj+
$(OBJ)\report.obj+
$(OBJ)\showerr.obj+
$(OBJ)\timer.obj
$(OBJ)\pcbpack.exe
nul
$(LIBDIR)\pcb_$(MDL).lib +
$(LIBDIR)\dos_$(MDL).lib +
$(LIBDIR)\misc_$(MDL).lib +
$(LIBDIR)\screen_$(MDL).lib +
$(LIBDIR)\scrnio_$(MDL).lib +
$(LIBDIR)\system_$(MDL).lib +
$(LIBDIR)\country$(MDL).lib +
$(BCROOT)\LIB\c$(MDL).lib
|


#============================================================
#              Compiler Configuration File
#============================================================
$(CFG): $(MAK)
  copy &&|
-a-
-d
-f-
-r
-v-
-y-
-w-eas
-wbbf
-wbig
-wdpu
-wdup
-wext
-wpin
-wret
-wstu
-wsus
-wvoi
-wzdi
-wamb
-wamp
-wasm
-waus
-wccc
-wdef
-weff
-wias
-will
-wnod
-wpar
-wpia
-wpro
-wrch
-wrvl
-wstv
-wuse
-wcln
-wcpt
-wrng
-wrpt
-wsig
-wucp
-wbei
-wdsz
-whid
-wibc
-winl
-wlin
-wlvc
-wmpc
-wmpd
-wncf
-wnci
-wnst
-wnvf
-wobi
-wofp
-wovl
-wpre
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
-UMEMCHECK
-DPCBSETUP
-DFIDO
-C
-G
-I$(INCLUDEPATH)
-K
-O
-P
-Z
-m$(MDL)
-n$(OBJ)
| $(CFG)
