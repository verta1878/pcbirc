# pcbcomm SDK — Open Watcom 2.0 Flat (32-bit)

**delta154 / pcb1541**

Prebuilt pcbcomm serial library for door developers linking against
PCBoard 15.4x under Open Watcom V2, 32-bit flat memory model (DOS + OS/2).

## Deliverables

| File | Size | Description |
|---|---|---|
| PCBCOMM_WF.LIB | 59,904 | All-in-one library — 19 modules, 10 backends + core + SDK layer + FOSSIL |
| commdrbl.obj | 2,728 | SDK layer: ser_rs232_* (COMMDRV-compatible API) |
| libsbl.obj | 1,936 | SDK layer: SBL linkable shim |
| fossil.obj | 11,636 | FOSSIL driver module (full OBJ) |

## Headers

Ship alongside the library (also in `toolkit/delta154/h/`):

| File | Size | Description |
|---|---|---|
| COMM.H | 6,720 | Door-developer include — `#include "COMM.H"` |
| pcbcomm.h | 2,307 | Core pcbcomm types (pcbcomm_port_t, constants) |
| watcompat.h | 11,028 | Borland-to-Watcom compatibility shim |
| borland.h | 3,046 | Borland type/macro bridge (pulls watcompat.h) |

## How to link

```c
/* In your door source: */
#include "COMM.H"
```

Link against PCBCOMM_WF.LIB:
```
wlink system dos4g file mydoor.obj library PCBCOMM_WF.LIB
```

Or link individual OBJs if you only need part of the SDK layer:
```
wlink system dos4g file mydoor.obj file commdrbl.obj file fossil.obj
```

## Build info

- Compiler: Open Watcom V2 (wcc386), Linux cross-compile
- Model: flat 32-bit (`-mf`)
- Target: DOS (`-bt=dos`)
- Optimization: `-oxs` (space + time)
- Defines: `-dPCBCOMM_V1 -dPCB1541`
- Build script: `main/build/scripts/bldpcbcomm.bat`
- Source: `pcb1541/pcbcomm/src/`

## 19 modules in PCBCOMM_WF.LIB

pcbcomm, irq, uart, uart_backend, int14, fossil, commdrbl, libsbl,
card_pool, boca_backend, cyclom_backend, digi_pcxe_backend,
digi_accel_backend, digi_fep, rocket_backend, easyio_backend,
arnet_backend, hub6_backend, gtek_backend.

## 10 backends

uart (8250/16550), boca (Boca 1610), cyclom (Cyclades Cyclom-Y),
digi_pcxe (DigiBoard PC/Xe), digi_accel (DigiBoard Accel),
rocket (Comtrol RocketPort), easyio (Stallion EasyIO),
arnet (Arnet SmartPort), hub6 (Intel HUB-6), gtek (GTEK BBS-550/8Fx).

## Checksums

See `checksums.sha256` and `checksums.md5` in the repo root.

## Built by

hexadecimal — pcbirc crew, November 2026
