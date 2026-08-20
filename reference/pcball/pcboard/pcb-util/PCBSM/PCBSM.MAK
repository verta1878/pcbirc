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
#       PCBSM.MAK - Makefile for PCBSM Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = PCBSM

ROOT     = \PROJ
SOURCE   = SOURCE
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);SOURCE;$(ROOT)\PCB\SOURCE\H;\LIBS\VMDATA

#=============================================================

!if $d(DEBUG)
CODEOPT=-DDEBUG
!endif

!if $d(TD)
CODEOPT = $(CODEOPT) -v
ASMOPT  = $(ASMOPT) /v
LINKOPT = $(LINKOPT) /v
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

ASMOPT = /m /mx /t /D__$(MDL)__                 # Assembler options

LINKOPT = /x /c

!if $d(TD)
COPT    = $(COPT) -v
ASMOPT  = $(ASMOPT) /v
LINKOPT = $(LINKOPT) /v
!endif


#=============================================================

.PATH.obj = $(OBJDIR)
.PATH.asm = $(SOURCE)
.PATH.c   = $(SOURCE)
.PATH.cpp = $(SOURCE)

#=============================================================

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

.cpp.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&

#=============================================================

EXE_DEPENDENCIES = \
  $(OBJDIR)\acct.obj          \
  $(OBJDIR)\adjust.obj        \
  $(OBJDIR)\batch.obj         \
  $(OBJDIR)\colors.obj        \
  $(OBJDIR)\conf.obj          \
  $(OBJDIR)\confmove.obj      \
  $(OBJDIR)\defedit.obj       \
  $(OBJDIR)\demo.obj          \
  $(OBJDIR)\edit.obj          \
  $(OBJDIR)\expdate.obj       \
  $(OBJDIR)\index.obj         \
  $(OBJDIR)\init.obj          \
  $(OBJDIR)\pack.obj          \
  $(OBJDIR)\packinpt.obj      \
  $(OBJDIR)\packon.obj        \
  $(OBJDIR)\pcbsm.obj         \
  $(OBJDIR)\phone.obj         \
  $(OBJDIR)\print.obj         \
  $(OBJDIR)\process.obj       \
  $(OBJDIR)\sort.obj          \
  $(OBJDIR)\undo.obj          \
  $(OBJDIR)\userinfo.obj      \
  $(OBJDIR)\usernet.obj       \
  $(OBJDIR)\users.obj         \
  $(LIBLIB)\dos_$(MDL).lib    \
  $(LIBLIB)\pcb_$(MDL).lib    \
  $(LIBLIB)\misc_$(MDL).lib   \
  $(LIBLIB)\screen_$(MDL).lib \
  $(LIBLIB)\scrnio_$(MDL).lib \
  $(LIBLIB)\system_$(MDL).lib \
  $(LIBLIB)\country$(MDL).lib \
  \LIBS\VMDATA\BC31_DOS\VMDATA.LIB

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) $(LINKOPT) /L$(LIBPATH) @&&|
/o- c0$(MDL).obj+
/o+ $(OBJDIR)\acct.obj+
/o+ $(OBJDIR)\adjust.obj+
/o+ $(OBJDIR)\batch.obj+
/o+ $(OBJDIR)\colors.obj+
/o+ $(OBJDIR)\conf.obj+
/o+ $(OBJDIR)\confmove.obj+
/o+ $(OBJDIR)\defedit.obj+
/o+ $(OBJDIR)\demo.obj+
/o+ $(OBJDIR)\edit.obj+
/o+ $(OBJDIR)\expdate.obj+
/o+ $(OBJDIR)\index.obj+
/o+ $(OBJDIR)\init.obj+
/o+ $(OBJDIR)\pack.obj+
/o+ $(OBJDIR)\packinpt.obj+
/o+ $(OBJDIR)\packon.obj+
/o+ $(OBJDIR)\pcbsm.obj+
/o+ $(OBJDIR)\phone.obj+
/o+ $(OBJDIR)\print.obj+
/o+ $(OBJDIR)\process.obj+
/o+ $(OBJDIR)\sort.obj+
/o+ $(OBJDIR)\undo.obj+
/o+ $(OBJDIR)\userinfo.obj+
/o+ $(OBJDIR)\usernet.obj+
/o+ $(OBJDIR)\users.obj
$(OBJDIR)\$(PROGNAME)
                # no map file
/o- $(LIBLIB)\dos_$(MDL).lib+
/o- $(LIBLIB)\pcb_$(MDL).lib+
/o- $(LIBLIB)\misc_$(MDL).lib+
/o- $(LIBLIB)\screen_$(MDL).lib+
/o- $(LIBLIB)\scrnio_$(MDL).lib+
/o- $(LIBLIB)\system_$(MDL).lib+
/o- $(LIBLIB)\country$(MDL).lib+
/o- \LIBS\VMDATA\BC31_DOS\VMDATA.LIB+
/o- math$(MDL).lib+
/o- emu.lib+
/o- c$(MDL).lib+
/o- overlay.lib
|

#=============================================================

# rules for individual files where necessary

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
-Vmd
-Y
-Z
-k-
-d
-m$(MDL)
-n$(OBJDIR)
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DPCBSM;USEDATE;BIGNDX;USEFLOAT
-D_FARDATA_=far
| $(CFG)

#=============================================================
