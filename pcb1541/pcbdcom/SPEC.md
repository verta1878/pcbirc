# pcbdcom — v1 interface specification

## Goal

Drop-in replacement for WCSC COMM-DRV at the interfaces sysops actually
touch: the config file, the CONFIG.SYS load line, the runtime INT 14h
API. Internally reimplemented in DOS 16-bit C using GPL Linux serial
driver sources as reference.

## Public interfaces (must match COMM-DRV)

### CONFIG.SYS load

```
DEVICE=C:\PCBDCOM\PCBDCOM.SYS /IRQ=4 /BASE=0x3F8 /BAUD=38400
```

Also works from AUTOEXEC.BAT as a TSR:

```
LH C:\PCBDCOM\PCBDCOM.EXE /IRQ=4 /BASE=0x3F8 /BAUD=38400
```

Same binary handles both — dispatches on load context (DS:SI vs PSP).

### Config file (PCBDCOM.CFG)

Line-oriented, one port per line. Matches COMM-DRV DRVSETUP output:

```
# Port  Card       SubPort  Base    IRQ  CardSeg  FOSSIL
  1     8250       0        0x3F8   4    0        Y
  2     8250       0        0x2F8   3    0        Y
  3     BOCA16     0        0x100   5    0        Y
  4     BOCA16     1        0x108   5    0        Y
  ...
  9     DIGI_PCXE  0        0x110   3    0xD000   N
  ...
```

Card types recognized in v1:
- `8250` — standard COM1..COM4 UART
- `BOCA` / `BOCA16` — Boca BB-1004/1008/2016 dumb multi-port
- `CYCLOM` — Cyclades Cyclom-Y
- `DIGI_PCXE` / `DIGI_PCXI` / `DIGI_ACCEL` — DigiBoard smart cards
- `ROCKET` — Comtrol RocketPort
- `EASYIO` — Stallion EasyIO
- `NONE` — port disabled

Card type v1.2 addition (Phase 1 confirmed):
- `ARNET_SPP` — Arnet SmartPort Plus. BIOS files (XABIOS.BIN,
  XACOOK.BIN, XACOMX.BIN) ship inside COMMDRV.RED and can be
  redistributed with pcbdcom under WCSC's PCBoard install disk
  terms. Card config data (ARNETSP4.DAT, ARNETSP8.DAT) is
  reference material, not code.

Deferred:
- `TCP_SOCKET` — TCP listener (15.41 feature)

### INT 14h dispatch (FOSSIL-compliant)

Standard FOSSIL functions supported:
- 00h Initialize port
- 01h Deinitialize port
- 02h Set baud rate
- 03h Get port status
- 04h Read character (blocking)
- 05h Write character (blocking)
- 06h Enable/disable RTS
- 08h Flush TX buffer
- 09h Purge TX buffer
- 0Ah Read character (non-blocking)
- 0Bh Write character (non-blocking)
- 0Ch Peek RX buffer
- 0Dh Set/get modem status
- 0Eh Extended baud rate

Port numbers 0..MAX_PORTS-1 map via PCBDCOM.CFG to physical
(card, subport) tuples.

## Backends (internal)

One `pcbdcom_backend_t` per card type, resolved at load time from
config file. Each backend implements:

```c
typedef struct {
    const char *name;                     /* "8250", "BOCA16", ... */
    int  (*probe)(pcbdcom_port_t *p);     /* detect chip presence   */
    int  (*init) (pcbdcom_port_t *p);     /* configure hardware     */
    void (*isr)  (pcbdcom_port_t *p);     /* IRQ dispatch entry     */
    int  (*read) (pcbdcom_port_t *p, void *buf, int n);
    int  (*write)(pcbdcom_port_t *p, const void *buf, int n);
    int  (*ioctl)(pcbdcom_port_t *p, int cmd, void *arg);
    void (*deinit)(pcbdcom_port_t *p);
} pcbdcom_backend_t;
```

Backends compiled in v1: uart (8250), boca (dumb multi), cyclom,
digi_pcxe, digi_accel, rocket, easyio.

## Source layout

```
src/
  pcbdcom.c        entry point, load-mode dispatch, config parser
  int14.c          FOSSIL INT 14h handler
  uart.c           standard 8250/16450/16550/16650 backend
  boca.c           Boca dumb-multiport backend (shared IRQ)
  cyclom.c         Cyclades Cyclom-Y backend
  digi_pcxe.c      DigiBoard PC/Xe backend
  digi_accel.c     DigiBoard AccelePort backend
  rocket.c         Comtrol RocketPort backend
  easyio.c         Stallion EasyIO backend
  ring.c           IRQ-safe ring buffer helpers
  irq.c            PIC control (mask/unmask) helpers
inc/
  pcbdcom.h        public types + port table
  uart.h           UART register constants (already present)
  backend.h        pcbdcom_backend_t + registry
```

## Porting rules

Each `src/*.c` starts with a comment block:

```c
/* Ported from Linux <path>/<file>.c (kernel <version>), GPLv2.
 * Original authors: <names>.
 * DOS 16-bit adaptations: pcbirc crew, GPLv3.
 * Adaptations:
 *   - <what changed and why>
 */
```

Adaptations kept minimal:
- Remove Linux tty_struct plumbing → talk to our port table directly
- Replace kmalloc/GFP_* → static arenas (DOS has no malloc-in-ISR)
- Replace outb_p (delay) → outp (Watcom/BC) or _outp (MSC)
- 32-bit types → 16-bit where target hardware allows (careful with
  16550 FIFO depth counters that need 16 bits)
- Ring buffer sizes shrunk from Linux's 4KB default to 512B per port
  (DOS conventional memory pressure)

## Build

Target compilers (from CONFIG.SYS route the sysop picks):
- BC31   — Borland C++ 3.1, model=large
- MSC70  — Microsoft C 7.0, model=large
- OWC    — OpenWatcom (v3 only, 15.41 optional linked-in build)

BUILD/PCBDCOM.MAK dispatches by %CC% env variable.

## Status

**v1.1 is code-complete as of 2026-08-30.** Phase 1 discovery
(see `GAP-ANALYSIS.md`) identified the following v1.2 additions
required for drop-in COMMDRV.OBJ replacement:

  1. `ser_rs232_*` symbol shim (13 entry points, Pascal calling
     convention, uppercase names) — for `MODEMDRV.C` link.
  2. INT 14h extensions AH=0x10 (commgo) and AH=0x12 (commstop) —
     PCBoard calls these directly, bypassing `ser_rs232_*`.
  3. 8th backend: **Arnet SmartPort Plus** — XABIOS.BIN + XACOOK.BIN
     + XACOMX.BIN + ARNETSP4/8.DAT confirmed in COMMDRV.RED.
  4. Possible 9th backend: WCSC ships 9 .DRV modules total; we cover
     8 with the additions above. Identity of the 9th TBD.
 All 7 backends have full
probe + init + ISR implementations. See src/README.md for per-file
lines-of-code, and each backend's file header + v1.1 TODO section
for remaining work (multi-port wiring in pcbdcom.c is the common
v1.1 task — most backends currently assume chan=0 pending config
parser extension).

## Non-goals for v1

- Not a general-purpose DOS FOSSIL replacement. Optimized for PCBoard's
  usage patterns (one long-running BBS process per node, low churn).
- No hot-reload. Reconfig means unload + reload the TSR.
- No PnP/PCI card enumeration. Sysop specifies card type + base in
  config file. Card auto-probe (v2) will use the same registry.
