#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* PACKFIDO.MAK - OpenWatcom makefile for the PACKFIDO Program               */
#* pcbirc crew, 2026-09-23.  GPLv3.                                          */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#
#       wmake -f PACKFIDO.MAK              16-bit DOS      PACKFIDO.EXE
#       wmake -f PACKFIDO.MAK os2          32-bit OS/2     PACKFIDO2.EXE
#       wmake -f PACKFIDO.MAK dos32        32-bit DOS/4G   PACKF32.EXE
#       wmake -f PACKFIDO.MAK all
#       wmake -f PACKFIDO.MAK clean
#
# Needs WATCOM set and $(%WATCOM)/binl64 (or BINNT) on the PATH.
#
# The Borland build of the same source lives in the 15.3 upgrade tree,
# pcb153\upd154\SOURCE\MISC\PACKFIDO\PACKFIDO.MAK.  ONE SOURCE, both
# compilers - and their output over the same AREAS.DAT is byte identical,
# which is the only cross-check this program can have, because no shipped
# binary ever packed the version 3 format.  See PACKFIDO.DOC.
#
# -d__OS2__ IS NOT OPTIONAL ON THE OS/2 TARGET
# Watcom does NOT predefine __OS2__ for -bt=os2v2; it defines __OS2V2__.
# Borland C++ for OS/2 defines __OS2__, and this repo's BUILD_OS2_OW.SH
# passes -d__OS2__ by hand for exactly that reason.  The source accepts
# either macro, so it is right whichever way it is built - but getting it
# wrong is SILENT: the OS/2 build just compiles the DOS path and nobody
# notices.  It happened once here, and the only tell was the binary coming
# out the same size as before the port.
#
# WHY -zp1 IS NOT IN THE FLAGS
# The record layout is not read through a struct.  NAREA_STRUCT is declared
# so the compiler can confirm it packs to 81 bytes, and the pack loop then
# works on raw bytes at fixed offsets.  Nothing depends on the default
# structure alignment, so nothing here needs -zp1 - and a build that
# forgot it would still be correct rather than silently wrong.
#
# WHY THE 32-BIT TARGETS ARE SAFE
# "unsigned int" is 4 bytes under wcc386.  Every field that is two bytes in
# AREAS.DAT is declared pcbword (unsigned short) for exactly that reason,
# and main() refuses to run if pcbword is not 2 bytes.  The compiler reports
# that guard as unreachable code on all three targets, which is it saying
# the check holds.

PROG    = PACKFIDO
SRC     = $(PROG).C

CC16    = wcc
CC32    = wcc386
LINK    = wlink

# -ox full optimization, -zq quiet, -w4 all warnings, -bt target
CF16    = -bt=dos    -ml -ox -zq -w4
CFOS2   = -bt=os2v2  -mf -ox -zq -w4 -d__OS2__
CF32    = -bt=dos    -mf -ox -zq -w4

#=============================================================

dos16 : $(PROG).EXE  .SYMBOLIC

all : dos16 os2 dos32  .SYMBOLIC

$(PROG).EXE : $(PROG).obj
  $(LINK) system dos option quiet name $(PROG).EXE file $(PROG).obj

$(PROG).obj : $(SRC)
  $(CC16) $(CF16) -fo=$(PROG).obj $(SRC)

os2 : $(PROG)2.EXE  .SYMBOLIC

$(PROG)2.EXE : pfos2.obj
  $(LINK) system os2v2 option quiet name $(PROG)2.EXE file pfos2.obj

pfos2.obj : $(SRC)
  $(CC32) $(CFOS2) -fo=pfos2.obj $(SRC)

dos32 : PACKF32.EXE  .SYMBOLIC

PACKF32.EXE : pf32.obj
  $(LINK) system dos4g option quiet name PACKF32.EXE file pf32.obj

pf32.obj : $(SRC)
  $(CC32) $(CF32) -fo=pf32.obj $(SRC)

clean : .SYMBOLIC
  @if exist $(PROG).obj del $(PROG).obj
  @if exist pfos2.obj   del pfos2.obj
  @if exist pf32.obj    del pf32.obj
  @if exist $(PROG).EXE del $(PROG).EXE
  @if exist $(PROG)2.EXE del $(PROG)2.EXE
  @if exist PACKF32.EXE del PACKF32.EXE

#=============================================================
# Verified 2026-09-23, Open Watcom 2.0 beta:
#
#   wcc    -bt=dos   -ml   PACKFIDO.EXE   19,878 bytes   C runtime
#   wcc386 -bt=os2v2 -mf   PACKFIDO2.EXE  16,554 bytes   native DosXxx
#   wcc386 -bt=dos   -mf   PACKF32.EXE    26,262 bytes   C runtime
#
#   The OS/2 binary is the SMALLEST of the three because it calls DosOpen,
#   DosRead, DosWrite, DosSetFilePtr, DosClose, DosDelete and DosMove
#   directly and never links the C file I/O runtime at all.  It was 19,763
#   bytes before the port - that drop is the evidence the port took.
#
#   0 errors.  2 warnings on every target, both W201 "Unreachable code" -
#   the two sizeof guards.  That is the result, not a defect.
#
#   The 16-bit DOS binary and the Borland C++ 3.1 binary were run over
#   identical copies of Clark's own AREAS.DAT:
#
#     684 records in, 683 out, conference 5 removed
#     48,495 bytes
#     ab6fd621afbcdd3673b7856bcb84e3c923116a72552d1557042eadd6a4cd6a4b
#
#   from BOTH, with identical console output.
