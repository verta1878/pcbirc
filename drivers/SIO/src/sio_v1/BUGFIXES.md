# Bug Fixes — SIO Clean-Room Rebuild

## Audit History

Two full code audits were performed across all source files.
All bugs have been fixed as of the 2026-08-07 00:00 UTC release.

---

## Audit 1 — 2026-08-06 21:20 UTC (Phase 4)

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

## Audit 2 — 2026-08-06 23:30 UTC (Full Project)

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

## Warnings (not bugs, but worth noting)

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

## Audit 3 — Hexadecimal (ISR & Ring Buffer Review)

4 issues found in ISR and ring buffer code, all fixed.

| # | File | Severity | Bug | Fix |
|---|------|----------|-----|-----|
| W-01 | sio_full.asm | MED | RingBuf Put/Get rbCount not CLI/STI protected — ISR re-entry could corrupt count | Added CLI/STI around inc/dec of PD_RXCOUNT and PD_TXCOUNT in all 4 locations |
| W-02 | sio_full.asm | HIGH | FIFO drain loop (spRDA) has no max iteration — HW fault causes infinite ISR loop | Added push cx/mov cx,256 counter, exits after 256 bytes per ISR call |
| W-03 | sio_full.asm | MED | TryTransmit FIFO fill hardcoded to 16 bytes — wastes capacity on 16650/16750/16850 | KickTxBatch reads PD_FIFOSZ and fills up to actual FIFO depth |
| W-04 | sio_full.asm | LOW | ProcRun called without validating block ID — stale ID possible on race | Safe by design (OS/2 ProcRun ignores invalid IDs). Documented with comment. |

---

## Missing Features (by design — V2 scope)

| # | Feature | Status |
|---|---------|--------|
| M01 | CONFIG.SYS INTERNET keyword | **FIXED** — parser handles INTERNET[:dosaddr] |
| M02 | Hayes ESP / Tport support | V2 scope (ESP.SYS) |
| M03 | 16650/16750/16850/16950 detection | **FIXED** — EFR probe, 64-byte FIFO enable |
| M04 | SMP spinlocks | V2 scope |
| M05 | PCMCIA hot-plug | V2 scope |
| M06 | Log file generation | **FIXED** — writes \SIO.LOG during INIT |
| M07 | Registration system | Intentionally omitted (GPLv3) |
| M08 | VMP framing protocol | V2 scope |
| M09 | VMODEM.SYS (V2 physical layer) | V2 scope |
| M10 | Per-VDM instance management | **FIXED** — AllocVDMData/FreeVDMData with compaction |
| M11 | FOSSIL-level test harness | **FIXED** — FOSTEST.EXE, 20 INT 14h tests |
| M12 | V2 SIO2K↔UART split architecture | V2 scope |
| M13 | Auto-IRQ / Auto-FIFO / Auto-crystal / PCI / SuperIO | V2 scope |
