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
#       fidoutil.mak - makefile for fidoutil program
#
#=============================================================

.silent
.autodepend

progname = fidoutil

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
source   = source
objdir   = $(bccompiler)
libroot  = $(tkit)
libh     = $(libroot)\h
liblib   = $(sdklib)

cfg      = $(progname).cfg
mak      = $(progname).mak

mdl      = l

includepath = $(include);$(libh);source;$(src)\source\h;$(src)\source\uucp\common

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
  analize.obj   \
  ci_build.obj  \
  convert.obj   \
  data.obj      \
  fidonet.obj   \
  maint.obj     \
  passthru.obj  \
  pcbfu.obj     \
  ptsetup.obj   \
  report.obj    \
  $(sdkobj)\dos\large\showerr2.obj \
  $(liblib)\dos_$(mdl).lib          \
  $(liblib)\doscls_$(mdl).lib       \
  $(liblib)\pcb_$(mdl).lib          \
  $(liblib)\misc_$(mdl).lib         \
  $(liblib)\screen_$(mdl).lib       \
  $(liblib)\scrnio_$(mdl).lib       \
  $(liblib)\system_$(mdl).lib       \
  $(liblib)\country$(mdl).lib       \
  $(sdklib)\vmdata_l.lib

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\analize.obj+
$(objdir)\ci_build.obj+
$(objdir)\convert.obj+
$(objdir)\data.obj+
$(objdir)\fidonet.obj+
$(objdir)\maint.obj+
#$(objdir)\packfido.obj+
$(objdir)\passthru.obj+
$(objdir)\pcbfu.obj+
$(objdir)\ptsetup.obj+
$(objdir)\report.obj+
$(sdkobj)\dos\large\showerr2.obj
$(objdir)\$(progname)
                # no map file
$(liblib)\dos_$(mdl).lib+
$(liblib)\doscls_$(mdl).lib+
$(liblib)\pcb_$(mdl).lib+
$(liblib)\misc_$(mdl).lib+
$(liblib)\screen_$(mdl).lib+
$(liblib)\scrnio_$(mdl).lib+
$(liblib)\system_$(mdl).lib+
$(liblib)\country$(mdl).lib+
$(sdklib)\vmdata_l.lib+
math$(mdl).lib+
emu.lib+
c$(mdl).lib+
|

#=============================================================

# rules for individual files where necessary

ci_build.obj: $(src)\source\util\pcbsetup\source\ci_build.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(src)\source\util\pcbsetup\source\ci_build.c

data.obj: $(src)\source\fido\data.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(src)\source\fido\data.cpp

#packfido.obj: $(src)\source\misc\packfido\packfido.c
#  $(compiler) +$(cfg) $(copt) $(codeopt) $(src)\source\misc\packfido\packfido.c

passthru.obj: $(src)\source\fido\passthru.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(src)\source\fido\passthru.cpp

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
-dpcbsetup;quiet_build;fido;pcbfu;
-dfidoutil
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


#=============================================================
# packfido and fidoutil - why the three packfido lines above are
# commented out, 2026-09-23.
#
# they had been re-enabled and repointed at
#     $(src)\source\misc\packfido\packfido.c
# which is a reconstruction of clark's standalone packfido.exe.  that
# cannot be linked into fidoutil.exe:
#
#   - it is a program.  it has main().  fidoutil has its own, and two
#     in one image is a link error.
#   - fidoutil does not want a program.  packfido.obj supplied exactly
#     one symbol, do_pack(), declared at convert.cpp:54 - and the only
#     call to it, at convert.cpp:125, is itself commented out.
#   - the model does not match either.  the reconstruction is small
#     model and compiled as c++ for the kit; this makefile is $(mdl).
#
# fidoutil.exe builds without it, and always did.  re-enabling it means
# writing a do_pack() module, not compiling the program.  the
# reconstructions and what is known about them are in
#     pcb153\source\misc\packfido\readme.md
#=============================================================