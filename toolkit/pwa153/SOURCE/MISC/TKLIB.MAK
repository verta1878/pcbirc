#=============================================================
#
#       MAKEFILE - MISC category library for the PCBoard toolkit
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
#       out of the shipped MISC_L.386, recorded in
#       attic\prebuilt-libs\BC31\README.md.
#
#       These are compiled but deliberately NOT put in the
#       library -- the programs that need them link them by path from
#       $(SDKOBJ)\<cat>\<model>:
#           SWAP
#
#       Compiler switches live in $(CFG) -- Clark's PCBOARD.CFG and
#       ALL.RES merged into one file, so the BCC command line stays
#       under the DOS 127-character limit.  There is no -DLIB there:
#       that switch empties _FARDATA_ and gives the door-SDK flavour
#       of these modules, which is not what PCBOARD, PCBSETUP and
#       FIDOUTIL link against.
#
#       VIRTUAL is compiled with -DVIRTUAL_HUGE:
#       VIRTUAL.C holds both implementations since the VIRTUAL1 merge.  Clark's
#       MISC_L.386 carries the huge-pointer one (PCBFILER needs it), which
#       is the VIRTUAL_HUGE branch; without the define you get the old
#       VIRTUAL1.  See SOURCE\MISC\VIRTUAL-MERGE.md.
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

SUBDIR   = MISC
LIBNAME  = MISC_L
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

VIRTUAL.obj: VIRTUAL.C
	bcc +$(CFG) -DVIRTUAL_HUGE -n$(OBJDIR) VIRTUAL.C

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
	ADDCHAR.obj \
	ALLDIGIT.obj \
	APPEND.obj \
	ASCII.obj \
	BD_DBLE.obj \
	BD_LONG.obj \
	BINARY.obj \
	BMSEARCH.obj \
	BS_DBLE.obj \
	BS_LONG.obj \
	BUILDSTR.obj \
	CHANGE.obj \
	CHKMOVE.obj \
	COMMA.obj \
	COPYFILE.obj \
	COPYFP.obj \
	CRYPT.obj \
	CTOD.obj \
	DAYOWEEK.obj \
	DBLE_BD.obj \
	DBLE_BS.obj \
	DBLE_PR.obj \
	DBL_LONG.obj \
	DCOMMA.obj \
	DELFILES.obj \
	DIRECTRY.obj \
	DISKFREE.obj \
	DRIVEOK.obj \
	DTOC.obj \
	EDITOR.obj \
	ENDSTR.obj \
	EVALUATE.obj \
	EXIST.obj \
	EXITFUNC.obj \
	FINDFOUR.obj \
	FINDNAME.obj \
	FMEMCPY.obj \
	FMEMSET.obj \
	FULLNAME.obj \
	HEXTOI.obj \
	INDEX.obj \
	ISSET.obj \
	JULIAN.obj \
	LASTCHAR.obj \
	LEFTSTR.obj \
	LONG_BD.obj \
	LONG_BS.obj \
	LONG_DBL.obj \
	LONG_PR.obj \
	MIDSTR.obj \
	MKUNIQUE.obj \
	MOVEFILE.obj \
	MSTRCPY.obj \
	PADSTR.obj \
	PRNREADY.obj \
	PROPER.obj \
	PR_DBLE.obj \
	PR_LONG.obj \
	PSEARCH.obj \
	RIGHTSTR.obj \
	RLE.obj \
	SETBIT.obj \
	SHARE.obj \
	SOUNDEX.obj \
	STRIPA.obj \
	STRIPB.obj \
	STRIPL.obj \
	STRIPR.obj \
	SUBST.obj \
	SWAPENV.obj \
	TIME.obj \
	TIMESTEN.obj \
	TTOC.obj \
	UNSETBIT.obj \
	VALIDATE.obj \
	VALIDSEM.obj \
	VIRTUAL.obj \
	WILDCARD.obj \
	ZSEARCH.obj \
	ZSORT.obj \
	ZSWAPINT.obj \
	ZSWAPLNG.obj \
	ZSWAPSTR.obj \
	ZSWAPVIR.obj \
	SWAP.obj

LIBF:
	-if exist $(LIBFILE) del $(LIBFILE)
	tlib $(LIBFILE) +$(OBJDIR)\ABORT
	tlib $(LIBFILE) +$(OBJDIR)\ADDCHAR
	tlib $(LIBFILE) +$(OBJDIR)\ALLDIGIT
	tlib $(LIBFILE) +$(OBJDIR)\APPEND
	tlib $(LIBFILE) +$(OBJDIR)\ASCII
	tlib $(LIBFILE) +$(OBJDIR)\BD_DBLE
	tlib $(LIBFILE) +$(OBJDIR)\BD_LONG
	tlib $(LIBFILE) +$(OBJDIR)\BINARY
	tlib $(LIBFILE) +$(OBJDIR)\BMSEARCH
	tlib $(LIBFILE) +$(OBJDIR)\BS_DBLE
	tlib $(LIBFILE) +$(OBJDIR)\BS_LONG
	tlib $(LIBFILE) +$(OBJDIR)\BUILDSTR
	tlib $(LIBFILE) +$(OBJDIR)\CHANGE
	tlib $(LIBFILE) +$(OBJDIR)\CHKMOVE
	tlib $(LIBFILE) +$(OBJDIR)\COMMA
	tlib $(LIBFILE) +$(OBJDIR)\COPYFILE
	tlib $(LIBFILE) +$(OBJDIR)\COPYFP
	tlib $(LIBFILE) +$(OBJDIR)\CRYPT
	tlib $(LIBFILE) +$(OBJDIR)\CTOD
	tlib $(LIBFILE) +$(OBJDIR)\DAYOWEEK
	tlib $(LIBFILE) +$(OBJDIR)\DBLE_BD
	tlib $(LIBFILE) +$(OBJDIR)\DBLE_BS
	tlib $(LIBFILE) +$(OBJDIR)\DBLE_PR
	tlib $(LIBFILE) +$(OBJDIR)\DBL_LONG
	tlib $(LIBFILE) +$(OBJDIR)\DCOMMA
	tlib $(LIBFILE) +$(OBJDIR)\DELFILES
	tlib $(LIBFILE) +$(OBJDIR)\DIRECTRY
	tlib $(LIBFILE) +$(OBJDIR)\DISKFREE
	tlib $(LIBFILE) +$(OBJDIR)\DRIVEOK
	tlib $(LIBFILE) +$(OBJDIR)\DTOC
	tlib $(LIBFILE) +$(OBJDIR)\EDITOR
	tlib $(LIBFILE) +$(OBJDIR)\ENDSTR
	tlib $(LIBFILE) +$(OBJDIR)\EVALUATE
	tlib $(LIBFILE) +$(OBJDIR)\EXIST
	tlib $(LIBFILE) +$(OBJDIR)\EXITFUNC
	tlib $(LIBFILE) +$(OBJDIR)\FINDFOUR
	tlib $(LIBFILE) +$(OBJDIR)\FINDNAME
	tlib $(LIBFILE) +$(OBJDIR)\FMEMCPY
	tlib $(LIBFILE) +$(OBJDIR)\FMEMSET
	tlib $(LIBFILE) +$(OBJDIR)\FULLNAME
	tlib $(LIBFILE) +$(OBJDIR)\HEXTOI
	tlib $(LIBFILE) +$(OBJDIR)\INDEX
	tlib $(LIBFILE) +$(OBJDIR)\ISSET
	tlib $(LIBFILE) +$(OBJDIR)\JULIAN
	tlib $(LIBFILE) +$(OBJDIR)\LASTCHAR
	tlib $(LIBFILE) +$(OBJDIR)\LEFTSTR
	tlib $(LIBFILE) +$(OBJDIR)\LONG_BD
	tlib $(LIBFILE) +$(OBJDIR)\LONG_BS
	tlib $(LIBFILE) +$(OBJDIR)\LONG_DBL
	tlib $(LIBFILE) +$(OBJDIR)\LONG_PR
	tlib $(LIBFILE) +$(OBJDIR)\MIDSTR
	tlib $(LIBFILE) +$(OBJDIR)\MKUNIQUE
	tlib $(LIBFILE) +$(OBJDIR)\MOVEFILE
	tlib $(LIBFILE) +$(OBJDIR)\MSTRCPY
	tlib $(LIBFILE) +$(OBJDIR)\PADSTR
	tlib $(LIBFILE) +$(OBJDIR)\PRNREADY
	tlib $(LIBFILE) +$(OBJDIR)\PROPER
	tlib $(LIBFILE) +$(OBJDIR)\PR_DBLE
	tlib $(LIBFILE) +$(OBJDIR)\PR_LONG
	tlib $(LIBFILE) +$(OBJDIR)\PSEARCH
	tlib $(LIBFILE) +$(OBJDIR)\RIGHTSTR
	tlib $(LIBFILE) +$(OBJDIR)\RLE
	tlib $(LIBFILE) +$(OBJDIR)\SETBIT
	tlib $(LIBFILE) +$(OBJDIR)\SHARE
	tlib $(LIBFILE) +$(OBJDIR)\SOUNDEX
	tlib $(LIBFILE) +$(OBJDIR)\STRIPA
	tlib $(LIBFILE) +$(OBJDIR)\STRIPB
	tlib $(LIBFILE) +$(OBJDIR)\STRIPL
	tlib $(LIBFILE) +$(OBJDIR)\STRIPR
	tlib $(LIBFILE) +$(OBJDIR)\SUBST
	tlib $(LIBFILE) +$(OBJDIR)\SWAPENV
	tlib $(LIBFILE) +$(OBJDIR)\TIME
	tlib $(LIBFILE) +$(OBJDIR)\TIMESTEN
	tlib $(LIBFILE) +$(OBJDIR)\TTOC
	tlib $(LIBFILE) +$(OBJDIR)\UNSETBIT
	tlib $(LIBFILE) +$(OBJDIR)\VALIDATE
	tlib $(LIBFILE) +$(OBJDIR)\VALIDSEM
	tlib $(LIBFILE) +$(OBJDIR)\VIRTUAL
	tlib $(LIBFILE) +$(OBJDIR)\WILDCARD
	tlib $(LIBFILE) +$(OBJDIR)\ZSEARCH
	tlib $(LIBFILE) +$(OBJDIR)\ZSORT
	tlib $(LIBFILE) +$(OBJDIR)\ZSWAPINT
	tlib $(LIBFILE) +$(OBJDIR)\ZSWAPLNG
	tlib $(LIBFILE) +$(OBJDIR)\ZSWAPSTR
	tlib $(LIBFILE) +$(OBJDIR)\ZSWAPVIR
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK

CLEAN:
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(OBJDIR)\*.asm del $(OBJDIR)\*.asm
	-if exist $(LIBFILE) del $(LIBFILE)
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK
