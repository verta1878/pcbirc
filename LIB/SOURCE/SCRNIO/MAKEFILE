SUBDIR = scrnio
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: lib

lib: \
	bgetkey.obj \
	curskeys.obj \
	error.obj \
	formdate.obj \
	genscrn.obj \
	getkey.obj \
	getkeyfl.obj \
	help.obj \
	initscrn.obj \
	inputall.obj \
	inputext.obj \
	inputln.obj \
	inputnum.obj \
	inputstr.obj \
	mallochk.obj \
	menu.obj \
	message.obj \
	scale.obj \
	scrninpt.obj \
	timedkey.obj

	if exist $(LIBDIR)\scrnio_l.lib del $(LIBDIR)\scrnio_l.lib
	cd $(OBJDIR)
	tlib /C ..\..\scrnio_l @..\..\..\..\source\scrnio\LIBADD.RSP
	cd ..\..\..\..\source\scrnio
