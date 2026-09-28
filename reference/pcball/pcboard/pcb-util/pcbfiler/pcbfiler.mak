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
#       PCBFILER.MAK - Makefile for PCBFILER Program
#
#=============================================================

.NOSILENT
.AUTODEPEND

PROGNAME = PCBFILER

ROOT     = \PROJ
SOURCE   = SOURCE
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);SOURCE;\LIBS\VMDATA

#=============================================================

!if $(DEBUG)
CODEOPT=-DDEBUG
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
  $(LIBLIB)\misc\large\swap.obj \
  $(OBJDIR)\action.obj        \
  $(OBJDIR)\automove.obj      \
  $(OBJDIR)\batch.obj         \
  $(OBJDIR)\confmenu.obj      \
  $(OBJDIR)\ctod2.obj         \
  $(OBJDIR)\defaults.obj      \
  $(OBJDIR)\dircolor.obj      \
  $(OBJDIR)\dirfiles.obj      \
  $(OBJDIR)\dupes.obj         \
  $(OBJDIR)\edit.obj          \
  $(OBJDIR)\editdirs.obj      \
  $(OBJDIR)\editor.obj        \
  $(OBJDIR)\editmove.obj      \
  $(OBJDIR)\editrule.obj      \
  $(OBJDIR)\findpath.obj      \
  $(OBJDIR)\findtext.obj      \
  $(OBJDIR)\getdiz.obj        \
  $(OBJDIR)\idx.obj           \
  $(OBJDIR)\idxname.obj       \
  $(OBJDIR)\init.obj          \
  $(OBJDIR)\pcbfiler.obj      \
  $(OBJDIR)\preedit.obj       \
  $(OBJDIR)\process.obj       \
  $(OBJDIR)\readdir.obj       \
  $(OBJDIR)\savedir.obj       \
  $(OBJDIR)\scandupe.obj      \
  $(OBJDIR)\shell.obj         \
  $(OBJDIR)\showfree.obj      \
  $(OBJDIR)\sort.obj          \
  $(OBJDIR)\unique.obj        \
  $(OBJDIR)\verify.obj        \
  $(OBJDIR)\zipv.obj          \
  $(OBJDIR)\arcv.obj          \
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
  $(LINKER) /x/c/L$(LIBPATH) @&&|
/o- c0$(MDL).obj+
/o- $(LIBLIB)\misc\large\swap.obj+
/o+ $(OBJDIR)\action.obj+
/o- $(OBJDIR)\automove.obj+
/o+ $(OBJDIR)\batch.obj+
/o+ $(OBJDIR)\confmenu.obj+
/o- $(OBJDIR)\ctod2.obj+
/o+ $(OBJDIR)\defaults.obj+
/o+ $(OBJDIR)\dircolor.obj+
/o+ $(OBJDIR)\dirfiles.obj+
/o+ $(OBJDIR)\dupes.obj+
/o- $(OBJDIR)\edit.obj+
/o+ $(OBJDIR)\editdirs.obj+
/o+ $(OBJDIR)\editor.obj+
/o+ $(OBJDIR)\editmove.obj+
/o+ $(OBJDIR)\editrule.obj+
/o+ $(OBJDIR)\findpath.obj+
/o+ $(OBJDIR)\findtext.obj+
/o- $(OBJDIR)\getdiz.obj+
/o+ $(OBJDIR)\idx.obj+
/o- $(OBJDIR)\idxname.obj+
/o+ $(OBJDIR)\init.obj+
/o+ $(OBJDIR)\pcbfiler.obj+
/o+ $(OBJDIR)\preedit.obj+
/o+ $(OBJDIR)\process.obj+
/o+ $(OBJDIR)\readdir.obj+
/o+ $(OBJDIR)\savedir.obj+
/o+ $(OBJDIR)\scandupe.obj+
/o+ $(OBJDIR)\shell.obj+
/o+ $(OBJDIR)\showfree.obj+
/o+ $(OBJDIR)\sort.obj+
/o- $(OBJDIR)\unique.obj+
/o+ $(OBJDIR)\verify.obj+
/o+ $(OBJDIR)\zipv.obj+
/o+ $(OBJDIR)\arcv.obj
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
-DVMDATA
-D_FARDATA_=far
| $(CFG)

#=============================================================
