#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* PACKFIDO.MAK - Makefile for the PACKFIDO Program                          */
#* pcbirc crew, 2026-09-23.  GPLv3.                                          */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#=============================================================
#
#       PACKFIDO.MAK - Makefile for PACKFIDO Program
#
#       Packs AREAS.DAT, the 15.22-and-later FIDO area file.
#       This is the 15.4 upgrade Clark never shipped; the
#       PACKFIDO.EXE in the install set is a 15.21 tool.
#       See PACKFIDO.DOC.
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = PACKFIDO

ROOT     = \OUT
SOURCE   = .
OBJDIR   = $(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = s

INCLUDEPATH = $(INCLUDE)

#=============================================================
#
#  NO LIBRARIES.  Every other utility in this tree links the
#  category libraries; this one links none, and that is
#  deliberate rather than an omission.
#
#  It opens AREAS.DAT, CNAMES.@@@ and CNAMES.ADD with plain
#  open/read/lseek and takes its three parameters on the
#  command line, so it has no dependency on the toolkit at
#  all.  That means it builds and runs today, on any of the
#  three compilers, without waiting on the small-model
#  category libraries that are still outstanding - and it can
#  be pointed at a COPY of a live AREAS.DAT, which is the only
#  way to test a program that has no shipped binary to be
#  compared against.
#
#  Clark's own PACKFIDO took no arguments: it read PCBOARD.DAT
#  from the current directory and found everything from there.
#  Matching that is a later step and needs the kit - see
#  pcb153\SOURCE\MISC\PACKFIDO\PACKFIDO.MAK, which does link it.
#
#=============================================================

!if $(DEBUG)
CODEOPT=-DDEBUG
!endif

COPT = -c -m$(MDL) -n$(OBJDIR)

!if $d(BC50)
#leave out -Oe due to a bug in Borland C 4.0 thru 5.0
COPT = $(COPT) -Obglmptv
!elif $d(TC30)
#leave out all of the extra -Oxxx switches for TC 3.0 because they aren't valid
!elif $d(BC31)
COPT = $(COPT) -Oebglmptv
!endif

#=============================================================

.PATH.obj = $(OBJDIR)
.PATH.c   = $(SOURCE)

#=============================================================

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

#=============================================================

EXE_DEPENDENCIES = \
  $(OBJDIR)\packfido.obj

#=============================================================

$(OBJDIR)\$(PROGNAME).exe: $(CFG) $(EXE_DEPENDENCIES)
  $(LINKER) /x/c/L$(LIBPATH) @&&|
c0$(MDL).obj+
$(OBJDIR)\packfido.OBJ
$(OBJDIR)\$(PROGNAME)
                # no map file

c$(MDL).lib
|

#=============================================================

$(CFG): $(MAK)
  copy &&|
-f-
-ff-
-C
-K
-G
-O
-Z
-k-
-d
-I$(INCLUDEPATH)
-L$(LIBPATH)
| $(CFG)

#=============================================================
