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
#       uuutil.mak - makefile for uuutil program
#
#=============================================================

.silent
.autodepend

progname = uuutil

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

includepath = $(include);$(libh);.;..\common;..\uuin;$(src)\source\h

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
.path.cpp = $(source)

#=============================================================

.c.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

.cpp.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

{..\common\}.c.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

{..\common\}.cpp.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

.asm.obj:
  tasm $(asmopt) $(.path.asm)\$.,$(.path.obj)\$&

#=============================================================

exe_dependencies = \
  $(objdir)\uuutil.obj        \
  $(objdir)\fidocfg.obj       \
  $(objdir)\ci_other.obj      \
  $(objdir)\ci_build.obj      \
  $(objdir)\uucp.obj          \
  $(sdkobj)\dos\large\showerr2.obj     \
  $(sdkobj)\toolkit\large\nolog.obj    \
  $(sdkobj)\toolkit\large\smalldly.obj \
  $(sdkobj)\toolkit\large\nostatus.obj \
  $(sdkobj)\toolkit\large\nodisp.obj   \
  $(sdkobj)\toolkit\large\noansi.obj   \
  $(sdkobj)\toolkit\large\notxt.obj    \
  $(sdkobj)\toolkit\large\nochat.obj   \
  $(sdkobj)\toolkit\large\nolang.obj    \
  $(sdkobj)\toolkit\large\nopcbsys.obj \
  $(sdkobj)\toolkit\large\noscreen.obj \
  $(sdkobj)\toolkit\large\noshell.obj  \
  $(sdkobj)\toolkit\large\nosys.obj    \
  $(liblib)\pcbkit_$(mdl).lib \
  $(liblib)\dos_$(mdl).lib    \
  $(liblib)\doscls_$(mdl).lib \
  $(liblib)\pcb_$(mdl).lib    \
  $(liblib)\misc_$(mdl).lib   \
  $(liblib)\screen_$(mdl).lib \
  $(liblib)\system_$(mdl).lib \
  $(liblib)\country$(mdl).lib

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\uuutil.obj+
$(objdir)\fidocfg.obj+
$(objdir)\ci_other.obj+
$(objdir)\ci_build.obj+
$(objdir)\uucp.obj+
$(sdkobj)\dos\large\showerr2.obj+
$(sdkobj)\toolkit\large\nolog.obj+
$(sdkobj)\toolkit\large\smalldly.obj+
$(sdkobj)\toolkit\large\nostatus.obj+
$(sdkobj)\toolkit\large\nodisp.obj+
$(sdkobj)\toolkit\large\noansi.obj+
$(sdkobj)\toolkit\large\notxt.obj+
$(sdkobj)\toolkit\large\nochat.obj+
$(sdkobj)\toolkit\large\nolang.obj+
$(sdkobj)\toolkit\large\nopcbsys.obj+
$(sdkobj)\toolkit\large\noscreen.obj+
$(sdkobj)\toolkit\large\noshell.obj+
$(sdkobj)\toolkit\large\nosys.obj
$(objdir)\$(progname)
                # no map file
$(liblib)\pcbkit_$(mdl).lib+
$(liblib)\dos_$(mdl).lib+
$(liblib)\doscls_$(mdl).lib+
$(liblib)\pcb_$(mdl).lib+
$(liblib)\misc_$(mdl).lib+
$(liblib)\screen_$(mdl).lib+
$(liblib)\system_$(mdl).lib+
$(liblib)\country$(mdl).lib+
math$(mdl).lib+
emu.lib+
c$(mdl).lib+
$(sdklib)\vmdata_l.lib
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
-z
-k-
-d
-i$(includepath)
-l$(libpath)
-dlib
-d___use_vmdata___
-d___use_cnames___
-d___use_bool___
-u___ci_init_vm___
-u___ci_init_cn___
-uquiet_build
-uvm_devel
-dnomemcheck
-dunix
| $(cfg)

#=============================================================



#=============================================================
#       clean - remove everything this makefile produces
#=============================================================

clean:
  -if exist $(objdir)\*.obj del $(objdir)\*.obj
  -if exist $(objdir)\*.map del $(objdir)\*.map
  -if exist $(objdir)\*.res del $(objdir)\*.res
  -if exist $(objdir)\$(progname).exe del $(objdir)\$(progname).exe
  -if exist $(cfg) del $(cfg)
