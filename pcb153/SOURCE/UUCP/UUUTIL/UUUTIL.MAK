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
#       UUUTIL.MAK - Makefile for UUUTIL Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = UUUTIL

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

INCLUDEPATH = $(INCLUDE);$(LIBH);.;..\COMMON;..\UUIN;$(SRC)\SOURCE\H

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
  $(OBJDIR)\uuutil.obj        \
  $(OBJDIR)\fidocfg.obj       \
  $(OBJDIR)\ci_other.obj      \
  $(OBJDIR)\ci_build.obj      \
  $(OBJDIR)\uucp.obj          \
  $(SDKOBJ)\dos\large\showerr2.obj     \
  $(SDKOBJ)\toolkit\large\nolog.obj    \
  $(SDKOBJ)\toolkit\large\smalldly.obj \
  $(SDKOBJ)\toolkit\large\nostatus.obj \
  $(SDKOBJ)\toolkit\large\nodisp.obj   \
  $(SDKOBJ)\toolkit\large\noansi.obj   \
  $(SDKOBJ)\toolkit\large\notxt.obj    \
  $(SDKOBJ)\toolkit\large\nochat.obj   \
  $(SDKOBJ)\toolkit\large\nolang.obj    \
  $(SDKOBJ)\toolkit\large\nopcbsys.obj \
  $(SDKOBJ)\toolkit\large\noscreen.obj \
  $(SDKOBJ)\toolkit\large\noshell.obj  \
  $(SDKOBJ)\toolkit\large\nosys.obj    \
  $(LIBLIB)\pcbkit_$(MDL).lib \
  $(LIBLIB)\dos_$(MDL).lib    \
  $(LIBLIB)\doscls_$(MDL).lib \
  $(LIBLIB)\pcb_$(MDL).lib    \
  $(LIBLIB)\misc_$(MDL).lib   \
  $(LIBLIB)\screen_$(MDL).lib \
  $(LIBLIB)\system_$(MDL).lib \
  $(LIBLIB)\country$(MDL).lib

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\uuutil.obj+
$(OBJDIR)\fidocfg.obj+
$(OBJDIR)\ci_other.obj+
$(OBJDIR)\ci_build.obj+
$(OBJDIR)\uucp.obj+
$(SDKOBJ)\dos\large\showerr2.obj+
$(SDKOBJ)\toolkit\large\nolog.obj+
$(SDKOBJ)\toolkit\large\smalldly.obj+
$(SDKOBJ)\toolkit\large\nostatus.obj+
$(SDKOBJ)\toolkit\large\nodisp.obj+
$(SDKOBJ)\toolkit\large\noansi.obj+
$(SDKOBJ)\toolkit\large\notxt.obj+
$(SDKOBJ)\toolkit\large\nochat.obj+
$(SDKOBJ)\toolkit\large\nolang.obj+
$(SDKOBJ)\toolkit\large\nopcbsys.obj+
$(SDKOBJ)\toolkit\large\noscreen.obj+
$(SDKOBJ)\toolkit\large\noshell.obj+
$(SDKOBJ)\toolkit\large\nosys.obj
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
c$(MDL).lib+
$(SDKLIB)\VMDATA_L.LIB
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
-Z
-k-
-d
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DLIB
-D___USE_VMDATA___
-D___USE_CNAMES___
-D___USE_BOOL___
-U___CI_INIT_VM___
-U___CI_INIT_CN___
-UQUIET_BUILD
-UVM_DEVEL
-DNOMEMCHECK
-DUNIX
| $(CFG)

#=============================================================



#=============================================================
#       CLEAN - remove everything this makefile produces
#=============================================================

CLEAN:
  -if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
  -if exist $(OBJDIR)\*.map del $(OBJDIR)\*.map
  -if exist $(OBJDIR)\*.res del $(OBJDIR)\*.res
  -if exist $(OBJDIR)\$(PROGNAME).EXE del $(OBJDIR)\$(PROGNAME).EXE
  -if exist $(CFG) del $(CFG)
