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
#       pcbnlc.mak - makefile for pcbnlc program
#
#=============================================================

.nosilent
.autodepend

progname = pcbnlc

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

includepath = .;$(include);$(libh);$(root)\pcb\source\h;\libs\vmdata;$(libsdir)\codebase\source

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
  $(compiler) +$(cfg) $(copt) $(codeopt){$< }

.asm.obj:
  tasm $(asmopt) $(.path.asm)\$.,$(.path.obj)\$&

#=============================================================

exe_dependencies =  \
  $(objdir)\pcbnlc.obj        \
  $(objdir)\diff.obj          \
  $(objdir)\dbase.obj         \
  $(objdir)\data.obj          \
  $(objdir)\fconfig.obj       \
  $(objdir)\fidomisc.obj      \
  $(objdir)\timer.obj         \
  $(liblib)\dos_$(mdl).lib    \
  $(liblib)\doscls_$(mdl).lib \
  $(liblib)\misc_$(mdl).lib   \
  $(liblib)\pcb_$(mdl).lib    \
  $(liblib)\screen_$(mdl).lib \
  $(liblib)\scrnio_$(mdl).lib \
  $(liblib)\system_$(mdl).lib \
  \libs\codebase\bor31\c4base.lib \
  \libs\vmdata\bc31_dos\vmdata.lib

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\pcbnlc.obj+
$(objdir)\diff.obj+
$(objdir)\dbase.obj+
$(objdir)\data.obj+
$(objdir)\fconfig.obj+
$(objdir)\fidomisc.obj+
$(objdir)\timer.obj
$(objdir)\pcbnlc
                # no map file
$(liblib)\dos_$(mdl).lib+
$(liblib)\doscls_$(mdl).lib+
$(liblib)\misc_$(mdl).lib+
$(liblib)\pcb_$(mdl).lib+
$(liblib)\screen_$(mdl).lib+
$(liblib)\scrnio_$(mdl).lib+
$(liblib)\system_$(mdl).lib+
$(liblib)\country$(mdl).lib+
\libs\codebase\bor31\c4base.lib+
\libs\vmdata\bc31_dos\vmdata.lib+
emu.lib+
math$(mdl).lib+
c$(mdl).lib
|

#=============================================================

# rules for individual files where necessary

$(objdir)\dbase.obj: $(root)\pcb\source\ppl\dbase.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\ppl\dbase.cpp

$(objdir)\data.obj: $(root)\pcb\source\fido\data.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\data.cpp

$(objdir)\fconfig.obj: $(root)\pcb\source\fido\fconfig.c
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\fconfig.c

$(objdir)\fidomisc.obj: $(root)\pcb\source\fido\fidomisc.cpp
  $(compiler) +$(cfg) $(copt) $(codeopt) $(root)\pcb\source\fido\fidomisc.cpp

$(objdir)\timer.obj:    $(root)\pcb\source\asm\timer.asm
  $(tasm) $(asmopt) $(root)\pcb\source\asm\timer.asm, $(objdir)\timer.obj

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
-dfido;pcbsetup;pcbnlc
-d_fardata_=far
| $(cfg)

#=============================================================
