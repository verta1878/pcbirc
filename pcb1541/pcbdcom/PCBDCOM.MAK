# ============================================================================
# PCBCOMM.MAK — build pcbcomm v1 for DOS 16-bit target
#
# Compilers supported (selected via %CC% env variable):
#   BC31   Borland C++ 3.1        make -f PCBCOMM.MAK CC=BC31
#   MSC70  Microsoft C 7.0        make -f PCBCOMM.MAK CC=MSC70
#   OWC    OpenWatcom 1.9         wmake -f PCBCOMM.MAK CC=OWC
#
# All builds produce two artifacts:
#   PCBCOMM.EXE   TSR (load from AUTOEXEC.BAT)
#   PCBCOMM.SYS   Device driver (load from CONFIG.SYS DEVICE=)
# ============================================================================

# ---- Common ----
INC    = -Iinc
DEFS   = -DPCBCOMM_V1
# TARGET selects which PCBoard line to build for:
#   TARGET=15.4  (default) — WCSC-parity backends only (lean)
#   TARGET=15.41           — adds Stallion/Chase/Equinox extended backends
!if "$(TARGET)" == "15.41"
DEFS = $(DEFS) -DPCB1541
EXTRA_OBJS = obj\stallion_brumby_backend.obj \
             obj\chase_iolan_backend.obj \
             obj\equinox_sst_backend.obj
!else
EXTRA_OBJS =
!endif

OBJS   = obj\pcbcomm.obj      \
         obj\uart.obj         \
         obj\uart_backend.obj \
         obj\boca_backend.obj \
         obj\cyclom_backend.obj \
         obj\card_pool.obj \
         obj\digi_fep.obj \
         obj\digi_pcxe_backend.obj \
         obj\digi_accel_backend.obj \
         obj\rocket_backend.obj \
         obj\easyio_backend.obj \
         obj\arnet_backend.obj \
         obj\hub6_backend.obj \
         obj\digi_comxi_backend.obj \
         obj\gtek_backend.obj \
         obj\stallion_brumby_backend.obj \
         obj\equinox_sst_backend.obj \
         obj\ser_rs232_shim.obj \
         obj\irq.obj          \
         obj\int14.obj

# ---- Borland C++ 3.1 ----
!if "$(CC)" == "BC31"
BCC    = bcc
MODEL  = -ml
CFLAGS = $(MODEL) -c -O -w $(INC) $(DEFS)
LFLAGS = $(MODEL)

all: PCBCOMM.EXE PCBCOMM.SYS

{src}.c{obj}.obj:
	$(BCC) $(CFLAGS) -oobj\$&.obj $<

PCBCOMM.EXE: $(OBJS)
	$(BCC) $(LFLAGS) -ePCBCOMM.EXE $(OBJS) $(EXTRA_OBJS)

PCBCOMM.SYS: $(OBJS)
	$(BCC) $(LFLAGS) -tD -ePCBCOMM.SYS $(OBJS) $(EXTRA_OBJS)

!endif

# ---- Microsoft C 7.0 ----
!if "$(CC)" == "MSC70"
MSC    = cl
MODEL  = /AL
CFLAGS = /c /Ox /W3 $(INC) $(DEFS) $(MODEL)
LFLAGS = $(MODEL)

all: PCBCOMM.EXE PCBCOMM.SYS

{src}.c{obj}.obj:
	$(MSC) $(CFLAGS) /Foobj\$&.obj $<

PCBCOMM.EXE: $(OBJS)
	link $(LFLAGS) $(OBJS) $(EXTRA_OBJS), PCBCOMM.EXE;

PCBCOMM.SYS: $(OBJS)
	link $(LFLAGS) $(OBJS) $(EXTRA_OBJS), PCBCOMM.SYS,, /NOD;

!endif

# ---- OpenWatcom 1.9 ----
!if "$(CC)" == "OWC"
WCC    = wcc
MODEL  = -ml
CFLAGS = -c -oxs -bt=dos $(MODEL) $(INC) $(DEFS)
LFLAGS = system dos

all: PCBCOMM.EXE PCBCOMM.SYS

{src}.c{obj}.obj:
	$(WCC) $(CFLAGS) -fo=obj\$&.obj $<

PCBCOMM.EXE: $(OBJS)
	wlink $(LFLAGS) file { $(OBJS) } name PCBCOMM.EXE

PCBCOMM.SYS: $(OBJS)
	wlink $(LFLAGS) file { $(OBJS) } name PCBCOMM.SYS format dos device

!endif

clean:
	del obj\*.obj
	del PCBCOMM.EXE
	del PCBCOMM.SYS
