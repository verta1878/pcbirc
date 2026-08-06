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

ROOT     = \PCBSRC
SOURCE   = SOURCE
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);SOURCE;$(ROOT)\MAIN\SOURCE\H

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
.PATH.c   = $(SOURCE);EXTRAOBJ
.PATH.cpp = $(SOURCE);EXTRAOBJ

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
  $(OBJDIR)\personal.obj      \
  $(OBJDIR)\phone.obj         \
  $(OBJDIR)\print.obj         \
  $(OBJDIR)\process.obj       \
  $(OBJDIR)\sort.obj          \
  $(OBJDIR)\undo.obj          \
  $(OBJDIR)\userinfo.obj      \
  $(OBJDIR)\usernet.obj       \
  $(OBJDIR)\users.obj         \
  $(OBJDIR)\abort.obj      \
  $(OBJDIR)\box.obj      \
  $(OBJDIR)\cnames.obj      \
  $(OBJDIR)\config.obj      \
  $(OBJDIR)\copyfile.obj      \
  $(OBJDIR)\datafil2.obj      \
  $(OBJDIR)\delete.obj      \
  $(OBJDIR)\diskfree.obj      \
  $(OBJDIR)\dmath.obj      \
  $(OBJDIR)\dosfread.obj      \
  $(OBJDIR)\endstr.obj      \
  $(OBJDIR)\findfour.obj      \
  $(OBJDIR)\getmode.obj      \
  $(OBJDIR)\insert.obj      \
  $(OBJDIR)\readscrn.obj      \
  $(OBJDIR)\savetext.obj      \
  $(OBJDIR)\scrollup.obj      \
  $(OBJDIR)\showerr2.obj      \
  $(OBJDIR)\smallsub.obj      \
  $(OBJDIR)\stripb.obj      \
  $(OBJDIR)\timechng.obj      \
  $(OBJDIR)\wherex.obj      \
  $(LIBLIB)\dos_$(MDL).386    \
  $(LIBLIB)\pcb_$(MDL).386    \
  $(LIBLIB)\misc_$(MDL).386   \
  $(LIBLIB)\screen_$(MDL).386 \
  $(LIBLIB)\scrnio_$(MDL).386 \
  $(LIBLIB)\system_$(MDL).386 \
  $(LIBLIB)\country$(MDL).386 \

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) $(LINKOPT) /L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\acct.obj+
$(OBJDIR)\adjust.obj+
$(OBJDIR)\batch.obj+
$(OBJDIR)\colors.obj+
$(OBJDIR)\conf.obj+
$(OBJDIR)\confmove.obj+
$(OBJDIR)\defedit.obj+
$(OBJDIR)\demo.obj+
$(OBJDIR)\edit.obj+
$(OBJDIR)\expdate.obj+
$(OBJDIR)\index.obj+
$(OBJDIR)\init.obj+
$(OBJDIR)\pack.obj+
$(OBJDIR)\packinpt.obj+
$(OBJDIR)\packon.obj+
$(OBJDIR)\pcbsm.obj+
$(OBJDIR)\personal.obj+
$(OBJDIR)\phone.obj+
$(OBJDIR)\print.obj+
$(OBJDIR)\process.obj+
$(OBJDIR)\sort.obj+
$(OBJDIR)\undo.obj+
$(OBJDIR)\userinfo.obj+
$(OBJDIR)\usernet.obj+
$(OBJDIR)\users.obj+
$(OBJDIR)\abort.obj+
$(OBJDIR)\box.obj+
$(OBJDIR)\cnames.obj+
$(OBJDIR)\config.obj+
$(OBJDIR)\copyfile.obj+
$(OBJDIR)\ctod.obj+
$(OBJDIR)\kbdstat.obj+
$(OBJDIR)\datafil2.obj+
$(OBJDIR)\delete.obj+
$(OBJDIR)\diskfree.obj+
$(OBJDIR)\dmath.obj+
$(OBJDIR)\dosfread.obj+
$(OBJDIR)\endstr.obj+
$(OBJDIR)\findfour.obj+
$(OBJDIR)\getmode.obj+
$(OBJDIR)\insert.obj+
$(OBJDIR)\readscrn.obj+
$(OBJDIR)\savetext.obj+
$(OBJDIR)\scrollup.obj+
$(OBJDIR)\showerr2.obj+
$(OBJDIR)\smallsub.obj+
$(OBJDIR)\stripb.obj+
$(OBJDIR)\timechng.obj+
$(OBJDIR)\wherex.obj
$(OBJDIR)\$(PROGNAME)

$(LIBLIB)\dos_$(MDL).386+
$(LIBLIB)\pcb_$(MDL).386+
$(LIBLIB)\misc_$(MDL).386+
$(LIBLIB)\screen_$(MDL).386+
$(LIBLIB)\scrnio_$(MDL).386+
$(LIBLIB)\system_$(MDL).386+
$(LIBLIB)\country$(MDL).386+
mathl.lib+
emu.lib+
cl.lib+
overlay.lib
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
$(OBJDIR)\abort.obj: EXTRAOBJ\ABORT.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\abort.obj -c EXTRAOBJ\ABORT.C

$(OBJDIR)\box.obj: EXTRAOBJ\BOX.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\box.obj -c EXTRAOBJ\BOX.C

$(OBJDIR)\cnames.obj: EXTRAOBJ\CNAMES.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\cnames.obj -c EXTRAOBJ\CNAMES.C

$(OBJDIR)\config.obj: EXTRAOBJ\CONFIG.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\config.obj -c EXTRAOBJ\CONFIG.C

$(OBJDIR)\copyfile.obj: EXTRAOBJ\COPYFILE.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\copyfile.obj -c EXTRAOBJ\COPYFILE.C

$(OBJDIR)\datafil2.obj: EXTRAOBJ\DATAFIL2.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\datafil2.obj -c EXTRAOBJ\DATAFIL2.C


$(OBJDIR)\delete.obj: EXTRAOBJ\DELETE.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\delete.obj -c EXTRAOBJ\DELETE.C

$(OBJDIR)\diskfree.obj: EXTRAOBJ\DISKFREE.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\diskfree.obj -c EXTRAOBJ\DISKFREE.C

$(OBJDIR)\dmath.obj: EXTRAOBJ\DMATH.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\dmath.obj -c EXTRAOBJ\DMATH.C

$(OBJDIR)\dosfread.obj: EXTRAOBJ\DOSFREAD.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\dosfread.obj -c EXTRAOBJ\DOSFREAD.C

$(OBJDIR)\endstr.obj: EXTRAOBJ\ENDSTR.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\endstr.obj -c EXTRAOBJ\ENDSTR.C

$(OBJDIR)\findfour.obj: EXTRAOBJ\FINDFOUR.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\findfour.obj -c EXTRAOBJ\FINDFOUR.C

$(OBJDIR)\getmode.obj: EXTRAOBJ\GETMODE.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\getmode.obj -c EXTRAOBJ\GETMODE.C

$(OBJDIR)\insert.obj: EXTRAOBJ\INSERT.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\insert.obj -c EXTRAOBJ\INSERT.C

$(OBJDIR)\readscrn.obj: EXTRAOBJ\READSCRN.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\readscrn.obj -c EXTRAOBJ\READSCRN.C

$(OBJDIR)\savetext.obj: EXTRAOBJ\SAVETEXT.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\savetext.obj -c EXTRAOBJ\SAVETEXT.C

$(OBJDIR)\scrollup.obj: EXTRAOBJ\SCROLLUP.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\scrollup.obj -c EXTRAOBJ\SCROLLUP.C

$(OBJDIR)\showerr2.obj: EXTRAOBJ\SHOWERR2.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\showerr2.obj -c EXTRAOBJ\SHOWERR2.C

$(OBJDIR)\smallsub.obj: EXTRAOBJ\SMALLSUB.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\smallsub.obj -c EXTRAOBJ\SMALLSUB.C

$(OBJDIR)\stripb.obj: EXTRAOBJ\STRIPB.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\stripb.obj -c EXTRAOBJ\STRIPB.C

$(OBJDIR)\timechng.obj: EXTRAOBJ\TIMECHNG.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\timechng.obj -c EXTRAOBJ\TIMECHNG.C

$(OBJDIR)\wherex.obj: EXTRAOBJ\WHEREX.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\wherex.obj -c EXTRAOBJ\WHEREX.C



$(OBJDIR)\ctod.obj: EXTRAOBJ\CTOD.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\ctod.obj -c EXTRAOBJ\CTOD.C

$(OBJDIR)\kbdstat.obj: EXTRAOBJ\KBDSTAT.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) -o$(OBJDIR)\kbdstat.obj -c EXTRAOBJ\KBDSTAT.C
