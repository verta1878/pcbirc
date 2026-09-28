#=============================================================
#
#       makefile - screen category library for the pcboard toolkit
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
#       out of the shipped screen_l.386, recorded in
#       attic\prebuilt-libs\bc31\readme.md.
#
#       compiler switches live in $(cfg) -- clark's pcboard.cfg and
#       all.res merged into one file, so the bcc command line stays
#       under the dos 127-character limit.  there is no -dlib there:
#       that switch empties _fardata_ and gives the door-sdk flavour
#       of these modules, which is not what pcboard, pcbsetup and
#       fidoutil link against.
#
#       scrollup is compiled with -b:
#       -b routes this one file through tasm.  its inline asm jumps to a local
#       label called "exit", which bcc's built-in assembler resolves to the
#       library function exit() and then rejects.
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

subdir   = screen
libname  = screen_$(mdl)
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

scrollup.obj: scrollup.c
	bcc +$(cfg) -b -n$(objdir) scrollup.c

dirs:
	-if not exist $(root)\nul md $(root)
	-if not exist $(root)\$(branch)\nul md $(root)\$(branch)
	-if not exist $(root)\$(branch)\sdk\nul md $(root)\$(branch)\sdk
	-if not exist $(sdk)\nul md $(sdk)
	-if not exist $(libdir)\nul md $(libdir)
	-if not exist $(sdk)\obj\nul md $(sdk)\obj
	-if not exist $(sdk)\obj\$(subdir)\nul md $(sdk)\obj\$(subdir)
	-if not exist $(objdir)\nul md $(objdir)

objs: ansi.obj \
	box.obj \
	boxcls.obj \
	cls.obj \
	clsbox.obj \
	clscolor.obj \
	cursor.obj \
	datestr.obj \
	delay.obj \
	delete.obj \
	fastputc.obj \
	getmode.obj \
	getsec.obj \
	giveup.obj \
	gotoxy.obj \
	growbox.obj \
	insert.obj \
	print.obj \
	printv.obj \
	prntcntr.obj \
	prntmove.obj \
	readscr2.obj \
	readscrn.obj \
	saverest.obj \
	saverst2.obj \
	scrolldn.obj \
	scrollup.obj \
	setatt.obj \
	setfont.obj \
	setrows.obj \
	sound.obj \
	time1.obj \
	time2.obj \
	timechng.obj \
	twodig.obj \
	twodig0.obj \
	wherex.obj \
	wherey.obj \
	window.obj

libf:
	-if exist $(libfile) del $(libfile)
	tlib $(libfile) +$(objdir)\ansi
	tlib $(libfile) +$(objdir)\box
	tlib $(libfile) +$(objdir)\boxcls
	tlib $(libfile) +$(objdir)\cls
	tlib $(libfile) +$(objdir)\clsbox
	tlib $(libfile) +$(objdir)\clscolor
	tlib $(libfile) +$(objdir)\cursor
	tlib $(libfile) +$(objdir)\datestr
	tlib $(libfile) +$(objdir)\delay
	tlib $(libfile) +$(objdir)\delete
	tlib $(libfile) +$(objdir)\fastputc
	tlib $(libfile) +$(objdir)\getmode
	tlib $(libfile) +$(objdir)\getsec
	tlib $(libfile) +$(objdir)\giveup
	tlib $(libfile) +$(objdir)\gotoxy
	tlib $(libfile) +$(objdir)\growbox
	tlib $(libfile) +$(objdir)\insert
	tlib $(libfile) +$(objdir)\print
	tlib $(libfile) +$(objdir)\printv
	tlib $(libfile) +$(objdir)\prntcntr
	tlib $(libfile) +$(objdir)\prntmove
	tlib $(libfile) +$(objdir)\readscr2
	tlib $(libfile) +$(objdir)\readscrn
	tlib $(libfile) +$(objdir)\saverest
	tlib $(libfile) +$(objdir)\saverst2
	tlib $(libfile) +$(objdir)\scrolldn
	tlib $(libfile) +$(objdir)\scrollup
	tlib $(libfile) +$(objdir)\setatt
	tlib $(libfile) +$(objdir)\setfont
	tlib $(libfile) +$(objdir)\setrows
	tlib $(libfile) +$(objdir)\sound
	tlib $(libfile) +$(objdir)\time1
	tlib $(libfile) +$(objdir)\time2
	tlib $(libfile) +$(objdir)\timechng
	tlib $(libfile) +$(objdir)\twodig
	tlib $(libfile) +$(objdir)\twodig0
	tlib $(libfile) +$(objdir)\wherex
	tlib $(libfile) +$(objdir)\wherey
	tlib $(libfile) +$(objdir)\window
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak

clean:
	-if exist $(objdir)\*.obj del $(objdir)\*.obj
	-if exist $(objdir)\*.asm del $(objdir)\*.asm
	-if exist $(libfile) del $(libfile)
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak
