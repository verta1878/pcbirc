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
#       PCBOARD.MAK - Makefile for PCBoard-for-DOS
#
#=============================================================
#
#  NOTE:  To use this makefile, you must first execute
#         \PROJ\BCDOS.BAT which sets up several environment
#         variables which are used to locate things such as
#         the compiler, the header files and the output
#         location.
#
#=============================================================

.NOSILENT
.AUTODEPEND

VERSION = 153
ROOT    = \PROJ
PROJ    = $(ROOT)\PCB
LIBROOT = $(ROOT)\LIB
LIBDIR  = $(LIBROOT)\BCDOS\$(BCCOMPILER)
SOURCE  = SOURCE
OBJ     = OBJ\$(BCCOMPILER)

CFG     = $(VERSION)\PCBOARD.CFG
MAK     = $(VERSION)\PCBOARD.MAK


!if $d(COMMDRV)
INCLUDEPATH = $(INCLUDE);$(SOURCE)\H;$(LIBROOT)\H;$(LIBSDIR)\COMMDRV\H
!else
INCLUDEPATH = $(INCLUDE);$(SOURCE)\H;$(LIBROOT)\H
!endif


MDL = l                               # memory model

ASM     = SOURCE\ASM
MAIN    = SOURCE\MAIN
DOS     = SOURCE\DOS
DISPLAY = SOURCE\DISPLAY
MODEM   = SOURCE\MODEM
MSG     = SOURCE\MSG
NODE    = SOURCE\NODE
FIDODIR = SOURCE\FIDO
PPL     = SOURCE\PPL
SUPPORT = SOURCE\SUPPORT
USERS   = SOURCE\USERS

!if $d(STATS)
STATS = PCBSTATS
!else
STATS =
!endif

!if $d(DEBUG)
#GEN = DEBUG -DDEBUGSCR
GEN = DEBUG
!else
GEN = NDEBUG
!endif


!if $d(COMM)
COM  = COMM
!if $d(FIDO)
DEFFIDO = FIDO
!else
DEFFIDO =
!endif
!else
COM  = LOCALONLY
DEFFIDO =
!endif


!if $d(MP)
DRV = MULTIPORT
!else
DRV =
!endif

!if !$d(DEBUG) && !$(TD)
COPT = -c -k- -P                                # TCC
ASMOPT = /m3 /mx /t /D__$(MDL)__ /D$(GEN)       # TASM
LINKOPT = /c /x                                 # TLINK
!elif $d(TD)
COPT = -c -v -k- -P -N                          # TCC
ASMOPT = /m3 /mx /t /zi /D__$(MDL)__ /D$(GEN)   # TASM
LINKOPT = /c /x /v                              # TLINK  (/x=no map /s=detailed map)
!else
COPT = -c -k- -P -N                             # TCC
ASMOPT = /m3 /mx /t /D__$(MDL)__ /D$(GEN)       # TASM
LINKOPT = /c /x                                 # TLINK
!endif

!if $d(BC31) || $d(BC50)
LINKOPT = $(LINKOPT) /P                         # TLINK 3.1 & 5.0 (not TC 3.0)
!endif

!if $d(BC50)
COPT = $(COPT) -Obglmptv -Z -Vmp -Vmd -ff -x- # BCC v4.x & v5.0 (leave out -Oe because of bug in BCC)
!elif $d(BC31)
COPT = $(COPT) -Obeglmptv -Z -V -Vmp -Vmd -ff  # BCC v3.x
!elif $d(TC30)
COPT = $(COPT) -O -Z # TCC v3.0
!endif

!if $d(KBD3)
COPT = $(COPT) -DKBD3
ASMOPT = $(ASMOPT) /DKBD3
!endif

!if $d(HDR)
COPT   = $(COPT) -H=$(OBJ)\project.sym
!endif

!if $d(BETA)
GEN = $(GEN) -DBETA
!endif

!if $d(DEMO)
GEN = $(GEN) -DPCB_DEMO
!endif

!if $d(386) && ! $d(TC30)    # TC 3.0 does not support 386 compilation
COPT   = $(COPT) -3 /DCPU386
ASMOPT = $(ASMOPT) /DCPU386
EXT    = 386
LIBEXT = 386
RES    = PCB_$(BCCOMPILER).386
!else
EXT    = RES
LIBEXT = LIB
RES    = PCB_$(BCCOMPILER).RES
!endif

LIBS = $(VERSION)\LIBS$(BCCOMPILER).$(EXT)

#source code generation options
CODEOPT = -D$(GEN) -D$(COM) -D$(STATS) -D$(NUMNODES) -D$(DRV) -D$(DEFFIDO) -DKEY=KEY_$(NODES)

!if $d(DBASE)
INCLUDEPATH = $(INCLUDEPATH);$(LIBSDIR)\CODEBASE\SOURCE
OPTLIB      = /o+ $(LIBSDIR)\codebase\bor31\c4base.lib+
DBASEOPT    = -DDBASE
!else
OPTLIB      =
DBASEOPT    =
!endif

VERSIONOBJ  = VERSION\$(BCCOMPILER).OBJ

##############################################################################


$(OBJ)\pcboard.exe: $(CFG) $(LIBS) step1.$$$ step2.$$$
        if exist step1.$$$ del step1.$$$
        if exist step2.$$$ del step2.$$$
        $(LINKER) $(LINKOPT) $(OBJ)\c0$(MDL) $(VERSIONOBJ) @153\$(RES),$(OBJ)\pcboard,$(OBJ)\pcboard,@$(LIBS)
#       $(LINKER) $(LINKOPT) $(LIBPATH)\c0$(MDL) $(VERSIONOBJ) @153\$(RES),$(OBJ)\pcboard,$(OBJ)\pcboard,@$(LIBS)


##############################################################################


step1.$$$: \
  $(OBJ)\pcboard.obj \
  $(OBJ)\callwait.obj \
  $(OBJ)\capture.obj \
  $(OBJ)\chat.obj \
  $(OBJ)\command.obj \
  $(OBJ)\crc32.obj \
  $(OBJ)\display.obj \
  $(OBJ)\diz.obj \
  $(OBJ)\doors.obj \
  $(OBJ)\event.obj \
  $(OBJ)\files.obj \
  $(OBJ)\help.obj \
  $(OBJ)\filelist.obj \
  $(OBJ)\index.obj \
  $(OBJ)\init.obj \
  $(OBJ)\inkey.obj \
  $(OBJ)\input.obj \
  $(OBJ)\log.obj \
  $(OBJ)\logview.obj \
  $(OBJ)\login.obj \
  $(OBJ)\msgbase.obj \
  $(OBJ)\messages.obj \
  $(OBJ)\misc.obj \
  $(OBJ)\modem.obj \
  $(OBJ)\modemasy.obj \
  $(OBJ)\modemfos.obj \
  $(OBJ)\modemos2.obj \
  $(OBJ)\modemdrv.obj \
  $(OBJ)\msgenter.obj \
  $(OBJ)\msgread.obj \
  $(OBJ)\msgscan.obj \
  $(OBJ)\newchat.obj \
  $(OBJ)\node.obj \
  $(OBJ)\nonovrly.obj \
  $(OBJ)\cmds.obj \
  $(OBJ)\recycle.obj \
  $(OBJ)\script.obj \
  $(OBJ)\shell.obj \
  $(OBJ)\stats.obj \
  $(OBJ)\status.obj \
  $(OBJ)\sys.obj \
  $(OBJ)\ticdelay.obj \
  $(OBJ)\transfer.obj \
  $(OBJ)\usernet.obj \
  $(OBJ)\users.obj \
  $(OBJ)\userstat.obj \
  $(OBJ)\xlate.obj \
  $(OBJ)\account.obj \
  $(OBJ)\blt.obj \
  $(OBJ)\confrnce.obj \
  $(OBJ)\copyfile.obj \
  $(OBJ)\dir.obj \
  $(OBJ)\dlpath.obj \
  $(OBJ)\language.obj \
  $(OBJ)\memory.obj \
  $(OBJ)\overlay.obj \
  $(OBJ)\pcbtext.obj \
  $(OBJ)\screen.obj \
  $(OBJ)\scrlback.obj \
  $(OBJ)\settings.obj \

  if exist bcc.res echo $(COPT) > copt.res
  if exist bcc.res echo -m$(MDL) $(CODEOPT) > input.res
  if exist bcc.res echo done > step1.$$$
  if exist bcc.res $(COMPILER) +$(CFG) -D___EXEC___ @copt.res @input.res @bcc.res
  if exist input.res del input.res
  if exist bcc.res del bcc.res
  if exist copt.res del copt.res
  if exist $(OBJ)\pcboard.exe del $(OBJ)\pcboard.exe


step2.$$$:  \
  $(OBJ)\dosopen.obj \
  $(OBJ)\dosclose.obj \
  $(OBJ)\dosread.obj \
  $(OBJ)\doswrite.obj \
  $(OBJ)\dostime.obj \
  $(OBJ)\ansi.obj \
  $(OBJ)\async.obj \
  $(OBJ)\bgkey.obj \
  $(OBJ)\cutil.obj \
  $(OBJ)\memmove.obj \
  $(OBJ)\noscroll.obj \
  $(OBJ)\timer.obj \
  $(OBJ)\FILESTUB.obj \
  $(OBJ)\LABEL.obj \
  $(OBJ)\PCBMISC.obj \
  $(OBJ)\SCREXEC.obj \
  $(OBJ)\EVALP.obj \
  $(OBJ)\EXECDB.obj \
  $(OBJ)\NEWSCR.obj \
  $(OBJ)\SCRMISC.obj \
  $(OBJ)\VAR.obj \
  $(OBJ)\MENU.obj \
  $(OBJ)\LRAND.obj \
  $(OBJ)\DBASE.obj \
  $(OBJ)\c0$(MDL).obj \
  $(OBJ)\envfix.obj \
  $(OBJ)\devioctl.obj \
  $(OBJ)\ratio.obj \
  $(OBJ)\recemsi.obj \
  $(OBJ)\fconfig.obj \
  $(OBJ)\hex.obj \
  $(OBJ)\crc-16f.obj \
  $(OBJ)\xmitemsi.obj \
  $(OBJ)\data.obj \
  $(OBJ)\dupechec.obj \
  $(OBJ)\passthru.obj \
  $(OBJ)\fidofunc.obj \
  $(OBJ)\fidomenu.obj \
  $(OBJ)\fidomisc.obj \
  $(OBJ)\fidoque.obj \
  $(OBJ)\fidomsg.obj \
  $(OBJ)\seenby.obj \
  $(OBJ)\pcbtoss.obj \
  $(OBJ)\tossmisc.obj \
  $(OBJ)\recwazoo.obj \
  $(OBJ)\umwf.obj \
  $(OBJ)\pcbmsgs.obj \
  $(OBJ)\msgstub.obj \
  $(OBJ)\showerr.obj \
  $(OBJ)\token.obj \
  $(OBJ)\userscan.obj \
  $(OBJ)\usersys.obj \
  $(OBJ)\usrmaint.obj \
  $(LIBDIR)\system_$(MDL).$(LIBEXT) \
  $(LIBDIR)\country$(MDL).$(LIBEXT) \
  $(LIBDIR)\dos_$(MDL).$(LIBEXT) \
  $(LIBDIR)\pcb_$(MDL).$(LIBEXT) \
  $(LIBDIR)\screen_$(MDL).$(LIBEXT) \
  $(LIBDIR)\misc_$(MDL).$(LIBEXT)

  if exist bcc.res echo -m$(MDL) $(CODEOPT) > input.res
  if exist bcc.res echo $(COPT) > copt.res
  if exist bcc.res echo done > step2.$$$
  if exist bcc.res $(COMPILER) +$(CFG) -D___EXEC___ @copt.res @input.res @bcc.res
  if exist input.res del input.res
  if exist bcc.res del bcc.res
  if exist copt.res del copt.res
  if exist $(OBJ)\pcboard.exe del $(OBJ)\pcboard.exe


##############################################################################


$(OBJ)\account.obj:  $(SUPPORT)\account.cpp
  echo $(SUPPORT)\$&.cpp >> bcc.res

$(OBJ)\blt.obj:      $(DISPLAY)\blt.c
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\callwait.obj: $(MAIN)\callwait.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\capture.obj:  $(SUPPORT)\capture.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\chat.obj:     $(MAIN)\chat.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\cmds.obj:     $(MAIN)\cmds.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\command.obj:  $(MAIN)\command.c \
                     $(PROJ)\comm.chk $(PROJ)\nodes.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\crc32.obj:    $(SUPPORT)\crc32.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\confrnce.obj: $(MAIN)\confrnce.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\copyfile.obj: $(SUPPORT)\copyfile.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\devioctl.obj: $(MODEM)\devioctl.c
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\dir.obj:      $(DISPLAY)\dir.c
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\display.obj:  $(DISPLAY)\display.c \
                     $(PROJ)\comm.chk
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\diz.obj:      $(SUPPORT)\diz.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\dlpath.obj:   $(MAIN)\dlpath.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\doors.obj:    $(MAIN)\doors.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\event.obj:    $(MAIN)\event.c \
                     $(PROJ)\comm.chk $(PROJ)\nodes.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\filelist.obj: $(MAIN)\filelist.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\files.obj:    $(DISPLAY)\files.c \
                     $(PROJ)\comm.chk
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\help.obj:     $(DISPLAY)\help.c
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\index.obj:    $(MAIN)\index.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\init.obj:     $(MAIN)\init.c \
                     $(PROJ)\comm.chk $(PROJ)\nodes.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\inkey.obj:    $(MAIN)\inkey.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\input.obj:    $(MAIN)\input.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\language.obj: $(MAIN)\language.c
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\log.obj:      $(NODE)\log.c \
                     $(PROJ)\nodes.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\login.obj:    $(NODE)\login.c \
                     $(PROJ)\comm.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\logview.obj:  $(NODE)\logview.c \
                     $(PROJ)\nodes.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\memory.obj:   $(SUPPORT)\memory.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\msgbase.obj:  $(MSG)\msgbase.c
  echo $(MSG)\$&.c >> bcc.res

$(OBJ)\messages.obj: $(MSG)\messages.c
  echo $(MSG)\$&.c >> bcc.res

$(OBJ)\misc.obj:     $(MAIN)\misc.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\modem.obj:    $(MODEM)\modem.c \
                     $(PROJ)\comm.chk
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\modemasy.obj: $(MODEM)\modemasy.c
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\modemfos.obj: $(MODEM)\modemfos.c
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\modemos2.obj: $(MODEM)\modemos2.c
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\modemdrv.obj: $(MODEM)\modemdrv.c
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\msgenter.obj: $(MSG)\msgenter.c \
                     $(PROJ)\comm.chk
  echo $(MSG)\$&.c >> bcc.res

$(OBJ)\msgread.obj:  $(MSG)\msgread.c \
                     $(PROJ)\comm.chk
  echo $(MSG)\$&.c >> bcc.res

$(OBJ)\msgscan.obj:  $(MSG)\msgscan.c \
                     $(PROJ)\comm.chk
  echo $(MSG)\$&.c >> bcc.res

$(OBJ)\newchat.obj:  $(NODE)\newchat.c \
                     $(PROJ)\comm.chk $(PROJ)\nodes.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\node.obj:     $(NODE)\node.c \
                     $(PROJ)\nodes.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\nonovrly.obj: $(SUPPORT)\nonovrly.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\overlay.obj:  $(SUPPORT)\overlay.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\pcboard.obj:  $(MAIN)\pcboard.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\pcbtext.obj:  $(DISPLAY)\pcbtext.c \
                     $(PROJ)\comm.chk
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\recycle.obj:  $(MAIN)\recycle.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\screen.obj  : $(DISPLAY)\screen.c
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\script.obj  : $(MAIN)\script.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\scrlback.obj: $(DISPLAY)\scrlback.c
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\settings.obj: $(USERS)\settings.c
  echo $(USERS)\$&.c >> bcc.res

$(OBJ)\shell.obj:    $(MAIN)\shell.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\showerr.obj:  $(SUPPORT)\showerr.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\stats.obj:    $(NODE)\stats.c \
                     $(PROJ)\nodes.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\status.obj:   $(DISPLAY)\status.c \
                     $(PROJ)\comm.chk
  echo $(DISPLAY)\$&.c >> bcc.res

$(OBJ)\sys.obj:      $(MAIN)\sys.c \
                     $(PROJ)\nodes.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\token.obj:    $(SUPPORT)\token.c
  echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\ticdelay.obj: $(MODEM)\ticdelay.c \
                     $(PROJ)\comm.chk
  echo $(MODEM)\$&.c >> bcc.res

$(OBJ)\transfer.obj: $(MAIN)\transfer.c \
                     $(PROJ)\comm.chk
  echo $(MAIN)\$&.c >> bcc.res

$(OBJ)\usernet.obj:  $(NODE)\usernet.c \
                     $(PROJ)\nodes.chk
  echo $(NODE)\$&.c >> bcc.res

$(OBJ)\users.obj:    $(USERS)\users.c
  echo $(USERS)\$&.c >> bcc.res

$(OBJ)\userscan.obj: $(USERS)\userscan.c
  echo $(USERS)\$&.c >> bcc.res

$(OBJ)\userstat.obj: $(USERS)\userstat.c
  echo $(USERS)\$&.c >> bcc.res

$(OBJ)\usersys.obj:  $(USERS)\usersys.c
  echo $(USERS)\$&.c >> bcc.res

$(OBJ)\usrmaint.obj: $(USERS)\usrmaint.c \
                     $(PROJ)\comm.chk
  echo $(USERS)\$&.c >> bcc.res

# $(OBJ)\version.obj:  $(SUPPORT)\version.c \
#                      $(PROJ)\nodes.chk
#   echo $(SUPPORT)\$&.c >> bcc.res

$(OBJ)\xlate.obj:    $(DISPLAY)\xlate.c
  echo $(DISPLAY)\$&.c >> bcc.res

##############################################################################

$(OBJ)\c0$(MDL).obj: $(ASM)\c0.asm
  $(TASM) $(ASMOPT) $(ASM)\c0.asm, $(OBJ)\c0$(MDL).obj

$(OBJ)\dosopen.obj:  $(DOS)\dosopen.c
  echo $(DOS)\$&.c >> bcc.res

$(OBJ)\dosclose.obj: $(DOS)\dosclose.c
  echo $(DOS)\$&.c >> bcc.res

$(OBJ)\dosread.obj:  $(DOS)\dosread.c
  echo $(DOS)\$&.c >> bcc.res

$(OBJ)\doswrite.obj: $(DOS)\doswrite.c
  echo $(DOS)\$&.c >> bcc.res

$(OBJ)\dostime.obj:  $(DOS)\dostime.c
  echo $(DOS)\$&.c >> bcc.res

$(OBJ)\ansi.obj:     $(ASM)\ansi.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

$(OBJ)\async.obj:    $(ASM)\async.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

$(OBJ)\bgkey.obj:    $(ASM)\bgkey.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

$(OBJ)\cutil.obj:    $(ASM)\cutil.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

$(OBJ)\memmove.obj:  $(ASM)\memmove.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

$(OBJ)\noscroll.obj: $(ASM)\noscroll.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

$(OBJ)\timer.obj:    $(ASM)\timer.asm
  $(TASM) $(ASMOPT) $(ASM)\$&.asm, $(OBJ)\$&.obj

##############################################################################

$(OBJ)\filestub.obj: $(PPL)\filestub.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\label.obj:    $(PPL)\label.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\pcbmisc.obj:  $(PPL)\pcbmisc.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\dbase.obj:    $(PPL)\dbase.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\screxec.obj:  $(PPL)\screxec.cpp \
                     $(PROJ)\comm.chk
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\evalp.obj:    $(PPL)\evalp.cpp \
                     $(PROJ)\comm.chk $(PROJ)\nodes.chk
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\execdb.obj:   $(PPL)\execdb.cpp \
                     $(PROJ)\comm.chk
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\newscr.obj:   $(PPL)\newscr.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\scrmisc.obj:  $(PPL)\scrmisc.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\var.obj:      $(PPL)\var.cpp \
                     $(PROJ)\comm.chk
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\menu.obj:     $(PPL)\menu.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\lrand.obj:    $(PPL)\lrand.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\ratio.obj:    $(PPL)\ratio.cpp
  echo $(PPL)\$&.cpp >> bcc.res

## $(OBJ)\qint.obj:     $(PPL)\qint.cpp
##   echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\pcbmsgs.obj:  $(PPL)\pcbmsgs.cpp
  echo $(PPL)\$&.cpp >> bcc.res

$(OBJ)\msgstub.obj:  $(PPL)\msgstub.cpp
  echo $(PPL)\$&.cpp >> bcc.res

##############################################################################

$(OBJ)\recemsi.obj:  $(FIDODIR)\recemsi.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\fconfig.obj:  $(FIDODIR)\fconfig.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\xmitemsi.obj: $(FIDODIR)\xmitemsi.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\fidomenu.obj: $(FIDODIR)\fidomenu.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\fidomisc.obj: $(FIDODIR)\fidomisc.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\pcbtoss.obj:  $(FIDODIR)\pcbtoss.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\tossmisc.obj: $(FIDODIR)\tossmisc.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\seenby.obj:   $(FIDODIR)\seenby.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\recwazoo.obj: $(FIDODIR)\recwazoo.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\fidofunc.obj: $(FIDODIR)\fidofunc.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\fidomsg.obj:  $(FIDODIR)\fidomsg.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\umwf.obj:     $(FIDODIR)\umwf.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\data.obj:     $(FIDODIR)\data.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\dupechec.obj: $(FIDODIR)\dupechec.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\passthru.obj: $(FIDODIR)\passthru.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\fidoque.obj:  $(FIDODIR)\fidoque.cpp
  echo $(FIDODIR)\$&.cpp >> bcc.res

$(OBJ)\hex.obj:      $(FIDODIR)\hex.c
  echo $(FIDODIR)\$&.c >> bcc.res

$(OBJ)\crc-16f.obj:  $(FIDODIR)\crc-16f.c
  echo $(FIDODIR)\$&.c >> bcc.res


##############################################################################

$(OBJ)\envfix.obj:  $(SUPPORT)\envfix.c
  echo $(SUPPORT)\$&.c >> bcc.res

##############################################################################


#============================================================
#              Compiler Configuration File
#============================================================
$(CFG): $(MAK)
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
-C
-K
-G
-O
-Z
-Y
-k
-d
-Qe-
-n$(OBJ)
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DPCBOARD
-DPCBCOMM
-DOSDRIVER
-DFOSSIL
-DCOMMDRV
-DBIGNDX
-D___USE_VAR___
-DS4ERROR_HOOK
-DNDEBUG
-DPCB152
-DPCB153
-DCRYPT
$(DBASEOPT)
-DMG
-DTOSSCLASS
| $(CFG)

#============================================================
#              LIBS Response File
#============================================================

!if $d(COMMDRV)

$(LIBS): $(MAK)
  copy &&|
/o- $(LIBDIR)\countryl.$(LIBEXT)+
/o- $(LIBDIR)\dos_l.$(LIBEXT)+
/o- $(LIBDIR)\screen_l.$(LIBEXT)+
/o- $(LIBDIR)\pcb_l.$(LIBEXT)+
/o- $(LIBDIR)\misc_l.$(LIBEXT)+
/o- $(LIBDIR)\system_l.$(LIBEXT)+
/o- $(LIBDIR)\\doscls_l.$(LIBEXT)+
/o+ $(LIBSDIR)\commdrv\lib\commdrbl.lib+
/o+ $(LIBSDIR)\commdrv\lib\libsbl.lib+
/o- $(LIBPATH)\emu.lib+
/o- $(LIBPATH)\mathl.lib+
/o- $(LIBPATH)\overlay.lib+
/o- $(LIBPATH)\cl.lib+ $(OPTLIB)
| $(LIBS)

!else

$(LIBS): $(MAK)
  copy &&|
/o- $(LIBDIR)\countryl.$(LIBEXT)+
/o- $(LIBDIR)\dos_l.$(LIBEXT)+
/o- $(LIBDIR)\screen_l.$(LIBEXT)+
/o- $(LIBDIR)\pcb_l.$(LIBEXT)+
/o- $(LIBDIR)\misc_l.$(LIBEXT)+
/o- $(LIBDIR)\system_l.$(LIBEXT)+
/o- $(LIBDIR)\\doscls_l.$(LIBEXT)+
/o- $(LIBPATH)\emu.lib+
/o- $(LIBPATH)\mathl.lib+
/o- $(LIBPATH)\overlay.lib+
/o- $(LIBPATH)\cl.lib+ $(OPTLIB)
| $(LIBS)

!endif



