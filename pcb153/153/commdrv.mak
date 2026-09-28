#
# commdrv.mak — build commdrv.obj, fossil.obj, commdrbl.lib, libsbl.lib
#
# usage: make -fcommdrv.mak all        (builds everything)
#        make -fcommdrv.mak commdrv    (commdrv.obj only)
#        make -fcommdrv.mak fossil     (fossil.obj only)
#        make -fcommdrv.mak libs       (commdrbl.lib + libsbl.lib only)
#
# requires: borland c++ 3.1 (bcc.exe, tlib.exe)
# source:   modem.c + modemasy.c + modemdrv.c → commdrv.obj or fossil.obj
#           commdrbl.c → commdrbl.lib
#           libsbl.c   → libsbl.lib
#

mak      = commdrv.mak
mdl      = l
compiler = bcc.exe
tlib     = tlib.exe

# paths — relative to repo root (mounted as drive root in dosbox)
srcdir   = toolkit\pwa153\source\toolkit
hdir     = pcbcbase\commdrv\h
libdir   = pcbcbase\commdrv\lib
objdir   = pcbcbase\commdrv\obj
commsrc  = pcbcbase\commdrv\src
bcinc    = bc31\include

# clark's header chain
pcb153h  = pcb153\source\h
tkith    = toolkit\pwa153\h

#--------------------------------------------------------------------------
# targets
#--------------------------------------------------------------------------

all: commdrv fossil libs

commdrv: $(objdir)\commdrv.obj

fossil: $(objdir)\fossil.obj

libs: $(libdir)\commdrbl.lib $(libdir)\libsbl.lib

clean:
	@echo commdrv clean: removing temp files only
	@echo objs at $(objdir) and libs at $(libdir) are kept (pre-built dependencies)
	-del *.obj
	-del commdrv.cfg

rebuild: clean all

#--------------------------------------------------------------------------
# config file — auto-generated from this makefile (clark's convention)
#--------------------------------------------------------------------------

commdrv.cfg: $(mak)
	copy &&|
-m$(mdl)
-c
-w-
-i$(hdir)
-i$(pcb153h)
-i$(tkith)
-i$(bcinc)
| commdrv.cfg

#--------------------------------------------------------------------------
# commdrv.obj — from modem.c + modemasy.c + modemdrv.c (one unit)
#
# clark compiled all three as one compilation unit.
# modem.c #includes modemasy.c and modemdrv.c at line 42.
# defines: -dcomm -dmultiport -dlib -dcommdrv
#--------------------------------------------------------------------------

$(objdir)\commdrv.obj: $(srcdir)\modem.c $(srcdir)\modemasy.c $(srcdir)\modemdrv.c commdrv.cfg
	$(compiler) +commdrv.cfg -dcomm -dmultiport -dlib -dcommdrv -o$@ $(srcdir)\modem.c

#--------------------------------------------------------------------------
# fossil.obj — standalone, no external headers needed
#--------------------------------------------------------------------------

$(objdir)\fossil.obj: $(commsrc)\fossil.c commdrv.cfg
	$(compiler) +commdrv.cfg -o$@ $(commsrc)\fossil.c

#--------------------------------------------------------------------------
# commdrbl.lib — 13 ser_rs232_* via int 14h fossil
#--------------------------------------------------------------------------

$(libdir)\commdrbl.lib: $(commsrc)\commdrbl.c $(hdir)\comm.h commdrv.cfg
	$(compiler) +commdrv.cfg -dcommdrv_driver commdrbl.c
	$(tlib) $@ +commdrbl.obj
	-del commdrbl.obj

#--------------------------------------------------------------------------
# libsbl.lib — 6 utility functions
#--------------------------------------------------------------------------

$(libdir)\libsbl.lib: $(commsrc)\libsbl.c $(hdir)\comm.h commdrv.cfg
	$(compiler) +commdrv.cfg libsbl.c
	$(tlib) $@ +libsbl.obj
	-del libsbl.obj
