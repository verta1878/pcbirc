#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* packfido.mak - openwatcom makefile for the packfido program               */
#* pcbirc crew, 2026-09-23.  gplv3.                                          */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#
#       wmake -f packfido.mak              16-bit dos      packfido.exe
#       wmake -f packfido.mak os2          32-bit os/2     packfido2.exe
#       wmake -f packfido.mak dos32        32-bit dos/4g   packf32.exe
#       wmake -f packfido.mak all
#       wmake -f packfido.mak clean
#
# needs watcom set and $(%watcom)/binl64 (or binnt) on the path.
#
# the borland build of the same source lives in the 15.3 upgrade tree,
# pcb153\upd154\source\misc\packfido\packfido.mak.  one source, both
# compilers - and their output over the same areas.dat is byte identical,
# which is the only cross-check this program can have, because no shipped
# binary ever packed the version 3 format.  see packfido.doc.
#
# -d__os2__ is not optional on the os/2 target
# watcom does not predefine __os2__ for -bt=os2v2; it defines __os2v2__.
# borland c++ for os/2 defines __os2__, and this repo's build_os2_ow.sh
# passes -d__os2__ by hand for exactly that reason.  the source accepts
# either macro, so it is right whichever way it is built - but getting it
# wrong is silent: the os/2 build just compiles the dos path and nobody
# notices.  it happened once here, and the only tell was the binary coming
# out the same size as before the port.
#
# why -zp1 is not in the flags
# the record layout is not read through a struct.  narea_struct is declared
# so the compiler can confirm it packs to 81 bytes, and the pack loop then
# works on raw bytes at fixed offsets.  nothing depends on the default
# structure alignment, so nothing here needs -zp1 - and a build that
# forgot it would still be correct rather than silently wrong.
#
# why the 32-bit targets are safe
# "unsigned int" is 4 bytes under wcc386.  every field that is two bytes in
# areas.dat is declared pcbword (unsigned short) for exactly that reason,
# and main() refuses to run if pcbword is not 2 bytes.  the compiler reports
# that guard as unreachable code on all three targets, which is it saying
# the check holds.

prog    = packfido
src     = $(prog).c

cc16    = wcc
cc32    = wcc386
link    = wlink

# -ox full optimization, -zq quiet, -w4 all warnings, -bt target
cf16    = -bt=dos    -ml -ox -zq -w4
cfos2   = -bt=os2v2  -mf -ox -zq -w4 -d__os2__
cf32    = -bt=dos    -mf -ox -zq -w4

#=============================================================

dos16 : $(prog).exe  .symbolic

all : dos16 os2 dos32  .symbolic

$(prog).exe : $(prog).obj
  $(link) system dos option quiet name $(prog).exe file $(prog).obj

$(prog).obj : $(src)
  $(cc16) $(cf16) -fo=$(prog).obj $(src)

os2 : $(prog)2.exe  .symbolic

$(prog)2.exe : pfos2.obj
  $(link) system os2v2 option quiet name $(prog)2.exe file pfos2.obj

pfos2.obj : $(src)
  $(cc32) $(cfos2) -fo=pfos2.obj $(src)

dos32 : packf32.exe  .symbolic

packf32.exe : pf32.obj
  $(link) system dos4g option quiet name packf32.exe file pf32.obj

pf32.obj : $(src)
  $(cc32) $(cf32) -fo=pf32.obj $(src)

clean : .symbolic
  @if exist $(prog).obj del $(prog).obj
  @if exist pfos2.obj   del pfos2.obj
  @if exist pf32.obj    del pf32.obj
  @if exist $(prog).exe del $(prog).exe
  @if exist $(prog)2.exe del $(prog)2.exe
  @if exist packf32.exe del packf32.exe

#=============================================================
# verified 2026-09-23, open watcom 2.0 beta:
#
#   wcc    -bt=dos   -ml   packfido.exe   19,878 bytes   c runtime
#   wcc386 -bt=os2v2 -mf   packfido2.exe  16,554 bytes   native dosxxx
#   wcc386 -bt=dos   -mf   packf32.exe    26,262 bytes   c runtime
#
#   the os/2 binary is the smallest of the three because it calls dosopen,
#   dosread, doswrite, dossetfileptr, dosclose, dosdelete and dosmove
#   directly and never links the c file i/o runtime at all.  it was 19,763
#   bytes before the port - that drop is the evidence the port took.
#
#   0 errors.  2 warnings on every target, both w201 "unreachable code" -
#   the two sizeof guards.  that is the result, not a defect.
#
#   the 16-bit dos binary and the borland c++ 3.1 binary were run over
#   identical copies of clark's own areas.dat:
#
#     684 records in, 683 out, conference 5 removed
#     48,495 bytes
#     ab6fd621afbcdd3673b7856bcb84e3c923116a72552d1557042eadd6a4cd6a4b
#
#   from both, with identical console output.
