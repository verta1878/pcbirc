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
#       MKPCBTXT.MAK - Makefile for MKPCBTXT Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = MKPCBTXT

ROOT     = \PCBSRC
SOURCE   = .
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);$(ROOT)\MAIN\SOURCE\H;$(ROOT)\MAIN\SOURCE\UTIL\PCBSM\SOURCE

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
  $(OBJDIR)\MKPCBTXT.OBJ      \
  $(OBJDIR)\STRS15.OBJ        \
  $(OBJDIR)\INPUTNUM.OBJ      \
  $(OBJDIR)\SCRNINPT.OBJ      \
  $(LIBLIB)\dos_$(MDL).386    \
  $(LIBLIB)\misc_$(MDL).386   \
  $(LIBLIB)\screen_$(MDL).386 \
  $(LIBLIB)\scrnio_$(MDL).386 \
  $(LIBLIB)\system_$(MDL).386 \
  $(LIBLIB)\country$(MDL).386

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH);\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31 @&&|
c0$(MDL).obj+
$(OBJDIR)\MKPCBTXT.OBJ+
$(OBJDIR)\STRS15.OBJ+
$(OBJDIR)\INPUTNUM.OBJ+
$(OBJDIR)\SCRNINPT.OBJ+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\box.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\delete.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\dosfread.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\getmode.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\insert.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\kbdstat.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\readscrn.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\scrollup.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\showerr2.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\stripb.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\timechng.obj+
\PCBSRC\MAIN\SOURCE\UTIL\PCBSM\BC31\wherex.obj
$(OBJDIR)\$(PROGNAME)
                # no map file
$(LIBLIB)\dos_$(MDL).386+
$(LIBLIB)\misc_$(MDL).386+
$(LIBLIB)\screen_$(MDL).386+
$(LIBLIB)\scrnio_$(MDL).386+
$(LIBLIB)\system_$(MDL).386+
$(LIBLIB)\country$(MDL).386+
cl.lib
|

#=============================================================

# rules for individual files where necessary

$(OBJDIR)\INPUTNUM.OBJ: $(ROOT)\lib\source\scrnio\inputnum.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\lib\source\scrnio\inputnum.c

$(OBJDIR)\SCRNINPT.OBJ: $(ROOT)\lib\source\scrnio\scrninpt.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\lib\source\scrnio\scrninpt.c

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
| $(CFG)

#=============================================================
