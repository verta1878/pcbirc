SUBDIR = doscls
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.cpp.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: lib

lib: \
	dosclass.obj

	del $(LIBDIR)\doscls_l.lib

	tlib $(LIBDIR)\doscls_l + $(OBJDIR)\dosclass

	del $(OBJDIR)\dosclass.obj

