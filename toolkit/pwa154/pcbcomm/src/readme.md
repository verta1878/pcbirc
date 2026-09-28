# pcbcomm source

Hardware-serial backends for PCBoard. Each backend implements
`pcbcomm_backend_t` (see `../inc/backend.h`).

## Source files

| File | Card family | Chip/method | Status |
|---|---|---|---|
| `uart_backend.c` | Standard 8250/16550 | UART registers | complete |
| `boca_backend.c` | Boca BB-1004/BB-2016 | 16550 + shared IRQ | complete |
| `hub6_backend.c` | Intel HUB6 | 8250 + port-select mux | complete |
| `gtek_backend.c` | GTEK BBS-550/8Fx | 16550 + shared IRQ + status reg | complete |
| `cyclom_backend.c` | Cyclades Cyclom-Y | CD1400 memory-mapped | complete |
| `digi_pcxe_backend.c` | DigiBoard PC/Xe | FEP protocol (digi_fep.c) | complete |
| `digi_accel_backend.c` | DigiBoard AccelePort | FEP protocol (digi_fep.c) | complete |
| `digi_fep.c` | (shared) DigiBoard FEP | dual-port RAM protocol | complete |
| `rocket_backend.c` | Comtrol RocketPort | MUDBAC + AIOP | complete |
| `easyio_backend.c` | Stallion EasyIO | CD1400 I/O-mapped | complete |
| `arnet_backend.c` | Arnet SmartPort Plus | UART + mux | complete |
| `pcbcomm.c` | Main entry, TSR loader, config parser, backend registry | — |
| `pcbcomms.c` | ser_rs232_* shim (COMMDRV API surface) | — |
| `pcbdcom.c` | Legacy pcbdcom compat wrapper | — |
| `int14.c` | INT 14h hook / FOSSIL dispatch | — |
| `irq.c` | IRQ handler install/uninstall, PIC programming | — |
| `uart.c` | UART chip probe + register I/O (shared by backends) | — |
| `card_pool.c` | Per-card state allocator | — |

## COMM-DRV card coverage

All 7 hardware backends from WCSC's COMMDRV distribution are covered:

| COMMDRV DRV | Our backend | Notes |
|---|---|---|
| COMMDV00 (GENERIC) | uart_backend | 8250/16550 |
| COMMDV01 (INTEL HUB6) | hub6_backend | port-select mux at 0x302 |
| COMMDV02 (DIGI-COMXI) | digi_pcxe_backend | same FEP protocol |
| COMMDV03 (ARNET-SPORT) | arnet_backend | SmartPort Plus |
| COMMDV04 (BOCA 1610) | boca_backend | shared IRQ |
| COMMDV05 (DIGI-PCX*) | digi_pcxe_backend | loads xabios.bin |
| COMMDV06 (GTEK 8Fx) | gtek_backend | shared IRQ + status reg |

Plus 3 backends beyond WCSC: cyclom, rocket, easyio.
COMMDV07 (INT14H/FOSSIL) handled by int14.c.
COMMDV08 (VxD) handled by VxD bridge, not a backend.
