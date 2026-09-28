# ============================================================================
# pcbcomm.mak — build pcbcomm v1 for dos 16-bit target
#
# compilers supported (selected via %cc% env variable):
#   bc31   borland c++ 3.1        make -f pcbcomm.mak cc=bc31
#   msc70  microsoft c 7.0        make -f pcbcomm.mak cc=msc70
#   owc    openwatcom 1.9         wmake -f pcbcomm.mak cc=owc
#
# all builds produce two artifacts:
#   pcbcomm.exe   tsr (load from autoexec.bat)
#   pcbcomm.sys   device driver (load from config.sys device=)
# ============================================================================

# ---- common ----
inc    = -iinc
defs   = -dpcbcomm_v1
# target selects which pcboard line to build for:
#   target=15.4  (default) — wcsc-parity backends only (lean)
#   target=15.41           — adds stallion/chase/equinox extended backends
!if "$(target)" == "15.41"
defs = $(defs) -dpcb1541
extra_objs = obj\stallion_brumby_backend.obj \
             obj\chase_iolan_backend.obj \
             obj\equinox_sst_backend.obj
!else
extra_objs =
!endif

objs   = obj\pcbcomm.obj      \
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

# ---- borland c++ 3.1 ----
!if "$(cc)" == "bc31"
bcc    = bcc
model  = -ml
cflags = $(model) -c -o -w $(inc) $(defs)
lflags = $(model)

all: pcbcomm.exe pcbcomm.sys

{src}.c{obj}.obj:
	$(bcc) $(cflags) -oobj\$&.obj $<

pcbcomm.exe: $(objs)
	$(bcc) $(lflags) -epcbcomm.exe $(objs) $(extra_objs)

pcbcomm.sys: $(objs)
	$(bcc) $(lflags) -td -epcbcomm.sys $(objs) $(extra_objs)

!endif

# ---- microsoft c 7.0 ----
!if "$(cc)" == "msc70"
msc    = cl
model  = /al
cflags = /c /ox /w3 $(inc) $(defs) $(model)
lflags = $(model)

all: pcbcomm.exe pcbcomm.sys

{src}.c{obj}.obj:
	$(msc) $(cflags) /foobj\$&.obj $<

pcbcomm.exe: $(objs)
	link $(lflags) $(objs) $(extra_objs), pcbcomm.exe;

pcbcomm.sys: $(objs)
	link $(lflags) $(objs) $(extra_objs), pcbcomm.sys,, /nod;

!endif

# ---- openwatcom 1.9 ----
!if "$(cc)" == "owc"
wcc    = wcc
model  = -ml
cflags = -c -oxs -bt=dos $(model) $(inc) $(defs)
lflags = system dos

all: pcbcomm.exe pcbcomm.sys

{src}.c{obj}.obj:
	$(wcc) $(cflags) -fo=obj\$&.obj $<

pcbcomm.exe: $(objs)
	wlink $(lflags) file { $(objs) } name pcbcomm.exe

pcbcomm.sys: $(objs)
	wlink $(lflags) file { $(objs) } name pcbcomm.sys format dos device

!endif

clean:
	del obj\*.obj
	del pcbcomm.exe
	del pcbcomm.sys
