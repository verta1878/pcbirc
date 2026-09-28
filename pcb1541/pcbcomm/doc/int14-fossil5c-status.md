# int14.c — FOSSIL 5 Revision C Status

## pcbcomm INT 14h handler for PCBoard
### pcbirc crew, GPLv3

## Architecture — How PCBoard Talks to Serial

PCBoard has four serial backends, selected at startup by `MODEM.C` (line 510):
COMM-DRV first if built, then FOSSIL, else bare-metal Async. Four files
implement the backends:

```
MODEM.C (selector — lines 30-35 declare all three)
  │
  ├── MODEMDRV.C  (521 lines)  COMMDRV_* — linked C API via <comm.h>
  ├── MODEMFOS.C  (811 lines)  FOSSIL_* — pure INT 14h (19 geninterrupt sites)
  ├── MODEMASY.C  (256 lines)  ASYNC_*  — calls ASYNC.ASM bare-metal UART
  └── MODEMOS2.C  (1,081 lines) OS/2 path

All in pcb153/SOURCE/MODEM/ (not SOURCE/MAIN/).
```

**ASYNC.ASM** (pcb153/SOURCE/ASM/, 1,893 lines, 79,225 bytes) is Clark's
bare-metal serial driver. Zero INT 14h calls. Programs the UART directly.
23 `ASYNC_*` functions plus 5 data exports = 28 PUBLIC statements total.

**MODEMFOS.C** is the FOSSIL client — pure INT 14h. 37 `FOSSIL_*` entry points.

**MODEMDRV.C** is the COMM-DRV client — NOT INT 14h. Uses WCSC's linked C
API: 13 `ser_rs232_*` functions via `<comm.h>` plus shared `pcb.opcb->`
structure. Only 2 `asm int 14h` sites (lines 264/271: AX=1000h commgo,
AX=1002h commstop). 33 `COMMDRV_*` entry points.

## pcbcomm Scope

**Primary: FOSSIL driver (option a)**. `MODEMFOS.C` is pure INT 14h — exactly
what `int14.c` provides. pcbcomm installs on INT 14h, PCBoard reaches it
through the existing `MODEMFOS.C` path, no new code needed in PCBoard.

**Future: COMM-DRV replacement (option c)**. Requires shipping a `comm.h`-
compatible linkable library with the 13 `ser_rs232_*` functions and byte-
compatible `port_param`/`opcb` layout. A TSR on INT 14h cannot satisfy this.
`ser_rs232_shim.c` is a start but `comm.h` itself is not in the repo —
it was part of the proprietary COMM-DRV SDK. This is the real unknown.

```
pcbcomm identity:
  Primary:   FOSSIL driver (option a — working now)
  Future:    COMM-DRV replacement (option c — needs comm.h)

PCBoard selection order (MODEM.C line 510):
  1. COMM-DRV loaded? → MODEMDRV.C → linked C API    → needs comm.h library
  2. FOSSIL loaded?   → MODEMFOS.C → INT 14h          → pcbcomm ✓
  3. Neither?         → MODEMASY.C → ASYNC.ASM → bare UART (no pcbcomm)
```

## Toolkit fossil.obj

`FOSSIL.OBJ = MODEMFOS.C compiled with -DCOMM -DMULTIPORT -DLIB`

Clark's `LIB` ifdef strips PCBoard internals (errorexittodos, recycle,
reportcom). The module name difference comes from the compiler output:
`bcc -oFOSSIL.OBJ MODEMFOS.C`. Source is at `pcb153/SOURCE/MODEM/MODEMFOS.C`
(811 lines). Target: `OUT/PWA153/SDK/BC31/OBJ/TOOLKIT/LARGE/FOSSIL.OBJ`.

UUXFER is the first consumer, blocked by `GPROT.CPP` `xor_val` (now fixed).

## MODEMFOS.C INT 14h Call Sites

These are the FOSSIL functions PCBoard actually calls through MODEMFOS.C:

```
Fn    AX/AH       Function              MODEMFOS.C usage
───   ─────       ────────              ────────────────
00h   AH=00h      Set baud/LCR          FOSSIL_setport
01h   AH=01h      TX char (wait)        FOSSIL_csendbyte
02h   AH=02h      RX char (wait)        FOSSIL_readin (1-byte path)
03h   AH=03h      Get status            FOSSIL_online (checks MSR DCD)
04h   AH=04h      Init                  FOSSIL_initializedriver
09h   AH=09h      Purge output          FOSSIL_clearoutbuf
0Ah   AH=0Ah      Purge input           FOSSIL_clearinbuf
0Ch   AH=0Ch      Peek RX               FOSSIL_checkcomm
0Fh   AX=0F02h    Flow control (CTS)    FOSSIL_openmodem (enables CTS/RTS)
10h   AX=1000h    commgo                FOSSIL_commgo + openmodem ctrl-C disable
10h   AX=1002h    commstop              FOSSIL_commstop
18h   AH=18h      Block read            FOSSIL_readin (multi-byte path) ← ES:DI
19h   AH=19h      Block write           FOSSIL_csendstr ← ES:DI
1Bh   AH=1Bh      Get driver info       FOSSIL_bytesinbuffer + outbytes ← ES:DI
      AX=0600h    DTR off               FOSSIL_turnoffdtr
      AX=0601h    DTR on                FOSSIL_turnondtr
```

**Critical:** 18h, 19h, and 1Bh are the three functions that were stubs.
They are precisely the ones PCBoard's FOSSIL client uses with `ES:DI`
buffers. Fixed — they now write/read via `PCBCOMM_FAR_PTR(ES, DI)`.

## COMM-DRV Extension Dispatch — CORRECTED

The COMM-DRV extensions are NOT AH=10h through AH=14h. They are all
**AH=10h with AL as the sub-function selector**:

```
AX value   AL    Operation
0x1000     00    commgo — start transmit
0x1001     01    query port count
0x1002     02    commstop — stop TX, flush buffers
0x1003     03    query backend name
0x1004     04    set/get baud rate
```

Both `MODEMFOS.C` and `MODEMDRV.C` send `AX=1000h` (commgo) and
`AX=1002h` (commstop). The previous `int14.c` had commgo at `case 0x10`
and commstop at `case 0x12` — so a commstop from PCBoard landed on
commgo. **Fixed:** all extensions now dispatch under `case 0x10` with
`switch(ch)` on AL.

## Function Coverage — int14.c

FOSSIL 00h–10h, 15h–1Bh, with the COMM-DRV extension riding inside
AH=10h on AL. No cases 11h–14h (absorbed into 10h sub-functions).
Matches `rlfossil.c` coverage (00h–10h, 18h–1Bh). Every function
PCBoard calls through MODEMFOS.C is implemented.

```
Fn   Name                    Status       Notes
───  ────                    ──────       ─────
00h  Set baud/LCR            DONE         Parses AL per FOSSIL encoding
01h  TX char (wait)          DONE         Blocks until sent
02h  RX char (wait)          DONE         Blocks until received
03h  Get status              DONE         cached_msr (all backends)
04h  Init                    DONE         Returns 0x1954, opens port
05h  Deinit                  DONE         Calls backend deinit
06h  Set DTR                 DONE         MCR register
07h  Timer tick params       DONE         Returns 18/55
08h  Flush output            DONE         Drains TX ring
09h  Purge output            DONE         Discards TX ring
0Ah  Purge input             DONE         Discards RX ring
0Bh  TX char (no wait)       DONE         Returns 1/0
0Ch  Peek RX                 DONE         Non-destructive
0Dh  Keyboard read           DONE         INT 16h AH=00h
0Eh  Keyboard peek           DONE         INT 16h AH=01h
0Fh  Flow control            DONE         RTS/CTS, XON/XOFF, none
10h  COMM-DRV ext (AL=sub)   DONE         AL=00 commgo, 01 count, 02 stop, 03 name, 04 baud
15h  Write char+attr         DONE         INT 10h AH=09h
16h  Timer chain             DONE         ES:DX callback, _chain_intr (low priority, never called)
17h  Reboot                  DONE         Cold/warm, all 3 compilers
18h  Block read              DONE         ES:DI via PCBCOMM_FAR_PTR ← PCBoard uses this
19h  Block write             DONE         ES:DI via PCBCOMM_FAR_PTR ← PCBoard uses this
1Ah  Break signal            DONE         LCR break bit
1Bh  Get driver info         DONE         ES:DI via PCBCOMM_FAR_PTR ← PCBoard uses this
```

No stubs remain. All functions PCBoard calls through MODEMFOS.C are implemented.

## Resolved Issues

```
1. [x] Derive toolkit FOSSIL.OBJ source
       CORRECTED: FOSSIL.OBJ is NOT just MODEMFOS.C. OMF source path is
       y:\modem.c — it's MODEM.C compiled with -DCOMM -DMULTIPORT -DLIB
       -DFOSSIL, which pulls in MODEMFOS.C + MODEMASY.C as one unit.
       Original extracted from TOOLKIT2.ZIP → TOOLKIT/BC/PCBKIT_L.EXE
       (9,157 bytes, Oct 11 1993). COMMDRV.OBJ (10,024 bytes) also
       extracted — same structure with COMMDRV_* instead of FOSSIL_*.
       See FOSSIL-OBJ-ANALYSIS.md for full OMF symbol dump.
2. [x] Fix GPROT.CPP xor_val — blocks UUXFER
       FIXED: renamed 'xor' (C++ reserved keyword) to 'xor_val' in
       GPROT.HPP. All 3 copies synced. The CPP already had xor_val.
3. [x] Fix 18h/19h/1Bh ES:DI stubs
       DONE: PCBCOMM_FAR_PTR(ES, DI). These are the functions PCBoard
       actually calls — they were the critical path, not cleanup.
       1Bh also hardened: always writes full 19 bytes regardless of CX,
       defensive against callers that don't set CX before calling.
       (Clark's MODEMFOS.C does set CX = sizeof(fossilstruct).)
4. [x] Add cached_msr to pcbcomm_port_t
       DONE: status_word() reads p->cached_msr. Works for all backends.
5. [x] Fix timer chain 16h
       DONE: ES:DX callback, _chain_intr. Low priority — MODEMFOS.C
       never calls 16h.
6. [x] Match MODEMDRV.C against int14.c
       FINDING: MODEMDRV.C uses linked C API, not INT 14h.
       FIXED: COMM-DRV extensions corrected from separate AH=10h-14h
       to AH=10h with AL sub-select. commstop was hitting commgo.
       ser_rs232_shim.c updated with opcb_t struct + opcb_refresh().
7. [x] LIBENTRY calling convention (hexadecimal, BC 3.1 compile test)
       All 13 ser_rs232_* symbols had wrong OMF mangling: C-style
       _lowercase instead of Pascal-style @UPPERCASE$Q… that PCBoard
       expects. Cause: missing LIBENTRY (= pascal) on prototypes and
       definitions. Fixed in comm.h (guard + 13 prototypes),
       commdrbl.c (13 definitions), libsbl.c (6 utility exports).
8. [x] comm.h + COMMDRBL.LIB + LIBSBL.LIB source created
       Clean-room reconstruction of WCSC COMM-DRV v15 SDK:
       - comm.h: 13 ser_rs232_* prototypes with LIBENTRY, port_param/
         opcb_block/aux_pcb structs, all constants (RS232ERR_*, CARD_*,
         LENGTH_*, PARITY_*, PROT_*, XMTOFF_STATE)
       - commdrbl.c: 13 functions via INT 14h to pcbcomm TSR. Block
         read/write via fn 18h/19h, refresh_info via 1Bh, viewpacket
         via 0Ch. rx/tx_buf_total stored, feeds outbuf_len.
       - libsbl.c: strerror, detect, baud/divisor, cardname, defaults
       Enables option (c) COMM-DRV replacement path via MODEMDRV.C.
9. [x] viewpacket fixed — now peeks via FOSSIL 0Ch, writes buf[0]
10.[x] refresh_info — rx_total/tx_total restored, stored in per-port
       struct, tx_buf_total feeds pp->outbuf_len
11.[x] fossil.c created (767 lines) — standalone toolkit FOSSIL source
       Clean-room reconstruction matching Clark's FOSSIL.OBJ OMF symbols.
       79/79 exports verified against original OBJ: 30 FOSSIL_* + 11
       ASYNC_* stubs + 30 _vtable pointers + 8 helpers. No project.h or
       model.h dependency — all types stubbed inline.
       Key findings from OMF analysis:
       - Source was y:\modem.c (MODEM.C), not MODEMFOS.C
       - FOSSIL_dofixups, ASYNC_dofixups, initializemodem are NOT exported
         (static near — called internally only within the compilation unit)
       - _Fossil (fossilstruct) is global, not static — exported as _Fossil
       - InBytes/OutBytes are macros: inbytes()/outbytes() under MULTIPORT
       - Function pointers (vtable) have C linkage (_lowercase in OMF),
         functions have pascal linkage (UPPERCASE in OMF) — no collision
       Build: bcc -ml -c -oFOSSIL.OBJ fossil.c
12.[x] COMMDRV.OBJ also extracted and analyzed (10,024 bytes)
       Same structure: MODEM.C + MODEMDRV.C + MODEMASY.C compiled with
       -DCOMM -DMULTIPORT -DLIB -DCOMMDRV. 79 exports (30 COMMDRV_* +
       11 ASYNC_* + 30 _vtable + 8 helpers). Additional imports:
       _ser_rs232, _ser_rs232_init, _pcb (port_param struct).
```

## Open Items

```
1. [x] comm.h — SOLVED. Clean-room reconstruction created.
       COMM-DRV runtime still at pcb1541/install/dist/target/COMMDRV/
       for behavior reference. WCSC COMM-DRV/DOS ($189.95) not needed.
       hexadecimal has a second comm.h at pcbcbase/COMMDRV/H/COMM.H
       (15,592 bytes with provenance and Delta notes) — diff and pick
       one to avoid ABI drift from having two.
2. [ ] Verify parse_baud_byte against rlfossil.c (Flag 4 — asserted,
       not tested). Check FTS-0015 table 1 against working C FOSSIL.
3. [ ] Compile fossil.c under BC 3.1 and diff OMF exports against
       Clark's FOSSIL.OBJ. Symbol list matches (79/79 verified by
       Python OMF parser) — need BC 3.1 compile to confirm no errors
       and byte-level OMF comparison.
4. [ ] Reconcile sysop/0's comm.h against hexadecimal's
       pcbcbase/COMMDRV/H/COMM.H (15,592 bytes) — pick one.
```

## File Locations

```
Driver side (pcbcomm):
  pcb154/pcbcomm/src/int14.c             FOSSIL 5C + COMM-DRV handler
  pcb154/pcbcomm/src/ser_rs232_shim.c    COMM-DRV API shim (13 functions + opcb)
  pcb154/pcbcomm/inc/pcbcomm.h           Port structure (+ cached_msr)
  pcb154/pcbcomm/inc/compat.h            Cross-compiler ISR macros (+ ES/DI/MK_FP)
  pcb154/pcbcomm/inc/comm.h              COMM-DRV SDK header (clean-room)
  pcb154/pcbcomm/inc/backend.h           Backend vtable
  pcb154/pcbcomm/inc/uart.h              UART register defines
  pcb154/pcbcomm/src/commdrbl.c          COMMDRBL.LIB source (13 ser_rs232_*)
  pcb154/pcbcomm/src/libsbl.c            LIBSBL.LIB source (utilities)

Toolkit:
  fossil.c                               Standalone FOSSIL.OBJ source (767 lines)
                                          Build: bcc -ml -c -oFOSSIL.OBJ fossil.c
  FOSSIL.OBJ                             Clark's original (9,157 B, Oct 1993)
  COMMDRV.OBJ                            Clark's original (10,024 B, Oct 1993)
  FOSSIL-OBJ-ANALYSIS.md                 OMF symbol dump + analysis

Client side (PCBoard) — all in pcb153/SOURCE/MODEM/:
  MODEM.C                                Backend selector (lines 30-35, 510)
  MODEMFOS.C    (811 lines, 37 exports)  FOSSIL client — pure INT 14h
  MODEMDRV.C    (521 lines, 33 exports)  COMM-DRV client — linked C API
  MODEMASY.C    (256 lines)              Async bare-metal client
  MODEMOS2.C    (1,081 lines)            OS/2 client
  pcb153/SOURCE/ASM/ASYNC.ASM            Bare-metal UART (1,893 lines, 23 functions + 5 data)

Reference:
  drivers/rlfossil/rlfossil.c            C FOSSIL reference (recovered)
  pcb1541/platform/fossil/FOSSIL_MAP.md  23 ASYNC_* function map (6,541 bytes)
  TOOLKIT2.ZIP → TOOLKIT/BC/PCBKIT_L.EXE  Original OBJ source archive
```
