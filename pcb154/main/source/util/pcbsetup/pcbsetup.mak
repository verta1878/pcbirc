#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* the information in this module is proprietary software belonging to       */
#* clark development company and is part of the pcboard source code library. */
#* you are granted the right to use this information for the building of any */
#* of the pcboard products you have licensed.  any other usage is forbidden  */
#* without prior written consent from clark development company, inc.        */
#*                                                                           */
#* be sure to read the source code license agreement before utilizing any    */
#* of the source code found herein.                                          */
#*                                                                           */
#* copyright (c) 1996  clark development company, inc.  all rights reserved. */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#=============================================================
#
#       pcbsetup.mak - makefile for pcbsetup program
#
#=============================================================

.silent
.autodepend

progname = pcbsetup

!ifndef root
root     = \out
!endif
source   = source
objdir   = $(bccompiler)
libroot  = $(root)\lib
libh     = $(libroot)\h
liblib   = $(libroot)\bcdos\$(bccompiler)

cfg      = $(progname).cfg
mak      = $(progname).mak

mdl      = l

includepath = $(include);$(libh);source;$(root)\pcb\source\h;\libs\vmdata

#=============================================================

!if $(debug)
codeopt=-ddebug
!endif

copt = -c

!if $d(bc50)
#leave out -oe due to a bug in borland c 4.0 thru 5.0
copt = $(copt) -obglmptv -x-
!elif $d(tc30)
#leave out all of the extra -oxxx switches for tc 3.0 because they aren't valid
!elif $d(bc31)
copt = $(copt) -oebglmptv
!endif

asmopt = /m /mx /t /d__$(mdl)__                 # assembler options

#=============================================================

.path.obj = $(objdir)
.path.asm = $(source)
.path.c   = $(source)
.path.cpp = $(source)

#=============================================================

.c.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

.cpp.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

.asm.obj:
  tasm $(asmopt) $(.path.asm)\$.,$(.path.obj)\$&

#=============================================================

exe_dependencies = \
  $(liblib)\misc\large\swap.obj \
  $(objdir)\pcbsetup.obj        \
  $(objdir)\account.obj         \
  $(objdir)\chkfile.obj         \
  $(objdir)\ci_build.obj        \
  $(objdir)\edit.obj            \
  $(objdir)\editblt.obj         \
  $(objdir)\editcmd.obj         \
  $(objdir)\editconf.obj        \
  $(objdir)\editdays.obj        \
  $(objdir)\editdirs.obj        \
  $(objdir)\editdoor.obj        \
  $(objdir)\editevnt.obj        \
  $(objdir)\editfido.obj        \
  $(objdir)\editfsec.obj        \
  $(objdir)\editftcn.obj        \
  $(objdir)\editlang.obj        \
  $(objdir)\editmod.obj         \
  $(objdir)\editpath.obj        \
  $(objdir)\editprot.obj        \
  $(objdir)\editpwrd.obj        \
  $(objdir)\editscpt.obj        \
  $(objdir)\edittcan.obj        \
  $(objdir)\editverb.obj        \
  $(objdir)\event.obj           \
  $(objdir)\fidoaka.obj         \
  $(objdir)\fidoarc.obj         \
  $(objdir)\fidocfg.obj         \
  $(objdir)\fconfig.obj         \
  $(objdir)\fidoemsi.obj        \
  $(objdir)\fidofreq.obj        \
  $(objdir)\fidoinfo.obj        \
  $(objdir)\fidolist.obj        \
  $(objdir)\fidoloc.obj         \
  $(objdir)\fidomgic.obj        \
  $(objdir)\fidonode.obj        \
  $(objdir)\fidonofr.obj        \
  $(objdir)\fidoorg.obj         \
  $(objdir)\fidopath.obj        \
  $(objdir)\fidotoss.obj        \
  $(objdir)\fidothis.obj        \
  $(objdir)\filesys.obj         \
  $(objdir)\fileconf.obj        \
  $(objdir)\filedisp.obj        \
  $(objdir)\fidoph.obj          \
  $(objdir)\fileques.obj        \
  $(objdir)\getcntxt.obj        \
  $(objdir)\init.obj            \
  $(objdir)\levels.obj          \
  $(objdir)\list.obj            \
  $(objdir)\misc.obj            \
  $(objdir)\modem.obj           \
  $(objdir)\modemacc.obj        \
  $(objdir)\modemswi.obj        \
  $(objdir)\node.obj            \
  $(objdir)\optcolor.obj        \
  $(objdir)\optctrl.obj         \
  $(objdir)\optfunc.obj         \
  $(objdir)\optlim.obj          \
  $(objdir)\optlog.obj          \
  $(objdir)\optmsg.obj          \
  $(objdir)\optos2.obj          \
  $(objdir)\optsys.obj          \
  $(objdir)\optxfer.obj         \
  $(objdir)\search.obj          \
  $(objdir)\shell.obj           \
  $(objdir)\subscrip.obj        \
  $(objdir)\sysop.obj           \
  $(objdir)\sysopcom.obj        \
  $(objdir)\sysopfun.obj        \
  $(objdir)\unique.obj          \
  $(objdir)\uucp.obj            \
  $(objdir)\data.obj            \
  $(objdir)\dos_$(mdl).lib      \
  $(objdir)\pcb_$(mdl).lib      \
  $(objdir)\misc_$(mdl).lib     \
  $(objdir)\screen_$(mdl).lib   \
  $(objdir)\scrnio_$(mdl).lib   \
  $(liblib)\system_$(mdl).lib   \
  $(liblib)\doscls_$(mdl).lib   \
  $(liblib)\country$(mdl).lib   \

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/p/l$(libpath) @&&|
/o- c0$(mdl).obj+
/o- $(liblib)\misc\large\swap.obj+
/o+ $(objdir)\pcbsetup.obj+
/o+ $(objdir)\account.obj+
/o+ $(objdir)\chkfile.obj+
/o+ $(objdir)\ci_build.obj+
/o+ $(objdir)\edit.obj+
/o+ $(objdir)\editblt.obj+
/o+ $(objdir)\editcmd.obj+
/o+ $(objdir)\editconf.obj+
/o+ $(objdir)\editdays.obj+
/o+ $(objdir)\editdirs.obj+
/o+ $(objdir)\editdoor.obj+
/o+ $(objdir)\editevnt.obj+
/o+ $(objdir)\editfido.obj+
/o+ $(objdir)\editfsec.obj+
/o+ $(objdir)\editftcn.obj+
/o+ $(objdir)\editlang.obj+
/o+ $(objdir)\editmod.obj+
/o+ $(objdir)\editpath.obj+
/o+ $(objdir)\editprot.obj+
/o+ $(objdir)\editpwrd.obj+
/o+ $(objdir)\editscpt.obj+
/o+ $(objdir)\edittcan.obj+
/o+ $(objdir)\editverb.obj+
/o+ $(objdir)\event.obj+
/o+ $(objdir)\fidoaka.obj+
/o+ $(objdir)\fidoarc.obj+
/o+ $(objdir)\fidocfg.obj+
/o+ $(objdir)\fconfig.obj+
/o+ $(objdir)\fidoemsi.obj+
/o+ $(objdir)\fidofreq.obj+
/o+ $(objdir)\fidoinfo.obj+
/o+ $(objdir)\fidolist.obj+
/o+ $(objdir)\fidoloc.obj+
/o+ $(objdir)\fidomgic.obj+
/o+ $(objdir)\fidonode.obj+
/o+ $(objdir)\fidonofr.obj+
/o+ $(objdir)\fidoorg.obj+
/o+ $(objdir)\fidopath.obj+
/o+ $(objdir)\fidotoss.obj+
/o+ $(objdir)\fidothis.obj+
/o+ $(objdir)\filesys.obj+
/o+ $(objdir)\fileconf.obj+
/o+ $(objdir)\filedisp.obj+
/o+ $(objdir)\fidoph.obj+
/o+ $(objdir)\fileques.obj+
/o+ $(objdir)\getcntxt.obj+
/o+ $(objdir)\init.obj+
/o+ $(objdir)\levels.obj+
/o+ $(objdir)\list.obj+
/o+ $(objdir)\misc.obj+
/o+ $(objdir)\modem.obj+
/o+ $(objdir)\modemacc.obj+
/o+ $(objdir)\modemswi.obj+
/o+ $(objdir)\node.obj+
/o+ $(objdir)\optcolor.obj+
/o+ $(objdir)\optctrl.obj+
/o+ $(objdir)\optfunc.obj+
/o+ $(objdir)\optlim.obj+
/o+ $(objdir)\optlog.obj+
/o+ $(objdir)\optmsg.obj+
/o+ $(objdir)\optos2.obj+
/o+ $(objdir)\optsys.obj+
/o+ $(objdir)\optxfer.obj+
/o+ $(objdir)\search.obj+
/o+ $(objdir)\shell.obj+
/o+ $(objdir)\subscrip.obj+
/o+ $(objdir)\sysop.obj+
/o+ $(objdir)\sysopcom.obj+
/o+ $(objdir)\sysopfun.obj+
/o+ $(objdir)\unique.obj+
/o+ $(objdir)\uucp.obj+
/o+ $(objdir)\data.obj+
$(objdir)\kbdstat.obj+
$(objdir)\vmfuncs.obj+
$(objdir)\dosfugts.obj+
$(objdir)\pcb\datafile.obj+
$(objdir)\pcb\datadflt.obj+
$(objdir)\pcb\chkexist.obj+
$(objdir)\pcb\endstr.obj+
$(objdir)\pcb\getdays.obj+
$(objdir)\pcb\config.obj+
$(objdir)\pcb\data120.obj+
$(objdir)\pcb\datawrit.obj+
$(objdir)\delete.obj+
$(objdir)\insert.obj+
$(objdir)\readscrn.obj+
$(objdir)\timechng.obj+
$(objdir)\getmode.obj+
$(objdir)\validate.obj+
$(objdir)\validsem.obj+
$(objdir)\int24hnd.obj
$(objdir)\$(progname)
$(objdir)\$(progname)
/o- $(objdir)\dos_$(mdl).lib+
/o- $(objdir)\pcb_$(mdl).lib+
/o- $(objdir)\misc_$(mdl).lib+
/o- $(objdir)\screen_$(mdl).lib+
/o- $(objdir)\scrnio_$(mdl).lib+
/o- $(liblib)\system_$(mdl).lib+
/o- $(liblib)\doscls_$(mdl).lib+
/o- $(liblib)\country$(mdl).lib+
/o- math$(mdl).lib+
/o- emu.lib+
/o- c$(mdl).lib+
/o- overlay.lib
|

#=============================================================

# rules for individual files where necessary

$(objdir)\fconfig.obj: $(root)\pcb\source\fido\fconfig.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\fconfig.c

$(objdir)\data.obj: $(root)\pcb\source\fido\data.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\data.cpp

#=============================================================

$(cfg): $(mak)
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
-c
-k
-g
-o
-p
-vmd
-vmp
-y
-z
-k-
-d
-m$(mdl)
-n$(objdir)
-i$(includepath)
-l$(libpath)
-dfido;travis;pcbsetup;usefloat;usedate;vmdata;ndebug;quiet_build
-d_fardata_=far
| $(cfg)

#=============================================================
