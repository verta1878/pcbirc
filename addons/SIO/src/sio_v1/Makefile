# ====================================================================
# SIO Suite — Master Build Configuration
# ====================================================================
# Cross-compiles OS/2 device drivers and utilities using OpenWatcom v2.
#
# Usage:
#   make all        — build everything
#   make sio        — build SIO.SYS only
#   make vsio       — build VSIO.SYS only
#   make vx00       — build VX00.SYS only
#   make vmodem     — build VMODEM.EXE only
#   make utils      — build SU, PMLM, VIEWPMLM, INSTALL
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
SIOINC   = sio

# --- Compiler flags ---
# 16-bit: for SIO.SYS PDD and VX00.SYS DOS driver
AFLAGS   = -bt=os2 -ms -fo=$@ -i=$(TKINC) -i=$(SIOINC)
CFLAGS16 = -bt=os2 -ms -s -ox -zq -i=$(WCINC) -i=$(WCOS2) -i=$(TKINC)

# 32-bit: for VSIO.SYS VDD and OS/2 applications
CFLAGS32 = -bt=os2v2 -mf -s -ox -zq -i=$(WCINC) -i=$(WCOS2) -i=$(TKINC)
CFLAGS32APP = -bt=os2v2 -mf -ox -zq -i=$(WCINC) -i=$(WCOS2) -i=$(TKINC)

# VX00 is a DOS .SYS — needs DOS target
AFLAGS_DOS = -bt=dos -ms -fo=$@ -i=$(TKINC) -i=$(SIOINC)

# --- Output directory ---
OUT      = out

# ====================================================================
# Targets
# ====================================================================

all: dirs sio vsio vx00 vmodem utils

dirs:
	@mkdir -p $(OUT)

# --- SIO.SYS (16-bit OS/2 PDD) ---
sio: dirs $(OUT)/SIO.SYS

$(OUT)/sio.obj: sio/sio.asm sio/uart.inc sio/siodata.inc \
                sio/sioinit.asm sio/sioopen.asm sio/sioio.asm \
                sio/siostatus.asm sio/sioioctl.asm sio/sioisr.asm \
                sio/siouart.asm sio/siobuf.asm
	$(ASM) $(AFLAGS) sio/sio.asm

$(OUT)/SIO.SYS: $(OUT)/sio.obj
	$(LINK) @sio/sio.lnk

# --- VSIO.SYS (32-bit OS/2 VDD) ---
vsio: dirs $(OUT)/VSIO.SYS

$(OUT)/vsio.obj: vsio/vsio.c vsio/vsio.h inc/vsio.h
	$(CC32) $(CFLAGS32) -fo=$(OUT)/vsio.obj vsio/vsio.c

$(OUT)/VSIO.SYS: $(OUT)/vsio.obj
	$(LINK) @vsio/vsio.lnk

# --- VX00.SYS (16-bit DOS device driver) ---
vx00: dirs $(OUT)/VX00.SYS

$(OUT)/vx00.obj: vx00/vx00.asm
	$(ASM) $(AFLAGS_DOS) vx00/vx00.asm

$(OUT)/VX00.SYS: $(OUT)/vx00.obj
	$(LINK) @vx00/vx00.lnk

# --- VMODEM.EXE (32-bit OS/2 console app) ---
vmodem: dirs $(OUT)/VMODEM.EXE

$(OUT)/vmodem.obj: vmodem/vmodem.c vmodem/vmodem.h vmodem/md5.h
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/vmodem.obj vmodem/vmodem.c

$(OUT)/md5.obj: vmodem/md5.c vmodem/md5.h
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/md5.obj vmodem/md5.c

$(OUT)/VMODEM.EXE: $(OUT)/vmodem.obj $(OUT)/md5.obj
	$(LINK) @vmodem/vmodem.lnk

# --- Utilities ---
utils: dirs $(OUT)/SU.EXE $(OUT)/PMLM.EXE $(OUT)/VIEWPMLM.EXE $(OUT)/INSTALL.EXE

$(OUT)/su.obj: su/su.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/su.obj su/su.c

$(OUT)/SU.EXE: $(OUT)/su.obj
	$(LINK) system os2v2 name $(OUT)/SU.EXE file $(OUT)/su.obj

$(OUT)/pmlm.obj: pmlm/pmlm.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/pmlm.obj pmlm/pmlm.c

$(OUT)/PMLM.EXE: $(OUT)/pmlm.obj
	$(LINK) system os2v2 name $(OUT)/PMLM.EXE file $(OUT)/pmlm.obj

$(OUT)/viewpmlm.obj: viewpmlm/viewpmlm.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/viewpmlm.obj viewpmlm/viewpmlm.c

$(OUT)/VIEWPMLM.EXE: $(OUT)/viewpmlm.obj
	$(LINK) system os2v2 name $(OUT)/VIEWPMLM.EXE file $(OUT)/viewpmlm.obj

$(OUT)/install.obj: install/install.c
	$(CC32) $(CFLAGS32APP) -fo=$(OUT)/install.obj install/install.c

$(OUT)/INSTALL.EXE: $(OUT)/install.obj
	$(LINK) system os2v2 name $(OUT)/INSTALL.EXE file $(OUT)/install.obj

# --- Clean ---
clean:
	rm -rf $(OUT)

.PHONY: all dirs sio vsio vx00 vmodem utils clean
