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
#       pcboard2.mak - makefile for pcboard-for-os/2
#
#=============================================================
#
#  note:  to use this makefile, you must first execute
#         \proj\bcos2.cmd which sets up several environment
#         variables which are used to locate things such as
#         the compiler, the header files and the output
#         location.
#
#=============================================================

.silent
.autodepend
.cacheautodepend

version = 153
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
!ifndef bcroot
bcroot   = \bcos2
!endif
proj    = $(src)
libroot = $(tkit)
libdir  = $(outdir)\sdk\bcos2\lib
source  = $(proj)\source
obj     = obj\bcos2

cfg     = $(proj)\$(version)\pcboard2.cfg
mak     = $(proj)\$(version)\pcboard2.mak

.path.obj = $(obj)

opt = -d$(numnodes)
!if $d(beta)
  opt = $(opt) -dbeta
!endif

!if $d(debug)
# opt = $(opt) -ddebug -ddebug2
# opt = $(opt) -ddebug
  opt = $(opt) -ddebug -debugscr
!endif

!if $d(td)
# opt = $(opt) -n
  dbg = /v
!endif

cc = $(compiler) +$(cfg) $(opt) $(dbg) -dkey=key_$(nodes)
includepath = $(include);$(source)\h;$(libroot)\h;$(libsdir)\codebase\source


#=============================================================
#               list macros
#=============================================================


exe_dependencies =  \
 $(libsdir)\codebase\os2bor1\b4.lib \
 $(libdir)\pcb.lib \
 $(libdir)\country.lib \
 $(libdir)\misc.lib \
 $(libdir)\dos.lib \
 $(libdir)\doscls.lib \
 $(libdir)\screen.lib \
 $(libdir)\system.lib \
 umwf.obj \
 seenby.obj \
 pcbtoss.obj \
 passthru.obj \
 fidoque.obj \
 fidomsg.obj \
 fidomisc.obj \
 data.obj \
 dupechec.obj \
 hex.obj \
 xmitemsi.obj \
 tossmisc.obj \
 recwazoo.obj \
 recemsi.obj \
 fidomenu.obj \
 fidofunc.obj \
 fconfig.obj \
 crc-16f.obj \
 var.obj \
 scrmisc.obj \
 msgstub.obj \
 menu.obj \
 lrand.obj \
 label.obj \
 filestub.obj \
 execdb.obj \
 evalp.obj \
 dbase.obj \
 screxec.obj \
 pcbmisc.obj \
 pcbmsgs.obj \
 newscr.obj \
 xmodem.obj \
 xlate.obj \
 transfer.obj \
 token.obj \
 ticdelay.obj \
 wild.obj \
 usrmaint.obj \
 usersys.obj \
 userstat.obj \
 userscan.obj \
 users.obj \
 usernet.obj \
 sys.obj \
 status.obj \
 stats.obj \
 showerr.obj \
 shell.obj \
 settings.obj \
 scrlback.obj \
 script.obj \
 screen.obj \
 recycle.obj \
 ratio.obj \
 pcbtext.obj \
 pcboard.obj \
 pcbmacro.obj \
 node.obj \
 newchat.obj \
 msgscan.obj \
 msgread.obj \
 msgenter.obj \
 msgbase.obj \
 modemos2.obj \
 modem.obj \
 misc.obj \
 messages.obj \
 memory.obj \
 logview.obj \
 login.obj \
 log.obj \
 language.obj \
 input.obj \
 inkey.obj \
 init.obj \
 index.obj \
 help.obj \
 files.obj \
 filelist.obj \
 event.obj \
 envfix.obj \
 dostime.obj \
 doswrite.obj \
 dosread.obj \
 dosopen.obj \
 dosclose.obj \
 doors.obj \
 dlpath.obj \
 diz.obj \
 display.obj \
 dir.obj \
 devioctl.obj \
 crc32.obj \
 copyfile.obj \
 confrnce.obj \
 command.obj \
 cmds.obj \
 chat.obj \
 capture.obj \
 callwait.obj \
 blt.obj \
 ansi.obj \
 account.obj \
 $(src)\source\misc\md5\os2\md5.obj

#=============================================================
#               explicit rules
#=============================================================
#
#  note:  a stack size of 40960 causes an error to occur when saving a
#         message with a file attachment, but 36864 does not, nor does
#         it occur when using a setting of 45056.
#
#  on 8/30/95, changed from 45056 to 49152.

$(obj)\pcboard2.exe: $(cfg) $(exe_dependencies) $(obj)\linker.res
   $(linker) $(dbg) /b:0x10000 /s:49152 /x /toe /ap /oc /l$(libpath) @&&|
$(bcroot)\lib\c02.obj+
$(obj)\pcboard.obj+
$(obj)\init.obj+
version\bcos2.obj+
$(obj)\callwait.obj+
$(obj)\inkey.obj+
$(obj)\devioctl.obj+
$(obj)\modemos2.obj+
$(obj)\modem.obj+
$(obj)\input.obj+
$(obj)\memory.obj+
$(obj)\ansi.obj+
$(obj)\display.obj+
$(obj)\xlate.obj+
$(obj)\pcbmacro.obj+
$(obj)\pcbtext.obj+
$(obj)\files.obj+
$(obj)\log.obj+
$(obj)\dostime.obj+
$(obj)\doswrite.obj+
$(obj)\dosread.obj+
$(obj)\dosopen.obj+
$(obj)\dosclose.obj+
$(obj)\usernet.obj+
$(obj)\msgbase.obj+
$(obj)\messages.obj+
$(obj)\msgscan.obj+
$(obj)\msgread.obj+
$(obj)\msgenter.obj+
$(obj)\umwf.obj+
$(obj)\seenby.obj+
$(obj)\pcbtoss.obj+
$(obj)\passthru.obj+
$(obj)\fidoque.obj+
$(obj)\fidomsg.obj+
$(obj)\fidomisc.obj+
$(obj)\data.obj+
$(obj)\dupechec.obj+
$(obj)\hex.obj+
$(obj)\xmitemsi.obj+
$(obj)\tossmisc.obj+
$(obj)\recwazoo.obj+
$(obj)\recemsi.obj+
$(obj)\fidomenu.obj+
$(obj)\fidofunc.obj+
$(obj)\fconfig.obj+
$(obj)\crc-16f.obj+
$(obj)\newscr.obj+
$(obj)\var.obj+
$(obj)\scrmisc.obj+
$(obj)\evalp.obj+
$(obj)\screxec.obj+
$(obj)\pcbmisc.obj+
$(obj)\execdb.obj+
$(obj)\dbase.obj+
$(obj)\pcbmsgs.obj+
$(obj)\msgstub.obj+
$(obj)\menu.obj+
$(obj)\lrand.obj+
$(obj)\label.obj+
$(obj)\filestub.obj+
$(obj)\xmodem.obj+
$(obj)\wild.obj+
$(obj)\usrmaint.obj+
$(obj)\usersys.obj+
$(obj)\userstat.obj+
$(obj)\userscan.obj+
$(obj)\users.obj+
$(obj)\index.obj+
$(obj)\token.obj+
$(obj)\ticdelay.obj+
$(obj)\sys.obj+
$(obj)\status.obj+
$(obj)\stats.obj+
$(obj)\showerr.obj+
$(obj)\settings.obj+
$(obj)\scrlback.obj+
$(obj)\script.obj+
$(obj)\screen.obj+
$(obj)\recycle.obj+
$(obj)\ratio.obj+
$(obj)\node.obj+
$(obj)\newchat.obj+
$(obj)\misc.obj+
$(obj)\logview.obj+
$(obj)\login.obj+
$(obj)\language.obj+
$(obj)\help.obj+
$(obj)\event.obj+
$(obj)\shell.obj+
$(obj)\doors.obj+
$(obj)\transfer.obj+
$(obj)\dlpath.obj+
$(obj)\filelist.obj+
$(obj)\copyfile.obj+
$(obj)\dir.obj+
$(obj)\diz.obj+
$(obj)\crc32.obj+
$(obj)\confrnce.obj+
$(obj)\command.obj+
$(obj)\cmds.obj+
$(obj)\chat.obj+
$(obj)\capture.obj+
$(obj)\blt.obj+
$(obj)\account.obj+
$(src)\source\misc\md5\os2\md5.obj
$(obj)\pcboard2
                # no map file
$(libsdir)\codebase\os2bor1\b4.lib+
$(libdir)\pcb.lib+
$(libdir)\country.lib+
$(libdir)\misc.lib+
$(libdir)\dos.lib+
$(libdir)\screen.lib+
$(libdir)\system.lib+
$(libdir)\doscls.lib+
$(libpath)\c2mt.lib+
$(libpath)\os2.lib
$(proj)\$(version)\pcboard2.def
|

  $(src)\153\lxbfix $(obj)\pcboard2.exe
  if exist $(obj)\linker.res del $(obj)\linker.res


#==============================================================================
# the following lines are used to group all of the modules that need to be
# compiled into a single input.res file and then launch the compiler just once
# instead of launching the compiler once per module
#==============================================================================

$(obj)\linker.res: $(obj)\compiler.res
  if exist $(obj)\compiler.res $(cc) -c @$(obj)\compiler.res
  if exist $(obj)\compiler.res del $(obj)\compiler.res
  echo need to link > $(obj)\linker.res


#=============================================================
#               individual file dependencies
#=============================================================
umwf.obj: $(source)\fido\umwf.cpp
        echo  $(source)\fido\umwf.cpp >> $(obj)\compiler.res

seenby.obj: $(source)\fido\seenby.cpp
        echo  $(source)\fido\seenby.cpp >> $(obj)\compiler.res

pcbtoss.obj: $(source)\fido\pcbtoss.cpp
        echo  $(source)\fido\pcbtoss.cpp >> $(obj)\compiler.res

passthru.obj: $(source)\fido\passthru.cpp
        echo  $(source)\fido\passthru.cpp >> $(obj)\compiler.res

fidoque.obj: $(source)\fido\fidoque.cpp
        echo  $(source)\fido\fidoque.cpp >> $(obj)\compiler.res

fidomsg.obj: $(source)\fido\fidomsg.cpp
        echo  $(source)\fido\fidomsg.cpp >> $(obj)\compiler.res

fidomisc.obj: $(source)\fido\fidomisc.cpp
        echo  $(source)\fido\fidomisc.cpp >> $(obj)\compiler.res

data.obj: $(source)\fido\data.cpp
        echo  $(source)\fido\data.cpp >> $(obj)\compiler.res

dupechec.obj: $(source)\fido\dupechec.cpp
        echo  $(source)\fido\dupechec.cpp >> $(obj)\compiler.res

hex.obj: $(source)\fido\hex.c
        echo  $(source)\fido\hex.c >> $(obj)\compiler.res

xmitemsi.obj: $(source)\fido\xmitemsi.c
        echo  $(source)\fido\xmitemsi.c >> $(obj)\compiler.res

tossmisc.obj: $(source)\fido\tossmisc.c
        echo  $(source)\fido\tossmisc.c >> $(obj)\compiler.res

recwazoo.obj: $(source)\fido\recwazoo.c
        echo  $(source)\fido\recwazoo.c >> $(obj)\compiler.res

recemsi.obj: $(source)\fido\recemsi.c
        echo  $(source)\fido\recemsi.c >> $(obj)\compiler.res

fidomenu.obj: $(source)\fido\fidomenu.c
        echo  $(source)\fido\fidomenu.c >> $(obj)\compiler.res

fidofunc.obj: $(source)\fido\fidofunc.c
        echo  $(source)\fido\fidofunc.c >> $(obj)\compiler.res

fconfig.obj: $(source)\fido\fconfig.c
        echo  $(source)\fido\fconfig.c >> $(obj)\compiler.res

crc-16f.obj: $(source)\fido\crc-16f.c
        echo  $(source)\fido\crc-16f.c >> $(obj)\compiler.res

var.obj: $(source)\ppl\var.cpp
        echo  $(source)\ppl\var.cpp >> $(obj)\compiler.res

scrmisc.obj: $(source)\ppl\scrmisc.cpp
        echo  $(source)\ppl\scrmisc.cpp >> $(obj)\compiler.res

msgstub.obj: $(source)\ppl\msgstub.cpp
        echo  $(source)\ppl\msgstub.cpp >> $(obj)\compiler.res

menu.obj: $(source)\ppl\menu.cpp
        echo  $(source)\ppl\menu.cpp >> $(obj)\compiler.res

lrand.obj: $(source)\ppl\lrand.cpp
        echo  $(source)\ppl\lrand.cpp >> $(obj)\compiler.res

label.obj: $(source)\ppl\label.cpp
        echo  $(source)\ppl\label.cpp >> $(obj)\compiler.res

filestub.obj: $(source)\ppl\filestub.cpp
        echo  $(source)\ppl\filestub.cpp >> $(obj)\compiler.res

execdb.obj: $(source)\ppl\execdb.cpp
        echo  $(source)\ppl\execdb.cpp >> $(obj)\compiler.res

evalp.obj: $(source)\ppl\evalp.cpp $(proj)\nodes.chk
        echo  $(source)\ppl\evalp.cpp >> $(obj)\compiler.res

dbase.obj: $(source)\ppl\dbase.cpp
        echo  $(source)\ppl\dbase.cpp >> $(obj)\compiler.res

screxec.obj: $(source)\ppl\screxec.cpp
        echo  $(source)\ppl\screxec.cpp >> $(obj)\compiler.res

pcbmisc.obj: $(source)\ppl\pcbmisc.cpp
        echo  $(source)\ppl\pcbmisc.cpp >> $(obj)\compiler.res

pcbmsgs.obj: $(source)\ppl\pcbmsgs.cpp
        echo  $(source)\ppl\pcbmsgs.cpp >> $(obj)\compiler.res

newscr.obj: $(source)\ppl\newscr.cpp
        echo  $(source)\ppl\newscr.cpp >> $(obj)\compiler.res

xmodem.obj: $(source)\support\xmodem.c
        echo  $(source)\support\xmodem.c >> $(obj)\compiler.res

xlate.obj: $(source)\display\xlate.c
        echo  $(source)\display\xlate.c >> $(obj)\compiler.res

wild.obj: $(source)\support\wild.c
        echo  $(source)\support\wild.c >> $(obj)\compiler.res

# version.obj: $(source)\support\version.c $(proj)\nodes.chk
#         echo  $(source)\support\version.c >> $(obj)\compiler.res

usrmaint.obj: $(source)\users\usrmaint.c
        echo  $(source)\users\usrmaint.c >> $(obj)\compiler.res

usersys.obj: $(source)\users\usersys.c
        echo  $(source)\users\usersys.c >> $(obj)\compiler.res

userstat.obj: $(source)\users\userstat.c
        echo  $(source)\users\userstat.c >> $(obj)\compiler.res

userscan.obj: $(source)\users\userscan.c
        echo  $(source)\users\userscan.c >> $(obj)\compiler.res

users.obj: $(source)\users\users.c
        echo  $(source)\users\users.c >> $(obj)\compiler.res

usernet.obj: $(source)\node\usernet.c $(proj)\nodes.chk
        echo  $(source)\node\usernet.c >> $(obj)\compiler.res

transfer.obj: $(source)\main\transfer.c
        echo  $(source)\main\transfer.c >> $(obj)\compiler.res

token.obj: $(source)\support\token.c
        echo  $(source)\support\token.c >> $(obj)\compiler.res

ticdelay.obj: $(source)\modem\ticdelay.c
        echo  $(source)\modem\ticdelay.c >> $(obj)\compiler.res

sys.obj: $(source)\main\sys.c
        echo  $(source)\main\sys.c >> $(obj)\compiler.res

status.obj: $(source)\display\status.c
        echo  $(source)\display\status.c >> $(obj)\compiler.res

stats.obj: $(source)\node\stats.c $(proj)\nodes.chk
        echo  $(source)\node\stats.c >> $(obj)\compiler.res

showerr.obj: $(source)\support\showerr.c
        echo  $(source)\support\showerr.c >> $(obj)\compiler.res

shell.obj: $(source)\main\shell.c
        echo  $(source)\main\shell.c >> $(obj)\compiler.res

settings.obj: $(source)\users\settings.c
        echo  $(source)\users\settings.c >> $(obj)\compiler.res

scrlback.obj: $(source)\display\scrlback.c
        echo  $(source)\display\scrlback.c >> $(obj)\compiler.res

script.obj: $(source)\main\script.c
        echo  $(source)\main\script.c >> $(obj)\compiler.res

screen.obj: $(source)\display\screen.c
        echo  $(source)\display\screen.c >> $(obj)\compiler.res

recycle.obj: $(source)\main\recycle.c
        echo  $(source)\main\recycle.c >> $(obj)\compiler.res

ratio.obj: $(source)\ppl\ratio.cpp
        echo  $(source)\ppl\ratio.cpp >> $(obj)\compiler.res

pcbtext.obj: $(source)\display\pcbtext.c
        echo  $(source)\display\pcbtext.c >> $(obj)\compiler.res

pcboard.obj: $(source)\main\pcboard.c
        echo  $(source)\main\pcboard.c >> $(obj)\compiler.res

pcbmacro.obj: $(source)\display\pcbmacro.c
        echo  $(source)\display\pcbmacro.c >> $(obj)\compiler.res

node.obj: $(source)\node\node.c $(proj)\nodes.chk
        echo  $(source)\node\node.c >> $(obj)\compiler.res

newchat.obj: $(source)\node\newchat.c $(proj)\nodes.chk
        echo  $(source)\node\newchat.c >> $(obj)\compiler.res

msgscan.obj: $(source)\msg\msgscan.c
        echo  $(source)\msg\msgscan.c >> $(obj)\compiler.res

msgread.obj: $(source)\msg\msgread.c
        echo  $(source)\msg\msgread.c >> $(obj)\compiler.res

msgenter.obj: $(source)\msg\msgenter.c
        echo  $(source)\msg\msgenter.c >> $(obj)\compiler.res

msgbase.obj: $(source)\msg\msgbase.c
        echo  $(source)\msg\msgbase.c >> $(obj)\compiler.res

modemos2.obj: $(source)\modem\modemos2.c
        echo  $(source)\modem\modemos2.c >> $(obj)\compiler.res

modem.obj: $(source)\modem\modem.c
        echo  $(source)\modem\modem.c >> $(obj)\compiler.res

misc.obj: $(source)\main\misc.c
        echo  $(source)\main\misc.c >> $(obj)\compiler.res

messages.obj: $(source)\msg\messages.c
        echo  $(source)\msg\messages.c >> $(obj)\compiler.res

memory.obj: $(source)\support\memory.c
        echo  $(source)\support\memory.c >> $(obj)\compiler.res

logview.obj: $(source)\node\logview.c $(proj)\nodes.chk
        echo  $(source)\node\logview.c >> $(obj)\compiler.res

login.obj: $(source)\node\login.c
        echo  $(source)\node\login.c >> $(obj)\compiler.res

log.obj: $(source)\node\log.c $(proj)\nodes.chk
        echo  $(source)\node\log.c >> $(obj)\compiler.res

language.obj: $(source)\main\language.c
        echo  $(source)\main\language.c >> $(obj)\compiler.res

input.obj: $(source)\main\input.c
        echo  $(source)\main\input.c >> $(obj)\compiler.res

inkey.obj: $(source)\main\inkey.c
        echo  $(source)\main\inkey.c >> $(obj)\compiler.res

init.obj: $(source)\main\init.c $(proj)\nodes.chk
        echo  $(source)\main\init.c >> $(obj)\compiler.res

index.obj: $(source)\main\index.c
        echo  $(source)\main\index.c >> $(obj)\compiler.res

help.obj: $(source)\display\help.c
        echo  $(source)\display\help.c >> $(obj)\compiler.res

files.obj: $(source)\display\files.c
        echo  $(source)\display\files.c >> $(obj)\compiler.res

filelist.obj: $(source)\main\filelist.c
        echo  $(source)\main\filelist.c >> $(obj)\compiler.res

event.obj: $(source)\main\event.c $(proj)\nodes.chk
        echo  $(source)\main\event.c >> $(obj)\compiler.res

envfix.obj: $(source)\support\envfix.c
        echo  $(source)\support\envfix.c >> $(obj)\compiler.res

dostime.obj: $(source)\dos\dostime.c
        echo  $(source)\dos\dostime.c >> $(obj)\compiler.res

doswrite.obj: $(source)\dos\doswrite.c
        echo  $(source)\dos\doswrite.c >> $(obj)\compiler.res

dosread.obj: $(source)\dos\dosread.c
        echo  $(source)\dos\dosread.c >> $(obj)\compiler.res

dosopen.obj: $(source)\dos\dosopen.c
        echo  $(source)\dos\dosopen.c >> $(obj)\compiler.res

dosclose.obj: $(source)\dos\dosclose.c
        echo  $(source)\dos\dosclose.c >> $(obj)\compiler.res

doors.obj: $(source)\main\doors.c
        echo  $(source)\main\doors.c >> $(obj)\compiler.res

dlpath.obj: $(source)\main\dlpath.c
        echo  $(source)\main\dlpath.c >> $(obj)\compiler.res

diz.obj: $(source)\support\diz.c
        echo  $(source)\support\diz.c >> $(obj)\compiler.res

display.obj: $(source)\display\display.c
        echo  $(source)\display\display.c >> $(obj)\compiler.res

dir.obj: $(source)\display\dir.c
        echo  $(source)\display\dir.c >> $(obj)\compiler.res

devioctl.obj: $(source)\modem\devioctl.c
        echo  $(source)\modem\devioctl.c >> $(obj)\compiler.res

crc32.obj: $(source)\support\crc32.c
        echo  $(source)\support\crc32.c >> $(obj)\compiler.res

copyfile.obj: $(source)\support\copyfile.c
        echo  $(source)\support\copyfile.c >> $(obj)\compiler.res

confrnce.obj: $(source)\main\confrnce.c
        echo  $(source)\main\confrnce.c >> $(obj)\compiler.res

command.obj: $(source)\main\command.c
        echo  $(source)\main\command.c >> $(obj)\compiler.res

cmds.obj: $(source)\main\cmds.c
        echo  $(source)\main\cmds.c >> $(obj)\compiler.res

chat.obj: $(source)\main\chat.c
        echo  $(source)\main\chat.c >> $(obj)\compiler.res

capture.obj: $(source)\support\capture.c
        echo  $(source)\support\capture.c >> $(obj)\compiler.res

callwait.obj: $(source)\main\callwait.c
        echo  $(source)\main\callwait.c >> $(obj)\compiler.res

blt.obj: $(source)\display\blt.c
        echo  $(source)\display\blt.c >> $(obj)\compiler.res

ansi.obj: $(source)\display\ansi.c
        echo  $(source)\display\ansi.c >> $(obj)\compiler.res

account.obj: $(source)\support\account.cpp
        echo  $(source)\support\account.cpp >> $(obj)\compiler.res

#=============================================================
#               compiler configuration file
#=============================================================
$(cfg): $(mak)
  copy &&|
-rt-
-xd-
-x-
-r
-g
-oz
-ob
-oe
-oc
-dpcb152;pcb153;___use_var___;___exec___;dbase;comm;multiport;osdriver;pcbstats;pcbcomm;fido;mg;tossclass;bigndx;kbd3
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
-c
-k
-a
-5
-dndebug
$(dbg)
| $(cfg)


#=============================================================
#       clean - remove everything this makefile produces
#=============================================================

clean:
  -if exist $(obj)\*.obj del $(obj)\*.obj
  -if exist $(obj)\*.map del $(obj)\*.map
  -if exist $(obj)\*.res del $(obj)\*.res
  -if exist $(obj)\pcboard2.exe del $(obj)\pcboard2.exe
  -if exist $(cfg) del $(cfg)
