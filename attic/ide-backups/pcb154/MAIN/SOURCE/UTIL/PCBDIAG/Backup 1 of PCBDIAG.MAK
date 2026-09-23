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
#       PCBDIAG.MAK - Makefile for PCBDIAG Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = PCBDIAG

ROOT     = \PROJ
SOURCE   = .
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);$(ROOT)\PCB\SOURCE\H;$(ROOT)\PCBSM\SOURCE

#=============================================================

!if $(DEBUG)
CODEOPT=-DDEBUG
!endif

COPT = -c -m$(MDL) -n$(OBJDIR)

!if $d(BC50)
#leave out -Oe due to a bug in Borland C 4.0 thru 5.0
COPT = $(COPT) -Obglmptv
!elif $d(TC30)
#leave out all of the extra -Oxxx switches for TC 3.0 because they aren't valid
!elif $d(BC31)
COPT = $(COPT) -Oebglmptv
!endif

ASMOPT = /m /mx /t /D__$(MDL)__                 # Assembler options

#=============================================================

.PATH.obj = $(OBJDIR)
.PATH.asm = $(SOURCE)
.PATH.c   = $(SOURCE)

#=============================================================

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&

#=============================================================

EXE_DEPENDENCIES = \
  $(OBJDIR)\ANALYZE.OBJ       \
  $(OBJDIR)\INDEX.OBJ         \
  $(OBJDIR)\INIT.OBJ          \
  $(OBJDIR)\PCBDIAG.OBJ       \
  $(OBJDIR)\PRINT.OBJ         \
  $(OBJDIR)\SCANUSER.OBJ      \
  $(OBJDIR)\SHARE.OBJ         \
  $(OBJDIR)\USERINFO.OBJ      \
  $(OBJDIR)\USERS.OBJ         \
  $(LIBLIB)\dos_$(MDL).lib    \
  $(LIBLIB)\pcb_$(MDL).lib    \
  $(LIBLIB)\misc_$(MDL).lib   \
  $(LIBLIB)\screen_$(MDL).lib \
  $(LIBLIB)\scrnio_$(MDL).lib \
  $(LIBLIB)\system_$(MDL).lib \
  $(LIBLIB)\country$(MDL).lib

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\ANALYZE.OBJ  +
$(OBJDIR)\INDEX.OBJ    +
$(OBJDIR)\INIT.OBJ     +
$(OBJDIR)\PCBDIAG.OBJ  +
$(OBJDIR)\PRINT.OBJ    +
$(OBJDIR)\SCANUSER.OBJ +
$(OBJDIR)\SHARE.OBJ    +
$(OBJDIR)\USERINFO.OBJ +
$(OBJDIR)\USERS.OBJ
$(OBJDIR)\$(PROGNAME)
                # no map file
$(LIBLIB)\dos_$(MDL).lib+
$(LIBLIB)\pcb_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\scrnio_$(MDL).lib+
$(LIBLIB)\system_$(MDL).lib+
$(LIBLIB)\country$(MDL).lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|

#=============================================================

# rules for individual files where necessary

$(OBJDIR)\USERS.OBJ: $(ROOT)\PCBSM\SOURCE\USERS.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\PCBSM\SOURCE\USERS.C

$(OBJDIR)\USERINFO.OBJ: $(ROOT)\PCBSM\SOURCE\USERINFO.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\PCBSM\SOURCE\USERINFO.C

#=============================================================

$(CFG): $(MAK)
  copy &&|
-wbbf
-wbig
-wdpu
-wdup
-weas
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
-f-
-ff-
-C
-K
-G
-O
-P
-Z
-k-
-d
-I$(INCLUDEPATH)
-L$(LIBPATH)
-D_FARDATA_=far
-DPCB152
-DPCB153
| $(CFG)

#=============================================================
