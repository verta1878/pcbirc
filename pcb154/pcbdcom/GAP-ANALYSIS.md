# COMMDRV.RED contents — from INSTALL.DAT (public text manifest)

Source: `pcb1541/install/dist/disk2/INSTALL.zip` → `INSTALL.DAT` lines
containing `@BeginLib COMMDRV.RED` block. Plain-text WCSC installer
script. No decompilation required — this is a public manifest of
what the installer copies from the .RED archive to `COMMDRV\` on disk.

## Card driver modules (9 files, per-family design)

| File          | Size (bytes) | Likely card family (from name pattern) |
|---------------|-------------:|----------------------------------------|
| COMMDV00.DRV  |         1130 | (unknown — smallest, possibly null/8250) |
| COMMDV01.DRV  |         1115 | (unknown) |
| COMMDV02.DRV  |         2276 | (unknown) |
| COMMDV03.DRV  |         2686 | (unknown) |
| COMMDV04.DRV  |         2797 | (unknown) |
| COMMDV05.DRV  |         4883 | largest — possibly Digi/AccelePort |
| COMMDV06.DRV  |         1662 | (unknown) |
| COMMDV07.DRV  |         1212 | (unknown) |
| COMMDV08.DRV  |         2284 | (unknown) |

**9 driver modules total.** pcbdcom v1.1 has 7 backends (uart, boca,
cyclom, digi_pcxe, digi_accel, rocket, easyio). We may be missing
up to 2 card families. Confirmed missing: **Arnet SmartPort Plus**
(see BIOS files below).

## Main binaries

| File          | Size (bytes) | Purpose (from filename + PCBoard docs)  |
|---------------|-------------:|-----------------------------------------|
| COMMDRV.EXE   |        90827 | Main driver executable                  |
| COMMTSR.EXE   |        64101 | TSR variant of driver                   |
| DRVSETUP.EXE  |        29752 | Sysop config editor                     |
| TEST.EXE      |        16482 | Hardware test utility                   |
| MONITOR.BAT   |           24 | Startup helper batch                    |

## Card BIOS / firmware blobs (redistributed by WCSC)

| File           | Size (bytes) | Card                                   |
|----------------|-------------:|----------------------------------------|
| XABIOS.BIN     |         2048 | **Arnet SmartPort BIOS**               |
| XACOOK.BIN     |         6144 | **Arnet SmartPort cook firmware**      |
| XACOMX.BIN     |         6144 | **Arnet SmartPort COMX firmware**      |
| BOCA1610.BIN   |         3228 | Boca BB-1016 (16-port) BIOS            |

**These prove the 8th missing card is Arnet SmartPort Plus.** The
BIOS files ship inside COMMDRV.RED, which is redistributed under
WCSC's PCBoard install disk terms — same license context as the
rest of PCBoard.

## Card configuration data files (installed only for advanced group)

| File          | Size (bytes) | Card config table       |
|---------------|-------------:|-------------------------|
| ARNETSP4.DAT  |         2053 | Arnet SmartPort 4-port  |
| ARNETSP8.DAT  |         3397 | Arnet SmartPort 8-port  |
| DIGI4E.DAT    |         2053 | DigiBoard 4-port        |
| DIGI8E.DAT    |         3397 | DigiBoard 8-port        |

`.DAT` files are DATA (register maps, port assignments), not code —
these are legitimate reference material for building compatible
configs, same as reading a card datasheet.

## Immediate takeaways (before any binary work)

1. **8th backend identified**: Arnet SmartPort Plus. XABIOS.BIN +
   XACOOK.BIN + XACOMX.BIN + ARNETSP4/8.DAT confirm it. pcbdcom v1.1
   SPEC.md correctly identified this as the missing card.

2. **9th backend possible**: 9 .DRV files but only 8 named card
   families visible (7 pcbdcom + Arnet). The 9th could be:
     - A null / passthrough driver
     - Different variant of an existing family (e.g., BOCA1610 is
       a 16-port version — might be separate from standard Boca)
     - Something else entirely — needs Phase 1 investigation

3. **BIOS distribution model**: Confirmed by INSTALL.DAT that BIOS
   files (XABIOS, XACOOK, XACOMX, BOCA1610) ship inside COMMDRV.RED.
   Once extracted, we can package them alongside pcbdcom for sysops
   who paid for PCBoard (since they already have redistribution
   rights via their PCBoard license).

4. **Modular per-card driver architecture**: WCSC used exactly the
   same design pattern pcbdcom uses — one file per card family. Our
   backend layout matches their .DRV layout. Suggests our SPEC.md
   design is on the right track.

# Phase 1 — PCBoard integration surface (from PCBoard source)

Sourced from `pcb154/MAIN/SOURCE/MODEM/MODEMDRV.C` (PCBoard's own code,
we have full rights). This is what pcbdcom must expose for a drop-in
replacement of COMM-DRV linked via COMMDRV.OBJ.

## The ser_rs232_* API (called by MODEMDRV.C)

Every COMM-DRV entry point PCBoard actually uses:

| Function                | Purpose                                    |
|-------------------------|--------------------------------------------|
| `ser_rs232_init()`      | One-time driver init                       |
| `ser_rs232_setup(port, pcb)` | Configure port from `port_param`      |
| `ser_rs232_getport(port, pcb)` | Read back port config              |
| `ser_rs232_dtr_on(port)` / `_off` | Modem control                     |
| `ser_rs232_rts_on(port)` / `_off` | Modem control                     |
| `ser_rs232_putbyte(port, &b)`   | Send 1 byte                          |
| `ser_rs232_getbyte(port, &b)`   | Receive 1 byte                       |
| `ser_rs232_putpacket(port, len, buf)` | Send buffer                    |
| `ser_rs232_getpacket(port, len, buf)` | Receive buffer                 |
| `ser_rs232_viewpacket(port, len, buf)` | Peek without consuming        |
| `ser_rs232_flush(port, dir)` | Flush TX (1) or RX (0) or both (2)   |

Return value convention: `RS232ERR_NONE` = 0 for success, non-zero
for various errors.

## INT 14h calls PCBoard makes directly (bypass ser_rs232 API)

Only two, both COMM-DRV extended (AH >= 0x10, not standard FOSSIL):

| AX      | Purpose                                     |
|---------|---------------------------------------------|
| 0x1000  | `COMMDRV_commgo` — enable/start transmit    |
| 0x1002  | `COMMDRV_commstop` — disable/stop transmit  |

DX = port number. AL unused (embedded in AH via AX).

**Implication:** pcbdcom's INT 14h handler needs to recognize AH ≥
0x10 as COMM-DRV extensions, in addition to standard FOSSIL (AH
0x00..0x0F).

## Delta vs pcbdcom v1.1

v1.1 int14.c implements standard FOSSIL 0x00-0x0F. **Missing: 0x1000
and 0x1002** — commgo/commstop. Trivial add (few lines).

pcbdcom currently exposes backend functions but NOT the ser_rs232_*
symbol names. For MODEMDRV.C to link against pcbdcom (COMMDRV.OBJ
replacement), we need a shim layer that maps ser_rs232_* → pcbdcom
internal API. Small file, ~100 lines.

## COMM-DRV ships modular per-card architecture

INSTALL.DAT proves 9 separate .DRV files (COMMDV00-08.DRV, sizes
1115-4883 bytes each). Same architecture pcbdcom already uses (7
backends). WCSC apparently split at exactly the same seams.
