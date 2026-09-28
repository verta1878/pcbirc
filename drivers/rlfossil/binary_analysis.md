# RLFOSSIL v1.2 — Binary Analysis & Recovery Plan

**Binary:** RLFOSSIL.EXE (37,558 bytes, May 3 1995)
**Author:** Andrew Sapozhnikov (Sapa), root@hq.icb.chel.su
**Compiler:** Borland C++ 3.1 (Small model, DOS real-mode)
**TCP Stack:** WatTCP (Waterloo TCP) statically linked

---

## 1. MZ Header

```
Last page:    182 bytes
Pages:        74  (37,558 bytes total)
Relocations:  6
Header:       32 paragraphs (512 bytes)
SS:           0x13DD
SP:           0x0080
CS:IP:        0000:0000
Min alloc:    2,778 paragraphs
Model:        Borland C Small (near code, near data)
```

## 2. Memory Layout

```
Offset    Size    Contents
0x0000    512     MZ header + relocation table
0x0200    ~7KB    Application code (FOSSIL + VMODEM + main)
0x1E00    ~25KB   WatTCP library (TCP/IP stack)
0x8200    ~4KB    Borland C runtime (startup, heap, printf)
0x9200    ~800    String data (messages, config keywords)
0x9500    end     BSS / stack
```

## 3. Application Code (~7KB)

### FOSSIL INT 14h Handler
```
Fn 00h  Set baud (no-op, returns status 0x6C10)
Fn 01h  Write char wait (→ txbuf ring buffer)
Fn 02h  Read char wait (← rxbuf ring buffer)
Fn 03h  Status (0x6C10 + RDA + DCD flags)
Fn 04h  Init (returns AX=1954h, BH=5, BL=1Bh)
Fn 05h  Deinit
Fn 06h  DTR control (drop = hangup TCP)
Fn 07h  Timer tick (no-op)
Fn 08h  Flush (drain txbuf → TCP, sock_flush)
Fn 09h  Purge output
Fn 0Ah  Purge input
Fn 0Bh  Write no wait
Fn 0Ch  Peek input
Fn 0Dh  Keyboard peek (not implemented)
Fn 0Eh  Keyboard read (not implemented)
Fn 0Fh  Flow control
Fn 10h  Ctrl-C/K checking (no-op)
Fn 18h  Block read (ES:DI far pointer)
Fn 19h  Block write (ES:DI far pointer)
Fn 1Ah  Break (no-op for TCP)
Fn 1Bh  Get FOSSIL info (19-byte struct)
```

### VMODEM AT Parser
```
ATZ          Reset socket, clear buffers
ATDP/ATDT    Dial (TCP connect, comma→dot conversion)
             Supports user@host format
ATH          Hangup (sock_close)
ATA          Answer incoming (after RING)
ATE0/ATE1    Echo off/on
ATS<n>=<v>   Set S-register (0-15)
```

### S-Registers
```
S0   Auto-answer (0=off, non-zero=on)
S1   Incoming IP port (513=rlogin, 23=telnet, 0=both)
S2   CD delay (ATA→CONNECT, in 55ms ticks, default 72 ≈ 4s)
S3   NC delay (disconnect→NO CARRIER, in 55ms ticks, default 18 ≈ 1s)
S4   Handshake mode (0=none, 1=rlogin, 2=telnet, 3=auto)
S5   Outgoing IP port (default 513=rlogin)
```

### Result Codes (exact strings from binary)
```
"CONNECT 9600/TCP/IP"   Connection established
"NO CARRIER"            Connection failed/lost
"NO DIALTONE"           DNS resolution failed
"RING"                  Incoming connection
"OK"                    Command accepted
```

### Handshake Protocols
```
rlogin:  \0 + login\0 + login\0 + term/speed\0 → wait for \0 response
telnet:  Accept connection (IAC filtering in I/O path)
auto:    Port 23 = telnet, port 513 = rlogin
```

## 4. Buffer Architecture

```
FOSSIL level:   1024 TX + 1024 RX (ring buffers, per port)
TCP level:      2048 TX + 2048 RX (WatTCP internal buffers)
AT command:     256 bytes (accumulator)
Max ports:      4 channels
```

## 5. WatTCP Config Keywords (from binary)

```
Standard WatTCP:           RLFOSSIL-specific:
  MY_IP                      IDNAME (login name)
  NETMASK                    AAMASK (auto-answer mask)
  NAMESERVER                 CD_DELAY
  GATEWAY                    NC_DELAY
  HOSTNAME                   TERM_TYPE
  DOMAINSLIST                INPORT
  BOOTP                      OUTPORT
  SOCKSDELAY                 HANDSHAKE
  INACTIVE
  INCLUDE
  MULTIHOMES
  ETHIP
  PRINT
  COOKIE
```

## 6. Borland C Runtime Strings

```
"Borland C++ - Copyright 1991 Borland Intl."
"Abnormal program termination"
"Null pointer assignment"
"Divide error"
"Ctrl-Breaks ignored"
```

## 7. TCP State Machine Strings (WatTCP)

```
Listen, SynSent, SynRec, Established,
FinWt1, FinWt2, ClosWt, LastAckTmWt, Closed
```

## 8. Included Documentation (.001-.007)

FidoNet DOS_INTERNET echo messages from Feb 1996:
```
.001  rowan_crowe: "Practical problems with virtual modems"
.002  Tony Langdon: Reply — fragmentation, datagram size
.003  rowan_crowe: Reply — Trailblazer PEP modem testing
.004  rowan_crowe: "Tons of questions" re: RLFOSSIL + BBS
.005  rowan_crowe: "RLFOSSIL" — BinkleyTerm transfer log
.006  Tony Langdon: "Success!" — rlogin working, nice response times
.007  Tony Langdon: Reply — serial port program discussion
```

Historical evidence of RLFOSSIL in production use with FidoNet
mailers (BinkleyTerm, FrontDoor) and BBS systems (Maximus).

---

## 9. Build Phases

### Phase A: Core Recovery (DONE)
- [x] FOSSIL INT 14h dispatch (all Fn 00h-1Bh)
- [x] Ring buffers (1024 TX + 1024 RX)
- [x] VMODEM AT parser (ATZ/ATD/ATH/ATA/ATE/ATS)
- [x] S-register system (S0-S5)
- [x] rlogin + telnet handshake
- [x] WatTCP config parsing (IDNAME, AAMASK, etc.)
- [x] Child process launcher (system())
- [x] INT 14h hook/unhook via DOS INT 21h

### Phase B: Binary Audit
- [ ] Disassemble INT 14h handler — verify dispatch table
- [ ] Map all function entry points
- [ ] Verify buffer sizes (1024/2048)
- [ ] Verify S-register count and defaults
- [ ] Check for undocumented features
- [ ] Verify WatTCP API calls (sock_init, tcp_open, etc.)
- [ ] Check for DV/TopView timeslice release (INT 15h/28h)
- [ ] Identify any crypto or registration code (none expected)

### Phase C: Compile with Borland C
- [ ] Set up Borland C++ 3.1 build environment
- [ ] Obtain WatTCP 2.x library (circa 1994-1995)
- [ ] Compile rlfossil.c with BC31 small model
- [ ] Link with WatTCP library
- [ ] Compare binary size (target: 37,558 bytes)
- [ ] Compare string offsets
- [ ] Verify MZ header fields match

### Phase D: Byte-Match Verification
- [ ] Binary diff original vs rebuild
- [ ] Identify compiler-specific code generation patterns
- [ ] Adjust optimization flags (-O1 vs -O2 vs -Od)
- [ ] Match Borland C runtime version
- [ ] Match WatTCP library version
- [ ] Match linker settings (segment order, alignment)
- [ ] Final byte-for-byte comparison

### Phase E: Modern Port
- [ ] Replace WatTCP with Winsock/BSD sockets
- [ ] Replace INT 14h hook with DLL/VxD approach
- [ ] Add to WinFOSSIL as rlogin/telnet backend
- [ ] Cross-compile for DOS (DJGPP/Watcom) + Win32 + Linux

### Phase F: Test Suite
- [ ] Ring buffer tests
- [ ] AT parser tests
- [ ] S-register tests
- [ ] rlogin handshake protocol test
- [ ] telnet IAC filtering test
- [ ] Multi-port concurrent test
- [ ] Config file parsing test
- [ ] Child process launch/exit test

---

## 10. Key Differences from WinFOSSIL

| Feature | RLFOSSIL | WinFOSSIL |
|---------|----------|-----------|
| Platform | DOS real-mode | Win95/NT/Modern |
| TCP stack | WatTCP (packet driver) | Winsock |
| INT 14h | Direct hook via setvect | VxD/VDD/DLL |
| TSR | No — launches child | Yes — stays resident |
| Ports | 1-4 | 1-4 |
| rlogin | Yes (port 513) | No |
| telnet | Yes (port 23) | Yes |
| S-registers | 16 (6 used) | None (registry) |
| Auto-answer | Yes (AAMASK) | Yes (ATS0=) |
| user@host | Yes | No |
| comma→dot | Yes (phone compat) | No |
| Multitasker | DV/TopView/Win3.x | Win32 native |

---

## 11. Phase B Audit Results

### Disassembly Method
ndisasm -b 16 real-mode disassembly of code segment (0x200+).
Full dispatch table decoded at CS:0x15DE.

### Dispatch Table (28 entries at CS:0x15DE)

```
Fn  Addr    Handler              Notes
00  0x1444  Status               Same handler as Fn 03h
01  0x1439  Write char wait      Push AL, call 0x0A7F (ring buffer put)
02  0x1434  Read char wait       Push 0, jmp 0x11ED (ring buffer get)
03  0x1444  Status               Returns 0x6C10 + RDA + DCD flags
04  0x1447  Init FOSSIL          Clears buffers, tcp_listen if autoanswer, AX=1954h
05  0x14BB  Deinit               Calls sock_close (0x6949), state=6
06  0x14D3  DTR control          AL=0 → sock_close (hangup)
07  0x15D3  Timer tick           NO-OP (I/O pump runs from timer ISR)
08  0x15D3  Flush                NO-OP (I/O pump handles data transfer)
09  0x1531  Purge output         Clear txbuf head/tail
0A  0x1544  Purge input          Clear rxbuf head/tail
0B  0x1553  Write no wait        Push AL, call 0x0A7F, check space
0C  0x155C  Peek input           JMP 0x11EB (ring buffer peek)
0D  0x155F  Keyboard peek        Push 1, call 0x7194 (kbhit wrapper)
0E  0x1576  Keyboard read        Push 0, call 0x7194 (getch wrapper)
0F  0x15D3  Flow control         NO-OP (TCP doesn't need flow control)
10  0x157E  Ctrl-C/K check       XOR AX,AX; return (always "no Ctrl-C")
11-17 0x15D3 Screen/reserved     NO-OP
18  0x1582  Block read           ES:DI far pointer, loop read
19  0x159A  Block write          ES:DI far pointer, loop write
1A  0x15D3  Break                NO-OP (no break signal over TCP)
1B  0x15B4  Get FOSSIL info      19-byte struct, call 0x07C2 first
```

### Port Structure Layout (0x1A12 = 6674 bytes per port)

```
Offset  Size   Field
0x000   2      Status/mode word (0x0002 on init)
0x002   2      Reserved
0x004   2      Port number (from S-register at 0x922)
0x006   2      rxbuf.tail
0x008   2      rxbuf.head
0x00A   2      txbuf.tail
0x00C   2      txbuf.head
0x00E   1024   rxbuf.data (1024 bytes)
0x40E   1024   txbuf.data (1024 bytes) [estimated]
0x80E   ?      AT command buffer (256 bytes)
0x892   2      FOSSIL state (1=active, 6=deactivated, 8=idle)
0x91E   2      TCP listen port number
0x920   2      S-register: auto-answer
0x922   2      S-register: default port / saved baud
0x924   2      S-register: CD delay
0x926   2      S-register: NC delay
0x93C   2      Timer countdown 1 (cleared on init)
0x93E   2      Timer countdown 2
0x940   2      Timer countdown 3
0x942   2      (cleared on init)
0x944   1      Handshake mode
0x945   1      Echo flag
0x946   4300   tcp_Socket (WatTCP internal, includes 2K TX + 2K RX buffers)
```

Total: 0x1A12 = 6674 bytes × 4 ports = 26,696 bytes

### Timer Interrupt Handler (INT 08h chain or INT 1Ch)

```
Entry:  Save all registers
        Switch to internal stack at 0x8AF1
        Call tcp_tick() via FAR pointer at [0x76ED]
        Loop 4 ports (SI += 0x1A12 per port):
          Decrement delay counters [SI], [SI+2], [SI+4]
        Release DV timeslice: INT 2Fh AX=1682h
        Release generic timeslice: INT 15h AX=1680h
        Restore registers, IRET
```

### Corrections to Phase A Recovery

| # | Issue | Fix |
|---|-------|-----|
| 1 | Fn 0Dh/0Eh keyboard marked "not implemented" | IMPLEMENTED — calls kbhit()/getch() |
| 2 | Fn 08h flush had full drain loop | NO-OP in original — I/O pump handles it |
| 3 | Fn 10h Ctrl-C was "no-op" | Returns AX=0 (no Ctrl-C pending) |
| 4 | No DV/TopView timeslice release | INT 2Fh AX=1682h + INT 15h AX=1680h |
| 5 | No internal stack switching | Handler uses stack at 0x8AF1 |
| 6 | Port struct size unknown | 0x1A12 (6674) bytes confirmed |
| 7 | tcp_Socket size guessed at 512 | Actually 4300 bytes (includes buffers) |
| 8 | Fn 00h independent of Fn 03h | Same handler (confirmed by jump table) |
| 9 | I/O pump called from main loop | Actually runs from timer ISR |

### AT Command Dispatch Table (CS:0x0CF1)

Binary uses a lookup table — no toupper(), separate entries for upper/lowercase.

```
Index  Char  Handler  Command
  0    NUL   0x0CE2   Default — returns "OK" for all unknown AT commands
  1    'A'   0x0C14   ATA (Answer incoming)
  2    'D'   0x0B52   ATD/ATDT/ATDP (Dial)
  3    'S'   0x0C3C   ATS<n>=<v> (S-register set)
  4    'Z'   0x0CA7   ATZ (Reset — close socket, reopen listener)
  5    'a'   0x0C14   ata (lowercase answer)
  6    'd'   0x0B52   atd (lowercase dial)
  7    's'   0x0C3C   ats (lowercase S-register)
  8    'z'   0x0CA7   atz (lowercase reset)
```

**Only 4 actual AT commands: A, D, S, Z.**
ATE, ATH, ATI, AT& are NOT implemented — all return "OK" via entry 0.
Hangup is ONLY via DTR drop (FOSSIL Fn 06h with AL=0).
Echo is NOT controlled by ATE — it's a transport-level state.

### Two-Level State Machine

```
Level 1: FOSSIL/VMODEM state at [port+0x892] (app-facing)
  1 = ACTIVE (FOSSIL initialized, command mode)
  2 = RINGING (incoming TCP, RING sent, waiting for ATA)
  3 = ONLINE (data transfer, connected)
  5 = DIALING (TCP connect in progress)
  6 = DEACTIVATED (FOSSIL deinit'd)
  7 = ANSWERING (handshake in progress after ATA)
  8 = IDLE (not initialized)

Level 2: I/O pump state at [port+0x945] (transport-facing)
  1 = rlogin handshake I/O
  3 = telnet IAC negotiation I/O
  24 = normal data transfer (0x18)
```

The FOSSIL state controls what the BBS app sees.
The I/O pump state controls the TCP layer behavior.
Both must be correct for data to flow.

### Revised Port Structure (6674 bytes confirmed)

```
Offset  Size   Field
0x000   2      Status word (0x0002 on init)
0x004   2      Port config / saved baud
0x006   2      rxbuf.tail
0x008   2      rxbuf.head
0x00A   2      txbuf.tail
0x00C   2      txbuf.head
0x00E   1024   rxbuf.data
0x40E   1024   txbuf.data
0x80E   2      AT cmd buffer (starts with 'A','T')
0x810   ~130   AT cmd body
0x892   2      FOSSIL/VMODEM state (Level 1)
0x91C   2      S0: autoanswer flag
0x91E   2      S1: incoming TCP port
0x920   2      S2: CD delay (55ms ticks)
0x922   2      S3: NC delay (55ms ticks)
0x924   2      S4: handshake mode (0-3)
0x926   2      S5: outgoing TCP port
0x928   20     S6-S15 (unused by app)
0x93C   2      Timer countdown 1
0x93E   2      Telnet negotiation complete flag
0x940   2      Telnet option tracking
0x942   2      rlogin handshake phase counter
0x944   1      Current IAC byte received
0x945   1      I/O pump state (Level 2)
0x946   4300   tcp_Socket (WatTCP: state + 2K TX + 2K RX buffers)
------
Total:  0x1A12 = 6674 bytes × 4 ports = 26,696 bytes
```

### Internal Function Map (20 app functions)

```
Address  Name (deduced)         Description
0x02C2   tcp_send_byte          Send one handshake byte via TCP
0x0321   accept_connection      Accept incoming + start handshake
0x03F0   io_pump_core           Main I/O pump (called from timer ISR)
0x0857   stuff_rxbuf_string     Put string into rxbuf (CONNECT msg etc.)
0x087D   txbuf_put_byte         Put byte into txbuf (for TCP send)
0x0A09   fossil_read            FOSSIL Fn 02h read handler
0x0A65   char_toupper           Character uppercase conversion
0x0A7F   fossil_write           FOSSIL Fn 01h write + VMODEM state machine
0x0B52   cmd_atd                ATD dial handler (comma→dot, resolve, connect)
0x0C14   cmd_ata                ATA answer handler (check state 2, handshake)
0x0C3C   cmd_ats                ATS register handler (parse reg=val, store)
0x0CA7   cmd_atz                ATZ reset handler (close, reopen, clear)
0x0CE2   cmd_default            Default AT handler (push "OK", return)
0x10C9   rxbuf_get_block        Block read from rxbuf (Fn 18h helper)
0x11ED   fossil_read_entry      Fn 02h entry point (calls 0x0A09)
0x1400   fossil_init_entry      Fn 04h entry + dispatch setup
0x142D   dispatch_jump          Jump table dispatch (SHL BX,1; JMP [table])
0x15D3   fossil_noop            Shared no-op return for unused functions
0x1690   timer_handler_inner    Timer ISR: tcp_tick + delay countdown + timeslice
```

### WatTCP Function Map (key functions)

```
Address  WatTCP Function
0x4022   tcp_tick / sock_tick
0x4429   tcp_listen / tcp_open
0x651E   sock_write (send buffer)
0x6949   sock_close
0x7194   keyboard wrapper (kbhit/getch)
0x7228   sock_read / sock_fastread
0x7E9D   resolve (DNS lookup)
0x7FF8   strtol (C library, used by ATS parser)
```

### Phase A Corrections Required

| # | Phase A had | Binary shows | Impact |
|---|-------------|-------------|--------|
| 1 | ATE controls echo | ATE not implemented | ✅ REMOVED from parser |
| 2 | ATH implemented | ATH not implemented | ✅ REMOVED, DTR-only hangup |
| 3 | toupper() on command chars | Raw table with upper+lower entries | Change AT parser to table lookup |
| 4 | S-regs at struct+0x920 | S-regs at struct+0x91C | Fix offsets |
| 5 | States 0-5 | States 1,2,3,5,6,7,8 | ✅ Fixed to 1,2,3,5,6,7,8 |
| 6 | State 2 = command | State 2 = ringing | ✅ Fixed |
| 7 | State 1 = online | State 3 = online | ✅ Fixed |
| 8 | Single state machine | Two-level (FOSSIL + I/O pump) | ✅ Added io_state field |
| 9 | Flush drains txbuf | Flush is no-op | Remove flush code |
| 10 | Fn 0Dh/0Eh not implemented | Fn 0Dh/0Eh call kbhit/getch | Already fixed in Phase B round 1 |
