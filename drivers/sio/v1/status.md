# SIO v1 Code Audit — August 8, 2026

## Quick Summary — Fixes and Changes Made This Project (2026-08)

The detailed audit sections below are long, so here's a plain-language
list of what actually changed in `sio1src/`, for reference:

- **REREG.EXE decision**: flagged, then unblocked once the author
  released SIO as freeware. Not yet built — tracked as a follow-up,
  not silently skipped.
- **No registration key exists anywhere in this source** — checked
  the whole tree; nothing to remove, nothing needed.
- **`SU.EXE` gained real commands** that existed in the original
  binary but had no equivalent here: `SU BOOT` (reboot via the
  documented `\DEV\DOS$` undocumented-but-verified IOCtl method),
  `SU DCD` (batch-file exit-code check), `SU ENHANCED` (status
  display, including the "Null Stripping"/"Error Replacement
  Character" strings you asked to have present even though they're
  informational-only in the original too).
- **`PrintPortDetect` was a dead stub** in `sioinit.asm` — implemented
  it for real, which also required adding a `pdComNum` field (the
  parsed COM port number was previously read and silently discarded).
- **Two safety checks the original had and this rebuild didn't**:
  overlapping I/O address detection, and a serial-mouse-shared-IRQ
  warning. Both added to `ValidatePortConfig` in `sioinit.asm`.
  *(Caught and fixed two bugs in my own draft of this before it
  shipped: a register/loop-counter collision, and index math that
  would have falsely flagged any 2-port config as self-overlapping.)*
- **Makefile was missing build rules** for `D4TEST.EXE`/`FOSTEST.EXE`
  entirely, despite this file's own M11 entry claiming FOSTEST was
  delivered. Fixed.
- **`ATD$` and `ATDL`** (dial-examples alias and redial-last) added to
  `vmodem.c` — present in the original binary, missing here.
- **Missing-Features tracking table corrected**: M04/M05 (SMP
  spinlocks, PCMCIA) were marked "V2 scope" but turned out to be
  unimplemented in v2 either — genuinely missing project-wide, not
  handled elsewhere as the label implied. M08 (VMP) was backwards —
  v1 already has real VMP support; it's *v2* that had declined it.
- **Registration banner replaced**: the original's "Registered to
  &lt;name&gt;" shareware banner is now "Registered: is free software
  released under the GNU General Public License v3.0 (GPLv3)".
- **Doc/status housekeeping**: `BUGFIXES.md`/`CHANGELOG.md`/
  `PROGRESS.md` merged into this file (no more separate files); a
  stray empty `inc_stub/` directory I'd accidentally left in an
  earlier delivery was removed; `FILE_ID.DIZ` added; a confusingly
  self-contradicting section header was retitled rather than deleted.
- **A packaging bug was caught and fixed**: this file was briefly
  missing entirely from a delivered zip (present locally, not
  actually zipped up) — confirmed fixed by checking the zip's actual
  contents, not just re-running the same command.

**Not in this project — implemented in the separate v2 (`sio2src`)
project instead**, since v1 has no equivalent feature to begin with:
the mouse-port-swap protection (`MODES.EXE`'s port-swap command,
which checks `CONFIG.SYS` for a configured serial mouse before
allowing a swap) belongs to v2's `MODES.EXE` — v1's `SU.EXE` has no
port-swapping feature at all, so there's nothing analogous to add
here.

---



Audited by: hexadecimal (pcbirc)
Source: evga's clean-room GPLv3 reimplementation
Total: 5,927 lines ASM (SIO.SYS) + 1,169 lines C (VMODEM)

## Overall Assessment: SOUND ✅

Clean-room design from published documentation. No reverse
engineering. Code follows OS/2 PDD conventions correctly.

## ISR (sioisr.asm, 395 lines) ✅

- **Shared IRQ handling**: Correct. Loops through all ports on
  the same IRQ, rescans until no more interrupts pending. This
  is the right pattern for shared PCI/ISA interrupts.

- **EOI timing**: Correct. EOI sent via DevHlp_EOI after all
  ports serviced, not per-port. Prevents spurious re-interrupts.

- **IIR reading**: Correct. Reads IIR first, checks IIR_PENDING
  bit. Handles all 5 interrupt types (RLS, RDA, timeout, THRE, MS).

- **FIFO drain**: Correct. RDA handler loops back to check LSR_DR
  after reading each byte — drains entire FIFO, not just one byte.

- **XON/XOFF**: Correct. Checks flow control flags in DCB before
  processing. XOFF received → sets pdXoffRecvd, stops TX.
  XON received → clears pdXoffRecvd, calls TryTransmit.

- **Unknown interrupt**: Correct. Reads LSR, MSR, and RBR to clear
  all pending conditions. Prevents stuck interrupts.

## Ring Buffer (siobuf.asm, 185 lines) ✅

- **Overflow check**: Correct. RingBufPut checks count vs size
  before writing. Returns CF on full — caller handles overflow.

- **Underflow check**: Correct. RingBufGet checks count for zero
  before reading. Returns CF on empty.

- **Wrap-around**: Correct. Both head and tail wrap to 0 when
  they reach rbSize. No off-by-one — uses `jb` (unsigned below).

- **Thread safety**: RingBufFlush uses CLI/STI for atomicity.
  Put/Get rely on single-producer/single-consumer model (ISR
  produces RX, strategy consumes RX; strategy produces TX,
  ISR consumes TX). This is correct for OS/2 PDD design.

- **NOTE**: rbBase stores a GDT selector:offset, not physical
  address. Comment in code is clear about this requirement.
  sioinit.asm must convert via DevHlp_PhysToGDTSelector.
  If this step is skipped → immediate fault. The comment
  warns about this — good defensive documentation.

## UART Detection (siouart.asm, 344 lines) ✅

- **Scratch register test**: Correct pattern (write 55h, read
  back, write AAh, read back). Distinguishes present vs absent.

- **FIFO detection**: Correct. Enables FIFO, reads IIR FIFO bits,
  then disables FIFO. Distinguishes 16450/16550/16550A.

- **16550 vs 16550A**: Correct. IIR_FIFO_BAD (only bit 7) =
  broken FIFO (original 16550). IIR_FIFO_OK (both bits) = working
  FIFO (16550A+). Broken FIFO treated as 16450 — safe default.

## Transmit (TryTransmit in sioisr.asm) ✅

- **Flow control checks**: Correct order — TX hold, XOFF received,
  break active, CTS (if enabled). All checked before sending.

- **Transmit immediate**: Correct. pdTxImm flag checked before
  buffer — allows urgent bytes (XON/XOFF) to jump the queue.

- **FIFO loading**: Correct. Loads up to 16 bytes on 16550A with
  FIFO enabled, 1 byte otherwise. Stops when buffer is empty.

- **Writer wakeup**: Correct. Calls DevHlp_ProcRun on pdWriteWait
  after freeing buffer space. Unblocks DosWrite callers.

## VMODEM (vmodem.c, 1,169 lines) ✅

- **Telnet IAC handling**: Complete. State machine handles:
  - Normal data (state 0)
  - IAC received (state 1) — checks for escaped IAC (255,255)
  - Command received (state 2) — WILL/WONT/DO/DONT
  - Subnegotiation (state 3) — collects until IAC SE

- **MD5 authentication**: Uses md5.c/md5.h. CRAM-MD5 for
  session authentication between VMODEM instances.

- **AT command parser**: Handles ATZ, ATH, ATD (dial), ATA
  (answer). Maps dial commands to TCP connect.

## Potential Issues (minor)

### 1. RingBufPut/Get not CLI/STI protected
The Put and Get functions don't use CLI/STI. This is correct
for the single-producer/single-consumer model BUT if a second
ISR fires during a strategy routine's RingBufGet (e.g., a
higher-priority IRQ that calls back into SIO), the count field
could be corrupted. This is unlikely in practice but theoretically
possible on shared-IRQ systems.

**Risk**: Very low. OS/2 serializes PDD strategy calls.
**Fix if needed**: Add CLI/STI around count update in Put/Get.

### 2. FIFO drain loop has no maximum iteration count
The ReceiveData handler loops back to check LSR_DR after each
byte. If a hardware fault causes LSR_DR to always read as set,
this becomes an infinite loop inside the ISR. All other interrupts
are blocked.

**Risk**: Very low. Only happens on faulty hardware.
**Fix if needed**: Add a maximum iteration count (e.g., 256
bytes per ISR invocation).

### 3. TryTransmit FIFO fill count is hardcoded to 16
The FIFO fill count for 16550A is hardcoded to 16 bytes. The
16650/16750/16850/16950 UARTs have 32/64/128/256-byte FIFOs.
Loading only 16 bytes wastes FIFO capacity on newer UARTs.

**Risk**: Performance only — no correctness issue.
**Fix**: SIO v2 (SIO2K) addresses this with auto FIFO sizing.

### 4. No check for pdReadWait/pdWriteWait validity
The ISR calls DevHlp_ProcRun on pdReadWait/pdWriteWait without
checking if the value is a valid block ID. If a strategy routine
frees the block ID between the ISR's test and call, ProcRun
gets a stale ID.

**Risk**: Low. OS/2 ProcRun ignores invalid IDs gracefully.
**Fix if needed**: Use DevHlp_ProcBlock with timeout in strategy.

## Recommendation

SIO v1 is safe to use as-is for PCBoard on OS/2. The 4 minor
issues are theoretical edge cases. SIO v2 (SIO2K) addresses
issues 3 (auto FIFO) and adds the split architecture for PCI
and custom device name support.

For telnet-only operation (no real hardware), VMODEM handles
everything correctly including telnet IAC filtering.

---

## 2026-08 Follow-up Audit — Latest Source Confirmed, Binary Comparison, New Fixes

**Which source is current:** this archive shipped two copies of the
v1 tree (`sio_v1` and `v1`). Diffed byte-for-byte: `sio_v1` contains
the Audit 3 ("hexadecimal" ISR & ring buffer review, W-01 through
W-04 — CLI/STI protection around `rbCount`, a 256-byte cap on the
FIFO drain loop, real `PD_FIFOSZ`-based TX batching) that `v1` does
not have — `v1`'s transmit path still has the placeholder comment
"Would read from TX buffer and write to THR" where `sio_v1` has a
real `KickTxBatch` call. `sio_v1` is confirmed newer/more complete
and was renamed to `sio1src`; `v1` was renamed to `bak`.

**Binary comparison** (against the shipped `SIO/*.EXE`/`*.SYS`, June
1999 build — 16-port version, per its own banner string):

- **`REREG.EXE` has no source anywhere in either tree.** `strings`
  shows it's a shareware registration-transfer tool: takes a prior
  registered `SIO.SYS`, checks port-count and major-version
  compatibility against the new install, and copies the
  registration/serial-number data across so the user doesn't have to
  re-register from scratch after upgrading. This was intentionally
  not reimplemented pending confirmation of the licensing situation.
  **Update:** the author has released SIO as freeware, so the
  original concern (cloning a paid product's registration-unlock
  mechanism) no longer applies. Reimplementing REREG.EXE as a
  clean-room utility is unblocked; not yet done — tracked here for a
  follow-up pass.
- **Tport hardware support and Hayes ESP Master/Slave IRQ
  validation** (`"Detected Tport"`, `"...ESP Master port!"`,
  `"...ESP Slave port! Must be 13"` in the binary's strings): absent
  from source, but already honestly documented in PROGRESS.md as
  "deferred to V2" — not a silent gap, no action needed here.
- **Two silent gaps found and fixed** (not previously documented as
  deferred anywhere):
  - `PrintPortDetect` (sioinit.asm) was a stub — `ProbeAllPorts`
    called it once per detected port, but the function was just
    `ret`, so no per-port UART detection message was ever printed,
    despite a full pre-formatted message-buffer layout
    (`PortDetectMsg`/`PortDetectNum`/`PortDetectAddr`/
    `PortDetectType` in sio.asm) sitting there unused. While
    implementing it, found the parsed COM port number
    (`ParseOnePort`'s `call ParseDecimal ; AX = port number`) was
    read and then discarded — never stored anywhere in `PORTDATA` —
    so there was no way to name a port even if the print function
    worked. Fixed by:
    - Adding a `pdComNum` field to `PORTDATA` (repurposing the
      previously-unused `pdPad` alignment byte).
    - Storing the parsed value in `ParseOnePort` and
      `ConfigDefaultPorts`.
    - Implementing `PrintPortDetect` for real, printing each piece
      (COM number, hex address, UART type name) with its own
      `PrintMsg`/new `PrintMsgStr` call rather than trying to patch
      values into the original fixed-width buffer — that buffer's
      `PortDetectNum` field is only 1 byte (COM10–16 need two
      digits) and `PortDetectType` is only 16 bytes (the longest
      UART name string, "Detected 16550A (high-speed)", is 30), so
      reusing it as designed would have silently truncated output.
  - Two validation checks present in the original binary's strings
    (`"Overlaping port addresses specified"`, `"...mouse driver must
    be loaded before SIO.SYS"`) had no equivalent in source at all.
    Added `ValidatePortConfig`, called from `CmdInit` right after
    port configuration is resolved:
    - Overlapping I/O address range detection: pairwise scan over
      all non-INTERNET ports, flagging any two whose 8-port ranges
      are within 8 of each other.
    - Serial-mouse IRQ warning: flags any non-INTERNET port
      configured on IRQ3 or IRQ4 (the classic shared IRQs for a
      mouse on COM2/COM1).

**Self-caught bugs in the above, fixed before landing:** an early
draft of `ValidatePortConfig` used `CX`/`BP` as countdown loop
counters and derived each port's array index as `ddNumPorts -
counter`; since `mul` (needed for the `PORTDATA` pointer arithmetic)
clobbers `DX`, and — more seriously — that indexing scheme pointed
the very first outer/inner pair at the *same* port when there were
only 2 configured ports, which would have made every 2-port
configuration falsely report itself as overlapping. Also caught a
stack imbalance on the early-exit-when-overlap-found path (same bug
class as this project's own historical A2-07). Rewrote using plain
incrementing indices held in memory (`VPCOuterIdx`/`VPCInnerIdx`)
instead of register countdown tricks, which sidesteps all three
problems and was verified correct by hand-tracing the 2-port case.

**Verification method:** no OpenWatcom/WASM toolchain or network
access was available in this environment, so none of this was
assembled. Verified instead by: manual push/pop stack-balance
counting, confirming every new label is referenced from exactly the
scope it's defined in (checked for accidental duplicate `@@`-local
labels across unrelated routines), confirming every new data symbol
referenced from `sioinit.asm` is defined exactly once in `sio.asm`,
and hand-tracing the overlap-check loop's index arithmetic for the
n=2 edge case. This is not a substitute for an actual build — that
should still happen before this ships.

---

# Merged from sio1src/ — BUGFIXES.md, CHANGELOG.md, PROGRESS.md

The three files below used to live inside `sio1src/` as separate
documents. Merged here into the single top-level STATUS.md per
request — content preserved as-is (headings demoted one level to
nest properly), not condensed, since these are the project's
audit/change trail. `sio1src/BUGFIXES.md`, `sio1src/CHANGELOG.md`,
and `sio1src/PROGRESS.md` have been removed now that their content
lives here.

## Bug Fixes (formerly sio1src/BUGFIXES.md)

### Audit History

Two full code audits were performed across all source files.
All bugs have been fixed as of the 2026-08-07 00:00 UTC release.

---

### Audit 1 — 2026-08-06 21:20 UTC (Phase 4)

15 bugs found, 12 fixed immediately, 3 deferred to integration.

| # | File | Severity | Bug | Fix |
|---|------|----------|-----|-----|
| A1-01 | siobuf.asm | HIGH | rbBase held physical addr from AllocPhys, used as far pointer in Put/Get | Added PhysToGDTSel mapping during INIT; documented virtual addr requirement |
| A1-02 | sioinit.asm | MED | INTERNET keyword detection only checked first char 'I', matched 'IRQ' | Check 'I','N','T' — three chars minimum |
| A1-03 | sioinit.asm | MED | PrintMsg used DevHlp_Save_Message with wrong calling convention | Changed to INT 21h AH=09h (works during driver INIT) |
| A1-04 | sioinit.asm | HIGH | COMn device headers patched but SDevNext pointers never linked | Added linking logic between headers |
| A1-05 | sioio.asm | HIGH | CmdRead/CmdWrite loaded transfer address as far pointer but PDD has physical | Documented dependency on PhysToVirt (fixed in Audit 2) |
| A1-06 | sioio.asm | HIGH | CmdWrite placeholder `mov al, 0` — writes nulls instead of data | Fixed in Audit 2 with PhysToVirt mapping |
| A1-07 | sioio.asm | HIGH | CmdRead NormGotByte never stored byte to caller's buffer | Fixed in Audit 2 with mapped buffer store |
| A1-08 | sioisr.asm | CRIT | ISR always reported "not ours" — EOI never sent | Added ESI accumulator across scan passes; separate total check |
| A1-09 | sioisr.asm | MED | Complex OFFSET arithmetic for DCB field access | Replaced with named EQU offsets (DCB_OFS_FLAGS1 etc.) |
| A1-10 | vsio.c | HIGH | VDMDATA allocated but never stored in g_vdms[] | Added memcpy into g_vdms array after init |
| A1-11 | vsio.c | MED | PDD function pointer from PDDCMD_REGISTER had swapped halves | Correctly assembled 16:16 far pointer from seg:ofs |
| A1-12 | vx00.asm | CRIT | Fn04_Init stack imbalance — extra word on stack before IRET | Rewrote to modify pushed BX on stack via BP |
| A1-13 | vmodem.c | MED | CmdQuerySReg called with wrong pointer after parser advanced | Use sprintf to build register number string |
| A1-14 | vmodem.c | MED | Shared secret parsed from modified addrBuf, found wrong quote | Parse from original addr input string |
| A1-15 | vmodem.c | HIGH | g_numPorts never set — main loop processes zero ports | Set to MAX_VMODEM_PORTS in main() |

---

### Audit 2 — 2026-08-06 23:30 UTC (Full Project)

16 bugs found (includes 3 deferred from Audit 1 + 13 new), all fixed.

| # | File | Severity | Bug | Fix |
|---|------|----------|-----|-----|
| A2-01 | sio_full.asm | CRIT | GetPort always returned port 0 — COM2+ all used COM1's state | Added CurPortIdx tracking, set during Open from device header match |
| A2-02 | sio_full.asm | MED | AllocPhys→PhysToGDTSel may not preserve EAX between calls | Tightened register flow, verified DevHlp return convention |
| A2-03 | sio_full.asm | CRIT | DoWrite stored 0x00 placeholder — every write produced nulls | PhysToVirt maps caller's transfer buffer, reads actual bytes |
| A2-04 | sio_full.asm | CRIT | DoRead extracted bytes but never stored to caller's buffer | PhysToVirt maps transfer address, stores via ES:BX+offset |
| A2-05 | sio_full.asm | CRIT | ISR spRDAstore incremented pointers but never wrote byte to buffer | Added `les bx,[PD_RXBASE]; mov es:[bx],al` — byte now written |
| A2-06 | sio_full.asm | HIGH | COM1-COM4 headers always chained regardless of NumPorts | InitHeaders terminates chain at actual port count |
| A2-07 | vx00.asm | CRIT | Fn00_SetBaud had 2 pushes but 0 pops of AX/DX — stack imbalance | Rewrote push/pop sequence — 3 push, 3 pop, balanced |
| A2-08 | vx00.asm | MED | Fn04_Init used BP without saving — corrupted caller's BP | Added push/pop BP around stack modification |
| A2-09 | vx00.asm | HIGH | Fn0C_PeekChar always returned FFFFh even with data available | Now reads byte from RBR when LSR shows data ready |
| A2-10 | vsio.c | HIGH | VsioByteOut sent partial baud divisor on DLL write while DLAB=1 | Buffers DLL/DLH changes, applies when DLAB cleared in LCR write |
| A2-11 | vsio.c | MED | g_numVDMs incremented but never decremented — 64-session hard limit | TERMINATE removes entry from g_vdms, shifts array, decrements |
| A2-12 | vmodem.c | CRIT | No port .active flag ever set TRUE — main loop skips everything | DosOpen scan in init sets active=TRUE for available ports |
| A2-13 | vmodem.c | CRIT | COM port handle (hCom) initialized to -1, never opened | DosOpen in init loop stores handle; sets non-blocking DCB |
| A2-14 | install.c | MED | Checked CONFIG.SYS for 'SIO.SYS' but driver is 'SIO2K.SYS' | Changed check string to 'SIO2K.SYS' |
| A2-15 | install.c | MED | Appended DEVICE=SIO.SYS — wrong driver name, missing UART.SYS | Now appends SIO2K.SYS + UART.SYS + VSIO2K.SYS |
| A2-16 | d4test.c | LOW | test_36_shared_open hardcoded COM1 instead of test port | Now uses port parameter passed to test suite |

---

### Warnings (not bugs, but worth noting)

| # | File | Warning |
|---|------|---------|
| W01 | sio_full.asm | ParseCmdLine stores port data at (COMn-1)*PD_SIZE but increments NumPorts sequentially. Out-of-order port definitions leave gaps. |
| W02 | vsio.c | VDHQueryVDM() function name assumed — actual MVDM API may differ. |
| W03 | vmodem.h | **FIXED** — conditional macro for OS/2 vs POSIX. |
| W04 | vmodem.c | **FIXED** — sends TTYPE IS response with configured terminal type. |
| W05 | su.c | **FIXED** — ShowSignalsH takes pre-opened handle. |
| W06 | pmlm.c | Race condition between RX count check and DosRead. |
| W07 | viewpmlm.c | ANSI escape codes require ANSI ON in OS/2 session settings. |
| W08 | sio.lnk | Relative paths to Watcom libraries. |
| W09 | vsio.lnk | Same relative path issue. |
| W10 | vmodem.lnk | Same + socket library references. |

---

### Audit 3 — Hexadecimal (ISR & Ring Buffer Review)

4 issues found in ISR and ring buffer code, all fixed.

| # | File | Severity | Bug | Fix |
|---|------|----------|-----|-----|
| W-01 | sio_full.asm | MED | RingBuf Put/Get rbCount not CLI/STI protected — ISR re-entry could corrupt count | Added CLI/STI around inc/dec of PD_RXCOUNT and PD_TXCOUNT in all 4 locations |
| W-02 | sio_full.asm | HIGH | FIFO drain loop (spRDA) has no max iteration — HW fault causes infinite ISR loop | Added push cx/mov cx,256 counter, exits after 256 bytes per ISR call |
| W-03 | sio_full.asm | MED | TryTransmit FIFO fill hardcoded to 16 bytes — wastes capacity on 16650/16750/16850 | KickTxBatch reads PD_FIFOSZ and fills up to actual FIFO depth |
| W-04 | sio_full.asm | LOW | ProcRun called without validating block ID — stale ID possible on race | Safe by design (OS/2 ProcRun ignores invalid IDs). Documented with comment. |

---

### Missing Features (by design — V2 scope)

| # | Feature | Status |
|---|---------|--------|
| M01 | CONFIG.SYS INTERNET keyword | **FIXED** — parser handles INTERNET[:dosaddr] |
| M02 | Hayes ESP / Tport support | V2 scope — verified: v2's `esp/esp.c` implements this |
| M03 | 16650/16750/16850/16950 detection | **FIXED** — EFR probe, 64-byte FIFO enable |
| M04 | SMP spinlocks | **Not actually delivered anywhere** — v2 (sio2src) has no spinlock code either; this label implied it was handled elsewhere, but it wasn't. Genuinely unimplemented in both versions. |
| M05 | PCMCIA hot-plug | **Not actually delivered anywhere** — same issue as M04: no PCMCIA code exists in v2 (sio2src) either. |
| M06 | Log file generation | **FIXED** — writes \SIO.LOG during INIT |
| M07 | Registration system | Intentionally omitted (GPLv3) |
| M08 | VMP framing protocol | **Corrected — this label was wrong.** v1's own `vmodem/vmodem.c` already implements VMP (IANA port 3141, `ATDV`/`#`-prefix dialing, `VMPORT.isVMP`) — it is NOT deferred to v2. In fact v2's own STATUS.md explicitly marks VMP as proprietary and skips it ("VMP is proprietary and skip"). So v1 has VMP and v2 deliberately doesn't; the "V2 scope" label had it backwards. |
| M09 | VMODEM.SYS (V2 physical layer) | V2 scope — verified: v2's `vmodem/vmodem_sys.c` exists |
| M10 | Per-VDM instance management | **FIXED** — AllocVDMData/FreeVDMData with compaction |
| M11 | FOSSIL-level test harness | **FIXED** — FOSTEST.EXE, 20 INT 14h tests |
| M12 | V2 SIO2K↔UART split architecture | V2 scope — verified: v2's `sio2k/sio2k.c` + `uart/uart.c` split exists |
| M13 | Auto-IRQ / Auto-FIFO / Auto-crystal / PCI / SuperIO | V2 scope — partially verified: v2 has `pci/pci.c` (PCI detection) and auto-FIFO sizing per its own BUGFIXES notes; auto-IRQ/auto-crystal/SuperIO not independently re-checked this pass |

**2026-08 verification note, answering "are we supposed to have these
features in v1?":** No — items still marked "V2 scope" above are
correctly out of v1's scope by design, and were checked against the
actual v2 (sio2src) source rather than taken on faith: M02/M09/M12
are confirmed present in v2. **Two labels were wrong and have been
corrected**: M04 and M05 were marked "V2 scope" but neither is
actually implemented in v2 either — they're unimplemented in the
whole project, not deferred-and-delivered elsewhere. M08 was
backwards — v1 already has VMP, and v2 explicitly declined it as
proprietary. None of this changes what v1 should ship; it changes
what the tracking table can be trusted to mean.

---

## Changelog (formerly sio1src/CHANGELOG.md)

### [1.2.0] — 2026-08-08 01:15 UTC — Wrench Audit Fixes

#### Fixed (from hexadecimal's ISR & ring buffer review)
- W-01: CLI/STI around rbCount in all ring buffer Put/Get paths
- W-02: FIFO drain loop capped at 256 iterations per ISR call
- W-03: KickTxBatch uses actual PD_FIFOSZ (16/32/64) not hardcoded 16
- W-04: ProcRun block ID validity documented (safe by OS/2 design)

---

### [1.1.0] — 2026-08-07 23:50 UTC — Feature Complete

#### Added
- M01: CONFIG.SYS INTERNET keyword support in parser
- M03: Extended UART detection — 16650, 16750 (with EFR probe, 64-byte FIFO)
- M06: Log file generation — writes \SIO.LOG during driver initialization
- M10: Proper VDM instance management (AllocVDMData/FreeVDMData with compaction)
- M11: FOSTEST.EXE — 20-test FOSSIL (INT 14h) conformance harness for DOS VDMs
- KickTxBatch — FIFO-aware batch transmit (fills FIFO in one ISR pass)

#### Fixed
- W03: vmodem.h soclose macro — conditional for OS/2 vs POSIX
- W04: TTYPE telnet subnegotiation response added
- W05: su.c ShowSignalsH avoids double port open
- VSIO VDD linking — inline memset/memmove/memcpy (no C runtime in kernel)

---

### [1.0.0] — 2026-08-07 00:00 UTC — Audited Release

#### All 16 audit bugs fixed
- See BUGFIXES.md for full details

#### Components
- SIO.SYS — 2,151 lines WASM, 6,140 bytes LX binary
- VSIO.SYS — 596 lines C, 2,731 bytes LX binary
- VX00.SYS — 953 lines ASM, 1,072 bytes MZ binary
- VMODEM.EXE — 1,150 lines C, 32,449 bytes LX binary
- SU.EXE — 326 lines C, 15,923 bytes LX binary
- PMLM.EXE — 274 lines C, 19,809 bytes LX binary
- VIEWPMLM.EXE — 189 lines C, 19,073 bytes LX binary
- INSTALL.EXE — 183 lines C, 21,425 bytes LX binary
- D4TEST.EXE — 592 lines C, 19,235 bytes LX binary

#### Totals
- 10,209 lines of source across 28 files
- 9 OS/2 binaries, 0 compile errors
- 16 bugs found and fixed, 10 warnings documented, 13 missing features noted

---

### [0.9.0] — 2026-08-06 23:30 UTC — V2 Docs Ingested

#### Added
- SIO2K v2.03 documentation ingested (DESIGN.TXT, TECHTALK.TXT, SAMPLE.CFG,
  PCI.INC, HISTORY.TXT, FAQ.TXT, MODES.TXT, LOGGER.TXT, PCI.TXT)
- Full V2 architecture analysis documented
- D4 conformance test mapping to VX00 FOSSIL functions

---

### [0.8.0] — 2026-08-06 22:30 UTC — Full Assembly

#### Changed
- SIO.SYS fully ported to WASM-compatible syntax (1,538→2,005 lines)
- All "short jump out of range" errors resolved
- Linked as LX format (matching original SIO.SYS)

---

### [0.7.0] — 2026-08-06 22:00 UTC — D4 Conformance + Build Environment

#### Added
- D4TEST.EXE — 37-test conformance harness for ASYNC IOCtl validation
- OpenWatcom v2 cross-compilation toolchain configured
- All 8 modules compile to .obj with 0 errors
- Linker scripts for all components

---

### [0.6.0] — 2026-08-06 21:45 UTC — Phase 5 Utilities

#### Added
- SU.EXE — port status, signal control, baud rate setting
- PMLM.EXE — line monitor with VIO display, column tracking, TX monitoring
- VIEWPMLM.EXE — trace file viewer with hex dump and keyboard navigation
- INSTALL.EXE — automated driver installation and CONFIG.SYS update

---

### [0.5.0] — 2026-08-06 21:20 UTC — Phase 4 Complete + Audit

#### Added
- VMODEM.EXE — telnet/VMP virtual modem with AT command set, MD5 auth
- Full code audit: 15 bugs found, 12 fixed

#### Fixed (from first audit)
- siobuf.asm: PhysToGDTSel mapping for ring buffer base addresses
- sioinit.asm: INTERNET keyword detection (was matching 'IRQ')
- sioinit.asm: PrintMsg calling convention (DevHlp_Save_Message → INT 21h)
- sioinit.asm: COMn header SDevNext linking
- sioio.asm: PhysToVirt mapping for Read/Write transfer buffers
- sioisr.asm: ISR service tracking (ESI accumulator across scan passes)
- sioisr.asm: DCB field access simplified with named EQU offsets
- vsio.c: VDMDATA stored in g_vdms array
- vsio.c: PDD function pointer assembly from seg:ofs
- vx00.asm: Fn04_Init stack imbalance (BP-based stack modification)
- vmodem.c: g_numPorts initialization
- vmodem.c: Shared secret parsing from original input
- vmodem.c: CmdQuerySReg pointer arithmetic

---

### [0.4.0] — 2026-08-06 21:00 UTC — Phase 3 VX00.SYS

#### Added
- VX00.SYS — complete FOSSIL driver (FTS-0001 Rev 5)
- All 20 INT 14h functions implemented
- DOS device driver header, INT 14h hook/chain

---

### [0.3.0] — 2026-08-06 20:45 UTC — Phase 2 VSIO.SYS

#### Added
- VSIO.SYS — Virtual Device Driver for DOS VDMs
- I/O port hooks for all 8 UART registers
- Virtual UART state per VDM
- PDD-VDD communication protocol (vsio.h)

---

### [0.2.0] — 2026-08-06 20:31 UTC — Phase 1 SIO.SYS

#### Added
- SIO.SYS — complete OS/2 Physical Device Driver
- Device header, strategy entry, command dispatch (32 commands)
- UART detection (8250→16550A), init, baud rate, modem signals
- Ring buffer operations (put/get/peek/flush/count/free)
- ISR with shared-IRQ scanning, RX/TX/LSR/MSR handlers, XON/XOFF
- CONFIG.SYS parser, UART probing, buffer allocation, header chaining
- Open/Close with IRQ claim/release, DTR/RTS per DCB
- Read/Write with timeout modes, blocking via DevHlp_ProcBlock
- Input/Output status and flush
- All 20 IOCtl functions (41h–74h)

---

### [0.1.0] — 2026-08-06 19:47 UTC — Project Foundation

#### Added
- Project structure with 8 component directories
- README.md with full API specification from documentation
- OS/2 Toolkit DDK headers extracted
- Original SIO documentation (SIOREF.TXT, VX00.TXT, VMODEM.TXT)

---

## Progress Report (formerly sio1src/PROGRESS.md)

**Last Updated:** August 8, 2026 01:30 UTC

### Contributors

| Name | Role |
|------|------|
| evga | Lead developer (Claude) |
| wrench | D4 conformance test design, FOSSIL function mapping |
| hexadecimal | ISR & ring buffer audit (4 fixes) |

---

### Project 1: Mystic BBS Source Recovery

**Status:** COMPLETE

- Recovered from partial binary-reconstructed archive
- 64,103 lines compiled with FPC 3.2.2, zero errors
- Working ELF binary boots and looks for mystic.dat
- 4 bugs fixed in fork files
- 40 upstream units pulled from FIDOSOFT/mysticbbs GPL v3
- 5 fork-specific functions written from scratch
- 14 CP437 encoding issues fixed

**Caveats:** 40 upstream units are placeholders for fork versions (A39-A61 changes unknown). 5 invented functions may differ from original behavior.

---

### Project 2: SIO V1 Clean-Room Rebuild

**Status:** FEATURE COMPLETE — V1.2.0

#### Components (9 binaries, all compile+link with OpenWatcom v2)

| Binary | Lines | Size | Format |
|--------|-------|------|--------|
| SIO.SYS | 2,449 | 6,944 B | LX (OS/2 PDD) |
| VSIO.SYS | 604 | 2,829 B | LX (OS/2 VDD) |
| VX00.SYS | 953 | 1,072 B | MZ (DOS driver) |
| VMODEM.EXE | 1,150 | 32,537 B | LX (OS/2 console) |
| SU.EXE | ~350 | 16,191 B | LX |
| PMLM.EXE | 274 | 19,809 B | LX |
| VIEWPMLM.EXE | 189 | 19,073 B | LX |
| INSTALL.EXE | 183 | 21,425 B | LX |
| D4TEST.EXE | 592 | 19,235 B | LX |

#### Test Harnesses
- **D4TEST.EXE:** 37 tests covering all 20 ASYNC IOCtl functions (41h-74h)
- **FOSTEST:** 20 tests for FTS-0001 INT 14h functions (for DOS VDMs)

#### Audit History

| Audit | By | Bugs Found | Fixed |
|-------|-----|-----------|-------|
| Audit 1 (Phase 4) | evga | 15 | 15/15 |
| Audit 2 (Full project) | evga | 16 | 16/16 |
| Audit 3 (ISR/buffers) | hexadecimal | 4 | 4/4 |
| **Total** | | **35** | **35/35** |

#### Key Bug Fixes (highlights)
- ISR never wrote received bytes to RX buffer (A2-05)
- Read/Write never mapped caller's transfer buffer (A2-03/04)
- GetPort always returned COM1 (A2-01)
- VX00 Fn04_Init stack imbalance (A2-07)
- VMODEM never opened COM ports (A2-12/13)
- VSIO sent partial baud divisors (A2-10)
- RingBuf count not CLI/STI protected (hex W-01)
- FIFO drain loop could hang ISR (hex W-02)
- FIFO fill hardcoded to 16 bytes (hex W-03)

#### Features Implemented
- All 20 ASYNC IOCtl functions (41h-74h)
- UART detection: 8250, 8250A, 16450, 16550, 16550A, 16650, 16750
- Full ISR with shared-IRQ, XON/XOFF, CTS/RTS handshake
- CONFIG.SYS parser with INTERNET keyword
- Ring buffers with CLI/STI-protected count fields
- FIFO-aware batch transmit (KickTxBatch)
- ISR drain loop with 256-byte safety limit
- Log file generation (\SIO.LOG)
- DCB with SIO forced bits (FIFO enable, trigger 8, TX load 16)
- FOSSIL driver (20 INT 14h functions, FTS-0001 Rev 5)
- Virtual modem with telnet, AT commands, MD5 auth, TTYPE subneg
- Line monitor (PMLM) with VIO display and trace output
- Trace viewer (VIEWPMLM) with hex dump
- Automated installer with CONFIG.SYS update

#### Remaining V1 Items (deferred to V2)
- Hayes ESP / Tport hardware support
- SMP spinlocks (uses CLI/STI only)
- PCMCIA hot-plug
- VMP framing protocol
- VMODEM.SYS physical layer driver
- PCI serial card scanning
- SuperIO chip support

---

### Project 3: SIO2K V2 Clean-Room Rebuild

**Status:** ALPHA — Core architecture built

#### Components Built

| Component | Lines | Status |
|-----------|-------|--------|
| sio2k_idc.h | 143 | IDC interface — 25 commands, 5 callbacks, complete |
| sio2k.c | 662 | Logical layer — all 20 IOCtls, Open/Close, DCB, callbacks |
| cfgparse.c + .h | 424 | Config parser — Os2Device, BaseUart, DosDevice sections |
| uart.c | 688 | Physical layer — detection, auto-FIFO, auto-crystal, block I/O |
| d4test.c | 592 | Test harness — copied from V1 (same IOCtl interface) |

**All 4 V2 modules compile clean with OpenWatcom v2.**

#### V2 Architecture

```
  OS/2 Applications
       │
  SIO2K.SYS  ← Logical (IOCtl, DCB, config)
       │ IDC
       ├── UART.SYS   ← Physical (8250→16950)
       ├── ESP.SYS    ← Physical (Hayes ESP)
       └── VMODEM.SYS ← Physical (virtual modem)
              │
         VMODEM.EXE   ← Telnet/VMP app
       │
  VSIO2K.SYS ← VDD (universal — works with any serial driver)
  VX00.SYS   ← FOSSIL (nearly unchanged from V1)
```

#### V2 Remaining Work
- VSIO2K.SYS (universal VDD)
- VMODEM.SYS (physical layer)
- ESP.SYS (Hayes ESP)
- MODES.EXE, LOGGER.EXE, PCI.EXE utilities
- PCI scanning with PCI.INC database
- SuperIO chip detection
- Block I/O (REP INSB/OUTSB) in real ASM
- Full code audit

---

### Build Environment

- **Compiler:** OpenWatcom v2 (2024-03-01 build)
- **Assembler:** WASM (x86, 16-bit and 32-bit)
- **C Compilers:** WCC (16-bit), WCC386 (32-bit)
- **Linker:** WLINK (NE, LX, MZ formats)
- **Target:** OS/2 2.0+ (LX format) and DOS (MZ format)
- **Host:** Linux x86_64 cross-compilation

---

### Release History

| Version | Date | Description |
|---------|------|-------------|
| V1.2.0 | 2026-08-08 01:15 | Hexadecimal ISR audit fixes |
| V1.1.0 | 2026-08-07 23:50 | Feature complete (M01,M03,M06,M10,M11,W03-W05) |
| V1.0.0 | 2026-08-07 00:00 | 16/16 bugs fixed, 9 binaries |
| V0.9.0 | 2026-08-06 23:30 | V2 docs ingested |
| V0.8.0 | 2026-08-06 22:30 | Full WASM assembly |
| V0.7.0 | 2026-08-06 22:00 | D4 + build environment |
| V0.6.0 | 2026-08-06 21:45 | Phase 5 utilities |
| V0.5.0 | 2026-08-06 21:20 | Phase 4 + first audit |
| V0.4.0 | 2026-08-06 21:00 | VX00.SYS FOSSIL |
| V0.3.0 | 2026-08-06 20:45 | VSIO.SYS VDD |
| V0.2.0 | 2026-08-06 20:31 | SIO.SYS PDD |
| V0.1.0 | 2026-08-06 19:47 | Project foundation |

---

## 2026-08 Follow-up — Registration Key Check

Asked whether `sio1src` needs a registration/license key, since this
is a clean-room rebuild. Checked: **no**, and it doesn't have one.

Grepped the entire tree (`.asm`, `.inc`, `.c`, `.h`) for anything
key/serial/registration/license/unlock/nag-related. Every hit is
unrelated to licensing — keyboard keystroke handling in
`viewpmlm.c`/`pmlm.c`/`vx00.asm`, OS/2 `DevHlp_Unlock`/
`DSK_UNLOCKDRIVE`-style memory/disk-lock APIs, and the VDM-manager
"PDD registration" handshake (a driver-registration mechanism,
unrelated to shareware licensing). No code anywhere checks a
registration key, a serial number, or shows a nag/watermark. This
matches BUGFIXES.md's own "Missing Features" table above (M07:
"Registration system — Intentionally omitted (GPLv3)").

For reference, the original binary's `SIO.SYS` does contain a real
shareware nag/watermark mechanism — `strings` shows a
`"Registered to <name>"` banner, unregistered-copy nag text, and a
long scrambled placeholder string shown in place of a name when
unregistered. `REREG.EXE`'s strings confirm it transfers a serial
number and checks port-count/major-version compatibility between an
old registered `SIO.SYS` and a new install, pointing users at a
1994-era BBS phone number to register.

**We do not have the keygen/watermark algorithm** — neither the
encoding scheme for the registered-name banner nor any serial-number
validation logic. That exists only in the original proprietary
binary and was correctly never reverse-engineered, consistent with
this project's clean-room policy — and doubly moot now that the
author has released SIO as freeware.
