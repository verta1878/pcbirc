SUBDIR = system
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

.cpp.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: lib

lib: \
	bgetkey.obj \
	sysdate.obj \
	systime.obj

	del $(LIBDIR)\system_l.lib

	tlib $(LIBDIR)\system_l + $(OBJDIR)\bgetkey
	tlib $(LIBDIR)\system_l + $(OBJDIR)\sysdate
	tlib $(LIBDIR)\system_l + $(OBJDIR)\systime

	del $(OBJDIR)\bgetkey.obj
	del $(OBJDIR)\sysdate.obj
	del $(OBJDIR)\systime.obj

