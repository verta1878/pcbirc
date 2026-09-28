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
#       pcbfiler.mak - makefile for pcbfiler program
#
#=============================================================

.nosilent
.autodepend

progname = pcbfiler

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

includepath = $(include);$(libh);source;\libs\vmdata

#=============================================================

!if $(debug)
codeopt=-ddebug
!endif

copt = -c

!if $d(bc50)
#leave out -oe due to a bug in borland c 4.0 thru 5.0
copt = $(copt) -obglmptv
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
  $(objdir)\action.obj        \
  $(objdir)\automove.obj      \
  $(objdir)\batch.obj         \
  $(objdir)\confmenu.obj      \
  $(objdir)\ctod2.obj         \
  $(objdir)\defaults.obj      \
  $(objdir)\dircolor.obj      \
  $(objdir)\dirfiles.obj      \
  $(objdir)\dupes.obj         \
  $(objdir)\edit.obj          \
  $(objdir)\editdirs.obj      \
  $(objdir)\editor.obj        \
  $(objdir)\editmove.obj      \
  $(objdir)\editrule.obj      \
  $(objdir)\findpath.obj      \
  $(objdir)\findtext.obj      \
  $(objdir)\getdiz.obj        \
  $(objdir)\idx.obj           \
  $(objdir)\idxname.obj       \
  $(objdir)\init.obj          \
  $(objdir)\pcbfiler.obj      \
  $(objdir)\preedit.obj       \
  $(objdir)\process.obj       \
  $(objdir)\readdir.obj       \
  $(objdir)\savedir.obj       \
  $(objdir)\scandupe.obj      \
  $(objdir)\shell.obj         \
  $(objdir)\showfree.obj      \
  $(objdir)\sort.obj          \
  $(objdir)\unique.obj        \
  $(objdir)\verify.obj        \
  $(objdir)\zipv.obj          \
  $(objdir)\arcv.obj          \
  $(liblib)\dos_$(mdl).lib    \
  $(liblib)\pcb_$(mdl).lib    \
  $(liblib)\misc_$(mdl).lib   \
  $(liblib)\screen_$(mdl).lib \
  $(liblib)\scrnio_$(mdl).lib \
  $(liblib)\system_$(mdl).lib \
  $(liblib)\country$(mdl).lib \
  \libs\vmdata\bc31_dos\vmdata.lib

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
/o- c0$(mdl).obj+
/o- $(liblib)\misc\large\swap.obj+
/o+ $(objdir)\action.obj+
/o- $(objdir)\automove.obj+
/o+ $(objdir)\batch.obj+
/o+ $(objdir)\confmenu.obj+
/o- $(objdir)\ctod2.obj+
/o+ $(objdir)\defaults.obj+
/o+ $(objdir)\dircolor.obj+
/o+ $(objdir)\dirfiles.obj+
/o+ $(objdir)\dupes.obj+
/o- $(objdir)\edit.obj+
/o+ $(objdir)\editdirs.obj+
/o+ $(objdir)\editor.obj+
/o+ $(objdir)\editmove.obj+
/o+ $(objdir)\editrule.obj+
/o+ $(objdir)\findpath.obj+
/o+ $(objdir)\findtext.obj+
/o- $(objdir)\getdiz.obj+
/o+ $(objdir)\idx.obj+
/o- $(objdir)\idxname.obj+
/o+ $(objdir)\init.obj+
/o+ $(objdir)\pcbfiler.obj+
/o+ $(objdir)\preedit.obj+
/o+ $(objdir)\process.obj+
/o+ $(objdir)\readdir.obj+
/o+ $(objdir)\savedir.obj+
/o+ $(objdir)\scandupe.obj+
/o+ $(objdir)\shell.obj+
/o+ $(objdir)\showfree.obj+
/o+ $(objdir)\sort.obj+
/o- $(objdir)\unique.obj+
/o+ $(objdir)\verify.obj+
/o+ $(objdir)\zipv.obj+
/o+ $(objdir)\arcv.obj
$(objdir)\$(progname)
                # no map file
/o- $(liblib)\dos_$(mdl).lib+
/o- $(liblib)\pcb_$(mdl).lib+
/o- $(liblib)\misc_$(mdl).lib+
/o- $(liblib)\screen_$(mdl).lib+
/o- $(liblib)\scrnio_$(mdl).lib+
/o- $(liblib)\system_$(mdl).lib+
/o- $(liblib)\country$(mdl).lib+
/o- \libs\vmdata\bc31_dos\vmdata.lib+
/o- math$(mdl).lib+
/o- emu.lib+
/o- c$(mdl).lib+
/o- overlay.lib
|

#=============================================================

# rules for individual files where necessary

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
-y
-z
-k-
-d
-m$(mdl)
-n$(objdir)
-i$(includepath)
-l$(libpath)
-dvmdata
-d_fardata_=far
| $(cfg)

#=============================================================
