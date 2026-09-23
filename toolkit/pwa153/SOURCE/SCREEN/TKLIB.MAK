#=============================================================
#
#       MAKEFILE - SCREEN category library for the PCBoard toolkit
#
#       Builds  $(LIBFILE)  from the sources in this folder.
#       Objects go to $(OBJDIR) and are kept, so a second MAKE only
#       recompiles what changed.  MAKE CLEAN removes both.
#
#       The repo folder is mounted as the drive root, so \OUT,
#       \TOOLKIT, \PCB153 and \BC31 are inside the repo no matter
#       what the repo folder is called.  Every macro is guarded, so
#       it can be overridden on the command line:
#
#           make -DCVER=BC50
#           make -DROOT=\OUT -DBRANCH=PWA154
#
#       Targets:  all (default) | DIRS | OBJS | LIBF | CLEAN
#
#       The module list is Clark's: it matches the module names read
#       out of the shipped SCREEN_L.386, recorded in
#       attic\prebuilt-libs\BC31\README.md.
#
#       Compiler switches live in $(CFG) -- Clark's PCBOARD.CFG and
#       ALL.RES merged into one file, so the BCC command line stays
#       under the DOS 127-character limit.  There is no -DLIB there:
#       that switch empties _FARDATA_ and gives the door-SDK flavour
#       of these modules, which is not what PCBOARD, PCBSETUP and
#       FIDOUTIL link against.
#
#       SCROLLUP is compiled with -B:
#       -B routes this one file through TASM.  Its inline asm jumps to a local
#       label called "exit", which BCC's built-in assembler resolves to the
#       library function exit() and then rejects.
#
#=============================================================

!ifndef ROOT
ROOT     = \OUT
!endif
!ifndef BRANCH
BRANCH   = PWA153
!endif
!ifndef CVER
CVER     = BC31
!endif
!ifndef MODEL
MODEL    = LARGE
!endif
!ifndef TKIT
TKIT     = \TOOLKIT\PWA153
!endif

SUBDIR   = SCREEN
LIBNAME  = SCREEN_L
CFGDIR   = $(TKIT)\CFG\$(CVER)
CFG      = $(CFGDIR)\TK.CFG
SDK      = $(ROOT)\$(BRANCH)\SDK\$(CVER)
LIBDIR   = $(SDK)\LIB
OBJDIR   = $(SDK)\OBJ\$(SUBDIR)\$(MODEL)
LIBFILE  = $(LIBDIR)\$(LIBNAME).LIB

.c.obj:
	bcc +$(CFG) -n$(OBJDIR) $<

.cpp.obj:
	bcc +$(CFG) -n$(OBJDIR) $<

.asm.obj:
	tasm /mx /d__l__ $<, $(OBJDIR)\$&.obj

all: DIRS OBJS LIBF

SCROLLUP.obj: SCROLLUP.C
	bcc +$(CFG) -B -n$(OBJDIR) SCROLLUP.C

DIRS:
	-if not exist $(ROOT)\NUL md $(ROOT)
	-if not exist $(ROOT)\$(BRANCH)\NUL md $(ROOT)\$(BRANCH)
	-if not exist $(ROOT)\$(BRANCH)\SDK\NUL md $(ROOT)\$(BRANCH)\SDK
	-if not exist $(SDK)\NUL md $(SDK)
	-if not exist $(LIBDIR)\NUL md $(LIBDIR)
	-if not exist $(SDK)\OBJ\NUL md $(SDK)\OBJ
	-if not exist $(SDK)\OBJ\$(SUBDIR)\NUL md $(SDK)\OBJ\$(SUBDIR)
	-if not exist $(OBJDIR)\NUL md $(OBJDIR)

OBJS: ANSI.obj \
	BOX.obj \
	BOXCLS.obj \
	CLS.obj \
	CLSBOX.obj \
	CLSCOLOR.obj \
	CURSOR.obj \
	DATESTR.obj \
	DELAY.obj \
	DELETE.obj \
	FASTPUTC.obj \
	GETMODE.obj \
	GETSEC.obj \
	GIVEUP.obj \
	GOTOXY.obj \
	GROWBOX.obj \
	INSERT.obj \
	PRINT.obj \
	PRINTV.obj \
	PRNTCNTR.obj \
	PRNTMOVE.obj \
	READSCR2.obj \
	READSCRN.obj \
	SAVEREST.obj \
	SAVERST2.obj \
	SCROLLDN.obj \
	SCROLLUP.obj \
	SETATT.obj \
	SETFONT.obj \
	SETROWS.obj \
	SOUND.obj \
	TIME1.obj \
	TIME2.obj \
	TIMECHNG.obj \
	TWODIG.obj \
	TWODIG0.obj \
	WHEREX.obj \
	WHEREY.obj \
	WINDOW.obj

LIBF:
	-if exist $(LIBFILE) del $(LIBFILE)
	tlib $(LIBFILE) +$(OBJDIR)\ANSI
	tlib $(LIBFILE) +$(OBJDIR)\BOX
	tlib $(LIBFILE) +$(OBJDIR)\BOXCLS
	tlib $(LIBFILE) +$(OBJDIR)\CLS
	tlib $(LIBFILE) +$(OBJDIR)\CLSBOX
	tlib $(LIBFILE) +$(OBJDIR)\CLSCOLOR
	tlib $(LIBFILE) +$(OBJDIR)\CURSOR
	tlib $(LIBFILE) +$(OBJDIR)\DATESTR
	tlib $(LIBFILE) +$(OBJDIR)\DELAY
	tlib $(LIBFILE) +$(OBJDIR)\DELETE
	tlib $(LIBFILE) +$(OBJDIR)\FASTPUTC
	tlib $(LIBFILE) +$(OBJDIR)\GETMODE
	tlib $(LIBFILE) +$(OBJDIR)\GETSEC
	tlib $(LIBFILE) +$(OBJDIR)\GIVEUP
	tlib $(LIBFILE) +$(OBJDIR)\GOTOXY
	tlib $(LIBFILE) +$(OBJDIR)\GROWBOX
	tlib $(LIBFILE) +$(OBJDIR)\INSERT
	tlib $(LIBFILE) +$(OBJDIR)\PRINT
	tlib $(LIBFILE) +$(OBJDIR)\PRINTV
	tlib $(LIBFILE) +$(OBJDIR)\PRNTCNTR
	tlib $(LIBFILE) +$(OBJDIR)\PRNTMOVE
	tlib $(LIBFILE) +$(OBJDIR)\READSCR2
	tlib $(LIBFILE) +$(OBJDIR)\READSCRN
	tlib $(LIBFILE) +$(OBJDIR)\SAVEREST
	tlib $(LIBFILE) +$(OBJDIR)\SAVERST2
	tlib $(LIBFILE) +$(OBJDIR)\SCROLLDN
	tlib $(LIBFILE) +$(OBJDIR)\SCROLLUP
	tlib $(LIBFILE) +$(OBJDIR)\SETATT
	tlib $(LIBFILE) +$(OBJDIR)\SETFONT
	tlib $(LIBFILE) +$(OBJDIR)\SETROWS
	tlib $(LIBFILE) +$(OBJDIR)\SOUND
	tlib $(LIBFILE) +$(OBJDIR)\TIME1
	tlib $(LIBFILE) +$(OBJDIR)\TIME2
	tlib $(LIBFILE) +$(OBJDIR)\TIMECHNG
	tlib $(LIBFILE) +$(OBJDIR)\TWODIG
	tlib $(LIBFILE) +$(OBJDIR)\TWODIG0
	tlib $(LIBFILE) +$(OBJDIR)\WHEREX
	tlib $(LIBFILE) +$(OBJDIR)\WHEREY
	tlib $(LIBFILE) +$(OBJDIR)\WINDOW
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK

CLEAN:
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.asm del $(OBJDIR)\*.asm
	-if exist $(LIBFILE) del $(LIBFILE)
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK
