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
!ifndef branch
branch   = pwa153
!endif
!ifndef cver
cver     = $(bccompiler)
!endif
!ifndef src
src      = \pcb153
!endif
!ifndef tkit
tkit     = \toolkit\pwa153
!endif
!ifndef libsdir
libsdir  = \pcbcbase
!endif
!ifndef libext
libext   = lib
!endif
outdir   = $(root)\$(branch)
sdk      = $(outdir)\sdk\$(cver)
sdklib   = $(sdk)\lib
sdkobj   = $(sdk)\obj
source   = .
objdir   = $(bccompiler)
libroot  = $(tkit)
libh     = $(libroot)\h
liblib   = $(sdklib)

cfg      = $(progname).cfg
mak      = $(progname).mak

mdl      = l

includepath = $(include);$(libh);$(src)\source\h;$(src)\source\util\pcbsm\source

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
  $(liblib)\dos_$(mdl).$(libext)    \
  $(liblib)\misc_$(mdl).$(libext)   \
  $(liblib)\screen_$(mdl).$(libext) \
  $(liblib)\scrnio_$(mdl).$(libext) \
  $(liblib)\system_$(mdl).$(libext) \
  $(liblib)\country$(mdl).$(libext)

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath);$(src)\source\util\pcbsm\$(cver) @&&|
c0$(mdl).obj+
$(objdir)\mkpcbtxt.obj+
$(objdir)\strs15.obj+
$(objdir)\inputnum.obj+
$(objdir)\scrninpt.obj+
$(src)\source\util\pcbsm\$(cver)\box.obj+
$(src)\source\util\pcbsm\$(cver)\delete.obj+
$(src)\source\util\pcbsm\$(cver)\dosfread.obj+
$(src)\source\util\pcbsm\$(cver)\getmode.obj+
$(src)\source\util\pcbsm\$(cver)\insert.obj+
$(src)\source\util\pcbsm\$(cver)\kbdstat.obj+
$(src)\source\util\pcbsm\$(cver)\readscrn.obj+
$(src)\source\util\pcbsm\$(cver)\scrollup.obj+
$(src)\source\util\pcbsm\$(cver)\showerr2.obj+
$(src)\source\util\pcbsm\$(cver)\stripb.obj+
$(src)\source\util\pcbsm\$(cver)\timechng.obj+
$(src)\source\util\pcbsm\$(cver)\wherex.obj
$(objdir)\$(progname)
                # no map file
$(liblib)\dos_$(mdl).$(libext)+
$(liblib)\misc_$(mdl).$(libext)+
$(liblib)\screen_$(mdl).$(libext)+
$(liblib)\scrnio_$(mdl).$(libext)+
$(liblib)\system_$(mdl).$(libext)+
$(liblib)\country$(mdl).$(libext)+
cl.lib
|

#=============================================================

# rules for individual files where necessary

$(objdir)\inputnum.obj: $(tkit)\source\scrnio\inputnum.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(tkit)\source\scrnio\inputnum.c

$(objdir)\scrninpt.obj: $(tkit)\source\scrnio\scrninpt.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(tkit)\source\scrnio\scrninpt.c

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


#=============================================================
#       clean - remove everything this makefile produces
#=============================================================

clean:
  -if exist $(objdir)\*.obj del $(objdir)\*.obj
  -if exist $(objdir)\*.map del $(objdir)\*.map
  -if exist $(objdir)\*.res del $(objdir)\*.res
  -if exist $(objdir)\$(progname).exe del $(objdir)\$(progname).exe
  -if exist $(cfg) del $(cfg)
