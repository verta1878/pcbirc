#=============================================================
#
#       MAKEFILE - VMDATA library for the PCBoard toolkit
#
#       Builds  $(LIBFILE)  from the sources in this folder.
#       Objects go to $(OBJDIR) and are kept, so a second MAKE only
#       recompiles what changed.  MAKE CLEAN removes both.
#
#       WHY THIS FILE EXISTS
#       --------------------
#       It did not, until 2026-09-23.  VMAVL.C and VMFUNCS.C sat
#       loose at SOURCE\ root, in no category folder, and
#       VMDATA_L.LIB was built BY HAND with bcc and tlib on
#       2026-09-22.  The library was in CHECKSUMS.sha256, linked by
#       PCBSM and MAKEIDX, verified -- and unreproducible.  BLDTK
#       built nine libraries; the tree needs ten.
#
#       NOT CLARK'S CODE.  Clark's virtual-memory library shipped as
#       \LIBS\VMDATA\BC31_DOS\VMDATA.LIB, which this repo has never
#       had.  VMFUNCS.C is a crew reimplementation ("Written by:
#       hexadecimal") and VMAVL.C is the AVL tree beside it.  See
#       MAIN\build\VMDATA-RECONSTRUCTION.md for the one-based index
#       contract and the seven no-ops that go live only when a
#       paging store exists.
#
#       Every macro is guarded, so it can be overridden:
#           make -DCVER=TC201 -DMODEL=SMALL
#
#       Targets:  all (default) | DIRS | OBJS | LIBF | CLEAN
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

SUBDIR   = VMDATA
LIBNAME  = VMDATA_L
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

OBJS: VMAVL.obj \
	VMFUNCS.obj

LIBF:
	-if exist $(LIBFILE) del $(LIBFILE)
	tlib $(LIBFILE) +$(OBJDIR)\VMAVL
	tlib $(LIBFILE) +$(OBJDIR)\VMFUNCS
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK

CLEAN:
	-if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
	-if exist $(LIBFILE) del $(LIBFILE)
	-if exist $(LIBDIR)\$(LIBNAME).BAK del $(LIBDIR)\$(LIBNAME).BAK
