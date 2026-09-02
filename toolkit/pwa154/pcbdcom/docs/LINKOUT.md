# Linking pcbdcom into PCBoard

PCBoard's `MODEMDRV.C` is `#ifdef COMMDRV`-gated and calls the
`ser_rs232_*` API. Clark's link recipe uses `COMMDRV.OBJ + FOSSIL.OBJ`
from the PCBoard toolkit .ZIP.

Substitute `PCBDCOM_BL.OBJ + FOSSIL.OBJ` and rebuild. Everything else
stays the same.

## Historical note

PCBoard 15.x kept `#ifdef COMMDRV` intact in `MODEMDRV.C`. pcbirc
preserves that block untouched and adds a parallel `#ifdef PCBDCOM`
block for our extensions. Either can be built; both work.

## What you gain

* GPLv3 source, no proprietary binary dependency
* 8 card families (v1.2 adds Arnet SmartPort Plus)
* Multi-port routing across all supported cards
* Standard FOSSIL INT 14h + COMM-DRV extensions (AH >= 0x10)
* Cross-compiler support (BC 3.1, OpenWatcom, MSC 7.0)

## What you keep

* Byte-exact `ser_rs232_*` API surface
* Pascal calling convention, uppercase symbols
* Binary-compatible `port_param` struct
* Return codes matching COMM-DRV `RS232ERR_*` constants
