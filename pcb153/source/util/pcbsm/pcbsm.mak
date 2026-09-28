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
#       pcbsm.mak - makefile for pcbsm program
#
#=============================================================

.silent
.autodepend

progname = pcbsm

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
source   = source
objdir   = $(bccompiler)
libroot  = $(tkit)
libh     = $(libroot)\h
liblib   = $(sdklib)

cfg      = $(progname).cfg
mak      = $(progname).mak

mdl      = l

includepath = $(include);source;$(src)\source\misc\help;$(src)\source\h;$(libh)

#=============================================================

!if $(debug)
codeopt=-ddebug
!endif

!if $d(td)
codeopt = $(codeopt) -v
asmopt  = $(asmopt) /v
linkopt = $(linkopt) /v
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

linkopt = /x /c

!if $d(td)
copt    = $(copt) -v
asmopt  = $(asmopt) /v
linkopt = $(linkopt) /v
!endif


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
  $(objdir)\acct.obj          \
  $(objdir)\adjust.obj        \
  $(objdir)\batch.obj         \
  $(objdir)\colors.obj        \
  $(objdir)\conf.obj          \
  $(objdir)\confmove.obj      \
  $(objdir)\defedit.obj       \
  $(objdir)\demo.obj          \
  $(objdir)\edit.obj          \
  $(objdir)\expdate.obj       \
  $(objdir)\index.obj         \
  $(objdir)\init.obj          \
  $(objdir)\pack.obj          \
  $(objdir)\packinpt.obj      \
  $(objdir)\packon.obj        \
  $(objdir)\pcbsm.obj         \
  $(objdir)\personal.obj      \
  $(objdir)\phone.obj         \
  $(objdir)\print.obj         \
  $(objdir)\process.obj       \
  $(objdir)\sort.obj          \
  $(objdir)\undo.obj          \
  $(objdir)\userinfo.obj      \
  $(objdir)\usernet.obj       \
  $(objdir)\users.obj         \
  $(objdir)\abort.obj      \
  $(objdir)\box.obj      \
  $(objdir)\cnames.obj      \
  $(objdir)\config.obj      \
  $(objdir)\copyfile.obj      \
  $(objdir)\datafil2.obj      \
  $(objdir)\delete.obj      \
  $(objdir)\diskfree.obj      \
  $(objdir)\dmath.obj      \
  $(objdir)\dosfread.obj      \
  $(objdir)\endstr.obj      \
  $(objdir)\findfour.obj      \
  $(objdir)\getmode.obj      \
  $(objdir)\insert.obj      \
  $(objdir)\readscrn.obj      \
  $(objdir)\savetext.obj      \
  $(objdir)\scrollup.obj      \
  $(objdir)\showerr2.obj      \
  $(objdir)\smallsub.obj      \
  $(objdir)\stripb.obj      \
  $(objdir)\timechng.obj      \
    $(objdir)\ctod.obj          \
  $(objdir)\kbdstat.obj       \
$(objdir)\wherex.obj      \
  $(liblib)\dos_$(mdl).$(libext)    \
  $(liblib)\pcb_$(mdl).$(libext)    \
  $(liblib)\misc_$(mdl).$(libext)   \
  $(liblib)\screen_$(mdl).$(libext) \
  $(liblib)\scrnio_$(mdl).$(libext) \
  $(liblib)\system_$(mdl).$(libext) \
  $(liblib)\country$(mdl).$(libext) \
  $(sdklib)\vmdata_l.lib

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) $(linkopt) /l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\acct.obj+
$(objdir)\adjust.obj+
$(objdir)\batch.obj+
$(objdir)\colors.obj+
$(objdir)\conf.obj+
$(objdir)\confmove.obj+
$(objdir)\defedit.obj+
$(objdir)\demo.obj+
$(objdir)\edit.obj+
$(objdir)\expdate.obj+
$(objdir)\index.obj+
$(objdir)\init.obj+
$(objdir)\pack.obj+
$(objdir)\packinpt.obj+
$(objdir)\packon.obj+
$(objdir)\pcbsm.obj+
$(objdir)\personal.obj+
$(objdir)\phone.obj+
$(objdir)\print.obj+
$(objdir)\process.obj+
$(objdir)\sort.obj+
$(objdir)\undo.obj+
$(objdir)\userinfo.obj+
$(objdir)\usernet.obj+
$(objdir)\users.obj+
$(objdir)\abort.obj+
$(objdir)\box.obj+
$(objdir)\cnames.obj+
$(objdir)\config.obj+
$(objdir)\copyfile.obj+
$(objdir)\ctod.obj+
$(objdir)\kbdstat.obj+
$(objdir)\datafil2.obj+
$(objdir)\delete.obj+
$(objdir)\diskfree.obj+
$(objdir)\dmath.obj+
$(objdir)\dosfread.obj+
$(objdir)\endstr.obj+
$(objdir)\findfour.obj+
$(objdir)\getmode.obj+
$(objdir)\insert.obj+
$(objdir)\readscrn.obj+
$(objdir)\savetext.obj+
$(objdir)\scrollup.obj+
$(objdir)\showerr2.obj+
$(objdir)\smallsub.obj+
$(objdir)\stripb.obj+
$(objdir)\timechng.obj+
$(objdir)\wherex.obj
$(objdir)\$(progname)

$(liblib)\dos_$(mdl).$(libext)+
$(liblib)\pcb_$(mdl).$(libext)+
$(liblib)\misc_$(mdl).$(libext)+
$(liblib)\screen_$(mdl).$(libext)+
$(liblib)\scrnio_$(mdl).$(libext)+
$(liblib)\system_$(mdl).$(libext)+
$(liblib)\country$(mdl).$(libext)+
$(sdklib)\vmdata_l.lib+
mathl.lib+
emu.lib+
cl.lib+
overlay.lib
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
-dpcbsm;usedate;bigndx;usefloat
-d_fardata_=far
| $(cfg)

#=============================================================
$(objdir)\abort.obj: extraobj\abort.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\abort.obj -c extraobj\abort.c

$(objdir)\box.obj: extraobj\box.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\box.obj -c extraobj\box.c

$(objdir)\cnames.obj: extraobj\cnames.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\cnames.obj -c extraobj\cnames.c

$(objdir)\config.obj: extraobj\config.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\config.obj -c extraobj\config.c

$(objdir)\copyfile.obj: extraobj\copyfile.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\copyfile.obj -c extraobj\copyfile.c

$(objdir)\datafil2.obj: extraobj\datafil2.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\datafil2.obj -c extraobj\datafil2.c


$(objdir)\delete.obj: extraobj\delete.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\delete.obj -c extraobj\delete.c

$(objdir)\diskfree.obj: extraobj\diskfree.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\diskfree.obj -c extraobj\diskfree.c

$(objdir)\dmath.obj: extraobj\dmath.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\dmath.obj -c extraobj\dmath.c

$(objdir)\dosfread.obj: extraobj\dosfread.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\dosfread.obj -c extraobj\dosfread.c

$(objdir)\endstr.obj: extraobj\endstr.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\endstr.obj -c extraobj\endstr.c

$(objdir)\findfour.obj: extraobj\findfour.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\findfour.obj -c extraobj\findfour.c

$(objdir)\getmode.obj: extraobj\getmode.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\getmode.obj -c extraobj\getmode.c

$(objdir)\insert.obj: extraobj\insert.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\insert.obj -c extraobj\insert.c

$(objdir)\readscrn.obj: extraobj\readscrn.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\readscrn.obj -c extraobj\readscrn.c

$(objdir)\savetext.obj: extraobj\savetext.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\savetext.obj -c extraobj\savetext.c

$(objdir)\scrollup.obj: extraobj\scrollup.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\scrollup.obj -c extraobj\scrollup.c

$(objdir)\showerr2.obj: extraobj\showerr2.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\showerr2.obj -c extraobj\showerr2.c

$(objdir)\smallsub.obj: extraobj\smallsub.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\smallsub.obj -c extraobj\smallsub.c

$(objdir)\stripb.obj: extraobj\stripb.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\stripb.obj -c extraobj\stripb.c

$(objdir)\timechng.obj: extraobj\timechng.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\timechng.obj -c extraobj\timechng.c

$(objdir)\wherex.obj: extraobj\wherex.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\wherex.obj -c extraobj\wherex.c



$(objdir)\ctod.obj: extraobj\ctod.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\ctod.obj -c extraobj\ctod.c

$(objdir)\kbdstat.obj: extraobj\kbdstat.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -o$(objdir)\kbdstat.obj -c extraobj\kbdstat.c


#=============================================================
#       clean - remove everything this makefile produces
#=============================================================

clean:
  -if exist $(objdir)\*.obj del $(objdir)\*.obj
  -if exist $(objdir)\*.map del $(objdir)\*.map
  -if exist $(objdir)\*.res del $(objdir)\*.res
  -if exist $(objdir)\$(progname).exe del $(objdir)\$(progname).exe
  -if exist $(cfg) del $(cfg)
