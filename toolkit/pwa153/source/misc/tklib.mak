#=============================================================
#
#       makefile - misc category library for the pcboard toolkit
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
#       out of the shipped misc_l.386, recorded in
#       attic\prebuilt-libs\bc31\readme.md.
#
#       these are compiled but deliberately not put in the
#       library -- the programs that need them link them by path from
#       $(sdkobj)\<cat>\<model>:
#           swap
#
#       compiler switches live in $(cfg) -- clark's pcboard.cfg and
#       all.res merged into one file, so the bcc command line stays
#       under the dos 127-character limit.  there is no -dlib there:
#       that switch empties _fardata_ and gives the door-sdk flavour
#       of these modules, which is not what pcboard, pcbsetup and
#       fidoutil link against.
#
#       virtual is compiled with -dvirtual_huge:
#       virtual.c holds both implementations since the virtual1 merge.  clark's
#       misc_l.386 carries the huge-pointer one (pcbfiler needs it), which
#       is the virtual_huge branch; without the define you get the old
#       virtual1.  see source\misc\virtual-merge.md.
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

subdir   = misc
libname  = misc_$(mdl)
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

all: dirs objs virtualobj libf

virtualobj:
	$(tkcc) $(cfgflag) -m$(mdl) -dvirtual_huge -n$(objdir) virtual.c

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
	addchar.obj \
	alldigit.obj \
	append.obj \
	ascii.obj \
	bd_dble.obj \
	bd_long.obj \
	binary.obj \
	bmsearch.obj \
	bs_dble.obj \
	bs_long.obj \
	buildstr.obj \
	change.obj \
	chkmove.obj \
	comma.obj \
	copyfile.obj \
	copyfp.obj \
	crypt.obj \
	ctod.obj \
	dayoweek.obj \
	dble_bd.obj \
	dble_bs.obj \
	dble_pr.obj \
	dbl_long.obj \
	dcomma.obj \
	delfiles.obj \
	directry.obj \
	diskfree.obj \
	driveok.obj \
	dtoc.obj \
	editor.obj \
	endstr.obj \
	evaluate.obj \
	exist.obj \
	exitfunc.obj \
	findfour.obj \
	findname.obj \
	fmemcpy.obj \
	fmemset.obj \
	fullname.obj \
	hextoi.obj \
	index.obj \
	isset.obj \
	julian.obj \
	lastchar.obj \
	leftstr.obj \
	long_bd.obj \
	long_bs.obj \
	long_dbl.obj \
	long_pr.obj \
	midstr.obj \
	mkunique.obj \
	movefile.obj \
	mstrcpy.obj \
	padstr.obj \
	prnready.obj \
	proper.obj \
	pr_dble.obj \
	pr_long.obj \
	psearch.obj \
	rightstr.obj \
	rle.obj \
	setbit.obj \
	share.obj \
	soundex.obj \
	stripa.obj \
	stripb.obj \
	stripl.obj \
	stripr.obj \
	subst.obj \
	swapenv.obj \
	time.obj \
	timesten.obj \
	ttoc.obj \
	unsetbit.obj \
	validate.obj \
	validsem.obj \
	wildcard.obj \
	zsearch.obj \
	zsort.obj \
	zswapint.obj \
	zswaplng.obj \
	zswapstr.obj \
	zswapvir.obj \
	swap.obj

libf:
	-if exist $(libfile) del $(libfile)
	tlib $(libfile) +$(objdir)\abort
	tlib $(libfile) +$(objdir)\addchar
	tlib $(libfile) +$(objdir)\alldigit
	tlib $(libfile) +$(objdir)\append
	tlib $(libfile) +$(objdir)\ascii
	tlib $(libfile) +$(objdir)\bd_dble
	tlib $(libfile) +$(objdir)\bd_long
	tlib $(libfile) +$(objdir)\binary
	tlib $(libfile) +$(objdir)\bmsearch
	tlib $(libfile) +$(objdir)\bs_dble
	tlib $(libfile) +$(objdir)\bs_long
	tlib $(libfile) +$(objdir)\buildstr
	tlib $(libfile) +$(objdir)\change
	tlib $(libfile) +$(objdir)\chkmove
	tlib $(libfile) +$(objdir)\comma
	tlib $(libfile) +$(objdir)\copyfile
	tlib $(libfile) +$(objdir)\copyfp
	tlib $(libfile) +$(objdir)\crypt
	tlib $(libfile) +$(objdir)\ctod
	tlib $(libfile) +$(objdir)\dayoweek
	tlib $(libfile) +$(objdir)\dble_bd
	tlib $(libfile) +$(objdir)\dble_bs
	tlib $(libfile) +$(objdir)\dble_pr
	tlib $(libfile) +$(objdir)\dbl_long
	tlib $(libfile) +$(objdir)\dcomma
	tlib $(libfile) +$(objdir)\delfiles
	tlib $(libfile) +$(objdir)\directry
	tlib $(libfile) +$(objdir)\diskfree
	tlib $(libfile) +$(objdir)\driveok
	tlib $(libfile) +$(objdir)\dtoc
	tlib $(libfile) +$(objdir)\editor
	tlib $(libfile) +$(objdir)\endstr
	tlib $(libfile) +$(objdir)\evaluate
	tlib $(libfile) +$(objdir)\exist
	tlib $(libfile) +$(objdir)\exitfunc
	tlib $(libfile) +$(objdir)\findfour
	tlib $(libfile) +$(objdir)\findname
	tlib $(libfile) +$(objdir)\fmemcpy
	tlib $(libfile) +$(objdir)\fmemset
	tlib $(libfile) +$(objdir)\fullname
	tlib $(libfile) +$(objdir)\hextoi
	tlib $(libfile) +$(objdir)\index
	tlib $(libfile) +$(objdir)\isset
	tlib $(libfile) +$(objdir)\julian
	tlib $(libfile) +$(objdir)\lastchar
	tlib $(libfile) +$(objdir)\leftstr
	tlib $(libfile) +$(objdir)\long_bd
	tlib $(libfile) +$(objdir)\long_bs
	tlib $(libfile) +$(objdir)\long_dbl
	tlib $(libfile) +$(objdir)\long_pr
	tlib $(libfile) +$(objdir)\midstr
	tlib $(libfile) +$(objdir)\mkunique
	tlib $(libfile) +$(objdir)\movefile
	tlib $(libfile) +$(objdir)\mstrcpy
	tlib $(libfile) +$(objdir)\padstr
	tlib $(libfile) +$(objdir)\prnready
	tlib $(libfile) +$(objdir)\proper
	tlib $(libfile) +$(objdir)\pr_dble
	tlib $(libfile) +$(objdir)\pr_long
	tlib $(libfile) +$(objdir)\psearch
	tlib $(libfile) +$(objdir)\rightstr
	tlib $(libfile) +$(objdir)\rle
	tlib $(libfile) +$(objdir)\setbit
	tlib $(libfile) +$(objdir)\share
	tlib $(libfile) +$(objdir)\soundex
	tlib $(libfile) +$(objdir)\stripa
	tlib $(libfile) +$(objdir)\stripb
	tlib $(libfile) +$(objdir)\stripl
	tlib $(libfile) +$(objdir)\stripr
	tlib $(libfile) +$(objdir)\subst
	tlib $(libfile) +$(objdir)\swapenv
	tlib $(libfile) +$(objdir)\time
	tlib $(libfile) +$(objdir)\timesten
	tlib $(libfile) +$(objdir)\ttoc
	tlib $(libfile) +$(objdir)\unsetbit
	tlib $(libfile) +$(objdir)\validate
	tlib $(libfile) +$(objdir)\validsem
	tlib $(libfile) +$(objdir)\virtual
	tlib $(libfile) +$(objdir)\wildcard
	tlib $(libfile) +$(objdir)\zsearch
	tlib $(libfile) +$(objdir)\zsort
	tlib $(libfile) +$(objdir)\zswapint
	tlib $(libfile) +$(objdir)\zswaplng
	tlib $(libfile) +$(objdir)\zswapstr
	tlib $(libfile) +$(objdir)\zswapvir
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak

clean:
	-if exist $(objdir)\*.obj del $(objdir)\*.obj
	-if exist $(objdir)\*.asm del $(objdir)\*.asm
	-if exist $(libfile) del $(libfile)
	-if exist $(libdir)\$(libname).bak del $(libdir)\$(libname).bak
