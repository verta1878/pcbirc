#=============================================================
#
#       MAKEFILE - PCB category library for the PCBoard toolkit
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
#       out of the shipped PCB_L.386, recorded in
#       attic\prebuilt-libs\BC31\README.md.
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
!ifndef TKIT
TKIT     = \TOOLKIT\PWA153
!endif

SUBDIR   = PCB
LIBNAME  = PCB_L
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

DIRS:
	-if not exist $(ROOT)\NUL md $(ROOT)
	-if not exist $(ROOT)\$(BRANCH)\NUL md $(ROOT)\$(BRANCH)
	-if not exist $(ROOT)\$(BRANCH)\SDK\NUL md $(ROOT)\$(BRANCH)\SDK
	-if not exist $(SDK)\NUL md $(SDK)
	-if not exist $(LIBDIR)\NUL md $(LIBDIR)
	-if not exist $(SDK)\OBJ\NUL md $(SDK)\OBJ
	-if not exist $(SDK)\OBJ\$(SUBDIR)\NUL md $(SDK)\OBJ\$(SUBDIR)
	-if not exist $(OBJDIR)\NUL md $(OBJDIR)

OBJS: ABORT.obj \
	ACCOUNT.obj \
	ADDBACKS.obj \
	ALLOW.obj \
	CHKEXIST.obj \
	CI_OTHER.obj \
	CNAMES.obj \
	CONFFUNC.obj \
	CONFIG.obj \
	DATA120.obj \
	DATADFLT.obj \
	DATAFIL2.obj \
	DATAFILE.obj \
	DATAREAD.obj \
	DATAWRIT.obj \
	EXITDOS.obj \
	GETDAYS.obj \
	PARSE.obj \
	PARSEPTH.obj \
	SAVETEXT.obj \
	SRCHPATH.obj \
	TEXT.obj

LIBF:
	-if exist $(LIBFILE) del $(LIBFILE)
	tlib $(LIBFILE) +$(OBJDIR)\ABORT
	tlib $(LIBFILE) +$(OBJDIR)\ACCOUNT
	tlib $(LIBFILE) +$(OBJDIR)\ADDBACKS
	tlib $(LIBFILE) +$(OBJDIR)\ALLOW
	tlib $(LIBFILE) +$(OBJDIR)\CHKEXIST
	tlib $(LIBFILE) +$(OBJDIR)\CI_OTHER
	tlib $(LIBFILE) +$(OBJDIR)\CNAMES
	tlib $(LIBFILE) +$(OBJDIR)\CONFFUNC
	tlib $(LIBFILE) +$(OBJDIR)\CONFIG
	tlib $(LIBFILE) +$(OBJDIR)\DATA120
	tlib $(LIBFILE) +$(OBJDIR)\DATADFLT
	tlib $(LIBFILE) +$(OBJDIR)\DATAFIL2
	tlib $(LIBFILE) +$(OBJDIR)\DATAFILE
	tlib $(LIBFILE) +$(OBJDIR)\DATAREAD
	tlib $(LIBFILE) +$(OBJDIR)\DATAWRIT
	tlib $(LIBFILE) +$(OBJDIR)\EXITDOS
	tlib $(LIBFILE) +$(OBJDIR)\GETDAYS
	tlib $(LIBFILE) +$(OBJDIR)\PARSE
	tlib $(LIBFILE) +$(OBJDIR)\PARSEPTH
	tlib $(LIBFILE) +$(OBJDIR)\SAVETEXT
	tlib $(LIBFILE) +$(OBJDIR)\SRCHPATH
	tlib $(LIBFILE) +$(OBJDIR)\TEXT
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK

CLEAN:
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.asm del $(OBJDIR)\*.asm
	-if exist $(LIBFILE) del $(LIBFILE)
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK
