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
#       PCBNLC.MAK - Makefile for PCBNLC Program
#
#=============================================================

.NOSILENT
.AUTODEPEND

PROGNAME = PCBNLC

ROOT     = \PROJ
SOURCE   = .
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = .;$(INCLUDE);$(LIBH);$(ROOT)\PCB\SOURCE\H;\LIBS\VMDATA;$(LIBSDIR)\CODEBASE\SOURCE

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
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT){$< }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&

#=============================================================

EXE_DEPENDENCIES =  \
  $(OBJDIR)\pcbnlc.obj        \
  $(OBJDIR)\diff.obj          \
  $(OBJDIR)\dbase.obj         \
  $(OBJDIR)\data.obj          \
  $(OBJDIR)\fconfig.obj       \
  $(OBJDIR)\fidomisc.obj      \
  $(OBJDIR)\timer.obj         \
  $(LIBLIB)\dos_$(MDL).lib    \
  $(LIBLIB)\doscls_$(MDL).lib \
  $(LIBLIB)\misc_$(MDL).lib   \
  $(LIBLIB)\pcb_$(MDL).lib    \
  $(LIBLIB)\screen_$(MDL).lib \
  $(LIBLIB)\scrnio_$(MDL).lib \
  $(LIBLIB)\system_$(MDL).lib \
  \LIBS\CODEBASE\BOR31\C4BASE.LIB \
  \LIBS\VMDATA\BC31_DOS\VMDATA.LIB

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\pcbnlc.obj+
$(OBJDIR)\diff.obj+
$(OBJDIR)\dbase.obj+
$(OBJDIR)\data.obj+
$(OBJDIR)\fconfig.obj+
$(OBJDIR)\fidomisc.obj+
$(OBJDIR)\timer.obj
$(OBJDIR)\PCBNLC
                # no map file
$(LIBLIB)\dos_$(MDL).lib+
$(LIBLIB)\doscls_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBLIB)\pcb_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\scrnio_$(MDL).lib+
$(LIBLIB)\system_$(MDL).lib+
$(LIBLIB)\country$(MDL).lib+
\LIBS\CODEBASE\BOR31\C4BASE.LIB+
\LIBS\VMDATA\BC31_DOS\VMDATA.LIB+
emu.lib+
math$(MDL).lib+
c$(MDL).lib
|

#=============================================================

# rules for individual files where necessary

$(OBJDIR)\DBASE.OBJ: $(ROOT)\PCB\SOURCE\PPL\DBASE.CPP
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\PCB\SOURCE\PPL\DBASE.CPP

$(OBJDIR)\DATA.OBJ: $(ROOT)\PCB\SOURCE\FIDO\DATA.CPP
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\PCB\SOURCE\FIDO\DATA.CPP

$(OBJDIR)\FCONFIG.OBJ: $(ROOT)\PCB\SOURCE\FIDO\FCONFIG.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\PCB\SOURCE\FIDO\FCONFIG.C

$(OBJDIR)\FIDOMISC.OBJ: $(ROOT)\PCB\SOURCE\FIDO\FIDOMISC.CPP
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\PCB\SOURCE\FIDO\FIDOMISC.CPP

$(OBJDIR)\timer.obj:    $(ROOT)\PCB\SOURCE\ASM\timer.asm
  $(TASM) $(ASMOPT) $(ROOT)\PCB\SOURCE\ASM\timer.asm, $(OBJDIR)\timer.obj

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
-DFIDO;PCBSETUP;PCBNLC
-D_FARDATA_=far
| $(CFG)

#=============================================================
