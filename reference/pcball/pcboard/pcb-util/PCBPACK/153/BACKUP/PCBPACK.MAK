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
#
#  NOTE:  To use this makefile, the following environment variables must
#         be defined.  The actual values can be different, but the variable
#         names themselves are used by the makefile.
#
#         set BCROOT=E:\TC
#         set COMPILER=%BCROOT%\BCC.EXE
#         set LINKER=%BCROOT%\TLINK.EXE
#         set INCLUDE=%BCROOT%\H
#
#=============================================================

.SILENT
.AUTODEPEND

VERSION = 1522
ROOT    = \PROJ
PROJ    = $(ROOT)\PCBPACK
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\BCDOS
SOURCE  = SOURCE
OBJ     = OBJ\BCDOS

CFG     = $(VERSION)\PCBPACK.CFG
MAK     = $(VERSION)\PCBPACK.MAK

INCLUDEPATH = $(INCLUDE);$(SOURCE);$(LIBROOT)\H

MDL = c

CODEOPT=
LINKOPT=-x

!if $(TD)
CODEOPT=$(CODEOPT) -v
LINKOPT=$(LINKOPT) -v
!endif

!if $(DEBUG)
CODEOPT=$(CODEOPT) -DDEBUG
!endif

!if $(DEMO)
CODEOPT=$(CODEOPT) -DDEMO
!endif

COPT = -c

ASMOPT =  /mx /t /D__$(MDL)__                   # Assembler options

.PATH.obj = $(OBJ)
.PATH.asm = $(SOURCE)
.PATH.c   = $(SOURCE)

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) { $(.PATH.c)\$. }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&;

$(OBJ)\pcbpack.exe:  $(CFG) \
  addext.obj     \
  crcblk.obj     \
  help.obj       \
  index.obj      \
  logs.obj       \
  packconf.obj   \
  pcbpack.obj    \
  repair.obj     \
  report.obj     \
  showerr.obj    \
  timer.obj      \

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
$(LIBDIR)\pcb_c.lib +
$(LIBDIR)\dos_c.lib +
$(LIBDIR)\misc_c.lib +
$(LIBDIR)\screen_c.lib +
$(LIBDIR)\scrnio_c.lib +
$(LIBDIR)\countryc.lib +
$(LIBDIR)\system_c.lib +
$(ROOT)\mc\mctmem.lib +
$(BCROOT)\LIB\cc.lib
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
-Z
-Ob
-Oe
-Og
-Ol
-Om
-Op
-Ot
-Ov
-P
-m$(MDL)
-n$(OBJ)
| $(CFG)
