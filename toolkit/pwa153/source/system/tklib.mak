#=============================================================
#
#       makefile - system category library for the pcboard toolkit
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
#           make -Dcver=bc50
#           make -Droot=\out -Dbranch=pwa154
#
#       memory model:
#           large (default):  make
#           small:            make -Dmdl=s -Dmodel=small -Dcfgname=tks
#
#       targets:  all (default) | dirs | objs | libf | clean
#
#       the module list is clark's: it matches the module names read
#       out of the shipped system_l.386, recorded in
#       attic\prebuilt-libs\bc31\readme.md.
#
#       one addition: kbdstat.  it is not clark's file.  it supplies the
#       kbdstatus global that bgetkey.c declares extern and nothing here
#       defined -- see the header comment in kbdstat.c.
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

subdir   = system
libname  = system_$(mdl)
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

objs: bgetkey.obj \
	sysdate.obj \
	systime.obj \
	kbdstat.obj

libf:
	-if exist $(libfile) del $(libfile)
	tlib $(libfile) +$(objdir)\bgetkey
	tlib $(libfile) +$(objdir)\sysdate
	tlib $(libfile) +$(objdir)\systime
	tlib $(libfile) +$(objdir)\kbdstat
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak

clean:
	-if exist $(objdir)\*.obj del $(objdir)\*.obj
	-if exist $(objdir)\*.asm del $(objdir)\*.asm
	-if exist $(libfile) del $(libfile)
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak
