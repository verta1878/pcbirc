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
#       mkpcbtxt.mak - makefile for mkpcbtxt program
#
#=============================================================

.silent
.autodepend

progname = mkpcbtxt

!ifndef root
root     = \out
!endif
source   = .
objdir   = $(bccompiler)
libroot  = $(root)\lib
libh     = $(libroot)\h
liblib   = $(libroot)\bcdos\$(bccompiler)

cfg      = $(progname).cfg
mak      = $(progname).mak

mdl      = l

includepath = $(include);$(libh);$(root)\main\source\h;$(root)\main\source\util\pcbsm\source

#=============================================================

!if $(debug)
codeopt=-ddebug
!endif

copt = -c -m$(mdl) -n$(objdir)

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

#=============================================================

.c.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

.asm.obj:
  tasm $(asmopt) $(.path.asm)\$.,$(.path.obj)\$&

#=============================================================

exe_dependencies = \
  $(objdir)\mkpcbtxt.obj      \
  $(objdir)\strs15.obj        \
  $(objdir)\inputnum.obj      \
  $(objdir)\scrninpt.obj      \
  $(liblib)\dos_$(mdl).386    \
  $(liblib)\misc_$(mdl).386   \
  $(liblib)\screen_$(mdl).386 \
  $(liblib)\scrnio_$(mdl).386 \
  $(liblib)\system_$(mdl).386 \
  $(liblib)\country$(mdl).386

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath);$(root)\main\source\util\pcbsm\bc31 @&&|
c0$(mdl).obj+
$(objdir)\mkpcbtxt.obj+
$(objdir)\strs15.obj+
$(objdir)\inputnum.obj+
$(objdir)\scrninpt.obj+
$(root)\main\source\util\pcbsm\bc31\box.obj+
$(root)\main\source\util\pcbsm\bc31\delete.obj+
$(root)\main\source\util\pcbsm\bc31\dosfread.obj+
$(root)\main\source\util\pcbsm\bc31\getmode.obj+
$(root)\main\source\util\pcbsm\bc31\insert.obj+
$(root)\main\source\util\pcbsm\bc31\kbdstat.obj+
$(root)\main\source\util\pcbsm\bc31\readscrn.obj+
$(root)\main\source\util\pcbsm\bc31\scrollup.obj+
$(root)\main\source\util\pcbsm\bc31\showerr2.obj+
$(root)\main\source\util\pcbsm\bc31\stripb.obj+
$(root)\main\source\util\pcbsm\bc31\timechng.obj+
$(root)\main\source\util\pcbsm\bc31\wherex.obj
$(objdir)\$(progname)
                # no map file
$(liblib)\dos_$(mdl).386+
$(liblib)\misc_$(mdl).386+
$(liblib)\screen_$(mdl).386+
$(liblib)\scrnio_$(mdl).386+
$(liblib)\system_$(mdl).386+
$(liblib)\country$(mdl).386+
cl.lib
|

#=============================================================

# rules for individual files where necessary

$(objdir)\inputnum.obj: $(root)\lib\source\scrnio\inputnum.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\lib\source\scrnio\inputnum.c

$(objdir)\scrninpt.obj: $(root)\lib\source\scrnio\scrninpt.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\lib\source\scrnio\scrninpt.c

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
-z
-k-
-d
-i$(includepath)
-l$(libpath)
-d_fardata_=far
| $(cfg)

#=============================================================
