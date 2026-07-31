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
#       PCBSETUP.MAK - Makefile for PCBSETUP Program
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = PCBSETUP

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

!if $(DEBUG)
CODEOPT=-DDEBUG
!endif

COPT = -c

!if $d(BC50)
#leave out -Oe due to a bug in Borland C 4.0 thru 5.0
COPT = $(COPT) -Obglmptv -x-
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
  $(OBJDIR)\pcbsetup.obj        \
  $(OBJDIR)\account.obj         \
  $(OBJDIR)\chkfile.obj         \
  $(OBJDIR)\ci_build.obj        \
  $(OBJDIR)\edit.obj            \
  $(OBJDIR)\editblt.obj         \
  $(OBJDIR)\editcmd.obj         \
  $(OBJDIR)\editconf.obj        \
  $(OBJDIR)\editdays.obj        \
  $(OBJDIR)\editdirs.obj        \
  $(OBJDIR)\editdoor.obj        \
  $(OBJDIR)\editevnt.obj        \
  $(OBJDIR)\editfido.obj        \
  $(OBJDIR)\editfsec.obj        \
  $(OBJDIR)\editftcn.obj        \
  $(OBJDIR)\editlang.obj        \
  $(OBJDIR)\editmod.obj         \
  $(OBJDIR)\editpath.obj        \
  $(OBJDIR)\editprot.obj        \
  $(OBJDIR)\editpwrd.obj        \
  $(OBJDIR)\editscpt.obj        \
  $(OBJDIR)\edittcan.obj        \
  $(OBJDIR)\editverb.obj        \
  $(OBJDIR)\event.obj           \
  $(OBJDIR)\fidoaka.obj         \
  $(OBJDIR)\fidoarc.obj         \
  $(OBJDIR)\fidocfg.obj         \
  $(OBJDIR)\fconfig.obj         \
  $(OBJDIR)\fidoemsi.obj        \
  $(OBJDIR)\fidofreq.obj        \
  $(OBJDIR)\fidoinfo.obj        \
  $(OBJDIR)\fidolist.obj        \
  $(OBJDIR)\fidoloc.obj         \
  $(OBJDIR)\fidomgic.obj        \
  $(OBJDIR)\fidonode.obj        \
  $(OBJDIR)\fidonofr.obj        \
  $(OBJDIR)\fidoorg.obj         \
  $(OBJDIR)\fidopath.obj        \
  $(OBJDIR)\fidotoss.obj        \
  $(OBJDIR)\fidothis.obj        \
  $(OBJDIR)\filesys.obj         \
  $(OBJDIR)\fileconf.obj        \
  $(OBJDIR)\filedisp.obj        \
  $(OBJDIR)\fidoph.obj          \
  $(OBJDIR)\fileques.obj        \
  $(OBJDIR)\getcntxt.obj        \
  $(OBJDIR)\init.obj            \
  $(OBJDIR)\levels.obj          \
  $(OBJDIR)\list.obj            \
  $(OBJDIR)\misc.obj            \
  $(OBJDIR)\modem.obj           \
  $(OBJDIR)\modemacc.obj        \
  $(OBJDIR)\modemswi.obj        \
  $(OBJDIR)\node.obj            \
  $(OBJDIR)\optcolor.obj        \
  $(OBJDIR)\optctrl.obj         \
  $(OBJDIR)\optfunc.obj         \
  $(OBJDIR)\optlim.obj          \
  $(OBJDIR)\optlog.obj          \
  $(OBJDIR)\optmsg.obj          \
  $(OBJDIR)\optos2.obj          \
  $(OBJDIR)\optsys.obj          \
  $(OBJDIR)\optxfer.obj         \
  $(OBJDIR)\search.obj          \
  $(OBJDIR)\shell.obj           \
  $(OBJDIR)\subscrip.obj        \
  $(OBJDIR)\sysop.obj           \
  $(OBJDIR)\sysopcom.obj        \
  $(OBJDIR)\sysopfun.obj        \
  $(OBJDIR)\unique.obj          \
  $(OBJDIR)\uucp.obj            \
  $(OBJDIR)\data.obj            \
  $(OBJDIR)\dos_$(MDL).lib      \
  $(OBJDIR)\pcb_$(MDL).lib      \
  $(OBJDIR)\misc_$(MDL).lib     \
  $(OBJDIR)\screen_$(MDL).lib   \
  $(OBJDIR)\scrnio_$(MDL).lib   \
  $(LIBLIB)\system_$(MDL).lib   \
  $(LIBLIB)\doscls_$(MDL).lib   \
  $(LIBLIB)\country$(MDL).lib   \

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/P/L$(LIBPATH) @&&|
/o- c0$(MDL).obj+
/o- $(LIBLIB)\misc\large\swap.obj+
/o+ $(OBJDIR)\pcbsetup.obj+
/o+ $(OBJDIR)\account.obj+
/o+ $(OBJDIR)\chkfile.obj+
/o+ $(OBJDIR)\ci_build.obj+
/o+ $(OBJDIR)\edit.obj+
/o+ $(OBJDIR)\editblt.obj+
/o+ $(OBJDIR)\editcmd.obj+
/o+ $(OBJDIR)\editconf.obj+
/o+ $(OBJDIR)\editdays.obj+
/o+ $(OBJDIR)\editdirs.obj+
/o+ $(OBJDIR)\editdoor.obj+
/o+ $(OBJDIR)\editevnt.obj+
/o+ $(OBJDIR)\editfido.obj+
/o+ $(OBJDIR)\editfsec.obj+
/o+ $(OBJDIR)\editftcn.obj+
/o+ $(OBJDIR)\editlang.obj+
/o+ $(OBJDIR)\editmod.obj+
/o+ $(OBJDIR)\editpath.obj+
/o+ $(OBJDIR)\editprot.obj+
/o+ $(OBJDIR)\editpwrd.obj+
/o+ $(OBJDIR)\editscpt.obj+
/o+ $(OBJDIR)\edittcan.obj+
/o+ $(OBJDIR)\editverb.obj+
/o+ $(OBJDIR)\event.obj+
/o+ $(OBJDIR)\fidoaka.obj+
/o+ $(OBJDIR)\fidoarc.obj+
/o+ $(OBJDIR)\fidocfg.obj+
/o+ $(OBJDIR)\fconfig.obj+
/o+ $(OBJDIR)\fidoemsi.obj+
/o+ $(OBJDIR)\fidofreq.obj+
/o+ $(OBJDIR)\fidoinfo.obj+
/o+ $(OBJDIR)\fidolist.obj+
/o+ $(OBJDIR)\fidoloc.obj+
/o+ $(OBJDIR)\fidomgic.obj+
/o+ $(OBJDIR)\fidonode.obj+
/o+ $(OBJDIR)\fidonofr.obj+
/o+ $(OBJDIR)\fidoorg.obj+
/o+ $(OBJDIR)\fidopath.obj+
/o+ $(OBJDIR)\fidotoss.obj+
/o+ $(OBJDIR)\fidothis.obj+
/o+ $(OBJDIR)\filesys.obj+
/o+ $(OBJDIR)\fileconf.obj+
/o+ $(OBJDIR)\filedisp.obj+
/o+ $(OBJDIR)\fidoph.obj+
/o+ $(OBJDIR)\fileques.obj+
/o+ $(OBJDIR)\getcntxt.obj+
/o+ $(OBJDIR)\init.obj+
/o+ $(OBJDIR)\levels.obj+
/o+ $(OBJDIR)\list.obj+
/o+ $(OBJDIR)\misc.obj+
/o+ $(OBJDIR)\modem.obj+
/o+ $(OBJDIR)\modemacc.obj+
/o+ $(OBJDIR)\modemswi.obj+
/o+ $(OBJDIR)\node.obj+
/o+ $(OBJDIR)\optcolor.obj+
/o+ $(OBJDIR)\optctrl.obj+
/o+ $(OBJDIR)\optfunc.obj+
/o+ $(OBJDIR)\optlim.obj+
/o+ $(OBJDIR)\optlog.obj+
/o+ $(OBJDIR)\optmsg.obj+
/o+ $(OBJDIR)\optos2.obj+
/o+ $(OBJDIR)\optsys.obj+
/o+ $(OBJDIR)\optxfer.obj+
/o+ $(OBJDIR)\search.obj+
/o+ $(OBJDIR)\shell.obj+
/o+ $(OBJDIR)\subscrip.obj+
/o+ $(OBJDIR)\sysop.obj+
/o+ $(OBJDIR)\sysopcom.obj+
/o+ $(OBJDIR)\sysopfun.obj+
/o+ $(OBJDIR)\unique.obj+
/o+ $(OBJDIR)\uucp.obj+
/o+ $(OBJDIR)\data.obj+
$(OBJDIR)\kbdstat.obj+
$(OBJDIR)\vmfuncs.obj+
$(OBJDIR)\dosfugts.obj+
$(OBJDIR)\PCB\DATAFILE.obj+
$(OBJDIR)\PCB\DATADFLT.obj+
$(OBJDIR)\PCB\CHKEXIST.obj+
$(OBJDIR)\PCB\ENDSTR.obj+
$(OBJDIR)\PCB\GETDAYS.obj+
$(OBJDIR)\PCB\CONFIG.obj+
$(OBJDIR)\PCB\DATA120.obj+
$(OBJDIR)\PCB\DATAWRIT.obj+
$(OBJDIR)\DELETE.obj+
$(OBJDIR)\INSERT.obj+
$(OBJDIR)\READSCRN.obj+
$(OBJDIR)\TIMECHNG.obj+
$(OBJDIR)\GETMODE.obj+
$(OBJDIR)\VALIDATE.obj+
$(OBJDIR)\VALIDSEM.obj+
$(OBJDIR)\INT24HND.obj
$(OBJDIR)\$(PROGNAME)
$(OBJDIR)\$(PROGNAME)
/o- $(OBJDIR)\dos_$(MDL).lib+
/o- $(OBJDIR)\pcb_$(MDL).lib+
/o- $(OBJDIR)\misc_$(MDL).lib+
/o- $(OBJDIR)\screen_$(MDL).lib+
/o- $(OBJDIR)\scrnio_$(MDL).lib+
/o- $(LIBLIB)\system_$(MDL).lib+
/o- $(LIBLIB)\doscls_$(MDL).lib+
/o- $(LIBLIB)\country$(MDL).lib+
/o- math$(MDL).lib+
/o- emu.lib+
/o- c$(MDL).lib+
/o- overlay.lib
|

#=============================================================

# rules for individual files where necessary

$(OBJDIR)\fconfig.obj: $(ROOT)\pcb\source\fido\fconfig.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\pcb\source\fido\fconfig.c

$(OBJDIR)\data.obj: $(ROOT)\pcb\source\fido\data.cpp
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\pcb\source\fido\data.cpp

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
-Vmp
-Y
-Z
-k-
-d
-m$(MDL)
-n$(OBJDIR)
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DFIDO;TRAVIS;PCBSETUP;USEFLOAT;USEDATE;VMDATA;NDEBUG;QUIET_BUILD
-D_FARDATA_=far
| $(CFG)

#=============================================================
