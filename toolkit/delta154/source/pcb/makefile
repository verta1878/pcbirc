SUBDIR = pcb
CVER   = bc31
CMOD   = large.386
CFGDIR = ..\..\cfg\$(CVER)
LIBDIR = ..\..\bcdos\$(CVER)
OBJDIR = $(LIBDIR)\$(SUBDIR)\$(CMOD)

.c.obj:
    bcc +$(CFGDIR)\pcboard.cfg -n$(OBJDIR) @$(CFGDIR)\all.res $<

all: obj lib

obj: \
	cnames.obj \
	datafil2.obj

lib: \
	account.obj  \
	addbacks.obj \
	ci_other.obj \
	dataread.obj \
	exitdos.obj  \
	parse.obj    \
	parsepth.obj \
	srchpath.obj

	del $(LIBDIR)\pcb_l.lib

	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\account
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\addbacks
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\ci_other
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\dataread
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\exitdos
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\parse
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\parsepth
	tlib $(LIBDIR)\pcb_l + $(OBJDIR)\srchpath

	del $(OBJDIR)\account.obj
	del $(OBJDIR)\addbacks.obj
	del $(OBJDIR)\ci_other.obj
	del $(OBJDIR)\dataread.obj
	del $(OBJDIR)\exitdos.obj
	del $(OBJDIR)\parse.obj
	del $(OBJDIR)\parsepth.obj
	del $(OBJDIR)\srchpath.obj

