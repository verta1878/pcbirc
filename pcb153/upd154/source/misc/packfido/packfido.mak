#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* packfido.mak - makefile for the packfido program                          */
#* pcbirc crew, 2026-09-23.  gplv3.                                          */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#=============================================================
#
#       packfido.mak - makefile for packfido program
#
#       packs areas.dat, the 15.22-and-later fido area file.
#       this is the 15.4 upgrade clark never shipped; the
#       packfido.exe in the install set is a 15.21 tool.
#       see packfido.doc.
#
#=============================================================

.silent
.autodepend

progname = packfido

root     = \out
source   = .
objdir   = $(bccompiler)

cfg      = $(progname).cfg
mak      = $(progname).mak

mdl      = s

includepath = $(include)

#=============================================================
#
#  no libraries.  every other utility in this tree links the
#  category libraries; this one links none, and that is
#  deliberate rather than an omission.
#
#  it opens areas.dat, cnames.@@@ and cnames.add with plain
#  open/read/lseek and takes its three parameters on the
#  command line, so it has no dependency on the toolkit at
#  all.  that means it builds and runs today, on any of the
#  three compilers, without waiting on the small-model
#  category libraries that are still outstanding - and it can
#  be pointed at a copy of a live areas.dat, which is the only
#  way to test a program that has no shipped binary to be
#  compared against.
#
#  clark's own packfido took no arguments: it read pcboard.dat
#  from the current directory and found everything from there.
#  matching that is a later step and needs the kit - see
#  pcb153\source\misc\packfido\packfido.mak, which does link it.
#
#=============================================================

!if $(debug)
codeopt=-ddebug
!endif

copt = -c -m$(mdl) -n$(objdir)

!if $d(bc50)
#leave out -oe due to a bug in borland c 4.0 thru 5.0
copt = $(copt) -obglmptv
!elif $d(tc30)
#leave out all of the extra -oxxx switches for tc 3.0 because they aren't valid
!elif $d(bc31)
copt = $(copt) -oebglmptv
!endif

#=============================================================

.path.obj = $(objdir)
.path.c   = $(source)

#=============================================================

.c.obj:
  $(compiler) +$(cfg) $(copt) $(codeopt) {$< }

#=============================================================

exe_dependencies = \
  $(objdir)\packfido.obj

#=============================================================

$(objdir)\$(progname).exe: $(cfg) $(exe_dependencies)
  $(linker) /x/c/l$(libpath) @&&|
c0$(mdl).obj+
$(objdir)\packfido.obj
$(objdir)\$(progname)
                # no map file

c$(mdl).lib
|

#=============================================================

$(cfg): $(mak)
  copy &&|
-f-
-ff-
-c
-k
-g
-o
-z
-k-
-d
-i$(includepath)
-l$(libpath)
| $(cfg)

#=============================================================
