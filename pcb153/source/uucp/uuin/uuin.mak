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
#       uuin.mak - makefile for uuin program
#
#=============================================================

.silent
.autodepend

progname = uuin

#uncomment this line if you want to use tlink instead of rtlink
#usetlink = 1

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

includepath = $(include);$(libh);..\common;$(src)\source\h

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
  $(objdir)\uuin.obj          \
  $(objdir)\pcbmsg.obj        \
  $(objdir)\msgbase.obj       \
  $(objdir)\umwf.obj          \
  $(objdir)\uucp.obj          \
  $(objdir)\exprtmsg.obj      \
  $(objdir)\uudecode.obj      \
  $(objdir)\uushwerr.obj      \
  $(objdir)\vmdata.obj        \
  $(objdir)\decode64.obj      \
  $(objdir)\decodeq.obj       \
  $(objdir)\decodem.obj       \
  $(objdir)\rejects.obj       \
  $(objdir)\vmdclass.obj      \
  $(objdir)\dresword.obj      \
  $(objdir)\ci_other.obj      \
  $(sdkobj)\toolkit\large\nolog.obj    \
  $(sdkobj)\toolkit\large\smalldly.obj \
  $(liblib)\pcbkit_$(mdl).lib \
  $(liblib)\dos_$(mdl).lib    \
  $(liblib)\doscls_$(mdl).lib \
  $(liblib)\pcb_$(mdl).lib    \
  $(liblib)\misc_$(mdl).lib   \
  $(liblib)\screen_$(mdl).lib \
  $(liblib)\system_$(mdl).lib \
  $(liblib)\country$(mdl).lib \
  $(sdklib)\vmdata_l.lib

#=============================================================

!if $d(usetlink)
!if $d(bc31)
$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\uuin.obj+
$(objdir)\pcbmsg.obj+
$(objdir)\msgbase.obj+
$(objdir)\umwf.obj+
$(objdir)\uucp.obj+
$(objdir)\exprtmsg.obj+
$(objdir)\uudecode.obj+
$(objdir)\uushwerr.obj+
$(objdir)\vmdata.obj+
$(objdir)\decode64.obj+
$(objdir)\decodeq.obj+
$(objdir)\decodem.obj+
$(objdir)\vmdclass.obj+
$(objdir)\dresword.obj+
$(objdir)\ci_other.obj+
..\bc31\stbdsgtn.obj+
..\bc31\stbdsptn.obj+
..\bc31\stbsgetn.obj+
..\bc31\stbsputn.obj+
$(sdkobj)\toolkit\large\nolog.obj+
$(sdkobj)\toolkit\large\smalldly.obj
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
!else
$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\uuin.obj+
$(objdir)\pcbmsg.obj+
$(objdir)\msgbase.obj+
$(objdir)\umwf.obj+
$(objdir)\uucp.obj+
$(objdir)\exprtmsg.obj+
$(objdir)\uudecode.obj+
$(objdir)\uushwerr.obj+
$(objdir)\vmdata.obj+
$(objdir)\decode64.obj+
$(objdir)\decodeq.obj+
$(objdir)\decodem.obj+
$(objdir)\vmdclass.obj+
$(objdir)\dresword.obj+
$(objdir)\ci_other.obj+
$(sdkobj)\toolkit\large\nolog.obj+
$(sdkobj)\toolkit\large\smalldly.obj
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
!endif
!else
!if $d(bc31)
$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  rtlink @&&|
#turbodebugger
shellout
output $(objdir)\uuin
#map s,n,a,l,v,x
file $(libpath)\c0$(mdl).obj
file $(objdir)\uuin.obj
file $(objdir)\pcbmsg.obj
file $(objdir)\msgbase.obj
file $(objdir)\umwf.obj
file $(objdir)\uucp.obj
file $(objdir)\exprtmsg.obj
file $(objdir)\uudecode.obj
file $(objdir)\uushwerr.obj
file $(objdir)\vmdata.obj
file $(objdir)\decode64.obj
file $(objdir)\decodeq.obj
file $(objdir)\decodem.obj
file $(objdir)\vmdclass.obj
file $(objdir)\dresword.obj
file $(objdir)\ci_other.obj
file ..\bc31\stbdsgtn.obj
file ..\bc31\stbdsptn.obj
file ..\bc31\stbsgetn.obj
file ..\bc31\stbsputn.obj
file $(sdkobj)\toolkit\large\nolog.obj
file $(sdkobj)\toolkit\large\smalldly.obj
library $(liblib)\system_$(mdl).lib
library $(liblib)\pcbkit_$(mdl).lib
library $(liblib)\dos_$(mdl).lib
library $(liblib)\doscls_$(mdl).lib
library $(liblib)\pcb_$(mdl).lib
library $(liblib)\misc_$(mdl).lib
library $(liblib)\screen_$(mdl).lib
library $(liblib)\country$(mdl).lib
library $(libpath)\math$(mdl).lib
library $(libpath)\emu.lib
library $(libpath)\c$(mdl).lib
library $(sdklib)\vmdata_l.lib
|
!else
$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  rtlink @&&|
#turbodebugger
shellout
output $(objdir)\uuin
#map s,n,a,l,v,x
file $(libpath)\c0$(mdl).obj
file $(objdir)\uuin.obj
file $(objdir)\pcbmsg.obj
file $(objdir)\msgbase.obj
file $(objdir)\umwf.obj
file $(objdir)\uucp.obj
file $(objdir)\exprtmsg.obj
file $(objdir)\uudecode.obj
file $(objdir)\uushwerr.obj
file $(objdir)\vmdata.obj
file $(objdir)\decode64.obj
file $(objdir)\decodeq.obj
file $(objdir)\decodem.obj
file $(objdir)\vmdclass.obj
file $(objdir)\dresword.obj
file $(objdir)\ci_other.obj
file $(sdkobj)\toolkit\large\nolog.obj
file $(sdkobj)\toolkit\large\smalldly.obj
library $(liblib)\system_$(mdl).lib
library $(liblib)\pcbkit_$(mdl).lib
library $(liblib)\dos_$(mdl).lib
library $(liblib)\doscls_$(mdl).lib
library $(liblib)\pcb_$(mdl).lib
library $(liblib)\misc_$(mdl).lib
library $(liblib)\screen_$(mdl).lib
library $(liblib)\country$(mdl).lib
library $(libpath)\math$(mdl).lib
library $(libpath)\emu.lib
library $(libpath)\c$(mdl).lib
library $(sdklib)\vmdata_l.lib
|
!endif
!endif

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
-d___use_ci_cache___
-d___use_users___
-d___use_cnames___
-dbounce
-udebug
-dunix
-d___uuin___
-dexclude_debug
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
