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
#       pcboard.mak - makefile for pcboard-for-dos
#
#=============================================================
#
#  note:  to use this makefile, you must first execute
#         bcdos.bat which sets up several environment
#         variables which are used to locate things such as
#         the compiler, the header files and the output
#         location.
#
#=============================================================

.nosilent
.autodepend

version = 153
!ifndef root
root     = \out
!endif
proj    = $(root)\main
libdir  = $(root)\lib\bcdos\$(bccompiler)
obj     = obj\$(bccompiler)

cfg     = $(version)\pcboard.cfg
mak     = $(version)\pcboard.mak


!if $d(commdrv)
includepath = $(include);source\h;..\lib\h;$(libsdir)\commdrv\h
!else
includepath = $(include);source\h;..\lib\h
!endif


mdl = l                               # memory model

asm     = source\asm
main    = source\main
dos     = source\dos
display = source\display
modem   = source\modem
msg     = source\msg
node    = source\node
fidodir = source\fido
ppl     = source\ppl
support = source\support
users   = source\users

!if $d(stats)
stats = pcbstats
!else
stats =
!endif

!if $d(debug)
#gen = debug -ddebugscr
gen = debug
!else
gen = ndebug
!endif


!if $d(comm)
com  = comm
!if $d(fido)
deffido = fido
!else
deffido =
!endif
!else
com  = localonly
deffido =
!endif


!if $d(mp)
drv = multiport
!else
drv =
!endif

!if !$d(debug) && !$(td)
copt = -c -p                                    # tcc
asmopt = /m3 /mx /t /d__$(mdl)__ /d$(gen)       # tasm
linkopt = /c /x                                 # tlink
!elif $d(td)
copt = -c -v -p -n                              # tcc
asmopt = /m3 /mx /t /zi /d__$(mdl)__ /d$(gen)   # tasm
linkopt = /c /x /v                              # tlink  (/x=no map /s=detailed map)
!else
copt = -c -p -n                                 # tcc
asmopt = /m3 /mx /t /d__$(mdl)__ /d$(gen)       # tasm
linkopt = /c /x                                 # tlink
!endif
asmopt = $(asmopt) /isource\h

!if $d(bc31) || $d(bc50)
linkopt = $(linkopt) /p                         # tlink 3.1 & 5.0 (not tc 3.0)
!endif

!if $d(bc50)
copt = $(copt) -oglmptv -x- -v -vmp -vmd -ff # bcc v4.x & v5.0 (leave out -oe because of bug in bcc)
!elif $d(bc31)
copt = $(copt) -od -v -vmp -vmd -ff # bcc v3.x
!elif $d(tc30)
copt = $(copt) # tcc v3.0
!endif

!if $d(kbd3)
copt = $(copt) -dkbd3
asmopt = $(asmopt) /dkbd3
!endif

!if $d(hdr)
copt   = $(copt) -h=$(obj)\project.sym
!endif

!if $d(beta)
gen = $(gen) -dbeta
!endif

!if $d(demo)
gen = $(gen) -dpcb_demo
!endif

!if $d(386) && ! $d(tc30)    # tc 3.0 does not support 386 compilation
copt   = $(copt) -3 /dcpu386
asmopt = $(asmopt) /dcpu386
ext    = 386
libext = 386
res    = pcb_$(bccompiler).386
!else
ext    = res
libext = lib
res    = pcb_$(bccompiler).res
!endif

libs = $(version)\libs$(bccompiler).$(ext)

#source code generation options
codeopt = -d$(gen) -d$(com) -d$(stats) -d$(numnodes) -d$(drv) -d$(deffido) -dkey=key_$(nodes)

!if $d(dbase)
includepath = $(includepath);$(libsdir)\codebase\source
optlib      = /o $(libsdir)\codebase\bor31\c4base.lib+
dbaseopt    = -ddbase
!else
optlib      =
dbaseopt    =
!endif

versionobj  = version\$(bccompiler).obj

##############################################################################


$(obj)\pcboardm.exe: $(cfg) $(libs) step1.$$$ step2.$$$
        if exist step1.$$$ del step1.$$$
        if exist step2.$$$ del step2.$$$
        $(linker) $(linkopt) $(obj)\c0$(mdl) @153\$(res),$(obj)\pcboardm,$(obj)\pcboardm,@$(libs)
#       $(linker) $(linkopt) $(libpath)\c0$(mdl) $(versionobj) @153\$(res),$(obj)\pcboard,$(obj)\pcboard,@$(libs)


##############################################################################


step1.$$$: \
  $(obj)\pcboard.obj \
  $(obj)\callwait.obj \
  $(obj)\capture.obj \
  $(obj)\chat.obj \
  $(obj)\command.obj \
  $(obj)\crc32.obj \
  $(obj)\display.obj \
  $(obj)\diz.obj \
  $(obj)\doors.obj \
  $(obj)\event.obj \
  $(obj)\files.obj \
  $(obj)\help.obj \
  $(obj)\filelist.obj \
  $(obj)\index.obj \
  $(obj)\init.obj \
  $(obj)\inkey.obj \
  $(obj)\input.obj \
  $(obj)\md5impl.obj \
  $(obj)\log.obj \
  $(obj)\logview.obj \
  $(obj)\login.obj \
  $(obj)\msgbase.obj \
  $(obj)\messages.obj \
  $(obj)\misc.obj \
  $(obj)\modem.obj \
  $(obj)\modemasy.obj \
  $(obj)\modemfos.obj \
  $(obj)\modemos2.obj \
  $(obj)\modemdrv.obj \
  $(obj)\msgenter.obj \
  $(obj)\msgread.obj \
  $(obj)\msgscan.obj \
  $(obj)\newchat.obj \
  $(obj)\node.obj \
  $(obj)\nonovrly.obj \
  $(obj)\cmds.obj \
  $(obj)\recycle.obj \
  $(obj)\script.obj \
  $(obj)\shell.obj \
  $(obj)\stats.obj \
  $(obj)\status.obj \
  $(obj)\sys.obj \
  $(obj)\ticdelay.obj \
  $(obj)\transfer.obj \
  $(obj)\usernet.obj \
  $(obj)\users.obj \
  $(obj)\userstat.obj \
  $(obj)\xlate.obj \
  $(obj)\account.obj \
  $(obj)\blt.obj \
  $(obj)\confrnce.obj \
  $(obj)\copyfile.obj \
  $(obj)\dir.obj \
  $(obj)\dlpath.obj \
  $(obj)\language.obj \
  $(obj)\memory.obj \
  $(obj)\overlay.obj \
  $(obj)\pcbtext.obj \
  $(obj)\screen.obj \
  $(obj)\scrlback.obj \
  $(obj)\settings.obj \

  if exist bcc.res echo $(copt) > copt.res
  if exist bcc.res echo -m$(mdl) $(codeopt) > input.res
  if exist bcc.res echo done > step1.$$$
  if exist bcc.res $(compiler) +$(cfg) -d___exec___ @copt.res @input.res @bcc.res
# if exist input.res del input.res
# if exist bcc.res del bcc.res
# if exist copt.res del copt.res
  if exist $(obj)\pcboardm.exe del $(obj)\pcboardm.exe


step2.$$$:  \
  $(obj)\dosopen.obj \
  $(obj)\dosclose.obj \
  $(obj)\dosread.obj \
  $(obj)\doswrite.obj \
  $(obj)\dostime.obj \
  $(obj)\ansi.obj \
  $(obj)\async.obj \
  $(obj)\bgkey.obj \
  $(obj)\cutil.obj \
  $(obj)\memmove.obj \
  $(obj)\noscroll.obj \
  $(obj)\timer.obj \
  $(obj)\filestub.obj \
  $(obj)\label.obj \
  $(obj)\pcbmisc.obj \
  $(obj)\screxec.obj \
  $(obj)\evalp.obj \
  $(obj)\execdb.obj \
  $(obj)\newscr.obj \
  $(obj)\scrmisc.obj \
  $(obj)\var.obj \
  $(obj)\menu.obj \
  $(obj)\lrand.obj \
  $(obj)\dbase.obj \
  $(obj)\c0$(mdl).obj \
  $(obj)\envfix.obj \
  $(obj)\devioctl.obj \
  $(obj)\ratio.obj \
  $(obj)\recemsi.obj \
  $(obj)\fconfig.obj \
  $(obj)\hex.obj \
  $(obj)\crc-16f.obj \
  $(obj)\xmitemsi.obj \
  $(obj)\data.obj \
  $(obj)\dupechec.obj \
  $(obj)\passthru.obj \
  $(obj)\fidofunc.obj \
  $(obj)\fidomenu.obj \
  $(obj)\fidomisc.obj \
  $(obj)\fidoque.obj \
  $(obj)\fidomsg.obj \
  $(obj)\seenby.obj \
  $(obj)\pcbtoss.obj \
  $(obj)\tossmisc.obj \
  $(obj)\recwazoo.obj \
  $(obj)\umwf.obj \
  $(obj)\pcbmsgs.obj \
  $(obj)\msgstub.obj \
  $(obj)\showerr.obj \
  $(obj)\token.obj \
  $(obj)\userscan.obj \
  $(obj)\usersys.obj \
  $(obj)\usrmaint.obj \
  $(libdir)\system_$(mdl).$(libext) \
  $(libdir)\country$(mdl).$(libext) \
  $(libdir)\dos_$(mdl).$(libext) \
  $(libdir)\pcb_$(mdl).$(libext) \
  $(libdir)\screen_$(mdl).$(libext) \
  $(libdir)\misc_$(mdl).$(libext)

  if exist bcc.res echo -m$(mdl) $(codeopt) > input.res
  if exist bcc.res echo $(copt) > copt.res
  if exist bcc.res echo done > step2.$$$
  if exist bcc.res $(compiler) +$(cfg) -d___exec___ @copt.res @input.res @bcc.res
# if exist input.res del input.res
# if exist bcc.res del bcc.res
# if exist copt.res del copt.res
  if exist $(obj)\pcboardm.exe del $(obj)\pcboardm.exe


##############################################################################


$(obj)\account.obj:  $(support)\account.cpp
  echo $(support)\$&.cpp >> bcc.res

$(obj)\blt.obj:      $(display)\blt.c
  echo $(display)\$&.c >> bcc.res

$(obj)\callwait.obj: $(main)\callwait.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\capture.obj:  $(support)\capture.c
  echo $(support)\$&.c >> bcc.res

$(obj)\chat.obj:     $(main)\chat.c
  echo $(main)\$&.c >> bcc.res

$(obj)\cmds.obj:     $(main)\cmds.c
  echo $(main)\$&.c >> bcc.res

$(obj)\command.obj:  $(main)\command.c \
                     $(proj)\comm.chk $(proj)\nodes.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\crc32.obj:    $(support)\crc32.c
  echo $(support)\$&.c >> bcc.res

$(obj)\confrnce.obj: $(main)\confrnce.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\copyfile.obj: $(support)\copyfile.c
  echo $(support)\$&.c >> bcc.res

$(obj)\devioctl.obj: $(modem)\devioctl.c
  echo $(modem)\$&.c >> bcc.res

$(obj)\dir.obj:      $(display)\dir.c
  echo $(display)\$&.c >> bcc.res

$(obj)\display.obj:  $(display)\display.c \
                     $(proj)\comm.chk
  echo $(display)\$&.c >> bcc.res

$(obj)\diz.obj:      $(support)\diz.c
  echo $(support)\$&.c >> bcc.res

$(obj)\dlpath.obj:   $(main)\dlpath.c
  echo $(main)\$&.c >> bcc.res

$(obj)\doors.obj:    $(main)\doors.c
  echo $(main)\$&.c >> bcc.res

$(obj)\event.obj:    $(main)\event.c \
                     $(proj)\comm.chk $(proj)\nodes.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\filelist.obj: $(main)\filelist.c
  echo $(main)\$&.c >> bcc.res

$(obj)\files.obj:    $(display)\files.c \
                     $(proj)\comm.chk
  echo $(display)\$&.c >> bcc.res

$(obj)\help.obj:     $(display)\help.c
  echo $(display)\$&.c >> bcc.res

$(obj)\index.obj:    $(main)\index.c
  echo $(main)\$&.c >> bcc.res

$(obj)\init.obj:     $(main)\init.c \
                     $(proj)\comm.chk $(proj)\nodes.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\inkey.obj:    $(main)\inkey.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\input.obj:    $(main)\input.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\md5impl.obj:  $(main)\md5impl.cpp
  echo $(main)\md5impl.cpp >> bcc.res

$(obj)\language.obj: $(main)\language.c
  echo $(main)\$&.c >> bcc.res

$(obj)\log.obj:      $(node)\log.c \
                     $(proj)\nodes.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\login.obj:    $(node)\login.c \
                     $(proj)\comm.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\logview.obj:  $(node)\logview.c \
                     $(proj)\nodes.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\memory.obj:   $(support)\memory.c
  echo $(support)\$&.c >> bcc.res

$(obj)\msgbase.obj:  $(msg)\msgbase.c
  echo $(msg)\$&.c >> bcc.res

$(obj)\messages.obj: $(msg)\messages.c
  echo $(msg)\$&.c >> bcc.res

$(obj)\misc.obj:     $(main)\misc.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\modem.obj:    $(modem)\modem.c \
                     $(proj)\comm.chk
  echo $(modem)\$&.c >> bcc.res

$(obj)\modemasy.obj: $(modem)\modemasy.c
  echo $(modem)\$&.c >> bcc.res

$(obj)\modemfos.obj: $(modem)\modemfos.c
  echo $(modem)\$&.c >> bcc.res

$(obj)\modemos2.obj: $(modem)\modemos2.c
  echo $(modem)\$&.c >> bcc.res

$(obj)\modemdrv.obj: $(modem)\modemdrv.c
  echo $(modem)\$&.c >> bcc.res

$(obj)\msgenter.obj: $(msg)\msgenter.c \
                     $(proj)\comm.chk
  echo $(msg)\$&.c >> bcc.res

$(obj)\msgread.obj:  $(msg)\msgread.c \
                     $(proj)\comm.chk
  echo $(msg)\$&.c >> bcc.res

$(obj)\msgscan.obj:  $(msg)\msgscan.c \
                     $(proj)\comm.chk
  echo $(msg)\$&.c >> bcc.res

$(obj)\newchat.obj:  $(node)\newchat.c \
                     $(proj)\comm.chk $(proj)\nodes.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\node.obj:     $(node)\node.c \
                     $(proj)\nodes.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\nonovrly.obj: $(support)\nonovrly.c
  echo $(support)\$&.c >> bcc.res

$(obj)\overlay.obj:  $(support)\overlay.c
  echo $(support)\$&.c >> bcc.res

$(obj)\pcboard.obj:  $(main)\pcboard.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\pcbtext.obj:  $(display)\pcbtext.c \
                     $(proj)\comm.chk
  echo $(display)\$&.c >> bcc.res

$(obj)\recycle.obj:  $(main)\recycle.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\screen.obj  : $(display)\screen.c
  echo $(display)\$&.c >> bcc.res

$(obj)\script.obj  : $(main)\script.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\scrlback.obj: $(display)\scrlback.c
  echo $(display)\$&.c >> bcc.res

$(obj)\settings.obj: $(users)\settings.c
  echo $(users)\$&.c >> bcc.res

$(obj)\shell.obj:    $(main)\shell.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\showerr.obj:  $(support)\showerr.c
  echo $(support)\$&.c >> bcc.res

$(obj)\stats.obj:    $(node)\stats.c \
                     $(proj)\nodes.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\status.obj:   $(display)\status.c \
                     $(proj)\comm.chk
  echo $(display)\$&.c >> bcc.res

$(obj)\sys.obj:      $(main)\sys.c \
                     $(proj)\nodes.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\token.obj:    $(support)\token.c
  echo $(support)\$&.c >> bcc.res

$(obj)\ticdelay.obj: $(modem)\ticdelay.c \
                     $(proj)\comm.chk
  echo $(modem)\$&.c >> bcc.res

$(obj)\transfer.obj: $(main)\transfer.c \
                     $(proj)\comm.chk
  echo $(main)\$&.c >> bcc.res

$(obj)\usernet.obj:  $(node)\usernet.c \
                     $(proj)\nodes.chk
  echo $(node)\$&.c >> bcc.res

$(obj)\users.obj:    $(users)\users.c
  echo $(users)\$&.c >> bcc.res

$(obj)\userscan.obj: $(users)\userscan.c
  echo $(users)\$&.c >> bcc.res

$(obj)\userstat.obj: $(users)\userstat.c
  echo $(users)\$&.c >> bcc.res

$(obj)\usersys.obj:  $(users)\usersys.c
  echo $(users)\$&.c >> bcc.res

$(obj)\usrmaint.obj: $(users)\usrmaint.c \
                     $(proj)\comm.chk
  echo $(users)\$&.c >> bcc.res

# $(obj)\version.obj:  $(support)\version.c \
#                      $(proj)\nodes.chk
#   echo $(support)\$&.c >> bcc.res

$(obj)\xlate.obj:    $(display)\xlate.c
  echo $(display)\$&.c >> bcc.res

##############################################################################

$(obj)\c0$(mdl).obj: $(asm)\c0.asm
  $(tasm) $(asmopt) $(asm)\c0.asm, $(obj)\c0$(mdl).obj

$(obj)\dosopen.obj:  $(dos)\dosopen.c
  echo $(dos)\$&.c >> bcc.res

$(obj)\dosclose.obj: $(dos)\dosclose.c
  echo $(dos)\$&.c >> bcc.res

$(obj)\dosread.obj:  $(dos)\dosread.c
  echo $(dos)\$&.c >> bcc.res

$(obj)\doswrite.obj: $(dos)\doswrite.c
  echo $(dos)\$&.c >> bcc.res

$(obj)\dostime.obj:  $(dos)\dostime.c
  echo $(dos)\$&.c >> bcc.res

$(obj)\ansi.obj:     $(asm)\ansi.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

$(obj)\async.obj:    $(asm)\async.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

$(obj)\bgkey.obj:    $(asm)\bgkey.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

$(obj)\cutil.obj:    $(asm)\cutil.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

$(obj)\memmove.obj:  $(asm)\memmove.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

$(obj)\noscroll.obj: $(asm)\noscroll.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

$(obj)\timer.obj:    $(asm)\timer.asm
  $(tasm) $(asmopt) $(asm)\$&.asm, $(obj)\$&.obj

##############################################################################

$(obj)\filestub.obj: $(ppl)\filestub.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\label.obj:    $(ppl)\label.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\pcbmisc.obj:  $(ppl)\pcbmisc.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\dbase.obj:    $(ppl)\dbase.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\screxec.obj:  $(ppl)\screxec.cpp \
                     $(proj)\comm.chk
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\evalp.obj:    $(ppl)\evalp.cpp \
                     $(proj)\comm.chk $(proj)\nodes.chk
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\execdb.obj:   $(ppl)\execdb.cpp \
                     $(proj)\comm.chk
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\newscr.obj:   $(ppl)\newscr.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\scrmisc.obj:  $(ppl)\scrmisc.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\var.obj:      $(ppl)\var.cpp \
                     $(proj)\comm.chk
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\menu.obj:     $(ppl)\menu.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\lrand.obj:    $(ppl)\lrand.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\ratio.obj:    $(ppl)\ratio.cpp
  echo $(ppl)\$&.cpp >> bcc.res

## $(obj)\qint.obj:     $(ppl)\qint.cpp
##   echo $(ppl)\$&.cpp >> bcc.res

$(obj)\pcbmsgs.obj:  $(ppl)\pcbmsgs.cpp
  echo $(ppl)\$&.cpp >> bcc.res

$(obj)\msgstub.obj:  $(ppl)\msgstub.cpp
  echo $(ppl)\$&.cpp >> bcc.res

##############################################################################

$(obj)\recemsi.obj:  $(fidodir)\recemsi.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\fconfig.obj:  $(fidodir)\fconfig.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\xmitemsi.obj: $(fidodir)\xmitemsi.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\fidomenu.obj: $(fidodir)\fidomenu.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\fidomisc.obj: $(fidodir)\fidomisc.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\pcbtoss.obj:  $(fidodir)\pcbtoss.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\tossmisc.obj: $(fidodir)\tossmisc.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\seenby.obj:   $(fidodir)\seenby.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\recwazoo.obj: $(fidodir)\recwazoo.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\fidofunc.obj: $(fidodir)\fidofunc.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\fidomsg.obj:  $(fidodir)\fidomsg.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\umwf.obj:     $(fidodir)\umwf.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\data.obj:     $(fidodir)\data.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\dupechec.obj: $(fidodir)\dupechec.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\passthru.obj: $(fidodir)\passthru.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\fidoque.obj:  $(fidodir)\fidoque.cpp
  echo $(fidodir)\$&.cpp >> bcc.res

$(obj)\hex.obj:      $(fidodir)\hex.c
  echo $(fidodir)\$&.c >> bcc.res

$(obj)\crc-16f.obj:  $(fidodir)\crc-16f.c
  echo $(fidodir)\$&.c >> bcc.res


##############################################################################

$(obj)\envfix.obj:  $(support)\envfix.c
  echo $(support)\$&.c >> bcc.res

##############################################################################


#============================================================
#              compiler configuration file
#============================================================
$(cfg): $(mak)
  copy &&|
-w-eas
-wbbf
-wbig
-wdpu
-wdup
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
-ff
-f
-k
-y
-d
-n$(obj)
-i$(includepath)
-l$(libpath)
-dpcboard
-dpcbcomm
-dosdriver
-dfossil
-dbigndx
-d___use_var___
-ds4error_hook
-dndebug
-dpcb152
-dpcb153
-dcrypt
$(dbaseopt)
-dmg
-dtossclass
| $(cfg)

#============================================================
#              libs response file
#============================================================

!if $d(commdrv)

$(libs): $(mak)
  copy &&|
/o- $(libdir)\countryl.$(libext)+
/o- $(libdir)\dos_l.$(libext)+
/o- $(libdir)\screen_l.$(libext)+
/o- $(libdir)\pcb_l.$(libext)+
/o- $(libdir)\misc_l.$(libext)+
/o- $(libdir)\system_l.$(libext)+
/o- $(libdir)\\doscls_l.$(libext)+
/o+ $(libsdir)\commdrv\lib\commdrbl.lib+
/o+ $(libsdir)\commdrv\lib\libsbl.lib+
/o- $(libpath)\emu.lib+
/o- $(libpath)\mathl.lib+
/o- $(libpath)\overlay.lib+
/o- $(libpath)\cl.lib+ $(optlib)
| $(libs)

!else

$(libs): $(mak)
  copy &&|
/o- $(libdir)\countryl.$(libext)+
/o- $(libdir)\dos_l.$(libext)+
/o- $(libdir)\screen_l.$(libext)+
/o- $(libdir)\pcb_l.$(libext)+
/o- $(libdir)\misc_l.$(libext)+
/o- $(libdir)\system_l.$(libext)+
/o- $(libdir)\\doscls_l.$(libext)+
/o- $(libpath)\emu.lib+
/o- $(libpath)\mathl.lib+
/o- $(libpath)\overlay.lib+
/o- $(libpath)\cl.lib+ $(optlib)
| $(libs)

!endif



