#=============================================================
#
#       makefile - vmdata library for the pcboard toolkit
#
#       builds  $(libfile)  from the sources in this folder.
#       objects go to $(objdir) and are kept, so a second make only
#       recompiles what changed.  make clean removes both.
#
#       why this file exists
#       --------------------
#       it did not, until 2026-09-23.  vmavl.c and vmfuncs.c sat
#       loose at source\ root, in no category folder, and
#       vmdata_l.lib was built by hand with bcc and tlib on
#       2026-09-22.  the library was in checksums.sha256, linked by
#       pcbsm and makeidx, verified -- and unreproducible.  bldtk
#       built nine libraries; the tree needs ten.
#
#       not clark's code.  clark's virtual-memory library shipped as
#       \libs\vmdata\bc31_dos\vmdata.lib, which this repo has never
#       had.  vmfuncs.c is a crew reimplementation ("written by:
#       hexadecimal") and vmavl.c is the avl tree beside it.  see
#       main\build\vmdata-reconstruction.md for the one-based index
#       contract and the seven no-ops that go live only when a
#       paging store exists.
#
#       every macro is guarded, so it can be overridden:
#           make -Dcver=tc201 -Dmodel=small
#
#       memory model:
#           large (default):  make
#           small:            make -Dmdl=s -Dmodel=small -Dcfgname=tks
#
#       targets:  all (default) | dirs | objs | libf | clean
#
#=============================================================

!ifndef root
root     = \out
!endif
!ifndef branch
branch   = pwa153
!endif
!ifndef cver
cver     = bc31
!endif
!ifndef model
model    = large
!endif
!ifndef mdl
mdl      = l
!endif
!ifndef cfgname
cfgname  = tk
!endif
!ifndef tkit
tkit     = \toolkit\pwa153
!endif
# tkcc: compiler command — bcc (bc31 default) or tcc (tc201)
!ifndef tkcc
tkcc     = bcc
!endif
# cfgflag: config-file switch — +$(cfg) for bcc, empty for tcc (reads turboc.cfg)
!if $(tkcc) == tcc
cfgflag  =
!else
cfgflag  = +$(cfg)
!endif

subdir   = vmdata
libname  = vmdata_$(mdl)
cfgdir   = $(tkit)\cfg\$(cver)
cfg      = $(cfgdir)\$(cfgname).cfg
sdk      = $(root)\$(branch)\sdk\$(cver)
libdir   = $(sdk)\lib
objdir   = $(sdk)\obj\$(subdir)\$(model)
libfile  = $(libdir)\$(libname).lib

.c.obj:
	$(tkcc) $(cfgflag) -m$(mdl) -n$(objdir) $<

.cpp.obj:
	$(tkcc) $(cfgflag) -m$(mdl) -n$(objdir) $<

all: dirs objs libf

dirs:
	-if not exist $(root)\nul md $(root)
	-if not exist $(root)\$(branch)\nul md $(root)\$(branch)
	-if not exist $(root)\$(branch)\sdk\nul md $(root)\$(branch)\sdk
	-if not exist $(sdk)\nul md $(sdk)
	-if not exist $(libdir)\nul md $(libdir)
	-if not exist $(sdk)\obj\nul md $(sdk)\obj
	-if not exist $(sdk)\obj\$(subdir)\nul md $(sdk)\obj\$(subdir)
	-if not exist $(objdir)\nul md $(objdir)

objs: vmavl.obj \
	vmfuncs.obj

libf:
	-if exist $(libfile) del $(libfile)
	tlib $(libfile) +$(objdir)\vmavl
	tlib $(libfile) +$(objdir)\vmfuncs
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak

clean:
	-if exist $(objdir)\*.obj del $(objdir)\*.obj
	-if exist $(libfile) del $(libfile)
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak
