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
#       FIDOUTIL.MAK - Makefile for FIDOUTIL Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = FIDOUTIL

# ROOT names this branch. Was \PROJ until 2026-09-17: neither
# \PROJ (Clark, one product) nor \PCBSRC named a branch, and this
# tree is one of four. Guarded so a build can still override it:
#     MAKE -f<this>.MAK -DROOT=\DELTA154
# Borland MAKE lets a makefile definition beat -D, so the !ifndef is
# required for the override to take effect.
!ifndef ROOT
ROOT     = \PWA153
!endif
SOURCE   = SOURCE
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
# Library path: our layout is <branch>\LIB\<compiler>\lib, not Clark's
# LIB\BCDOS\<compiler>. Repointed 2026-09-17 to match the repo
# (toolkit/<branch>/<compiler>/lib) rather than bridge it with copies.
LIBLIB   = $(LIBROOT)\$(BCCOMPILER)\lib

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);SOURCE;$(ROOT)\SOURCE\H;\LIBS\VMDATA

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
  analize.obj   \
  ci_build.obj  \
  convert.obj   \
  data.obj      \
  fidonet.obj   \
  maint.obj     \
  passthru.obj  \
  pcbfu.obj     \
  ptsetup.obj   \
  report.obj    \
  $(LIBLIB)\dos\large\showerr2.obj \
  $(LIBLIB)\dos_$(MDL).lib          \
  $(LIBLIB)\doscls_$(MDL).lib       \
  $(LIBLIB)\pcb_$(MDL).lib          \
  $(LIBLIB)\misc_$(MDL).lib         \
  $(LIBLIB)\screen_$(MDL).lib       \
  $(LIBLIB)\scrnio_$(MDL).lib       \
  $(LIBLIB)\system_$(MDL).lib       \
  $(LIBLIB)\country$(MDL).lib       \
  \LIBS\VMDATA\BC31_DOS\VMDATA.LIB

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\analize.obj+
$(OBJDIR)\ci_build.obj+
$(OBJDIR)\convert.obj+
$(OBJDIR)\data.obj+
$(OBJDIR)\fidonet.obj+
$(OBJDIR)\maint.obj+
$(OBJDIR)\passthru.obj+
$(OBJDIR)\pcbfu.obj+
$(OBJDIR)\ptsetup.obj+
$(OBJDIR)\report.obj+
$(LIBLIB)\dos\large\showerr2.obj
$(OBJDIR)\$(PROGNAME)
                # no map file
$(LIBLIB)\dos_$(MDL).lib+
$(LIBLIB)\doscls_$(MDL).lib+
$(LIBLIB)\pcb_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\scrnio_$(MDL).lib+
$(LIBLIB)\system_$(MDL).lib+
$(LIBLIB)\country$(MDL).lib+
\LIBS\VMDATA\BC31_DOS\VMDATA.LIB+
math$(MDL).lib+
emu.lib+
c$(MDL).lib+
|

#=============================================================

# rules for individual files where necessary

ci_build.obj: $(ROOT)\source\util\pcbsetup\source\ci_build.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\source\util\pcbsetup\source\ci_build.c

data.obj: $(ROOT)\source\fido\data.cpp
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\source\fido\data.cpp

# packfido.obj DROPPED from both lists above, 2026-09-17.
#
# packfido.c is missing -- Clark kept it at $(ROOT)\packfido\, a
# top-level directory beside $(ROOT)\md5\, outside the source tree, and
# it was never archived. See PACKFIDO-MISSING.md in this directory.
#
# It is NOT needed. packfido.obj supplies exactly one symbol, do_pack(),
# and FIDOUTIL never calls it: SOURCE\CONVERT.CPP line 53 declares
# "void do_pack(void);" and line 124 is "//do_pack();" -- commented out.
# Those two lines are the only occurrences of do_pack anywhere in the
# archive. So the object contributed nothing to the linked image and
# FIDOUTIL.EXE builds without it.
#
# Rule kept, commented, so a recovered packfido.c can be restored by
# uncommenting these three places:
#
# packfido.obj: $(ROOT)\source\packfido.c
#   $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\source\packfido.c

passthru.obj: $(ROOT)\source\fido\passthru.cpp
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\source\fido\passthru.cpp

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
-DPCBSETUP;QUIET_BUILD;FIDO;PCBFU;
-D_FARDATA_=far
-DFIDOUTIL
| $(CFG)

#=============================================================
