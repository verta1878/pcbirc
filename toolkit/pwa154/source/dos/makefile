SUBDIR = dos
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
	doserror.obj \
	dosfgets.obj \
	dosfread.obj \
	dosfugts.obj

lib: \
	int24hnd.obj \
	chkappen.obj \
	chkcreat.obj \
	chklock.obj  \
	chkopen.obj  \
	chkread.obj  \
	chkunlnk.obj \
	chkwrite.obj \
	dosappen.obj \
	dosclose.obj \
	doscomit.obj \
	doscreat.obj \
	dosdup.obj   \
	dosfclos.obj \
	dosfind.obj  \
	dosflush.obj \
	dosfopen.obj \
	dosfputs.obj \
	dosfseek.obj \
	dosftrun.obj \
	dosfwrit.obj \
	doslseek.obj \
	dosopen.obj  \
	dosread.obj  \
	dosrewin.obj \
	dosstbuf.obj \
	dostrunc.obj \
	doswrite.obj \
	extended.obj \
	getdrive.obj \
	getpath.obj  \
	handlers.obj \
	isopen.obj   \
	setdrive.obj \
	strnchr.obj

	del $(LIBDIR)\dos_l.lib

	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfgets

	tlib $(LIBDIR)\dos_l + $(OBJDIR)\int24hnd
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chkappen
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chkcreat
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chklock
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chkopen
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chkread
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chkunlnk
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\chkwrite
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosappen
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosclose
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\doscomit
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\doscreat
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosdup
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfclos
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfind
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosflush
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfopen
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfputs
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfseek
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosftrun
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosfwrit
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\doslseek
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosopen
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosread
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosrewin
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dosstbuf
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\dostrunc
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\doswrite
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\extended
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\getdrive
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\getpath
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\handlers
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\isopen
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\setdrive
	tlib $(LIBDIR)\dos_l + $(OBJDIR)\strnchr

	del $(OBJDIR)\int24hnd.obj
	del $(OBJDIR)\chkappen.obj
	del $(OBJDIR)\chkcreat.obj
	del $(OBJDIR)\chklock.obj
	del $(OBJDIR)\chkopen.obj
	del $(OBJDIR)\chkread.obj
	del $(OBJDIR)\chkunlnk.obj
	del $(OBJDIR)\chkwrite.obj
	del $(OBJDIR)\dosappen.obj
	del $(OBJDIR)\dosclose.obj
	del $(OBJDIR)\doscomit.obj
	del $(OBJDIR)\doscreat.obj
	del $(OBJDIR)\dosdup.obj
	del $(OBJDIR)\dosfclos.obj
	del $(OBJDIR)\dosfind.obj
	del $(OBJDIR)\dosflush.obj
	del $(OBJDIR)\dosfopen.obj
	del $(OBJDIR)\dosfputs.obj
	del $(OBJDIR)\dosfseek.obj
	del $(OBJDIR)\dosftrun.obj
	del $(OBJDIR)\dosfwrit.obj
	del $(OBJDIR)\doslseek.obj
	del $(OBJDIR)\dosopen.obj
	del $(OBJDIR)\dosread.obj
	del $(OBJDIR)\dosrewin.obj
	del $(OBJDIR)\dosstbuf.obj
	del $(OBJDIR)\dostrunc.obj
	del $(OBJDIR)\doswrite.obj
	del $(OBJDIR)\extended.obj
	del $(OBJDIR)\getdrive.obj
	del $(OBJDIR)\getpath.obj
	del $(OBJDIR)\handlers.obj
	del $(OBJDIR)\isopen.obj
	del $(OBJDIR)\setdrive.obj
	del $(OBJDIR)\strnchr.obj

