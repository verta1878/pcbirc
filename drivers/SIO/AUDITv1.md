# SIO v1 Code Audit — August 8, 2026

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
