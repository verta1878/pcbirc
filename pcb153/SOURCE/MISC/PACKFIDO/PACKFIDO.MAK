#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* PACKFIDO.MAK - build PACKFIDO.EXE, aiming at Clark's shipped binary.      */
#* pcbirc crew, 2026-09-23.  GPLv3.                                          */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#
# Modelled on Clark's own utility makefile, reference\pcball\pcboard\
# pcb-util\PCBTEXT\MKPCBTXT.MAK - same switches, same link shape, same
# small model.  The target is
#
#     pcb1541\install\dist\target\PACKFIDO.EXE   23,214 bytes
#     sha256 ef584fc957a28d051a7c40937ed5eed916dfd043bc9ae7ca3f2996fc6a8f63a8
#
# byte for byte.  Anything else is not done.
#
# MDL = s because the shipped binary has 8 relocations and a 512-byte
# header - small model.  MKPCBTXT.EXE, which Clark built with MDL=s, has 10.
#
# -P compiles as C++.  It is not optional: the kit libraries are C++
# objects, and a C compile leaves every kit symbol undefined.
#
# STILL MISSING: the small-model category libraries.  This links against
# PCBKBC$(MDL).LIB, which in turn needs MISC_$(MDL) and DOS_$(MDL) for
# retrycount(), findstartofname(), _int23hnd and _int24hnd.  Only the
# LARGE-model category libraries are built today.

.SILENT
.AUTODEPEND

PROGNAME = PACKFIDO
MDL      = s

!if !$d(ROOT)
ROOT     = \
!endif

LIBH     = $(ROOT)toolkit\pwa153\H
LIBLIB   = $(ROOT)toolkit\pwa153\bc31\lib

!if !$d(BC31PATH)
BC31PATH = \BC31
!endif

COMPILER = $(BC31PATH)\BIN\BCC.EXE
LINKER   = $(BC31PATH)\BIN\TLINK.EXE

INCLUDEPATH = $(BC31PATH)\INCLUDE;$(LIBH)
LIBPATH     = $(BC31PATH)\LIB

# Clark's switches, verbatim from MKPCBTXT.MAK / MKPCBTXT.CFG.
COPT = -c -m$(MDL) -P -Oebglmptv -f- -ff- -C -K -G -O -Z -k- -d

#=============================================================

all: $(PROGNAME).exe

$(PROGNAME).obj: $(PROGNAME).C
  $(COMPILER) $(COPT) -I$(INCLUDEPATH) $(PROGNAME).C

$(PROGNAME).exe: $(PROGNAME).obj
  $(LINKER) /x/c/L$(LIBPATH) @&&|
$(LIBPATH)\c0$(MDL).obj+
$(PROGNAME).obj
$(PROGNAME).exe
                # no map file
$(LIBLIB)\PCBKBC$(MDL).LIB+
$(LIBLIB)\MISC_$(MDL).LIB+
$(LIBLIB)\DOS_$(MDL).LIB+
$(LIBPATH)\c$(MDL).lib
|

clean:
  if exist $(PROGNAME).obj del $(PROGNAME).obj
  if exist $(PROGNAME).exe del $(PROGNAME).exe

#=============================================================
# Verify, once it links:
#
#   the only test is the sha256 against Clark's binary.  There is no
#   behavioural harness here and there should not be one - if the bytes
#   match, behaviour matches by construction.
