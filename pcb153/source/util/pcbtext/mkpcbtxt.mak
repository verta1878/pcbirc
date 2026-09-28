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
!ifndef LIBEXT
LIBEXT   = LIB
!endif
OUTDIR   = $(ROOT)\$(BRANCH)
SDK      = $(OUTDIR)\SDK\$(CVER)
SDKLIB   = $(SDK)\LIB
SDKOBJ   = $(SDK)\OBJ
SOURCE   = .
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(TKIT)
LIBH     = $(LIBROOT)\H
LIBLIB   = $(SDKLIB)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);$(SRC)\SOURCE\H;$(SRC)\SOURCE\UTIL\PCBSM\SOURCE

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
  $(LIBLIB)\dos_$(MDL).$(LIBEXT)    \
  $(LIBLIB)\misc_$(MDL).$(LIBEXT)   \
  $(LIBLIB)\screen_$(MDL).$(LIBEXT) \
  $(LIBLIB)\scrnio_$(MDL).$(LIBEXT) \
  $(LIBLIB)\system_$(MDL).$(LIBEXT) \
  $(LIBLIB)\country$(MDL).$(LIBEXT)

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH);$(SRC)\SOURCE\UTIL\PCBSM\$(CVER) @&&|
c0$(MDL).obj+
$(OBJDIR)\MKPCBTXT.OBJ+
$(OBJDIR)\STRS15.OBJ+
$(OBJDIR)\INPUTNUM.OBJ+
$(OBJDIR)\SCRNINPT.OBJ+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\box.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\delete.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\dosfread.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\getmode.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\insert.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\kbdstat.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\readscrn.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\scrollup.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\showerr2.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\stripb.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\timechng.obj+
$(SRC)\SOURCE\UTIL\PCBSM\$(CVER)\wherex.obj
$(OBJDIR)\$(PROGNAME)
                # no map file
$(LIBLIB)\dos_$(MDL).$(LIBEXT)+
$(LIBLIB)\misc_$(MDL).$(LIBEXT)+
$(LIBLIB)\screen_$(MDL).$(LIBEXT)+
$(LIBLIB)\scrnio_$(MDL).$(LIBEXT)+
$(LIBLIB)\system_$(MDL).$(LIBEXT)+
$(LIBLIB)\country$(MDL).$(LIBEXT)+
cl.lib
|

#=============================================================

# rules for individual files where necessary

$(OBJDIR)\INPUTNUM.OBJ: $(TKIT)\SOURCE\SCRNIO\inputnum.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(TKIT)\SOURCE\SCRNIO\inputnum.c

$(OBJDIR)\SCRNINPT.OBJ: $(TKIT)\SOURCE\SCRNIO\scrninpt.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(TKIT)\SOURCE\SCRNIO\scrninpt.c

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


#=============================================================
#       CLEAN - remove everything this makefile produces
#=============================================================

CLEAN:
  -if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
  -if exist $(OBJDIR)\*.map del $(OBJDIR)\*.map
  -if exist $(OBJDIR)\*.res del $(OBJDIR)\*.res
  -if exist $(OBJDIR)\$(PROGNAME).EXE del $(OBJDIR)\$(PROGNAME).EXE
  -if exist $(CFG) del $(CFG)
