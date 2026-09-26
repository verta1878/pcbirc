#
# COMMDRV.MAK — Build COMMDRV.OBJ, FOSSIL.OBJ, commdrbl.lib, libsbl.lib
#
# Usage: MAKE -fCOMMDRV.MAK all        (builds everything)
#        MAKE -fCOMMDRV.MAK commdrv    (COMMDRV.OBJ only)
#        MAKE -fCOMMDRV.MAK fossil     (FOSSIL.OBJ only)
#        MAKE -fCOMMDRV.MAK libs       (commdrbl.lib + libsbl.lib only)
#
# Requires: Borland C++ 3.1 (BCC.EXE, TLIB.EXE)
# Source:   MODEM.C + MODEMASY.C + MODEMDRV.C → COMMDRV.OBJ or FOSSIL.OBJ
#           commdrbl.c → commdrbl.lib
#           libsbl.c   → libsbl.lib
#

MAK      = COMMDRV.MAK
MDL      = l
COMPILER = BCC.EXE
TLIB     = TLIB.EXE

# Paths — relative to repo root (mounted as drive root in DOSBox)
SRCDIR   = TOOLKIT\PWA153\SOURCE\TOOLKIT
HDIR     = PCBCBASE\COMMDRV\H
LIBDIR   = PCBCBASE\COMMDRV\LIB
OBJDIR   = PCBCBASE\COMMDRV\OBJ
COMMSRC  = PCBCBASE\COMMDRV\SRC
BCINC    = BC31\INCLUDE

# Clark's header chain
PCB153H  = PCB153\SOURCE\H
TKITH    = TOOLKIT\PWA153\H

#--------------------------------------------------------------------------
# Targets
#--------------------------------------------------------------------------

all: commdrv fossil libs

commdrv: $(OBJDIR)\COMMDRV.OBJ

fossil: $(OBJDIR)\FOSSIL.OBJ

libs: $(LIBDIR)\commdrbl.lib $(LIBDIR)\libsbl.lib

clean:
	-del $(OBJDIR)\COMMDRV.OBJ
	-del $(OBJDIR)\FOSSIL.OBJ
	-del $(LIBDIR)\commdrbl.lib
	-del $(LIBDIR)\libsbl.lib
	-del *.obj
	-del COMMDRV.CFG

#--------------------------------------------------------------------------
# Config file — auto-generated from this makefile (Clark's convention)
#--------------------------------------------------------------------------

COMMDRV.CFG: $(MAK)
	copy &&|
-m$(MDL)
-c
-w-
-I$(HDIR)
-I$(PCB153H)
-I$(TKITH)
-I$(BCINC)
| COMMDRV.CFG

#--------------------------------------------------------------------------
# COMMDRV.OBJ — from MODEM.C + MODEMASY.C + MODEMDRV.C (one unit)
#
# Clark compiled all three as one compilation unit.
# MODEM.C #includes MODEMASY.C and MODEMDRV.C at line 42.
# Defines: -DCOMM -DMULTIPORT -DLIB -DCOMMDRV
#--------------------------------------------------------------------------

$(OBJDIR)\COMMDRV.OBJ: $(SRCDIR)\MODEM.C $(SRCDIR)\MODEMASY.C $(SRCDIR)\MODEMDRV.C COMMDRV.CFG
	$(COMPILER) +COMMDRV.CFG -DCOMM -DMULTIPORT -DLIB -DCOMMDRV -o$@ $(SRCDIR)\MODEM.C

#--------------------------------------------------------------------------
# FOSSIL.OBJ — standalone, no external headers needed
#--------------------------------------------------------------------------

$(OBJDIR)\FOSSIL.OBJ: $(COMMSRC)\FOSSIL.C COMMDRV.CFG
	$(COMPILER) +COMMDRV.CFG -o$@ $(COMMSRC)\FOSSIL.C

#--------------------------------------------------------------------------
# commdrbl.lib — 13 ser_rs232_* via INT 14h FOSSIL
#--------------------------------------------------------------------------

$(LIBDIR)\commdrbl.lib: $(COMMSRC)\commdrbl.c $(HDIR)\COMM.H COMMDRV.CFG
	$(COMPILER) +COMMDRV.CFG -DCOMMDRV_DRIVER commdrbl.c
	$(TLIB) $@ +commdrbl.obj
	-del commdrbl.obj

#--------------------------------------------------------------------------
# libsbl.lib — 6 utility functions
#--------------------------------------------------------------------------

$(LIBDIR)\libsbl.lib: $(COMMSRC)\libsbl.c $(HDIR)\COMM.H COMMDRV.CFG
	$(COMPILER) +COMMDRV.CFG libsbl.c
	$(TLIB) $@ +libsbl.obj
	-del libsbl.obj
