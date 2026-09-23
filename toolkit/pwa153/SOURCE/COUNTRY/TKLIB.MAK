#=============================================================
#
#       MAKEFILE - COUNTRY category library for the PCBoard toolkit
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
#       out of the shipped COUNTRYL.386, recorded in
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

SUBDIR   = COUNTRY
LIBNAME  = COUNTRYL
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

OBJS: COUNTRY.obj \
	DCOMMA.obj \
	DATE.obj

LIBF:
	-if exist $(LIBFILE) del $(LIBFILE)
	tlib $(LIBFILE) +$(OBJDIR)\COUNTRY
	tlib $(LIBFILE) +$(OBJDIR)\DCOMMA
	tlib $(LIBFILE) +$(OBJDIR)\DATE
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK

CLEAN:
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.asm del $(OBJDIR)\*.asm
	-if exist $(LIBFILE) del $(LIBFILE)
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK
