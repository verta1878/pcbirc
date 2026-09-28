#=============================================================
#
#       makefile - dos category library for the pcboard toolkit
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
#       out of the shipped dos_l.386, recorded in
#       attic\prebuilt-libs\bc31\readme.md.
#
#       one addition: int24hnd.  it is in clark's own makefile but missing
#       from the .386 we were handed, so the shipped copy was short of it.
#
#       these are compiled but deliberately not put in the
#       library -- the programs that need them link them by path from
#       $(sdkobj)\<cat>\<model>:
#           showerr2
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

subdir   = dos
libname  = dos_$(mdl)
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

objs: chkappen.obj \
	chkcreat.obj \
	chkdosfo.obj \
	chkfopen.obj \
	chkfprnt.obj \
	chklock.obj \
	chkopen.obj \
	chkread.obj \
	chkunlnk.obj \
	chkwrite.obj \
	dosappen.obj \
	dosclose.obj \
	doscomit.obj \
	doscreat.obj \
	dosdup.obj \
	doserror.obj \
	dosfclos.obj \
	dosfgets.obj \
	dosfind.obj \
	dosflush.obj \
	dosfngts.obj \
	dosfopen.obj \
	dosfputs.obj \
	dosfread.obj \
	dosfseek.obj \
	dosftrun.obj \
	dosfugts.obj \
	dosfwrit.obj \
	doslseek.obj \
	dosopen.obj \
	dosread.obj \
	dosrewin.obj \
	dosstbuf.obj \
	dostrunc.obj \
	doswrite.obj \
	extended.obj \
	getdrive.obj \
	getpath.obj \
	handlers.obj \
	isopen.obj \
	say.obj \
	setdrive.obj \
	strnchr.obj \
	showerr.obj \
	int24hnd.obj \
	showerr2.obj

libf:
	-if exist $(libfile) del $(libfile)
	tlib $(libfile) +$(objdir)\chkappen
	tlib $(libfile) +$(objdir)\chkcreat
	tlib $(libfile) +$(objdir)\chkdosfo
	tlib $(libfile) +$(objdir)\chkfopen
	tlib $(libfile) +$(objdir)\chkfprnt
	tlib $(libfile) +$(objdir)\chklock
	tlib $(libfile) +$(objdir)\chkopen
	tlib $(libfile) +$(objdir)\chkread
	tlib $(libfile) +$(objdir)\chkunlnk
	tlib $(libfile) +$(objdir)\chkwrite
	tlib $(libfile) +$(objdir)\dosappen
	tlib $(libfile) +$(objdir)\dosclose
	tlib $(libfile) +$(objdir)\doscomit
	tlib $(libfile) +$(objdir)\doscreat
	tlib $(libfile) +$(objdir)\dosdup
	tlib $(libfile) +$(objdir)\doserror
	tlib $(libfile) +$(objdir)\dosfclos
	tlib $(libfile) +$(objdir)\dosfgets
	tlib $(libfile) +$(objdir)\dosfind
	tlib $(libfile) +$(objdir)\dosflush
	tlib $(libfile) +$(objdir)\dosfngts
	tlib $(libfile) +$(objdir)\dosfopen
	tlib $(libfile) +$(objdir)\dosfputs
	tlib $(libfile) +$(objdir)\dosfread
	tlib $(libfile) +$(objdir)\dosfseek
	tlib $(libfile) +$(objdir)\dosftrun
	tlib $(libfile) +$(objdir)\dosfugts
	tlib $(libfile) +$(objdir)\dosfwrit
	tlib $(libfile) +$(objdir)\doslseek
	tlib $(libfile) +$(objdir)\dosopen
	tlib $(libfile) +$(objdir)\dosread
	tlib $(libfile) +$(objdir)\dosrewin
	tlib $(libfile) +$(objdir)\dosstbuf
	tlib $(libfile) +$(objdir)\dostrunc
	tlib $(libfile) +$(objdir)\doswrite
	tlib $(libfile) +$(objdir)\extended
	tlib $(libfile) +$(objdir)\getdrive
	tlib $(libfile) +$(objdir)\getpath
	tlib $(libfile) +$(objdir)\handlers
	tlib $(libfile) +$(objdir)\isopen
	tlib $(libfile) +$(objdir)\say
	tlib $(libfile) +$(objdir)\setdrive
	tlib $(libfile) +$(objdir)\strnchr
	tlib $(libfile) +$(objdir)\showerr
	tlib $(libfile) +$(objdir)\int24hnd
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak

clean:
	-if exist $(objdir)\*.obj del $(objdir)\*.obj
	-if exist $(objdir)\*.asm del $(objdir)\*.asm
	-if exist $(libfile) del $(libfile)
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak
