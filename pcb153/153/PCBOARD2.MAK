#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* The information in this module is proprietary software belonging to       */
#* Clark Development Company and is part of the PCBoard source code library. */
#* You are granted the right to use this information for the building of any */
#* of the PCBoard products you have licensed.  Any other usage is forbidden  */
#* without prior written consent from Clark Development Company, Inc.        */
#*                                                                           */
#* Be sure to read the source code license agreement before utilizing any    */
#* of the source code found herein.                                          */
#*                                                                           */
#* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#=============================================================
#
#       PCBOARD2.MAK - Makefile for PCBoard-for-OS/2
#
#=============================================================
#
#  NOTE:  To use this makefile, you must first execute
#         \PROJ\BCOS2.CMD which sets up several environment
#         variables which are used to locate things such as
#         the compiler, the header files and the output
#         location.
#
#=============================================================

.SILENT
.AUTODEPEND
.CACHEAUTODEPEND

VERSION = 153
ROOT    = \PROJ
PROJ    = $(ROOT)\PCB
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\BCOS2
SOURCE  = $(PROJ)\SOURCE
OBJ     = $(PROJ)\OBJ\BCOS2

CFG     = $(PROJ)\$(VERSION)\PCBOARD2.CFG
MAK     = $(PROJ)\$(VERSION)\PCBOARD2.MAK

.PATH.obj = $(OBJ)

OPT = -D$(NUMNODES)
!if $d(BETA)
  OPT = $(OPT) -DBETA
!endif

!if $d(DEBUG)
# OPT = $(OPT) -DDEBUG -DDEBUG2
# OPT = $(OPT) -DDEBUG
  OPT = $(OPT) -DDEBUG -DEBUGSCR
!endif

!if $d(TD)
# OPT = $(OPT) -N
  DBG = /v
!endif

CC = $(COMPILER) +$(CFG) $(OPT) $(DBG) -DKEY=KEY_$(NODES)
INCLUDEPATH = $(INCLUDE);$(SOURCE)\H;$(LIBROOT)\H;$(LIBSDIR)\CODEBASE\SOURCE


#=============================================================
#               List Macros
#=============================================================


EXE_DEPENDENCIES =  \
 $(LIBSDIR)\codebase\os2bor1\b4.lib \
 $(LIBDIR)\pcb.lib \
 $(LIBDIR)\country.lib \
 $(LIBDIR)\misc.lib \
 $(LIBDIR)\dos.lib \
 $(LIBDIR)\doscls.lib \
 $(LIBDIR)\screen.lib \
 $(LIBDIR)\system.lib \
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
 $(ROOT)\md5\os2\md5.obj

#=============================================================
#               Explicit Rules
#=============================================================
#
#  NOTE:  A stack size of 40960 causes an error to occur when saving a
#         message with a file attachment, but 36864 does not, nor does
#         it occur when using a setting of 45056.
#
#  On 8/30/95, changed from 45056 to 49152.

$(OBJ)\pcboard2.exe: $(CFG) $(EXE_DEPENDENCIES) $(OBJ)\linker.res
   $(LINKER) $(DBG) /B:0x10000 /S:49152 /x /Toe /ap /Oc /L$(LIBPATH) @&&|
D:\BCOS2\LIB\C02.OBJ+
$(OBJ)\pcboard.obj+
$(OBJ)\init.obj+
VERSION\BCOS2.obj+
$(OBJ)\callwait.obj+
$(OBJ)\inkey.obj+
$(OBJ)\devioctl.obj+
$(OBJ)\modemos2.obj+
$(OBJ)\modem.obj+
$(OBJ)\input.obj+
$(OBJ)\memory.obj+
$(OBJ)\ansi.obj+
$(OBJ)\display.obj+
$(OBJ)\xlate.obj+
$(OBJ)\pcbmacro.obj+
$(OBJ)\pcbtext.obj+
$(OBJ)\files.obj+
$(OBJ)\log.obj+
$(OBJ)\dostime.obj+
$(OBJ)\doswrite.obj+
$(OBJ)\dosread.obj+
$(OBJ)\dosopen.obj+
$(OBJ)\dosclose.obj+
$(OBJ)\usernet.obj+
$(OBJ)\msgbase.obj+
$(OBJ)\messages.obj+
$(OBJ)\msgscan.obj+
$(OBJ)\msgread.obj+
$(OBJ)\msgenter.obj+
$(OBJ)\umwf.obj+
$(OBJ)\seenby.obj+
$(OBJ)\pcbtoss.obj+
$(OBJ)\passthru.obj+
$(OBJ)\fidoque.obj+
$(OBJ)\fidomsg.obj+
$(OBJ)\fidomisc.obj+
$(OBJ)\data.obj+
$(OBJ)\dupechec.obj+
$(OBJ)\hex.obj+
$(OBJ)\xmitemsi.obj+
$(OBJ)\tossmisc.obj+
$(OBJ)\recwazoo.obj+
$(OBJ)\recemsi.obj+
$(OBJ)\fidomenu.obj+
$(OBJ)\fidofunc.obj+
$(OBJ)\fconfig.obj+
$(OBJ)\crc-16f.obj+
$(OBJ)\newscr.obj+
$(OBJ)\var.obj+
$(OBJ)\scrmisc.obj+
$(OBJ)\evalp.obj+
$(OBJ)\screxec.obj+
$(OBJ)\pcbmisc.obj+
$(OBJ)\execdb.obj+
$(OBJ)\dbase.obj+
$(OBJ)\pcbmsgs.obj+
$(OBJ)\msgstub.obj+
$(OBJ)\menu.obj+
$(OBJ)\lrand.obj+
$(OBJ)\label.obj+
$(OBJ)\filestub.obj+
$(OBJ)\xmodem.obj+
$(OBJ)\wild.obj+
$(OBJ)\usrmaint.obj+
$(OBJ)\usersys.obj+
$(OBJ)\userstat.obj+
$(OBJ)\userscan.obj+
$(OBJ)\users.obj+
$(OBJ)\index.obj+
$(OBJ)\token.obj+
$(OBJ)\ticdelay.obj+
$(OBJ)\sys.obj+
$(OBJ)\status.obj+
$(OBJ)\stats.obj+
$(OBJ)\showerr.obj+
$(OBJ)\settings.obj+
$(OBJ)\scrlback.obj+
$(OBJ)\script.obj+
$(OBJ)\screen.obj+
$(OBJ)\recycle.obj+
$(OBJ)\ratio.obj+
$(OBJ)\node.obj+
$(OBJ)\newchat.obj+
$(OBJ)\misc.obj+
$(OBJ)\logview.obj+
$(OBJ)\login.obj+
$(OBJ)\language.obj+
$(OBJ)\help.obj+
$(OBJ)\event.obj+
$(OBJ)\shell.obj+
$(OBJ)\doors.obj+
$(OBJ)\transfer.obj+
$(OBJ)\dlpath.obj+
$(OBJ)\filelist.obj+
$(OBJ)\copyfile.obj+
$(OBJ)\dir.obj+
$(OBJ)\diz.obj+
$(OBJ)\crc32.obj+
$(OBJ)\confrnce.obj+
$(OBJ)\command.obj+
$(OBJ)\cmds.obj+
$(OBJ)\chat.obj+
$(OBJ)\capture.obj+
$(OBJ)\blt.obj+
$(OBJ)\account.obj+
$(ROOT)\md5\os2\md5.obj
$(OBJ)\pcboard2
                # no map file
$(LIBSDIR)\codebase\os2bor1\b4.lib+
$(LIBDIR)\pcb.lib+
$(LIBDIR)\country.lib+
$(LIBDIR)\misc.lib+
$(LIBDIR)\dos.lib+
$(LIBDIR)\screen.lib+
$(LIBDIR)\system.lib+
$(LIBDIR)\doscls.lib+
$(LIBPATH)\C2MT.LIB+
$(LIBPATH)\OS2.LIB
$(PROJ)\$(VERSION)\pcboard2.def
|

  \PROJ\PCB\153\LXBFIX $(OBJ)\pcboard2.exe
  if exist $(OBJ)\linker.res del $(OBJ)\linker.res


#==============================================================================
# the following lines are used to group all of the modules that need to be
# compiled into a single input.res file and then launch the compiler just once
# instead of launching the compiler once per module
#==============================================================================

$(OBJ)\linker.res: $(OBJ)\compiler.res
  if exist $(OBJ)\compiler.res $(CC) -c @$(OBJ)\compiler.res
  if exist $(OBJ)\compiler.res del $(OBJ)\compiler.res
  echo need to link > $(OBJ)\linker.res


#=============================================================
#               Individual File Dependencies
#=============================================================
UMWF.obj: $(SOURCE)\FIDO\UMWF.CPP
        echo  $(SOURCE)\FIDO\UMWF.CPP >> $(OBJ)\compiler.res

SEENBY.obj: $(SOURCE)\FIDO\SEENBY.CPP
        echo  $(SOURCE)\FIDO\SEENBY.CPP >> $(OBJ)\compiler.res

PCBTOSS.obj: $(SOURCE)\FIDO\PCBTOSS.CPP
        echo  $(SOURCE)\FIDO\PCBTOSS.CPP >> $(OBJ)\compiler.res

PASSTHRU.obj: $(SOURCE)\FIDO\PASSTHRU.CPP
        echo  $(SOURCE)\FIDO\PASSTHRU.CPP >> $(OBJ)\compiler.res

FIDOQUE.obj: $(SOURCE)\FIDO\FIDOQUE.CPP
        echo  $(SOURCE)\FIDO\FIDOQUE.CPP >> $(OBJ)\compiler.res

FIDOMSG.obj: $(SOURCE)\FIDO\FIDOMSG.CPP
        echo  $(SOURCE)\FIDO\FIDOMSG.CPP >> $(OBJ)\compiler.res

FIDOMISC.obj: $(SOURCE)\FIDO\FIDOMISC.CPP
        echo  $(SOURCE)\FIDO\FIDOMISC.CPP >> $(OBJ)\compiler.res

DATA.obj: $(SOURCE)\FIDO\DATA.CPP
        echo  $(SOURCE)\FIDO\DATA.CPP >> $(OBJ)\compiler.res

DUPECHEC.obj: $(SOURCE)\FIDO\DUPECHEC.CPP
        echo  $(SOURCE)\FIDO\DUPECHEC.CPP >> $(OBJ)\compiler.res

HEX.obj: $(SOURCE)\FIDO\HEX.C
        echo  $(SOURCE)\FIDO\HEX.C >> $(OBJ)\compiler.res

XMITEMSI.obj: $(SOURCE)\FIDO\XMITEMSI.C
        echo  $(SOURCE)\FIDO\XMITEMSI.C >> $(OBJ)\compiler.res

TOSSMISC.obj: $(SOURCE)\FIDO\TOSSMISC.C
        echo  $(SOURCE)\FIDO\TOSSMISC.C >> $(OBJ)\compiler.res

RECWAZOO.obj: $(SOURCE)\FIDO\RECWAZOO.C
        echo  $(SOURCE)\FIDO\RECWAZOO.C >> $(OBJ)\compiler.res

RECEMSI.obj: $(SOURCE)\FIDO\RECEMSI.C
        echo  $(SOURCE)\FIDO\RECEMSI.C >> $(OBJ)\compiler.res

FIDOMENU.obj: $(SOURCE)\FIDO\FIDOMENU.C
        echo  $(SOURCE)\FIDO\FIDOMENU.C >> $(OBJ)\compiler.res

FIDOFUNC.obj: $(SOURCE)\FIDO\FIDOFUNC.C
        echo  $(SOURCE)\FIDO\FIDOFUNC.C >> $(OBJ)\compiler.res

FCONFIG.obj: $(SOURCE)\FIDO\FCONFIG.C
        echo  $(SOURCE)\FIDO\FCONFIG.C >> $(OBJ)\compiler.res

CRC-16F.obj: $(SOURCE)\FIDO\CRC-16F.C
        echo  $(SOURCE)\FIDO\CRC-16F.C >> $(OBJ)\compiler.res

var.obj: $(SOURCE)\PPL\var.cpp
        echo  $(SOURCE)\PPL\var.cpp >> $(OBJ)\compiler.res

scrmisc.obj: $(SOURCE)\PPL\scrmisc.cpp
        echo  $(SOURCE)\PPL\scrmisc.cpp >> $(OBJ)\compiler.res

MSGSTUB.obj: $(SOURCE)\PPL\MSGSTUB.CPP
        echo  $(SOURCE)\PPL\MSGSTUB.CPP >> $(OBJ)\compiler.res

menu.obj: $(SOURCE)\PPL\menu.cpp
        echo  $(SOURCE)\PPL\menu.cpp >> $(OBJ)\compiler.res

LRAND.obj: $(SOURCE)\PPL\LRAND.CPP
        echo  $(SOURCE)\PPL\LRAND.CPP >> $(OBJ)\compiler.res

LABEL.obj: $(SOURCE)\PPL\LABEL.CPP
        echo  $(SOURCE)\PPL\LABEL.CPP >> $(OBJ)\compiler.res

FILESTUB.obj: $(SOURCE)\PPL\FILESTUB.CPP
        echo  $(SOURCE)\PPL\FILESTUB.CPP >> $(OBJ)\compiler.res

execdb.obj: $(SOURCE)\PPL\execdb.cpp
        echo  $(SOURCE)\PPL\execdb.cpp >> $(OBJ)\compiler.res

evalp.obj: $(SOURCE)\PPL\evalp.cpp $(PROJ)\nodes.chk
        echo  $(SOURCE)\PPL\evalp.cpp >> $(OBJ)\compiler.res

dbase.obj: $(SOURCE)\PPL\dbase.cpp
        echo  $(SOURCE)\PPL\dbase.cpp >> $(OBJ)\compiler.res

screxec.obj: $(SOURCE)\PPL\screxec.cpp
        echo  $(SOURCE)\PPL\screxec.cpp >> $(OBJ)\compiler.res

pcbmisc.obj: $(SOURCE)\PPL\pcbmisc.cpp
        echo  $(SOURCE)\PPL\pcbmisc.cpp >> $(OBJ)\compiler.res

pcbmsgs.obj: $(SOURCE)\PPL\pcbmsgs.cpp
        echo  $(SOURCE)\PPL\pcbmsgs.cpp >> $(OBJ)\compiler.res

newscr.obj: $(SOURCE)\PPL\newscr.cpp
        echo  $(SOURCE)\PPL\newscr.cpp >> $(OBJ)\compiler.res

XMODEM.obj: $(SOURCE)\SUPPORT\XMODEM.C
        echo  $(SOURCE)\SUPPORT\XMODEM.C >> $(OBJ)\compiler.res

XLATE.obj: $(SOURCE)\DISPLAY\XLATE.C
        echo  $(SOURCE)\DISPLAY\XLATE.C >> $(OBJ)\compiler.res

WILD.obj: $(SOURCE)\SUPPORT\WILD.C
        echo  $(SOURCE)\SUPPORT\WILD.C >> $(OBJ)\compiler.res

# version.obj: $(SOURCE)\SUPPORT\version.c $(PROJ)\nodes.chk
#         echo  $(SOURCE)\SUPPORT\version.c >> $(OBJ)\compiler.res

usrmaint.obj: $(SOURCE)\USERS\usrmaint.c
        echo  $(SOURCE)\USERS\usrmaint.c >> $(OBJ)\compiler.res

usersys.obj: $(SOURCE)\USERS\usersys.c
        echo  $(SOURCE)\USERS\usersys.c >> $(OBJ)\compiler.res

USERSTAT.obj: $(SOURCE)\USERS\USERSTAT.C
        echo  $(SOURCE)\USERS\USERSTAT.C >> $(OBJ)\compiler.res

userscan.obj: $(SOURCE)\USERS\userscan.c
        echo  $(SOURCE)\USERS\userscan.c >> $(OBJ)\compiler.res

USERS.obj: $(SOURCE)\USERS\USERS.C
        echo  $(SOURCE)\USERS\USERS.C >> $(OBJ)\compiler.res

USERNET.obj: $(SOURCE)\NODE\USERNET.C $(PROJ)\nodes.chk
        echo  $(SOURCE)\NODE\USERNET.C >> $(OBJ)\compiler.res

TRANSFER.obj: $(SOURCE)\MAIN\TRANSFER.C
        echo  $(SOURCE)\MAIN\TRANSFER.C >> $(OBJ)\compiler.res

token.obj: $(SOURCE)\SUPPORT\token.c
        echo  $(SOURCE)\SUPPORT\token.c >> $(OBJ)\compiler.res

TICDELAY.obj: $(SOURCE)\MODEM\TICDELAY.C
        echo  $(SOURCE)\MODEM\TICDELAY.C >> $(OBJ)\compiler.res

SYS.obj: $(SOURCE)\MAIN\SYS.C
        echo  $(SOURCE)\MAIN\SYS.C >> $(OBJ)\compiler.res

STATUS.obj: $(SOURCE)\DISPLAY\STATUS.C
        echo  $(SOURCE)\DISPLAY\STATUS.C >> $(OBJ)\compiler.res

stats.obj: $(SOURCE)\NODE\stats.c $(PROJ)\nodes.chk
        echo  $(SOURCE)\NODE\stats.c >> $(OBJ)\compiler.res

SHOWERR.obj: $(SOURCE)\SUPPORT\SHOWERR.C
        echo  $(SOURCE)\SUPPORT\SHOWERR.C >> $(OBJ)\compiler.res

SHELL.obj: $(SOURCE)\MAIN\SHELL.C
        echo  $(SOURCE)\MAIN\SHELL.C >> $(OBJ)\compiler.res

settings.obj: $(SOURCE)\USERS\settings.c
        echo  $(SOURCE)\USERS\settings.c >> $(OBJ)\compiler.res

SCRLBACK.obj: $(SOURCE)\DISPLAY\SCRLBACK.C
        echo  $(SOURCE)\DISPLAY\SCRLBACK.C >> $(OBJ)\compiler.res

SCRIPT.obj: $(SOURCE)\MAIN\SCRIPT.C
        echo  $(SOURCE)\MAIN\SCRIPT.C >> $(OBJ)\compiler.res

screen.obj: $(SOURCE)\DISPLAY\screen.c
        echo  $(SOURCE)\DISPLAY\screen.c >> $(OBJ)\compiler.res

RECYCLE.obj: $(SOURCE)\MAIN\RECYCLE.C
        echo  $(SOURCE)\MAIN\RECYCLE.C >> $(OBJ)\compiler.res

ratio.obj: $(SOURCE)\PPL\ratio.cpp
        echo  $(SOURCE)\PPL\ratio.cpp >> $(OBJ)\compiler.res

pcbtext.obj: $(SOURCE)\DISPLAY\pcbtext.c
        echo  $(SOURCE)\DISPLAY\pcbtext.c >> $(OBJ)\compiler.res

PCBOARD.obj: $(SOURCE)\MAIN\PCBOARD.C
        echo  $(SOURCE)\MAIN\PCBOARD.C >> $(OBJ)\compiler.res

PCBMACRO.obj: $(SOURCE)\DISPLAY\PCBMACRO.C
        echo  $(SOURCE)\DISPLAY\PCBMACRO.C >> $(OBJ)\compiler.res

node.obj: $(SOURCE)\NODE\node.c $(PROJ)\nodes.chk
        echo  $(SOURCE)\NODE\node.c >> $(OBJ)\compiler.res

NEWCHAT.obj: $(SOURCE)\NODE\NEWCHAT.C $(PROJ)\nodes.chk
        echo  $(SOURCE)\NODE\NEWCHAT.C >> $(OBJ)\compiler.res

msgscan.obj: $(SOURCE)\MSG\msgscan.c
        echo  $(SOURCE)\MSG\msgscan.c >> $(OBJ)\compiler.res

MSGREAD.obj: $(SOURCE)\MSG\MSGREAD.C
        echo  $(SOURCE)\MSG\MSGREAD.C >> $(OBJ)\compiler.res

MSGENTER.obj: $(SOURCE)\MSG\MSGENTER.C
        echo  $(SOURCE)\MSG\MSGENTER.C >> $(OBJ)\compiler.res

msgbase.obj: $(SOURCE)\MSG\msgbase.c
        echo  $(SOURCE)\MSG\msgbase.c >> $(OBJ)\compiler.res

modemos2.obj: $(SOURCE)\MODEM\modemos2.c
        echo  $(SOURCE)\MODEM\modemos2.c >> $(OBJ)\compiler.res

modem.obj: $(SOURCE)\MODEM\modem.c
        echo  $(SOURCE)\MODEM\modem.c >> $(OBJ)\compiler.res

MISC.obj: $(SOURCE)\MAIN\MISC.C
        echo  $(SOURCE)\MAIN\MISC.C >> $(OBJ)\compiler.res

MESSAGES.obj: $(SOURCE)\MSG\MESSAGES.C
        echo  $(SOURCE)\MSG\MESSAGES.C >> $(OBJ)\compiler.res

memory.obj: $(SOURCE)\SUPPORT\memory.c
        echo  $(SOURCE)\SUPPORT\memory.c >> $(OBJ)\compiler.res

LOGVIEW.obj: $(SOURCE)\NODE\LOGVIEW.C $(PROJ)\nodes.chk
        echo  $(SOURCE)\NODE\LOGVIEW.C >> $(OBJ)\compiler.res

LOGIN.obj: $(SOURCE)\NODE\LOGIN.C
        echo  $(SOURCE)\NODE\LOGIN.C >> $(OBJ)\compiler.res

log.obj: $(SOURCE)\NODE\log.c $(PROJ)\nodes.chk
        echo  $(SOURCE)\NODE\log.c >> $(OBJ)\compiler.res

LANGUAGE.obj: $(SOURCE)\MAIN\LANGUAGE.C
        echo  $(SOURCE)\MAIN\LANGUAGE.C >> $(OBJ)\compiler.res

INPUT.obj: $(SOURCE)\MAIN\INPUT.C
        echo  $(SOURCE)\MAIN\INPUT.C >> $(OBJ)\compiler.res

INKEY.obj: $(SOURCE)\MAIN\INKEY.C
        echo  $(SOURCE)\MAIN\INKEY.C >> $(OBJ)\compiler.res

INIT.obj: $(SOURCE)\MAIN\INIT.C $(PROJ)\nodes.chk
        echo  $(SOURCE)\MAIN\INIT.C >> $(OBJ)\compiler.res

INDEX.obj: $(SOURCE)\MAIN\INDEX.C
        echo  $(SOURCE)\MAIN\INDEX.C >> $(OBJ)\compiler.res

help.obj: $(SOURCE)\DISPLAY\help.c
        echo  $(SOURCE)\DISPLAY\help.c >> $(OBJ)\compiler.res

FILES.obj: $(SOURCE)\DISPLAY\FILES.C
        echo  $(SOURCE)\DISPLAY\FILES.C >> $(OBJ)\compiler.res

filelist.obj: $(SOURCE)\MAIN\filelist.c
        echo  $(SOURCE)\MAIN\filelist.c >> $(OBJ)\compiler.res

EVENT.obj: $(SOURCE)\MAIN\EVENT.C $(PROJ)\nodes.chk
        echo  $(SOURCE)\MAIN\EVENT.C >> $(OBJ)\compiler.res

ENVFIX.obj: $(SOURCE)\SUPPORT\ENVFIX.C
        echo  $(SOURCE)\SUPPORT\ENVFIX.C >> $(OBJ)\compiler.res

DOSTIME.obj: $(SOURCE)\DOS\DOSTIME.C
        echo  $(SOURCE)\DOS\DOSTIME.C >> $(OBJ)\compiler.res

doswrite.obj: $(SOURCE)\DOS\doswrite.c
        echo  $(SOURCE)\DOS\doswrite.c >> $(OBJ)\compiler.res

dosread.obj: $(SOURCE)\DOS\dosread.c
        echo  $(SOURCE)\DOS\dosread.c >> $(OBJ)\compiler.res

dosopen.obj: $(SOURCE)\DOS\dosopen.c
        echo  $(SOURCE)\DOS\dosopen.c >> $(OBJ)\compiler.res

dosclose.obj: $(SOURCE)\DOS\dosclose.c
        echo  $(SOURCE)\DOS\dosclose.c >> $(OBJ)\compiler.res

DOORS.obj: $(SOURCE)\MAIN\DOORS.C
        echo  $(SOURCE)\MAIN\DOORS.C >> $(OBJ)\compiler.res

DLPATH.obj: $(SOURCE)\MAIN\DLPATH.C
        echo  $(SOURCE)\MAIN\DLPATH.C >> $(OBJ)\compiler.res

diz.obj: $(SOURCE)\SUPPORT\diz.c
        echo  $(SOURCE)\SUPPORT\diz.c >> $(OBJ)\compiler.res

DISPLAY.obj: $(SOURCE)\DISPLAY\DISPLAY.C
        echo  $(SOURCE)\DISPLAY\DISPLAY.C >> $(OBJ)\compiler.res

DIR.obj: $(SOURCE)\DISPLAY\DIR.C
        echo  $(SOURCE)\DISPLAY\DIR.C >> $(OBJ)\compiler.res

devioctl.obj: $(SOURCE)\MODEM\devioctl.c
        echo  $(SOURCE)\MODEM\devioctl.c >> $(OBJ)\compiler.res

crc32.obj: $(SOURCE)\SUPPORT\crc32.c
        echo  $(SOURCE)\SUPPORT\crc32.c >> $(OBJ)\compiler.res

COPYFILE.obj: $(SOURCE)\SUPPORT\COPYFILE.C
        echo  $(SOURCE)\SUPPORT\COPYFILE.C >> $(OBJ)\compiler.res

CONFRNCE.obj: $(SOURCE)\MAIN\CONFRNCE.C
        echo  $(SOURCE)\MAIN\CONFRNCE.C >> $(OBJ)\compiler.res

COMMAND.obj: $(SOURCE)\MAIN\COMMAND.C
        echo  $(SOURCE)\MAIN\COMMAND.C >> $(OBJ)\compiler.res

CMDS.obj: $(SOURCE)\MAIN\CMDS.C
        echo  $(SOURCE)\MAIN\CMDS.C >> $(OBJ)\compiler.res

CHAT.obj: $(SOURCE)\MAIN\CHAT.C
        echo  $(SOURCE)\MAIN\CHAT.C >> $(OBJ)\compiler.res

capture.obj: $(SOURCE)\SUPPORT\capture.c
        echo  $(SOURCE)\SUPPORT\capture.c >> $(OBJ)\compiler.res

CALLWAIT.obj: $(SOURCE)\MAIN\CALLWAIT.C
        echo  $(SOURCE)\MAIN\CALLWAIT.C >> $(OBJ)\compiler.res

blt.obj: $(SOURCE)\DISPLAY\blt.c
        echo  $(SOURCE)\DISPLAY\blt.c >> $(OBJ)\compiler.res

ANSI.obj: $(SOURCE)\DISPLAY\ANSI.C
        echo  $(SOURCE)\DISPLAY\ANSI.C >> $(OBJ)\compiler.res

account.obj: $(SOURCE)\SUPPORT\account.cpp
        echo  $(SOURCE)\SUPPORT\account.cpp >> $(OBJ)\compiler.res

#=============================================================
#               Compiler Configuration File
#=============================================================
$(CFG): $(MAK)
  copy &&|
-RT-
-xd-
-x-
-R
-G
-Oz
-Ob
-Oe
-Oc
-DPCB152;PCB153;___USE_VAR___;___EXEC___;DBASE;COMM;MULTIPORT;OSDRIVER;PCBSTATS;PCBCOMM;FIDO;MG;TOSSCLASS;BIGNDX;KBD3
-L$(LIBPATH)
-I$(INCLUDEPATH)
-n$(OBJ)
-P
-vi
-sm
-d
-k-
-O
-Ot
-C
-K
-a
-5
-DNDEBUG
$(DBG)
| $(CFG)

