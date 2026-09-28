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
#       pcbmoni.mak - makefile for pcbmoni program
#
#=============================================================

.nosilent
.autodepend

progname = pcbmoni

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

mdl      = s

includepath = $(include);$(libh)

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
  $(compiler) +$(cfg) $(copt) $(codeopt) -i$(includepath) {$< }

.asm.obj:
  tasm $(asmopt) $(.path.asm)\$.,$(.path.obj)\$&

#=============================================================

exe_dependencies =  \
  $(liblib)\dos\small\showerr2.obj \
  $(objdir)\pcbmoni.obj       \
  $(objdir)\scrninpt.obj      \
  $(liblib)\dos_$(mdl).lib    \
  $(liblib)\misc_$(mdl).lib   \
  $(liblib)\screen_$(mdl).lib \
  $(liblib)\scrnio_$(mdl).lib \
  $(liblib)\system_$(mdl).lib \
  $(liblib)\country$(mdl).lib

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(liblib)\dos\small\showerr2.obj+
$(objdir)\pcbmoni.obj+
$(objdir)\scrninpt.obj
$(objdir)\$(progname)
                # no map file
$(liblib)\dos_$(mdl).lib+
$(liblib)\misc_$(mdl).lib+
$(liblib)\screen_$(mdl).lib+
$(liblib)\scrnio_$(mdl).lib+
$(liblib)\system_$(mdl).lib+
$(liblib)\country$(mdl).lib+
c$(mdl).lib
|

#=============================================================

# rules for individual files where necessary

$(objdir)\scrninpt.obj: $(libroot)\source\scrnio\scrninpt.c
  $(compiler) +$(cfg) $(copt) $(codeopt) -i$(includepath) $(libroot)\source\scrnio\scrninpt.c

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
| $(cfg)

#=============================================================
