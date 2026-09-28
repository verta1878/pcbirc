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


.nosilent
.autodepend

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

# process debug
#debug = 1
#td = 1
#debugscr = 1
#dbgnewdel = 1
dbase = 1
#___use_var___ = 1

#!if $d(___use_vars___)
#___use_var___ = 1
#!endif

!if $d(dbase)
dbase = 1
!endif

!if $d(debug) && !$d(td)
td = 1
!endif

!if $d(debug) && !$d(debugscr)
debugscr = 1
!endif

!if $d(debug) && !$d(dbgnewdel)
dbgnewdel = 1
!endif

!if $d(debug) && !$d(errbrk)
errbrk = 1
!endif

srcdef  = ___comp___
objpth  = obj\ppl
dstname = pplc
model   = l

objlst  = $(objpth)\scomp.obj    \
          $(objpth)\newscr.obj   \
          $(objpth)\scrcomp.obj  \
          $(objpth)\scrmisc.obj  \
          $(objpth)\pcbmisc.obj  \
          $(objpth)\var.obj      \
          $(objpth)\label.obj    \
          $(objpth)\ceh.obj      \
          $(objpth)\h2name.obj

liblst  = country$(model).386        \
          dos_$(model).386           \
          misc_$(model).386          \
          system_$(model).386        \
          $(sdklib)\pcbkit_$(model).lib     \
          math$(model).lib           \
          emu.lib                    \
          c$(model).lib

################################################################################

.path.obj = $(objpth)
.path.c   = source\compiler

################################################################################

cc = bcc
co = -c -m$(model)
cd = -dlib;comm;$(srcdef)

ac = tasm.exe
ao = /m3

lc = tlink
lo = /yx+ /ye- /x

!if $d(td)
co = $(co) -v
lo = $(lo) /v
!endif

!if $d(___use_var___)
cd = $(cd);___use_var___
!endif

!if $d(debug)
cd = $(cd);debug
!endif

!if $d(debugscr)
cd = $(cd);debugscr
!endif

!if $d(dbgnewdel)
cd = $(cd);dbgnewdel
!endif

!if $d(errbrk)
cd = $(cd);errbrk
!endif

!if $d(pcb_demo)
cd = $(cd);pcb_demo
!endif

!if $d(386)
co = $(co) -3
!endif

!if $d(pcb152)
cd = $(cd);pcb152
!endif

################################################################################





libpth = $(libpth)$(libpath)
libpth = $(libpth);$(sdklib)

################################################################################

# implicit rules

.c.obj:
        $(cc) +153\pplc.cfg -p $(co) $(cd) {$< }

{source\compiler}.cpp.obj:
        $(cc) +153\pplc.cfg $(co) $(cd) {$< }

{source\ppl}.cpp.obj:
        $(cc) +153\pplc.cfg $(co) $(cd) {$< }

#{x:\sdrlib}.cpp.obj:
#        $(cc) $(co) $(cd) {$< }

{source\compiler}.asm.obj:
        $(ac) $(ao) $<,$(objpth)\$&

################################################################################

# explicit rules

$(objpth)\pplc.exe: $(objlst)
        $(lc) $(lo) /l$(libpth) @&&|
c0$(model).obj $(**:turboc.cfg=)
$(objpth)\pplc.exe
# no map file
$(liblst)
|


#=============================================================
#       clean - remove everything this makefile produces
#=============================================================

clean:
  -if exist $(objpth)\*.obj del $(objpth)\*.obj
  -if exist $(objpth)\*.map del $(objpth)\*.map
  -if exist $(objpth)\*.res del $(objpth)\*.res
  -if exist $(objpth)\pplc.exe del $(objpth)\pplc.exe
