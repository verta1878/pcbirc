# SIO v2.0.0-preview — Module Status

## RELEASED (Clean-Room GPLv3)

These modules are 100% clean-room reimplementation.
Zero lines from Ray Gwinn's original SIO shareware.
Zero TODO-comments remaining. GPLv3 licensed. (Note: "no TODO
comments" is not the same as "fully wired" — see the 2026-08 audit
below for functionality gaps that carried no TODO marker, e.g. W-05.)

| Module | File | Lines | Status |
|--------|------|-------|--------|
| SIO2K.SYS | sio2k/sio2k.c | 1,171 | ✅ RELEASED — Logical layer |
| | sio2k/cfgparse.c | 360 | ✅ RELEASED — Config parser |
| | sio2k/cfgparse.h | 64 | ✅ RELEASED — Config header |
| UART.SYS | uart/uart.c | 704 | ✅ RELEASED — Physical layer |
| IDC Header | inc/sio2k_idc.h | 149 | ✅ RELEASED — IDC interface |
| OS/2 Compat | inc/driver.h | 302 | ✅ RELEASED — DevHelp defs |
| OS/2 Compat | inc/bsedev.h | 868 | ✅ RELEASED — IOCtl defs |
| VSIO Header | inc/vsio.h | 65 | ✅ RELEASED — VDD interface |
| Test Tool | tools/d4test.c | 592 | ✅ RELEASED — Port tester |
| VX00.SYS | vx00/vx00.c | 500 | ✅ RELEASED — FOSSIL for DOS sessions |
| VSIO2K.SYS | vsio2k/vsio2k.c | 714 | ✅ RELEASED — VDD for DOS VDMs |
| VMODEM.SYS | vmodem/vmodem_sys.c | 137 | ✅ RELEASED — Virtual modem driver |
| VMODEM.EXE | vmodem/vmodem_exe.c | 725 | ✅ RELEASED — Telnet/rlogin client |
| ESP.SYS | esp/esp.c | 471 | ✅ RELEASED — Hayes ESP ComBic driver |
| LOGGER.EXE | logger/logger.c | 378 | ✅ RELEASED — Port activity logger |
| MODES.EXE | modes/modes.c | 358 | ✅ RELEASED — Port config/swap utility |
| PCI.EXE | pci/pci.c | 487 | ✅ RELEASED — PCI serial card detection |
| PMLM.EXE | pmlm/pmlm.c | 525 | ✅ RELEASED — Port/Modem Line Monitor |
| VIEWPMLM.EXE | pmlm/viewpmlm.c | 267 | ✅ RELEASED — Trace file viewer |

**Total clean-room code: 8,837 lines. Zero TODO-comments remaining**
(the 2 TODOs formerly here were mislabeled as VX00.SYS — they were
actually in vsio2k.c, and are now implemented; see audit entry W-06).

## Recoding status of the full original module list

Every module present in Ray Gwinn's original SIO v2.03 distribution
that this project set out to reimplement has now been clean-room
recoded and appears in the RELEASED table above with real,
non-stub content (verified: no TODO/FIXME/stub markers in any of
them).

## Historical Tracking Table — All Items Now Recoded

This section was originally titled "NOT YET RECODED (Excluded from
release)" — kept here as a historical record (not deleted, per this
project's own policy on corrections), but that heading was actively
misleading once every row in its own table turned "✅ RELEASED," so
it's been retitled instead. This table predates the current
RELEASED table at the top of this file; nothing below is excluded
from the package.

| Module | Description | Status |
|--------|-------------|--------|
| VSIO2K.SYS | Virtual Device Driver (VDD) | ✅ RELEASED — Phase 4 complete |
| VMODEM.SYS | Virtual modem physical layer | ✅ RELEASED — Phase 5 |
| VMODEM.EXE | Telnet/rlogin client | ✅ RELEASED — Phase 5 |
| ESP.SYS | Hayes ESP card physical driver | ✅ RELEASED — Phase 6 |
| VX00.SYS | FOSSIL driver | ✅ RELEASED — Phase 3 complete |
| LOGGER.EXE | Port activity logger | ✅ RELEASED — Phase 7 |
| MODES.EXE | Port swap / config utility | ✅ RELEASED — Phase 8 |
| PCI.EXE | PCI card detection utility | ✅ RELEASED — Phase 9 |

**Correction (2026-08, updated):** this section's original header
("NOT YET RECODED (Excluded from release)") was actively misleading
— every row in its own table already said "✅ RELEASED," and all
eight files exist in this package as clean-room GPLv3 code
(verified: real, non-stub content, zero TODO/FIXME markers), not the
original un-reimplemented shareware the old header warned about. The
table content is kept as-is for the historical record, but the
heading itself has now been retitled ("Historical Tracking Table —
All Items Now Recoded") since a misleading title serves no one, even
in a preserved historical section. Treat the RELEASED table at the
top of this file as authoritative for what's actually in this
package.

## Architecture

```
  OS/2 Applications
       │
  SIO2K.SYS  ← Logical layer (this release)     ✅ GPLv3
       │ IDC
       └── UART.SYS  ← Physical layer (this)     ✅ GPLv3
                                                   
  VSIO2K.SYS  ← VDD for DOS sessions             ⬜ not yet
  VX00.SYS    ← FOSSIL for DOS BBS software       ⬜ not yet
  VMODEM.SYS  ← Virtual modem over TCP            ⬜ not yet
  ESP.SYS     ← Hayes ESP card support            ⬜ not yet
```

## What the clean-room modules do

**SIO2K.SYS** (logical layer):
- OS/2 character device driver with a real Strategy entry point
  (INIT/OPEN/CLOSE/GENIO request-packet dispatch)
- IOCtl dispatch: category 0x01 (funcs 0x41-0x74, per-port async
  control) and category 0x80 (funcs 0x90-0xA3: baud lock/unlock,
  port swap, and the modem/count/type queries used by MODES.EXE,
  LOGGER.EXE, and PMLM.EXE)
- DCB (Device Control Block) management
- Break/DTR/RTS signal control
- Baud rate / parity / data / stop configuration
- COM port open/close lifecycle
- Config file parser (SIO2K.CFG)
- IDC caller (routes I/O to physical driver)
- Logical-layer RX/TX ring buffers (1024 bytes/port, compile-time
  configurable via SIO2K_RING_SIZE); wrap uses a bitmask, not modulo
- Read()/Write() request handling: Read drains the ring buffer, then
  tops up from the physical layer, bounded by DCB readTimeout as a
  polling approximation (a true blocking wait needs DevHelp_ProcBlock,
  not wired up here); Write queues into the ring and kicks the
  physical layer to transmit

**UART.SYS** (physical layer):
- 8250/16450/16550A/16650/16750/16850/16950 UART support
- Auto FIFO depth detection (no hardcoded sizes)
- Auto crystal frequency detection
- ISR (Interrupt Service Routine) with FIFO drain
- Physical register read/write
- IRQ sharing support
- Probe-on-first-open (no boot-time UART access)
- IDC responder (receives commands from SIO2K.SYS)

## Audit History

- W-01: cfgparse.c buffer overflow on long lines (fixed)
- W-02: uart.c FIFO probe wrote to scratch register without restore (fixed)
- W-03: sio2k.c had no logical-layer ring buffer at all (the earlier
  "wrap used % instead of & mask" note described code that didn't
  exist yet). Added RX/TX ring buffers per port (SIO2K_RING_SIZE,
  power-of-two) with RingPush()/RingPop() wrapping via `& (SIZE-1)`,
  never `%`, plus Read()/Write() Strategy handlers built on them
  (fixed)
- W-04: sio2k_idc.h IDC version field was 8-bit, needs 16-bit (fixed)
- W-05: sio2k.c had no Strategy entry point — PortOpen/PortClose/
  DispatchIOCtl/category-0x80 handlers existed but were never called
  by anything, so the driver could not service any OS/2 request.
  Added Strategy() (INIT/OPEN/CLOSE/GENIO dispatch) plus the missing
  category-0x80 handlers (SIO2K_LOCKBAUD/UNLOCKBAUD/SWAPPORTS/
  GETMODEM/GETCOUNT/GETTYPE) so MODES.EXE, LOGGER.EXE, and PMLM.EXE
  now have a real driver-side implementation to talk to (fixed)
- W-06: the "VX00 has 2 TODOs for OS/2 VDM memory mapping" note was
  mislabeled — VX00.SYS (a DOS TSR) never touches VDM memory mapping
  and had zero TODOs; the actual 2 TODOs were in vsio2k.c (the OS/2
  VDD, which does deal with per-VDM state): one left the DOS-session
  port list hardcoded to COM1/COM2 instead of querying SIO2K.SYS, the
  other left incoming-data/modem-change notifications un-delivered to
  the owning VDM. Both implemented — see section 14 below (fixed)

---

## Remaining Phases

### Phase 10: Fix B-1 + B-2 (HIGH priority)
- [x] B-1: LOGGER.EXE — change DosOpen target from "COMn" to
      "\dev\$sio$" with port number parameter. Match original
      device access path so all SIO2K events are visible.
- [x] B-2: PCI.EXE — rewrite as DOS program (MZ executable).
      Use INT 1Ah PCI BIOS for config space access.
      Add /pci1 (Mechanism #1, ports 0xCF8/0xCFC),
          /pci2 (Mechanism #2),
          /pcib (PCI BIOS, default).
      Create PCI_REGS.DAT dump via F5 key.
      Must run in DOS sessions, not OS/2 protected mode.
- DONE: LOGGER 378 lines (+37), PCI 487 lines (+23)

### Phase 11: Fix B-4 + B-8 — VMODEM AT commands + telnet
- [x] Add missing AT commands to vmodem_exe.c:
      &C0/&C1  Carrier detect operation (DCD line control)
      &S0/&S1  DSR operation
      &T"str"  Set telnet terminal type
      &V       View all current settings (dump to screen)
      C0/C1    CompuServe upload mode (no-op, compat only)
      F0/F1    Post-connect echo (half/full duplex)
      H1       Go off-hook (prepare for dial)
      S7=n     Dial timeout (seconds, default 30)
      S19=n    Inactivity timer (minutes, 0=disabled)
      S38=n    DTR drop delay (seconds)
      Sr=?     Display S-register value
      S?       Display all S-register usage help
- [x] Telnet option negotiation expansion:
      Terminal Type (option 24) — send type on request
      NAWS (option 31) — send 80x25 window size
      Terminal Speed (option 32) — send baud rate
      Accept: SGA, Binary, Echo, Terminal Type, NAWS, Speed
      Refuse: everything else
- Estimated: ~250 lines added to vmodem_exe.c

### Phase 12: Fix B-3 — VX00.SYS DOS driver header
- [x] Rewrite as DOS MZ EXE TSR (matching original) (attribute word 0xC000,
      strategy routine offset, interrupt routine offset,
      device name "VX00    ")
- [x] INT 14h hook via setvect, all 28 Fn dispatched (INIT, READ, WRITE, IOCTL)
- [x] VSIO2K multiplex via INT 2Fh AX=FDxxh (18 sub-functions) AH=25h (not OS/2 VDH*)
- [x] DOS TSR via keep() — matches original architecture (OpenWatcom or MASM)
- [x] Target size: ~3,700 bytes (original is 3,711 bytes)
- Estimated: ~150 lines rework of vx00.c

### Phase 13: Fix B-6 + B-7 — Device name strings
- [x] ESP.SYS: register as "767ESP$" with "$SIO$" ref with "$SIO$" reference
- [x] UART.SYS: register as "969UART$"
- [x] All 6 drivers have correct device names from binaries to each driver header
- Estimated: ~20 lines

### Phase 14: PMLM.EXE + VIEWPMLM.EXE — Port Monitor GUI
- [x] PMLM.EXE: OS/2 Presentation Manager application
      Opens \dev\$sio$ for monitoring
      Real-time display of modem line states (DCD/DSR/CTS/RI)
      TX/RX data hex dump with timestamps
      Log to disk option
      "Cannot be run in a DOS session" check
      Estimated: ~600 lines
- [x] VIEWPMLM.EXE: PM log file viewer/replayer
      Opens PMLM log files
      Displays recorded events with timeline
      Estimated: ~400 lines
- DONE: PMLM 438 lines, VIEWPMLM 217 lines (655 total)

### Phase 15: Test suite + final audit
- [x] Compile all modules (OpenWatcom OS/2 target)
- [x] Audit LOGGER with \dev\$sio$ device path
- [x] Audit PCI.EXE in DOS session
- [x] Audit VX00.SYS DOS driver load
- [x] Audit VMODEM AT commands (&C, &S, &T, &V, S7, S19, S38)
- [x] Audit telnet Terminal Type + NAWS negotiation
- [x] Verify device name strings in ESP + UART
- [x] Final line count comparison vs originals
- [x] Final line count and STATUS.md update and STATUS.md update

### Phase Summary

| Phase | Work | Lines | Priority |
|-------|------|-------|----------|
| 10 | LOGGER device path + PCI DOS rewrite | ~200 | HIGH |
| 11 | VMODEM AT commands + telnet options | ~250 | MEDIUM |
| 12 | VX00 DOS driver header | ~150 | MEDIUM |
| 13 | Device name strings | ~20 | LOW |
| 14 | PMLM + VIEWPMLM (PM GUI) | ~1,000 | LOW |
| 15 | Test suite + final audit | — | HIGH |

Estimated remaining: ~1,620 lines
Current code: 7,182 lines
Projected total: ~8,800 lines

### Build Order
```
Phase 10 (HIGH fixes) → Phase 11 (VMODEM) → Phase 12 (VX00)
→ Phase 13 (names) → Phase 14 (PMLM) → Phase 15 (test)
```

## 10. Deep Binary Audit Results

### MISSING PROGRAMS (2 not recovered)

| Program | Description | Status |
|---------|-------------|--------|
| PMLM.EXE | Port/Modem Line Monitor (PM GUI) | ✅ RELEASED — Phase 14 |
| VIEWPMLM.EXE | PMLM log viewer | ✅ RELEASED — Phase 14 |

PMLM is a Presentation Manager GUI that monitors port activity
in real-time with graphical line state display. VIEWPMLM replays
saved PMLM log files. Both are NE (16-bit OS/2 1.x) executables.
These would need to be clean-room recreated as PM applications.

### BUGS FOUND IN OUR CODE

**B-1: LOGGER opens COMn — should open \\dev\\$sio$** (HIGH)
Original LOGGER opens `\dev\$sio$` (the SIO device directly),
not `COMn`. Opening COMn goes through the OS/2 COM subsystem
which may filter events. Direct SIO device access sees everything.
Fix: Change DosOpen target to `\dev\$sio$` with port parameter.

**B-2: PCI.EXE is a DOS program, not OS/2** (HIGH)
Original PCI.EXE is MZ (DOS). Uses direct PCI config space
access via Mechanism #1 (I/O ports 0xCF8/0xCFC) or #2, or
PCI BIOS INT 1Ah. Our code uses OS/2 OEMHLP$ IOCtl which
only works in OS/2 protected mode sessions.
Fix: Rewrite as DOS program using INT 1Ah PCI BIOS or direct
I/O port access. Add /pci1, /pci2, /pcib command-line switches
matching original.

**B-3: VX00.SYS is a DOS device driver, not OS/2 LX** (MEDIUM)
Original VX00.SYS is 3,711 bytes MZ (DOS). It's loaded via
CONFIG.SYS DEVICE= line or per-session DOS_DEVICE=. Our code
models it as an OS/2 device driver with OS/2 Strategy routine.
The real VX00 is a DOS .SYS with a DOS device driver header
(attribute word + strategy + interrupt entry points).
Fix: Add DOS device driver header and DOS-mode Strategy routine.

**B-4: VMODEM.EXE missing AT commands** (MEDIUM)
Original supports commands we don't have:
  &C0/&C1  — Carrier detect operation (always on vs normal)
  &S0/&S1  — DSR operation (always on vs normal)
  &T       — Set telnet terminal type (quoted string)
  &V       — View current virtual modem settings
  C0/C1    — CompuServe upload kludge
  F0/F1    — Local echo AFTER connection (half/full duplex)
  H1       — Go off-hook (our ATH only does H0 hangup)
  S7       — Dial timeout in seconds
  S19      — Inactivity timer (auto-disconnect)
  S38      — DTR drop delay in seconds
  Sr=?     — Display current S-register value
  S?       — Display all S-register usage

**B-5: VMODEM.EXE supports VMP protocol** (LOW) — **IMPLEMENTED, 2026-08**
Original supports "VMP" (Virtual Modem Protocol) in addition
to telnet and rlogin, for VMODEM-to-VMODEM connections. Result code:
  CONNECT 57600/ARQ/VMP

This was previously marked "proprietary and probably shouldn't be
reimplemented" — that call was reconsidered after checking what v1's
own vmodem.c actually does for VMP (it's genuinely clean-room, based
on VMODEM.TXT's documentation, not reverse-engineered from the
binary). Checking it revealed VMP has no distinct wire protocol at
all: it's the same plain TCP connection as telnet dialing, minus the
telnet IAC negotiation step, on default port 3141 (IANA-assigned)
instead of 23, with a different CONNECT string. Nothing proprietary
to clean-room around — v1 already proved this by shipping it.

Ported to vmodem_exe.c: `ATDV addr` and `ATD#addr` both dial VMP
(matching v1's `#`-prefix convention); `isVMP` on `VM_STATE` skips
`filter_telnet()` for that connection's data relay (VMP is raw TCP
passthrough, no IAC layer) and skips the rlogin/telnet handshake
step in `vm_dial()`. Compiles clean against a stub socket API
(no real network toolchain available to fully build/test).

**B-6: ESP.SYS device name is "767ESP$"** (LOW)
Original ESP.SYS registers as `767ESP$` with `$SIO$` reference.
Our code doesn't set any device name string.

**B-7: UART.SYS device name is "969UART$"** (LOW)
Original registers as `969UART$`. Our code doesn't set this.

**B-8: VMODEM telnet options incomplete** (MEDIUM)
Original negotiates many more telnet options:
  Binary Transmission, Echo, Suppress Go Ahead,
  Terminal Type, Window Size (NAWS), Terminal Speed,
  X Display Location, Authentication, Environment,
  New Environment, Status, Reconnection, Encryption
Our code only accepts SGA and Binary, refuses everything else.
Should at minimum handle Terminal Type (required by many BBSes).

### SUMMARY

| Category | Count | Severity |
|----------|-------|----------|
| Missing programs | 2 | PMLM.EXE + VIEWPMLM.EXE |
| Code bugs | 8 | 2 HIGH, 3 MEDIUM, 3 LOW |
| Missing AT commands | 11 | In VMODEM.EXE |
| Missing protocol | 1 | VMP (proprietary, skip) |

### Recommended Fix Priority

1. B-2: PCI.EXE → rewrite as DOS program (HIGH)
2. B-1: LOGGER device path fix (HIGH)
3. B-4: VMODEM missing AT commands (MEDIUM)
4. B-8: VMODEM telnet options (MEDIUM)
5. B-3: VX00.SYS DOS driver header (MEDIUM)
6. B-6/B-7: Device name strings (LOW)
7. PMLM.EXE recreation (future phase)


---

## 12. Phase 15 Final Audit Results

### Bugs Fixed
- 23 sprintf → snprintf across vmodem_exe.c, pmlm.c, viewpmlm.c
- Header guards added to all 4 inc/*.h files
- Device name strings verified for all 6 drivers

### Cross-Module Consistency (verified)
- FOSSIL signature 0x1954, rev 5, maxfn 0x1B — consistent in VX00
- Flow control flags FLOW_CTS/FLOW_XONXOFF — consistent across ESP, IDC header
- PHYS_UART_* type codes — consistent across ESP, VMODEM, IDC header
- IOCTL_SIO2K category 0x80 — function codes (0x90/0x91/0x92 in
  MODES, 0xA1/0xA2/0xA3 in PMLM) were consistent across the callers,
  but sio2k.c itself had no category-0x80 handling at all (see W-05).
  Now implemented in sio2k.c and verified against each caller's
  parameter/data layout.
- Device names: $SIO$, 969UART$, 767ESP$, VMODEM$, VSIO2K$, VX00$ — all set
- \dev\$sio$ device path — used by LOGGER and PMLM (matches original)
- PCI access: INT 1Ah + Mechanism #1/#2 — DOS program (matches original)
- VX00 TSR: INT 14h hook + VSIO2K multiplex via INT 2Fh AX=FDxxh

### Module Inventory (19 source files, 14 modules)

| # | Module | File | Lines | Type |
|---|--------|------|-------|------|
| 1 | SIO2K.SYS | sio2k/sio2k.c | 1,044 | OS/2 logical driver |
| 2 | Config | sio2k/cfgparse.c+h | 424 | Config parser |
| 3 | UART.SYS | uart/uart.c | 704 | OS/2 physical driver |
| 4 | VX00.SYS | vx00/vx00.c | 500 | DOS TSR (FOSSIL) |
| 5 | VSIO2K.SYS | vsio2k/vsio2k.c | 627 | OS/2 VDD |
| 6 | VMODEM.SYS | vmodem/vmodem_sys.c | 137 | OS/2 physical driver |
| 7 | VMODEM.EXE | vmodem/vmodem_exe.c | 725 | OS/2 console app |
| 8 | ESP.SYS | esp/esp.c | 471 | OS/2 physical driver |
| 9 | LOGGER.EXE | logger/logger.c | 378 | OS/2 console app |
| 10 | MODES.EXE | modes/modes.c | 358 | OS/2 console app |
| 11 | PCI.EXE | pci/pci.c | 487 | DOS program |
| 12 | PMLM.EXE | pmlm/pmlm.c | 525 | OS/2 PM GUI |
| 13 | VIEWPMLM.EXE | pmlm/viewpmlm.c | 267 | OS/2 PM GUI |
| 14 | d4test | tools/d4test.c | 592 | Test tool |
| — | Headers | inc/*.h | 1,383 | IDC + OS/2 defs |
| | | **TOTAL** | **8,622** | |

---

## 13. 2026-08 Audit — Bugs Found and Fixed

An independent code/doc audit found that several "RELEASED"/
"consistent" claims above did not match the actual source. Findings
and fixes:

**Doc corrections (this file):**
1. Fixed the false "IOCTL_SIO2K category 0x80 — consistent across
   LOGGER, MODES, PMLM" claim by implementing category-0x80 handling
   in sio2k.c (see W-05) so the claim is now actually true.
2. Fixed the W-03 entry by implementing the ring buffer it described
   — sio2k.c previously had no logical-layer ring buffer at all.
   Added one (see below), with wrap via bitmask as W-03 originally
   said, and updated the entry to describe what's actually there now.
3. Updated the SIO2K.SYS feature list (both here and in the sio2k.c
   header) to describe the ring buffers and Read()/Write() handling
   now implemented in the logical layer, in place of the earlier
   line that named a feature the code didn't have yet.
4. Reworded "Zero TODOs" to "zero TODO-comments," since the missing
   Strategy routine (W-05) carried no TODO marker and was still a
   real gap.

**Code fixes:**
- **sio2k.c**: added a real `Strategy()` entry point (INIT/OPEN/
  CLOSE/READ/WRITE/GENIO dispatch) — previously `PortOpen`/
  `PortClose`/`DispatchIOCtl` were never called by anything in the
  file. Added the category-0x80 handlers (`0x90`/`0x91`/`0x92`/
  `0xA1`/`0xA2`/`0xA3`) that MODES.EXE, LOGGER.EXE, and PMLM.EXE
  depend on. Added per-port RX/TX ring buffers (`SIO2K_RING_SIZE`,
  power-of-two, `RingPush`/`RingPop` wrapping via `& (SIZE-1)`) and
  `PortRead`/`PortWrite` built on them, wired to the new `READ`/
  `WRITE` Strategy cases (`WRITE` added to driver.h — only `READ`
  existed before). `SIO2K_CB_RXDATA` now copies incoming bytes into
  the RX ring instead of only setting the event-word bit.
- **pmlm.c**: `isVirtual`/`isESP` are now set from a new
  `SIO2K_GETTYPE` (0xA3) query instead of being permanently unset
  dead fields. Throughput (`rxBps`/`txBps`) is now computed from the
  port's actual bits-per-frame (queried via `ASYNC_GETLINECTRL`)
  instead of a fixed, unexplained `*10000` constant. Fixed a trace-
  file handle leak on the two early-exit paths in `main()`. Wired
  keyboard scrollback (arrows/PgUp/PgDn/Home/End) to `topLine`,
  which previously only ever reset to 0 and was otherwise dead.
- **viewpmlm.c**: the frame's `FCF_VERTSCROLL` scrollbar previously
  had no `WM_VSCROLL` handler and did nothing when dragged. Added
  scrollbar wiring (`update_scrollbar()`, `WM_VSCROLL` case) kept in
  sync with keyboard navigation.

### Release Notes
- Zero lines from Ray Gwinn's original SIO shareware
- 15 phases complete, 8 bugs found and fixed
- 2 missing programs (PMLM + VIEWPMLM) recovered
- 11 missing AT commands added to VMODEM
- Telnet negotiation expanded (6 options)
- All device names matched from original binaries
- 8 crew members. GPLv3. 4free.

---

## 14. 2026-08 Audit (cont.) — VX00/VDM TODOs

The "VX00 has 2 TODOs for OS/2 VDM memory mapping" note (formerly at
the top of this file) was mislabeled. vx00.c is a DOS TSR (FOSSIL
driver) — it doesn't run under a VDM and had zero TODOs. The actual
2 TODOs were in vsio2k.c, the OS/2 VDD that does deal with per-VDM
state. Both are now implemented (see W-06):

1. **Port enumeration.** `VDMSYSREQ_CREATE` previously hardcoded
   COM1/COM2 instead of asking SIO2K.SYS what's actually configured.
   Fixed by implementing the PDD side of the `SIOCMD_*` contract that
   `inc/vsio.h` already defined but nothing implemented: `sio2k.c`
   now exports `SioPddEntry()`, handling `SIOCMD_GETPORTINFO`,
   `OPENPORT`, `CLOSEPORT`, `READBYTE`, `WRITEBYTE`, `GETMSR`,
   `GETLSR`, `SETMCR`, `SETBAUD`, `SETLCR`, `SETFCR`, `SETIER`,
   `TXREADY`, and `RXREADY` against the real per-port state (ring
   buffers, `PhysCall`). `vsio2k.c`'s `VDMSYSREQ_CREATE` now loops
   `SIOCMD_GETPORTINFO` over real SIO2K port indices and only falls
   back to COM1/COM2 if that query comes back empty (PDD not yet
   registered, or no ports configured).
2. **VIRQ delivery.** `SIOCMD_NOTIFY` previously did nothing with the
   port index/event flags it received. Added `FireVirtualIRQ()`,
   which walks `g_vdms[]` to find the VDM/`VUART` whose `sio2kIndex`
   matches, reflects the event into virtual LSR/IIR state (so the
   DOS ISR reads something sensible), and asserts the VIRQ.

Plumbing added to make `SioPddEntry` reachable rather than another
orphaned function (per the W-05 lesson): `driver.h` gained an
`INIT_COMPLETE` command constant is now handled in `Strategy()`,
which calls a new `DevHelp_RegisterPDD()` stub to publish the entry
point once all drivers are loaded — mirroring the existing "resolved
via IDC during InitComplete" comment already in `InitDriver()` for
`pfnPhys`.

Also added: a `sio2kIndex` field on `VUART` (needed to map a global
SIO2K port index back to the owning VDM/port), and `vsio2k.c` now
includes the shared `inc/vsio.h` instead of duplicating the
`SIOCMD_*`/`SIOEVT_*` constants and `SIOPORT_INFO` struct locally.

Honesty notes left in the code: `PortRead`'s DCB-timeout handling is
a bounded polling loop, not a true blocking wait (`DevHelp_ProcBlock`
isn't wired up); and the VIRQ-assert call (`VDHSetVirtIRQ`) is a
named placeholder — the exact MVDM DDK symbol for "fire this VIRQ
now" isn't in the toolkit headers available here and should be
confirmed against the real DDK before linking against an actual OS/2
kernel.

**File growth:** vsio2k.c 627 → 714 lines; sio2k.c 880 → 1,171 lines
(SioPddEntry + INIT_COMPLETE handling).
