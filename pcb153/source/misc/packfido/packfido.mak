#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* packfido.mak - build packfido.exe, aiming at clark's shipped binary.      */
#* pcbirc crew, 2026-09-23.  gplv3.                                          */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#
# modelled on clark's own utility makefile, reference\pcball\pcboard\
# pcb-util\pcbtext\mkpcbtxt.mak - same switches, same link shape, same
# small model.  the target is
#
#     pcb1541\install\dist\target\packfido.exe   23,214 bytes
#     sha256 ef584fc957a28d051a7c40937ed5eed916dfd043bc9ae7ca3f2996fc6a8f63a8
#
# byte for byte.  anything else is not done.
#
# mdl = s because the shipped binary has 8 relocations and a 512-byte
# header - small model.  mkpcbtxt.exe, which clark built with mdl=s, has 10.
#
# -p compiles as c++.  it is not optional: the kit libraries are c++
# objects, and a c compile leaves every kit symbol undefined.
#
# still missing: the small-model category libraries.  this links against
# pcbkbc$(mdl).lib, which in turn needs misc_$(mdl) and dos_$(mdl) for
# retrycount(), findstartofname(), _int23hnd and _int24hnd.  only the
# large-model category libraries are built today.

.silent
.autodepend

progname = packfido
mdl      = s

!if !$d(root)
root     = \
!endif

libh     = $(root)toolkit\pwa153\h
liblib   = $(root)toolkit\pwa153\bc31\lib

!if !$d(bc31path)
bc31path = \bc31
!endif

compiler = $(bc31path)\bin\bcc.exe
linker   = $(bc31path)\bin\tlink.exe

includepath = $(bc31path)\include;$(libh)
libpath     = $(bc31path)\lib

# clark's switches, verbatim from mkpcbtxt.mak / mkpcbtxt.cfg.
copt = -c -m$(mdl) -p -oebglmptv -f- -ff- -c -k -g -o -z -k- -d

#=============================================================

all: $(progname).exe

$(progname).obj: $(progname).c
  $(compiler) $(copt) -i$(includepath) $(progname).c

$(progname).exe: $(progname).obj
  $(linker) /x/c/l$(libpath) @&&|
$(libpath)\c0$(mdl).obj+
$(progname).obj
$(progname).exe
                # no map file
$(liblib)\pcbkbc$(mdl).lib+
$(liblib)\misc_$(mdl).lib+
$(liblib)\dos_$(mdl).lib+
$(libpath)\c$(mdl).lib
|

clean:
  if exist $(progname).obj del $(progname).obj
  if exist $(progname).exe del $(progname).exe

#=============================================================
# verify, once it links:
#
#   the only test is the sha256 against clark's binary.  there is no
#   behavioural harness here and there should not be one - if the bytes
#   match, behaviour matches by construction.
