SUBDIR = toolkit
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: lib

lib: \
	altmodem.obj \
	nodisp.obj   \
	pcbdat.obj

	del $(LIBDIR)\toolkitl.lib

	tlib $(LIBDIR)\toolkitl + $(OBJDIR)\altmodem
	tlib $(LIBDIR)\toolkitl + $(OBJDIR)\nodisp
	tlib $(LIBDIR)\toolkitl + $(OBJDIR)\pcbdat

	del $(OBJDIR)\altmodem.obj
	del $(OBJDIR)\nodisp.obj
	del $(OBJDIR)\pcbdat.obj

