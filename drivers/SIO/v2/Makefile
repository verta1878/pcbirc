# ====================================================================
# SIO v2.0.0 (SIO2K) — Master Build Configuration
# ====================================================================
# Cross-compiles OS/2 device drivers and utilities using OpenWatcom v2.
# Previously this project had no Makefile at all — every module had
# to be built by hand. Modeled on sio1src/Makefile's structure so the
# two projects build the same way.
#
# Usage:
#   make all        — build everything
#   make sio2k      — build SIO2K.SYS only (logical layer)
#   make uart       — build UART.SYS only (physical layer)
#   make vsio2k     — build VSIO2K.SYS only (VDD)
#   make vmodem     — build VMODEM.SYS + VMODEM.EXE only
#   make esp        — build ESP.SYS only
#   make vx00       — build VX00.SYS only
#   make utils      — build LOGGER, MODES, PCI, PMLM, VIEWPMLM
#   make tools      — build D4TEST
#   make clean      — remove build artifacts
#
# Prerequisites:
#   OpenWatcom v2 with WATCOM env var set to install directory.
#   e.g. export WATCOM=/usr/local/watcom
# ====================================================================

# --- Toolchain ---
WATCOM  ?= /home/claude/watcom
ASM      = $(WATCOM)/binl64/wasm
CC16     = $(WATCOM)/binl64/wcc
CC32     = $(WATCOM)/binl64/wcc386
LINK     = $(WATCOM)/binl64/wlink

# --- Include paths ---
WCINC    = $(WATCOM)/h
WCOS2    = $(WATCOM)/h/os2
TKINC    = inc

# --- Compiler flags ---
# 16-bit: for SIO2K.SYS/UART.SYS/VMODEM.SYS/ESP.SYS (OS/2 PDDs)
CFLAGS16 = -bt=os2 -ms -s -ox -zq -i=$(WCINC) -i=$(WCOS2) -i=$(TKINC)

# 16-bit DOS target: for VX00.SYS (FOSSIL driver runs in a DOS session)
CFLAGS16_DOS = -bt=dos -ms -s -ox -zq -i=$(WCINC) -i=$(TKINC)

# 32-bit: for VSIO2K.SYS (VDD) and OS/2 console applications
CFLAGS32    = -bt=os2v2 -mf -s -ox -zq -i=$(WCINC) -i=$(WCOS2) -i=$(TKINC)
CFLAGS32APP = -bt=os2v2 -mf -ox -zq -i=$(WCINC) -i=$(WCOS2) -i=$(TKINC)

# --- Output directory ---
OUT      = out

# ====================================================================
# Targets
# ====================================================================

all: dirs sio2k uart vsio2k vmodem esp vx00 utils tools

dirs:
	@mkdir -p $(OUT)

# --- SIO2K.SYS (16-bit OS/2 PDD — logical layer) ---
sio2k: dirs $(OUT)/SIO2K.SYS

$(OUT)/sio2k.obj: sio2k/sio2k.c inc/sio2k_idc.h inc/driver.h inc/vsio.h \
                  sio2k/cfgparse.h
	$(CC16) $(CFLAGS16) -fo=$(OUT)/sio2k.obj sio2k/sio2k.c

$(OUT)/cfgparse.obj: sio2k/cfgparse.c sio2k/cfgparse.h
	$(CC16) $(CFLAGS16) -fo=$(OUT)/cfgparse.obj sio2k/cfgparse.c

$(OUT)/SIO2K.SYS: $(OUT)/sio2k.obj $(OUT)/cfgparse.obj
	$(LINK) system os2 name $(OUT)/SIO2K.SYS \
	    file $(OUT)/sio2k.obj,$(OUT)/cfgparse.obj

# --- UART.SYS (16-bit OS/2 PDD — physical layer) ---
uart: dirs $(OUT)/UART.SYS

$(OUT)/uart.obj: uart/uart.c inc/sio2k_idc.h
	$(CC16) $(CFLAGS16) -fo=$(OUT)/uart.obj uart/uart.c

$(OUT)/UART.SYS: $(OUT)/uart.obj
	$(LINK) system os2 name $(OUT)/UART.SYS file $(OUT)/uart.obj

# --- VSIO2K.SYS (32-bit OS/2 VDD) ---
vsio2k: dirs $(OUT)/VSIO2K.SYS

$(OUT)/vsio2k.obj: vsio2k/vsio2k.c inc/vsio.h
	$(CC32) $(CFLAGS32) -fo=$(OUT)/vsio2k.obj vsio2k/vsio2k.c

$(OUT)/VSIO2K.SYS: $(OUT)/vsio2k.obj
	$(LINK) system os2v2 dll option quiet name $(OUT)/VSIO2K.SYS \
	    file $(OUT)/vsio2k.obj export VDDInit

# --- VMODEM.SYS (16-bit OS/2 PDD, physical layer) + VMODEM.EXE (32-bit app) ---
vmodem: dirs $(OUT)/VMODEM.SYS $(OUT)/VMODEM.EXE

$(OUT)/vmodem_sys.obj: vmodem/vmodem_sys.c inc/sio2k_idc.h
	$(CC16) $(CFLAGS16) -fo=$(OUT)/vmodem_sys.obj vmodem/vmodem_sys.c

$(OUT)/VMODEM.SYS: $(OUT)/vmodem_sys.obj
	$(LINK) system os2 name $(OUT)/VMODEM.SYS file $(OUT)/vmodem_sys.obj

$(OUT)/vmodem_exe.obj: vmodem/vmodem_exe.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/vmodem_exe.obj vmodem/vmodem_exe.c

$(OUT)/VMODEM.EXE: $(OUT)/vmodem_exe.obj
	$(LINK) system os2v2 name $(OUT)/VMODEM.EXE file $(OUT)/vmodem_exe.obj

# --- ESP.SYS (16-bit OS/2 PDD — Hayes ESP physical layer) ---
esp: dirs $(OUT)/ESP.SYS

$(OUT)/esp.obj: esp/esp.c inc/sio2k_idc.h
	$(CC16) $(CFLAGS16) -fo=$(OUT)/esp.obj esp/esp.c

$(OUT)/ESP.SYS: $(OUT)/esp.obj
	$(LINK) system os2 name $(OUT)/ESP.SYS file $(OUT)/esp.obj

# --- VX00.SYS (16-bit DOS device driver — FOSSIL for DOS sessions) ---
vx00: dirs $(OUT)/VX00.SYS

$(OUT)/vx00.obj: vx00/vx00.c
	$(CC16) $(CFLAGS16_DOS) -fo=$(OUT)/vx00.obj vx00/vx00.c

$(OUT)/VX00.SYS: $(OUT)/vx00.obj
	$(LINK) system dos name $(OUT)/VX00.SYS file $(OUT)/vx00.obj

# --- Utilities (32-bit OS/2 console/PM apps) ---
utils: dirs $(OUT)/LOGGER.EXE $(OUT)/MODES.EXE $(OUT)/PCI.EXE \
            $(OUT)/PMLM.EXE $(OUT)/VIEWPMLM.EXE

$(OUT)/logger.obj: logger/logger.c inc/bsedev.h
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/logger.obj logger/logger.c

$(OUT)/LOGGER.EXE: $(OUT)/logger.obj
	$(LINK) system os2v2 name $(OUT)/LOGGER.EXE file $(OUT)/logger.obj

$(OUT)/modes.obj: modes/modes.c inc/bsedev.h
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/modes.obj modes/modes.c

$(OUT)/MODES.EXE: $(OUT)/modes.obj
	$(LINK) system os2v2 name $(OUT)/MODES.EXE file $(OUT)/modes.obj

$(OUT)/pci.obj: pci/pci.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/pci.obj pci/pci.c

$(OUT)/PCI.EXE: $(OUT)/pci.obj
	$(LINK) system os2v2 name $(OUT)/PCI.EXE file $(OUT)/pci.obj

$(OUT)/pmlm.obj: pmlm/pmlm.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/pmlm.obj pmlm/pmlm.c

$(OUT)/PMLM.EXE: $(OUT)/pmlm.obj
	$(LINK) system os2v2 name $(OUT)/PMLM.EXE file $(OUT)/pmlm.obj

$(OUT)/viewpmlm.obj: pmlm/viewpmlm.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/viewpmlm.obj pmlm/viewpmlm.c

$(OUT)/VIEWPMLM.EXE: $(OUT)/viewpmlm.obj
	$(LINK) system os2v2 name $(OUT)/VIEWPMLM.EXE file $(OUT)/viewpmlm.obj

# --- Test tools ---
tools: dirs $(OUT)/D4TEST.EXE

$(OUT)/d4test.obj: tools/d4test.c inc/bsedev.h
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/d4test.obj tools/d4test.c

$(OUT)/D4TEST.EXE: $(OUT)/d4test.obj
	$(LINK) system os2v2 name $(OUT)/D4TEST.EXE file $(OUT)/d4test.obj

# --- Clean ---
clean:
	rm -rf $(OUT)

.PHONY: all dirs sio2k uart vsio2k vmodem esp vx00 utils tools clean
