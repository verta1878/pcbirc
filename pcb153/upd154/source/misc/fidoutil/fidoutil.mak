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
  $(liblib)\dos\large\showerr2.obj \
  $(liblib)\dos_$(mdl).lib          \
  $(liblib)\doscls_$(mdl).lib       \
  $(liblib)\pcb_$(mdl).lib          \
  $(liblib)\misc_$(mdl).lib         \
  $(liblib)\screen_$(mdl).lib       \
  $(liblib)\scrnio_$(mdl).lib       \
  $(liblib)\system_$(mdl).lib       \
  $(liblib)\country$(mdl).lib       \
  \libs\vmdata\bc31_dos\vmdata.lib

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
$(liblib)\dos\large\showerr2.obj
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
\libs\vmdata\bc31_dos\vmdata.lib+
math$(mdl).lib+
emu.lib+
c$(mdl).lib+
|

#=============================================================

# rules for individual files where necessary

ci_build.obj: $(root)\pcbsetup\source\ci_build.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcbsetup\source\ci_build.c

data.obj: $(root)\pcb\source\fido\data.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\data.cpp

#packfido.obj: $(root)\packfido\packfido.c
#  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\packfido\packfido.c

passthru.obj: $(root)\pcb\source\fido\passthru.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\passthru.cpp

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
-d_fardata_=far
-dfidoutil
| $(cfg)

#=============================================================


#=============================================================
# packfido and fidoutil - why the three packfido lines are
# commented out, 2026-09-23.
#
# they pointed at $(root)\packfido\packfido.c, a path that has
# never existed in this repo.  the reconstructions are at
#     pcb153\source\misc\packfido\        byte-exact, 15.21 layout
#     pcb153\upd154\source\misc\packfido\ areas.dat v3, borland
#     pcb154\main\source\misc\packfido\   openwatcom and os/2
# and none of them can be linked in here.  they are programs, with
# main().  fidoutil has its own.  what fidoutil wanted from
# packfido.obj was one symbol, do_pack(), declared at
# convert.cpp:54 - and the only call to it, at convert.cpp:125, is
# commented out in clark's own source.
#
# fidoutil.exe builds without it.  verified 2026-09-23: 11 objects,
# 0 errors, 153,674 bytes.  see build.md.
#=============================================================