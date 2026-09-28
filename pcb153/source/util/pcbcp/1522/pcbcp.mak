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
#       pcbcp.mak - makefile for project d:\bcos2\examples\test\pcbcp.prj
#               created on 05/25/95 at 10:50
#
#=============================================================

.silent
.autodepend

#=============================================================
#               translator definitions
#=============================================================

version = 1522
root    = \proj
proj    = $(root)\pcbcp
libroot = $(root)\lib
libdir  = $(libroot)\bcos2
source  = source
help    = $(proj)\help
obj     = $(proj)\obj
cfg     = $(version)\pcbcp.cfg
mak     = $(version)\pcbcp.mak

!if $d(debug)
  debug = -v
!else
  debug =
!endif

!if $d(validator)
  cc = $(compiler) +$(cfg) -dvalidator
!else
  cc = $(compiler) +$(cfg)
!endif

includepath = $(include);$(source);$(libroot)\h


.path.c   = $(source)
.path.obj = $(obj)

#=============================================================
#               implicit rules
#=============================================================
.c.obj:
  $(cc) -c {$< }


#=============================================================
#               list macros
#=============================================================
link_exclude =  \
 $(obj)\main.res

link_include =  \
 $(libdir)\system.lib \
 $(libdir)\misc.lib \
 $(libdir)\screen.lib \
 $(libdir)\dos.lib \
 $(source)\pcbcp.def \
 $(obj)\user.obj \
 $(obj)\thrd.obj \
 $(obj)\pnt.obj \
 $(obj)\main.obj \
 $(obj)\help.obj \
 $(obj)\init.obj \
 $(obj)\file.obj \
 $(obj)\dlg.obj

#=============================================================
#               explicit rules
#=============================================================

$(obj)\done: $(obj)\pcbcp.exe $(obj)\pcbcp.hlp
      echo all done > $(obj)\done

$(obj)\pcbcp.exe: $(cfg) $(link_include) $(link_exclude)
  $(linker) $(debug) /b:0x10000 /x /toe /aa /l$(libpath) @&&|
$(libpath)\c02.obj+
$(obj)\user.obj+
$(obj)\thrd.obj+
$(obj)\pnt.obj+
$(obj)\main.obj+
$(obj)\help.obj+
$(obj)\init.obj+
$(obj)\file.obj+
$(obj)\dlg.obj
$(obj)\pcbcp.exe
$(obj)\pcbcp.map
$(libdir)\system.lib+
$(libdir)\misc.lib+
$(libdir)\screen.lib+
$(libdir)\dos.lib+
!if $d(validator)
  d:\toolkt21\os2lib\validatr.lib+
!endif
$(libpath)\c2mt.lib+
$(libpath)\os2.lib
$(source)\pcbcp.def
|
  rc.exe $(obj)\main.res $(obj)\pcbcp.exe

#=============================================================
#               individual file dependencies
#=============================================================
$(obj)\pcbcp.hlp: $(help)\pcbcp.ipf $(help)\dlg.ipf $(help)\edit.ipf $(help)\file.ipf $(help)\help.ipf $(help)\menu.ipf $(help)\action.ipf $(help)\option.ipf
         cd help
         $(ipfcomp) pcbcp.ipf
         cd ..

$(obj)\main.res: $(source)\main.rc $(source)\help.rc
         $(brcc) -r -i$(includepath) -fo $(obj)\main.res $(source)\main.rc

user.obj: user.c

thrd.obj: thrd.c

pnt.obj:  pnt.c

main.obj: main.c

help.obj: help.c

init.obj: init.c

file.obj: file.c

dlg.obj: dlg.c

#=============================================================
#               compiler configuration file
#=============================================================
$(cfg): $(mak)
  copy &&|
-rt-
-xd-
-x-
-oz
-ob
-oe
-oc
-dbackground_thread
-l$(libpath)
-i$(includepath)
-n$(obj)
-p
-vi
-sm
-d
-k-
-o
-ot
-w
-c
-k
-a4
-5
$(debug)
| $(cfg)


