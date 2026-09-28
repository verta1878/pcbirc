#=============================================================
#
#       makefile - pcb category library for the pcboard toolkit
#
#       builds  $(libfile)  from the sources in this folder.
#       objects go to $(objdir) and are kept, so a second make only
#       recompiles what changed.  make clean removes both.
#
#       the repo folder is mounted as the drive root, so \out,
#       \toolkit, \pcb153 and \bc31 are inside the repo no matter
#       what the repo folder is called.  every macro is guarded, so
#       it can be overridden on the command line:
#
#           make -dcver=bc50
#           make -droot=\out -dbranch=pwa154
#
#       memory model:
#           large (default):  make
#           small:            make -dmdl=s -dmodel=small -dcfgname=tks
#
#       targets:  all (default) | dirs | objs | libf | clean
#
#       the module list is clark's: it matches the module names read
#       out of the shipped pcb_l.386, recorded in
#       attic\prebuilt-libs\bc31\readme.md.
#
#       compiler switches live in $(cfg) -- clark's pcboard.cfg and
#       all.res merged into one file, so the bcc command line stays
#       under the dos 127-character limit.  there is no -dlib there:
#       that switch empties _fardata_ and gives the door-sdk flavour
#       of these modules, which is not what pcboard, pcbsetup and
#       fidoutil link against.
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

subdir   = pcb
libname  = pcb_$(mdl)
cfgdir   = $(tkit)\cfg\$(cver)
cfg      = $(cfgdir)\$(cfgname).cfg
sdk      = $(root)\$(branch)\sdk\$(cver)
libdir   = $(sdk)\lib
objdir   = $(sdk)\obj\$(subdir)\$(model)
libfile  = $(libdir)\$(libname).lib

.c.obj:
	bcc +$(cfg) -n$(objdir) $<

.cpp.obj:
	bcc +$(cfg) -n$(objdir) $<

.asm.obj:
	tasm /mx /d__$(mdl)__ $<, $(objdir)\$&.obj

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

objs: abort.obj \
	account.obj \
	addbacks.obj \
	allow.obj \
	chkexist.obj \
	ci_other.obj \
	cnames.obj \
	conffunc.obj \
	config.obj \
	data120.obj \
	datadflt.obj \
	datafil2.obj \
	datafile.obj \
	dataread.obj \
	datawrit.obj \
	exitdos.obj \
	getdays.obj \
	parse.obj \
	parsepth.obj \
	savetext.obj \
	srchpath.obj \
	text.obj

libf:
	-if exist $(libfile) del $(libfile)
	tlib $(libfile) +$(objdir)\abort
	tlib $(libfile) +$(objdir)\account
	tlib $(libfile) +$(objdir)\addbacks
	tlib $(libfile) +$(objdir)\allow
	tlib $(libfile) +$(objdir)\chkexist
	tlib $(libfile) +$(objdir)\ci_other
	tlib $(libfile) +$(objdir)\cnames
	tlib $(libfile) +$(objdir)\conffunc
	tlib $(libfile) +$(objdir)\config
	tlib $(libfile) +$(objdir)\data120
	tlib $(libfile) +$(objdir)\datadflt
	tlib $(libfile) +$(objdir)\datafil2
	tlib $(libfile) +$(objdir)\datafile
	tlib $(libfile) +$(objdir)\dataread
	tlib $(libfile) +$(objdir)\datawrit
	tlib $(libfile) +$(objdir)\exitdos
	tlib $(libfile) +$(objdir)\getdays
	tlib $(libfile) +$(objdir)\parse
	tlib $(libfile) +$(objdir)\parsepth
	tlib $(libfile) +$(objdir)\savetext
	tlib $(libfile) +$(objdir)\srchpath
	tlib $(libfile) +$(objdir)\text
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak

clean:
	-if exist $(objdir)\*.obj del $(objdir)\*.obj
	-if exist $(objdir)\*.asm del $(objdir)\*.asm
	-if exist $(libfile) del $(libfile)
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak
