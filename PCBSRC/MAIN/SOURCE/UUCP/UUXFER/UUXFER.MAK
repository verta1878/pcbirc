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
#       UUXFER.MAK - Makefile for UUXFER Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = UUXFER

ROOT     = \PROJ
SOURCE   = .
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);.;..\COMMON;..\UUIN;$(ROOT)\PCB\SOURCE\H

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
.PATH.cpp = $(SOURCE)

#=============================================================

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

.cpp.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

{..\common\}.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

{..\common\}.cpp.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&

#=============================================================

EXE_DEPENDENCIES = \
  $(OBJDIR)\uuxfer.obj        \
  $(OBJDIR)\uucp.obj          \
  $(OBJDIR)\gprot.obj         \
  $(OBJDIR)\commbuf.obj       \
  $(OBJDIR)\uushwerr.obj      \
  $(OBJDIR)\dresword.obj      \
  $(LIBLIB)\toolkit\large\fossil.obj   \
  $(LIBLIB)\toolkit\large\nolog.obj    \
  $(LIBLIB)\toolkit\large\nostatus.obj \
  $(LIBLIB)\toolkit\large\nodisp.obj   \
  $(LIBLIB)\toolkit\large\noansi.obj   \
  $(LIBLIB)\toolkit\large\notxt.obj    \
  $(LIBLIB)\toolkit\large\nochat.obj   \
  $(LIBLIB)\toolkit\large\nolang.obj   \
  $(LIBLIB)\toolkit\large\nopcbsys.obj \
  $(LIBLIB)\toolkit\large\noscreen.obj \
  $(LIBLIB)\toolkit\large\noshell.obj  \
  $(LIBLIB)\toolkit\large\nosys.obj    \
  $(LIBLIB)\pcbkit_$(MDL).lib \
  $(LIBLIB)\dos_$(MDL).lib    \
  $(LIBLIB)\doscls_$(MDL).lib \
  $(LIBLIB)\pcb_$(MDL).lib    \
  $(LIBLIB)\misc_$(MDL).lib   \
  $(LIBLIB)\screen_$(MDL).lib \
  $(LIBLIB)\system_$(MDL).lib \
  $(LIBLIB)\country$(MDL).lib

#=============================================================

!if $d(BC31)
$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\uuxfer.obj+
$(OBJDIR)\uucp.obj+
$(OBJDIR)\gprot.obj+
$(OBJDIR)\commbuf.obj+
$(OBJDIR)\uushwerr.obj+
$(OBJDIR)\dresword.obj+
..\bc31\stbdsgtn.obj+
..\bc31\stbdsptn.obj+
..\bc31\stbsgetn.obj+
..\bc31\stbsputn.obj+
$(LIBLIB)\toolkit\large\fossil.obj+
$(LIBLIB)\toolkit\large\nolog.obj+
$(LIBLIB)\toolkit\large\nostatus.obj+
$(LIBLIB)\toolkit\large\nodisp.obj+
$(LIBLIB)\toolkit\large\noansi.obj+
$(LIBLIB)\toolkit\large\notxt.obj+
$(LIBLIB)\toolkit\large\nochat.obj+
$(LIBLIB)\toolkit\large\nolang.obj+
$(LIBLIB)\toolkit\large\nopcbsys.obj+
$(LIBLIB)\toolkit\large\noscreen.obj+
$(LIBLIB)\toolkit\large\noshell.obj+
$(LIBLIB)\toolkit\large\nosys.obj
$(OBJDIR)\$(PROGNAME)
                # no map file
$(LIBLIB)\pcbkit_$(MDL).lib+
$(LIBLIB)\dos_$(MDL).lib+
$(LIBLIB)\doscls_$(MDL).lib+
$(LIBLIB)\pcb_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\system_$(MDL).lib+
$(LIBLIB)\country$(MDL).lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|
!else
$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\uuxfer.obj+
$(OBJDIR)\uucp.obj+
$(OBJDIR)\gprot.obj+
$(OBJDIR)\commbuf.obj+
$(OBJDIR)\uushwerr.obj+
$(OBJDIR)\dresword.obj+
$(LIBLIB)\toolkit\large\fossil.obj+
$(LIBLIB)\toolkit\large\nolog.obj+
$(LIBLIB)\toolkit\large\nostatus.obj+
$(LIBLIB)\toolkit\large\nodisp.obj+
$(LIBLIB)\toolkit\large\noansi.obj+
$(LIBLIB)\toolkit\large\notxt.obj+
$(LIBLIB)\toolkit\large\nochat.obj+
$(LIBLIB)\toolkit\large\nolang.obj+
$(LIBLIB)\toolkit\large\nopcbsys.obj+
$(LIBLIB)\toolkit\large\noscreen.obj+
$(LIBLIB)\toolkit\large\noshell.obj+
$(LIBLIB)\toolkit\large\nosys.obj
$(OBJDIR)\$(PROGNAME)
                # no map file
$(LIBLIB)\pcbkit_$(MDL).lib+
$(LIBLIB)\dos_$(MDL).lib+
$(LIBLIB)\doscls_$(MDL).lib+
$(LIBLIB)\pcb_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\system_$(MDL).lib+
$(LIBLIB)\country$(MDL).lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|
!endif

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
-Z
-k-
-d
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DLIB
-D___USE_CI_CACHE___
-D___USE_USERS___
-D___USE_CNAMES___
-DBOUNCE
-UDEBUG
-DNOMEMCHECK
-DUNIX
-D___UUIN___
-DEXCLUDE_DEBUG
| $(CFG)

#=============================================================
