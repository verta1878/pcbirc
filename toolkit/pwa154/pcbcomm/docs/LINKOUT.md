# Linking pcbcomm into PCBoard

PCBoard's `MODEMDRV.C` is `#ifdef COMMDRV`-gated and calls the
`ser_rs232_*` API. Clark's link recipe uses `COMMDRV.OBJ + FOSSIL.OBJ`
from the PCBoard toolkit .ZIP.

Substitute our `COMMDRV.OBJ + FOSSIL.OBJ` from `pcbcbase/COMMDRV/OBJ/`
and add `commdrbl.lib + libsbl.lib` from `pcbcbase/COMMDRV/LIB/`.

## How it links

SDK doors and utilities link STUBS — they call PCBoard's API at
runtime, not the hardware directly. PCBOARD.EXE links REAL CODE
(ASYNC.ASM + full MODEM.C chain + commdrbl.lib + libsbl.lib) because
it talks to hardware.

That's why UUXFER links FOSSIL.OBJ + NO*.OBJ stubs while PCBOARD.MAK
links ASYNC.ASM + the full modem chain.

## Historical note

PCBoard 15.x kept `#ifdef COMMDRV` intact in `MODEMDRV.C`. pcbirc
preserves that block untouched and adds a parallel `#ifdef PCBCOMM`
block for our extensions. Either can be built; both work.

Clark had the COMMDRV infrastructure ready in PCBOARD.MAK but never
finished the libs before the bank closed Clark Development. The crew's
commdrbl.lib + libsbl.lib complete what Clark started.

## What you gain

* GPLv3 source, no proprietary binary dependency
* 15 card families (all Clark boards + 6 post-WCSC)
* Multi-port routing across all supported cards
* Standard FOSSIL INT 14h + COMM-DRV extensions (AH >= 0x10)
* Cross-compiler support (BC 3.1, OpenWatcom — MSC 7.0 pending)

## What you keep

* Byte-exact `ser_rs232_*` API surface (13 functions, cdecl)
* Binary-compatible `port_param` struct
* Return codes matching COMM-DRV `RS232ERR_*` constants
