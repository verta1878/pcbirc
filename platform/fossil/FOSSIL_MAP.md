# FOSSIL Driver Map — PCBoard Platform Support

## Architecture

```
Caller (telnet/modem)
  │
  ▼
netmodem2irc (transport — wrench)
  │
  ▼
FOSSIL Driver (platform-specific — wrench)
  │
  ▼
PCBoard (ASYNC_* API — 23 functions)
```

PCBoard calls 23 ASYNC_* functions for all serial I/O.
Each platform needs a FOSSIL driver that implements these
23 functions using the platform's native serial/socket API.

## The 23 ASYNC Functions

```
INIT / OPEN / CLOSE
  ASYNC_INIT(irq, addr, inbuf, outbuf, insize, outsize, cts, share)
  ASYNC_OPENCOM(baud, databits)
  ASYNC_CLOSECOM()

READ
  ASYNC_COMMINKEY()        → one byte, non-blocking (-1 if none)
  ASYNC_CGETBUF(buf, max)  → fill buffer, return count
  ASYNC_CGETSTR(buf, max)  → read until CR/LF, return count
  ASYNC_CHECKCOMM()        → 1 if data waiting, 0 if not

WRITE
  ASYNC_CSENDBYTE(ch)      → send one byte
  ASYNC_CSENDSTR(str, len) → send string

STATUS
  ASYNC_ONLINE()           → 1 if connected, 0 if not
  ASYNC_CDSTILLUP()        → quick carrier check

FLOW CONTROL
  ASYNC_COMMGO()           → resume transmission
  ASYNC_COMMSTOP()         → pause transmission
  ASYNC_COMMPAUSE()        → brief pause
  ASYNC_CLEARINBUF()       → flush input buffer
  ASYNC_CLEAROUTBUF()      → flush output buffer

MODEM LINES
  ASYNC_TURNONDTR()        → raise DTR (or no-op)
  ASYNC_TURNOFFDTR()       → drop DTR (hangup)
  ASYNC_TURNONRTS()        → raise RTS
  ASYNC_TURNOFFRTS()       → drop RTS
  ASYNC_TURNONFIFO(level)  → enable FIFO (no-op for sockets)
  ASYNC_TURNONXMIT()       → enable transmitter
  ASYNC_SETPORT(baud, db)  → configure port (no-op for sockets)
```

## Globals (all platforms must provide)

```c
char volatile CDokay;         /* 1=connected, 0=disconnected */
char volatile _CTSokay;       /* 1=clear to send (always 1 on sockets) */
char volatile _RingDetect;    /* 0 (no ring on sockets) */
char CDup;                    /* 1=carrier up */
char B8250, B16550, B16550A, B16650;  /* UART type (0 on sockets) */
int volatile _InBytes;        /* bytes received counter */
int volatile _OutBytes;       /* bytes sent counter */
int volatile _OverrunErrors;  /* 0 on sockets */
int volatile _ParityErrors;   /* 0 on sockets */
int volatile _FramingErrors;  /* 0 on sockets */
```

## Platform Map

### DOS — netfosdl ✅ DELIVERED
```
File: platform/fossil/dos/netfosdl.pas (323 lines)
Deps: fossil.pas (398), serial.pas (203), serial_irq.pas (195)
Lang: Free Pascal
API:  INT 14h FOSSIL calls
How:  netmodem2irc → netfosdl → FOSSIL INT 14h → PCBoard
      ASYNC.C reads/writes through INT 14h
Note: ADF recommended as FOSSIL driver under DOSBox
      (x00 hangs on modem reset, BNU stuck at 300bps)
CPU:  dos_idle() INT 2Fh/1680h between polls (hog fix)
```

### Linux — netfosll ✅ DELIVERED
```
File: platform/fossil/linux/async_linux.c (322 lines)
Lang: C (OpenWatcom wcc386 -bt=linux)
API:  socket read()/write()/select()
How:  netmodem2irc → Unix socket → async_linux.c → PCBoard
      No FOSSIL concept — direct socket fd
Conn: Option A (recommended): socketpair, fd via env PCBFD
      Option B: TCP loopback localhost:port
      Option C: pseudo-terminal (pty)
CPU:  select() with 10ms timeout (no busy-wait)
Hang: recv() returns 0 → CDokay = 0
```

### OS/2 — netfosol ✅ DELIVERED
```
File: platform/fossil/os2/netfosol.pas (447 lines)
Lang: Free Pascal or C
API:  DosRead/DosWrite on named pipe or socket
      OR wrap evga's SIO.SYS via DosDevIOCtl
How:  Option A: netmodem2irc → named pipe → netfosol → PCBoard
      Option B: netmodem2irc → TCP socket → netfosol → PCBoard  
      Option C: netmodem2irc → SIO.SYS → VX00.SYS → PCBoard
Deps: evga's SIO v1 or v2 (addons/SIO/)
Note: OS/2 PCBoard (PCBOARD2.EXE) can also use native COM
      ports through DosDevIOCtl — no FOSSIL needed for
      direct serial. FOSSIL only needed for telnet/virtual.
```

### Windows — netfoswl ✅ DELIVERED
```
File: platform/fossil/windows/netfoswl.pas (411 lines)
Lang: C (OpenWatcom wcc386 -bt=nt)
API:  Winsock2 recv()/send()/select()
      OR named pipe CreateFile/ReadFile/WriteFile
How:  netmodem2irc → TCP socket → async_windows.c → PCBoard
Note: Similar to Linux socket approach but with Winsock API
      WSAStartup, closesocket instead of close, etc.
```

### FreeBSD / Mac — future
```
Same as Linux (POSIX sockets). async_linux.c should work
with minor #ifdef adjustments for header paths.
```

## Common Socket Design (m_fossil_socket.pas)

wrench's m_fossil_socket.pas (189 lines) defines the shared
socket FOSSIL logic used by all POSIX platforms. The C port
(async_linux.c) implements the same 23 functions.

Key design decisions:
- select() with 10ms timeout for CPU-friendly polling
- CDokay set to 0 when recv() returns 0 (peer disconnect)
- TURNOFFDTR maps to shutdown(fd, SHUT_WR) (simulate hangup)
- All baud/FIFO/flow control functions are no-ops on sockets
- Byte counters (_InBytes, _OutBytes) maintained for stats

## Testing Plan

Each platform driver tested with:
1. Echo test — send bytes, verify they come back
2. Hangup test — disconnect client, verify CDokay → 0
3. Stress test — sustained transfer, check byte counters
4. CPU test — idle connection, verify <5% CPU usage
5. Multi-node — two simultaneous connections

## File Naming Convention

```
platform/fossil/
├── common/
│   ├── m_fossil_socket.pas   Shared socket design (Pascal)
│   └── serial_ext.pas        Serial extensions
├── dos/
│   ├── netfosdl.pas          DOS FOSSIL driver
│   ├── fossil.pas            FOSSIL INT 14h API
│   ├── serial.pas            Serial port access
│   └── serial_irq.pas        IRQ handler
├── linux/
│   ├── async_linux.c         Linux ASYNC (C, OpenWatcom)
│   └── README.md
├── os2/
│   └── README.md             (netfosol — wrench)
└── windows/
    └── README.md             (planned)
```

## Who Does What

| Platform | Driver | Person | Status |
|----------|--------|--------|--------|
| DOS | netfosdl | wrench | ✅ delivered |
| Linux | async_linux.c | wrench | ✅ delivered |
| OS/2 | netfosol | wrench | ✅ delivered |
| Windows | netfoswl | wrench | ✅ delivered |
| SIO.SYS | OS/2 PDD | evga | ✅ delivered |
| Common | m_fossil_socket.pas | wrench | ✅ delivered |
| ASYNC.C | DOS FOSSIL bridge | hexadecimal | ✅ done |
| LINUX_SERIAL_PLAN.md | spec | hexadecimal | ✅ done |
