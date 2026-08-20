SUBDIR = country
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: obj lib

obj: \
	country.obj

lib: \
	dcomma.obj \
	date.obj

	del $(LIBDIR)\countryl.lib

	tlib $(LIBDIR)\countryl + $(OBJDIR)\country

	tlib $(LIBDIR)\countryl + $(OBJDIR)\dcomma
	tlib $(LIBDIR)\countryl + $(OBJDIR)\date

	del $(OBJDIR)\dcomma.obj
	del $(OBJDIR)\date.obj

