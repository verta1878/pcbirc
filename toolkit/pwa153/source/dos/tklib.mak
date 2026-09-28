#=============================================================
#
#       MAKEFILE - DOS category library for the PCBoard toolkit
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
#       Memory model:
#           Large (default):  make
#           Small:            make -DMDL=s -DMODEL=SMALL -DCFGNAME=TKS
#
#       Targets:  all (default) | DIRS | OBJS | LIBF | CLEAN
#
#       The module list is Clark's: it matches the module names read
#       out of the shipped DOS_L.386, recorded in
#       attic\prebuilt-libs\BC31\README.md.
#
#       One addition: INT24HND.  It is in Clark's own MAKEFILE but missing
#       from the .386 we were handed, so the shipped copy was short of it.
#
#       These are compiled but deliberately NOT put in the
#       library -- the programs that need them link them by path from
#       $(SDKOBJ)\<cat>\<model>:
#           SHOWERR2
#
#       Compiler switches live in $(CFG) -- Clark's PCBOARD.CFG and
#       ALL.RES merged into one file, so the BCC command line stays
#       under the DOS 127-character limit.  There is no -DLIB there:
#       that switch empties _FARDATA_ and gives the door-SDK flavour
#       of these modules, which is not what PCBOARD, PCBSETUP and
#       FIDOUTIL link against.
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
!ifndef MDL
MDL      = l
!endif
!ifndef CFGNAME
CFGNAME  = TK
!endif
!ifndef TKIT
TKIT     = \TOOLKIT\PWA153
!endif

SUBDIR   = DOS
LIBNAME  = DOS_$(MDL)
CFGDIR   = $(TKIT)\CFG\$(CVER)
CFG      = $(CFGDIR)\$(CFGNAME).CFG
SDK      = $(ROOT)\$(BRANCH)\SDK\$(CVER)
LIBDIR   = $(SDK)\LIB
OBJDIR   = $(SDK)\OBJ\$(SUBDIR)\$(MODEL)
LIBFILE  = $(LIBDIR)\$(LIBNAME).LIB

.c.obj:
	bcc +$(CFG) -n$(OBJDIR) $<

.cpp.obj:
	bcc +$(CFG) -n$(OBJDIR) $<

.asm.obj:
	tasm /mx /d__$(MDL)__ $<, $(OBJDIR)\$&.obj

all: DIRS OBJS LIBF

DIRS:
	-if not exist $(ROOT)\NUL md $(ROOT)
	-if not exist $(ROOT)\$(BRANCH)\NUL md $(ROOT)\$(BRANCH)
	-if not exist $(ROOT)\$(BRANCH)\SDK\NUL md $(ROOT)\$(BRANCH)\SDK
	-if not exist $(SDK)\NUL md $(SDK)
	-if not exist $(LIBDIR)\NUL md $(LIBDIR)
	-if not exist $(SDK)\OBJ\NUL md $(SDK)\OBJ
	-if not exist $(SDK)\OBJ\$(SUBDIR)\NUL md $(SDK)\OBJ\$(SUBDIR)
	-if not exist $(OBJDIR)\NUL md $(OBJDIR)

OBJS: CHKAPPEN.obj \
	CHKCREAT.obj \
	CHKDOSFO.obj \
	CHKFOPEN.obj \
	CHKFPRNT.obj \
	CHKLOCK.obj \
	CHKOPEN.obj \
	CHKREAD.obj \
	CHKUNLNK.obj \
	CHKWRITE.obj \
	DOSAPPEN.obj \
	DOSCLOSE.obj \
	DOSCOMIT.obj \
	DOSCREAT.obj \
	DOSDUP.obj \
	DOSERROR.obj \
	DOSFCLOS.obj \
	DOSFGETS.obj \
	DOSFIND.obj \
	DOSFLUSH.obj \
	DOSFNGTS.obj \
	DOSFOPEN.obj \
	DOSFPUTS.obj \
	DOSFREAD.obj \
	DOSFSEEK.obj \
	DOSFTRUN.obj \
	DOSFUGTS.obj \
	DOSFWRIT.obj \
	DOSLSEEK.obj \
	DOSOPEN.obj \
	DOSREAD.obj \
	DOSREWIN.obj \
	DOSSTBUF.obj \
	DOSTRUNC.obj \
	DOSWRITE.obj \
	EXTENDED.obj \
	GETDRIVE.obj \
	GETPATH.obj \
	HANDLERS.obj \
	ISOPEN.obj \
	SAY.obj \
	SETDRIVE.obj \
	STRNCHR.obj \
	SHOWERR.obj \
	INT24HND.obj \
	SHOWERR2.obj

LIBF:
	-if exist $(LIBFILE) del $(LIBFILE)
	tlib $(LIBFILE) +$(OBJDIR)\CHKAPPEN
	tlib $(LIBFILE) +$(OBJDIR)\CHKCREAT
	tlib $(LIBFILE) +$(OBJDIR)\CHKDOSFO
	tlib $(LIBFILE) +$(OBJDIR)\CHKFOPEN
	tlib $(LIBFILE) +$(OBJDIR)\CHKFPRNT
	tlib $(LIBFILE) +$(OBJDIR)\CHKLOCK
	tlib $(LIBFILE) +$(OBJDIR)\CHKOPEN
	tlib $(LIBFILE) +$(OBJDIR)\CHKREAD
	tlib $(LIBFILE) +$(OBJDIR)\CHKUNLNK
	tlib $(LIBFILE) +$(OBJDIR)\CHKWRITE
	tlib $(LIBFILE) +$(OBJDIR)\DOSAPPEN
	tlib $(LIBFILE) +$(OBJDIR)\DOSCLOSE
	tlib $(LIBFILE) +$(OBJDIR)\DOSCOMIT
	tlib $(LIBFILE) +$(OBJDIR)\DOSCREAT
	tlib $(LIBFILE) +$(OBJDIR)\DOSDUP
	tlib $(LIBFILE) +$(OBJDIR)\DOSERROR
	tlib $(LIBFILE) +$(OBJDIR)\DOSFCLOS
	tlib $(LIBFILE) +$(OBJDIR)\DOSFGETS
	tlib $(LIBFILE) +$(OBJDIR)\DOSFIND
	tlib $(LIBFILE) +$(OBJDIR)\DOSFLUSH
	tlib $(LIBFILE) +$(OBJDIR)\DOSFNGTS
	tlib $(LIBFILE) +$(OBJDIR)\DOSFOPEN
	tlib $(LIBFILE) +$(OBJDIR)\DOSFPUTS
	tlib $(LIBFILE) +$(OBJDIR)\DOSFREAD
	tlib $(LIBFILE) +$(OBJDIR)\DOSFSEEK
	tlib $(LIBFILE) +$(OBJDIR)\DOSFTRUN
	tlib $(LIBFILE) +$(OBJDIR)\DOSFUGTS
	tlib $(LIBFILE) +$(OBJDIR)\DOSFWRIT
	tlib $(LIBFILE) +$(OBJDIR)\DOSLSEEK
	tlib $(LIBFILE) +$(OBJDIR)\DOSOPEN
	tlib $(LIBFILE) +$(OBJDIR)\DOSREAD
	tlib $(LIBFILE) +$(OBJDIR)\DOSREWIN
	tlib $(LIBFILE) +$(OBJDIR)\DOSSTBUF
	tlib $(LIBFILE) +$(OBJDIR)\DOSTRUNC
	tlib $(LIBFILE) +$(OBJDIR)\DOSWRITE
	tlib $(LIBFILE) +$(OBJDIR)\EXTENDED
	tlib $(LIBFILE) +$(OBJDIR)\GETDRIVE
	tlib $(LIBFILE) +$(OBJDIR)\GETPATH
	tlib $(LIBFILE) +$(OBJDIR)\HANDLERS
	tlib $(LIBFILE) +$(OBJDIR)\ISOPEN
	tlib $(LIBFILE) +$(OBJDIR)\SAY
	tlib $(LIBFILE) +$(OBJDIR)\SETDRIVE
	tlib $(LIBFILE) +$(OBJDIR)\STRNCHR
	tlib $(LIBFILE) +$(OBJDIR)\SHOWERR
	tlib $(LIBFILE) +$(OBJDIR)\INT24HND
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK

CLEAN:
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.asm del $(OBJDIR)\*.asm
	-if exist $(LIBFILE) del $(LIBFILE)
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK
