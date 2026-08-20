SUBDIR = screen
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: obj lib

obj: \
	box.obj \
	getmode.obj

lib: \
	boxcls.obj \
	cls.obj \
	clsbox.obj \
	clscolor.obj \
	cursor.obj \
	datestr.obj \
	delay.obj \
	fastputc.obj \
	giveup.obj \
	gotoxy.obj \
	print.obj \
	printv.obj \
	prntcntr.obj \
	prntmove.obj \
	readscr2.obj \
	saverest.obj \
	saverst2.obj \
	setatt.obj \
	setfont.obj \
	setrows.obj \
	sound.obj \
	time1.obj \
	time2.obj \
	wherey.obj

	del $(LIBDIR)\screen_l.lib

	tlib $(LIBDIR)\screen_l + $(OBJDIR)\boxcls
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\cls
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\clsbox
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\clscolor
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\cursor
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\datestr
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\delay
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\fastputc
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\giveup
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\gotoxy
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\print
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\printv
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\prntcntr
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\prntmove
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\readscr2
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\saverest
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\saverst2
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\setatt
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\setfont
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\setrows
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\sound
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\time1
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\time2
	tlib $(LIBDIR)\screen_l + $(OBJDIR)\wherey

	del $(OBJDIR)\boxcls.obj
	del $(OBJDIR)\cls.obj
	del $(OBJDIR)\clsbox.obj
	del $(OBJDIR)\clscolor.obj
	del $(OBJDIR)\cursor.obj
	del $(OBJDIR)\datestr.obj
	del $(OBJDIR)\delay.obj
	del $(OBJDIR)\fastputc.obj
	del $(OBJDIR)\giveup.obj
	del $(OBJDIR)\gotoxy.obj
	del $(OBJDIR)\print.obj
	del $(OBJDIR)\printv.obj
	del $(OBJDIR)\prntcntr.obj
	del $(OBJDIR)\prntmove.obj
	del $(OBJDIR)\readscr2.obj
	del $(OBJDIR)\saverest.obj
	del $(OBJDIR)\saverst2.obj
	del $(OBJDIR)\setatt.obj
	del $(OBJDIR)\setfont.obj
	del $(OBJDIR)\setrows.obj
	del $(OBJDIR)\sound.obj
	del $(OBJDIR)\time1.obj
	del $(OBJDIR)\time2.obj
	del $(OBJDIR)\wherey.obj

