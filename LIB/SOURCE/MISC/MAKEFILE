SUBDIR = misc
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

.asm.obj:
    tasm /mx /d__l__ $<, $(OBJDIR)\$&.obj

all: obj lib

obj: \
	append.obj   \
	bmsearch.obj \
	ctod.obj     \
	diskfree.obj \
	julian.obj   \
	proper.obj   \
	psearch.obj  \
	swap.obj     \
	subst.obj

lib: \
	buildstr.obj \
	fmemset.obj  \
	fmemcpy.obj  \
	swapenv.obj  \
	addchar.obj  \
	alldigit.obj \
	ascii.obj    \
	bd_dble.obj  \
	bd_long.obj  \
	bs_long.obj  \
	change.obj   \
	comma.obj    \
	crypt.obj    \
	dayoweek.obj \
	dble_bd.obj  \
	delfiles.obj \
	dtoc.obj     \
	exist.obj    \
	findname.obj \
	fullname.obj \
	hextoi.obj   \
	isset.obj    \
	lastchar.obj \
	long_bd.obj  \
	long_bs.obj  \
	mkunique.obj \
	mstrcpy.obj  \
	padstr.obj   \
	prnready.obj \
	rle.obj      \
	setbit.obj   \
	share.obj    \
	soundex.obj  \
	stripa.obj   \
	stripl.obj   \
	stripr.obj   \
	time.obj     \
	timesten.obj \
	ttoc.obj     \
	unsetbit.obj

	del $(LIBDIR)\misc_l.lib

	tlib $(LIBDIR)\misc_l + $(OBJDIR)\julian

	tlib $(LIBDIR)\misc_l + $(OBJDIR)\buildstr
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\fmemset
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\fmemcpy
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\swapenv
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\addchar
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\alldigit
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\ascii
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\bd_dble
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\bd_long
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\bs_long
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\change
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\comma
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\crypt
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\dayoweek
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\dble_bd
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\delfiles
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\dtoc
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\exist
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\findname
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\fullname
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\hextoi
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\isset
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\lastchar
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\long_bd
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\long_bs
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\mkunique
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\mstrcpy
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\padstr
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\prnready
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\rle
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\setbit
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\share
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\soundex
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\stripa
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\stripl
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\stripr
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\time
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\timesten
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\ttoc
	tlib $(LIBDIR)\misc_l + $(OBJDIR)\unsetbit

	del $(OBJDIR)\buildstr.obj
	del $(OBJDIR)\fmemset.obj
	del $(OBJDIR)\fmemcpy.obj
	del $(OBJDIR)\swapenv.obj
	del $(OBJDIR)\addchar.obj
	del $(OBJDIR)\alldigit.obj
	del $(OBJDIR)\ascii.obj
	del $(OBJDIR)\bd_dble.obj
	del $(OBJDIR)\bd_long.obj
	del $(OBJDIR)\bs_long.obj
	del $(OBJDIR)\change.obj
	del $(OBJDIR)\comma.obj
	del $(OBJDIR)\crypt.obj
	del $(OBJDIR)\dayoweek.obj
	del $(OBJDIR)\dble_bd.obj
	del $(OBJDIR)\delfiles.obj
	del $(OBJDIR)\dtoc.obj
	del $(OBJDIR)\exist.obj
	del $(OBJDIR)\findname.obj
	del $(OBJDIR)\fullname.obj
	del $(OBJDIR)\hextoi.obj
	del $(OBJDIR)\isset.obj
	del $(OBJDIR)\lastchar.obj
	del $(OBJDIR)\long_bd.obj
	del $(OBJDIR)\long_bs.obj
	del $(OBJDIR)\mkunique.obj
	del $(OBJDIR)\mstrcpy.obj
	del $(OBJDIR)\padstr.obj
	del $(OBJDIR)\prnready.obj
	del $(OBJDIR)\rle.obj
	del $(OBJDIR)\setbit.obj
	del $(OBJDIR)\share.obj
	del $(OBJDIR)\soundex.obj
	del $(OBJDIR)\stripa.obj
	del $(OBJDIR)\stripl.obj
	del $(OBJDIR)\stripr.obj
	del $(OBJDIR)\time.obj
	del $(OBJDIR)\timesten.obj
	del $(OBJDIR)\ttoc.obj
	del $(OBJDIR)\unsetbit.obj

