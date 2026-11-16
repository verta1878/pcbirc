# ============================================================
#  BLDPCBCOMM.MAK — Open Watcom V2, 32-bit flat model
#  pcbcomm serial library: 19 modules -> PCBCOMM_WF.LIB
#  delta154 / pcb1541
#
#  Usage:  wmake -f bldpcbcomm.mak
#          wmake -f bldpcbcomm.mak clean
#
#  Requires: WATCOM environment variable set
#  Built by: hexadecimal — pcbirc crew, November 2026
# ============================================================

# --- Paths ---
SRCDIR = pcb1541\pcbcomm\src
INCDIR = pcb1541\pcbcomm\inc
TKDIR  = toolkit\delta154\h
OBJDIR = out\delta154\sdk\ow2flat\obj
OUTDIR = out\delta154\sdk\ow2flat

# --- Compiler ---
CC     = wcc386
CFLAGS = -oxs -bt=dos -mf -i$(%WATCOM)\h -i$(INCDIR) -i$(TKDIR) -dPCBCOMM_V1 -dPCB1541

# --- Librarian ---
LIB    = wlib
LFLAGS = -n

# --- Module list (19) ---
OBJS = &
    $(OBJDIR)\pcbcomm.obj &
    $(OBJDIR)\irq.obj &
    $(OBJDIR)\uart.obj &
    $(OBJDIR)\uart_backend.obj &
    $(OBJDIR)\int14.obj &
    $(OBJDIR)\fossil.obj &
    $(OBJDIR)\commdrbl.obj &
    $(OBJDIR)\libsbl.obj &
    $(OBJDIR)\card_pool.obj &
    $(OBJDIR)\boca_backend.obj &
    $(OBJDIR)\cyclom_backend.obj &
    $(OBJDIR)\digi_pcxe_backend.obj &
    $(OBJDIR)\digi_accel_backend.obj &
    $(OBJDIR)\digi_fep.obj &
    $(OBJDIR)\rocket_backend.obj &
    $(OBJDIR)\easyio_backend.obj &
    $(OBJDIR)\arnet_backend.obj &
    $(OBJDIR)\hub6_backend.obj &
    $(OBJDIR)\gtek_backend.obj

# --- SDK-layer OBJs shipped alongside the library ---
SDK_OBJS = &
    $(OUTDIR)\commdrbl.obj &
    $(OUTDIR)\libsbl.obj &
    $(OUTDIR)\fossil.obj

# --- Target ---
TARGET = $(OUTDIR)\PCBCOMM_WF.LIB

# --- Default target ---
all: dirs $(TARGET) $(SDK_OBJS) .SYMBOLIC
    @echo.
    @echo === PCBCOMM_WF.LIB built successfully ===

# --- Create output directories ---
dirs: .SYMBOLIC
    @if not exist out md out
    @if not exist out\delta154 md out\delta154
    @if not exist out\delta154\sdk md out\delta154\sdk
    @if not exist out\delta154\sdk\ow2flat md out\delta154\sdk\ow2flat
    @if not exist $(OBJDIR) md $(OBJDIR)

# --- Inference rule: .c -> .obj ---
.c: $(SRCDIR)

.c.obj:
    $(CC) $(CFLAGS) -fo=$(OBJDIR)\$^&.obj $<

# --- Build the library from all OBJs ---
$(TARGET): $(OBJS)
    $(LIB) $(LFLAGS) $@ $(OBJS)

# --- Copy SDK-layer OBJs alongside library ---
$(OUTDIR)\commdrbl.obj: $(OBJDIR)\commdrbl.obj
    copy $(OBJDIR)\commdrbl.obj $(OUTDIR)\commdrbl.obj >nul

$(OUTDIR)\libsbl.obj: $(OBJDIR)\libsbl.obj
    copy $(OBJDIR)\libsbl.obj $(OUTDIR)\libsbl.obj >nul

$(OUTDIR)\fossil.obj: $(OBJDIR)\fossil.obj
    copy $(OBJDIR)\fossil.obj $(OUTDIR)\fossil.obj >nul

# --- Clean ---
clean: .SYMBOLIC
    @if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
    @if exist $(TARGET) del $(TARGET)
    @if exist $(OUTDIR)\commdrbl.obj del $(OUTDIR)\commdrbl.obj
    @if exist $(OUTDIR)\libsbl.obj del $(OUTDIR)\libsbl.obj
    @if exist $(OUTDIR)\fossil.obj del $(OUTDIR)\fossil.obj
    @echo Cleaned.
