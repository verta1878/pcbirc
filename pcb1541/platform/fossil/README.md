# FOSSIL Drivers — Usage Guide

## Overview

netmodem2irc provides FOSSIL drivers for 4 platforms. Each driver
bridges TCP/IP (Telnet callers) to the BBS software's serial I/O API.

```
Caller (Telnet)
  → NMServer (TCP listener)
    → FOSSIL driver (platform-specific)
      → BBS software (PCBoard, Renegade, Mystic, etc.)
```

## Naming

| Platform | Driver | Binary | Status |
|----------|--------|--------|--------|
| DOS | netfosdl | NETFOSDL.EXE | ✅ complete |
| OS/2 | netfosol | NETFOSOL.SYS | ✅ complete |
| Linux | netfosll | async_linux.o | ✅ complete |
| Windows | netfoswl | NETFOSWL.EXE | ✅ complete |

## DOS — netfosdl

TSR (Terminate and Stay Resident) FOSSIL driver. Intercepts INT 14h
and provides FSC-0015 Rev 5 compliant serial I/O to DOS BBS software.

### Files
```
fossil/dos/
├── netfosdl.pas      TSR loader
├── fossil.pas        INT 14h dispatch (27 functions)
├── serial.pas        16550 UART Port[] I/O
└── serial_irq.pas    ISR + 4KB ring buffer
```

### Usage
```
LH NETFOSDL /P:1 /B:115200
PCBOARD.EXE /N:1
```

### In DOSBox
```ini
[autoexec]
mount C C:\PCB
C:
LH NETFOSDL /P:1 /B:115200
PCBOARD.EXE /N:1
```

## Linux — netfosll (async_linux.c)

Socket-based ASYNC layer. Replaces FOSSIL INT 14h with POSIX
socket operations. All 23 ASYNC functions implemented.

### Files
```
fossil/linux/
└── async_linux.c     23 ASYNC functions (317 lines)
```

### Build (OpenWatcom)
```
wcc386 async_linux.c -bt=linux -mf -5 -ox
```

### Three Init Modes

The driver reads environment variables to determine how to connect:

**Mode 1: File descriptor pass (recommended)**
```bash
# netmodem2irc passes the connected socket fd
export PCBFD=5
./pcboard
```
netmodem2irc accepts the Telnet caller, creates a socketpair,
passes one fd to the BBS via PCBFD. Simplest, no port conflicts.

**Mode 2: Accept on port**
```bash
# BBS listens, netmodem2irc connects
export PCBPORT=2323
./pcboard
# Then: netmodem2irc connects to localhost:2323
```

**Mode 3: Connect to port**
```bash
# netmodem2irc listens, BBS connects
export NMPORT=2323
./pcboard
# netmodem2irc is already listening on localhost:2323
```

### CPU Usage
All reads use `select()` with 10ms timeout. No busy-polling.
The process yields CPU when no data is available.

### Carrier Detect
Socket EOF maps to carrier loss. The `CDokay` global goes to 0
when the socket closes. `shutdown(SHUT_WR)` simulates DTR drop
(modem hangup).

## OS/2 — netfosol (planned)

Native OS/2 FOSSIL driver using DosDevIOCtl for serial I/O
and so32dll/tcp32dll for sockets.

See: `docs/OS2_PORT_PLAN.md` for the 10-phase port plan.
See: evga's SIO2K rebuild (separate package, GPLv3) for the
OS/2 PDD/VDD/FOSSIL driver stack.

## Windows — netfoswl (planned)

Windows FOSSIL driver for DOS apps running under NTVDM or DOSBox.
Replaces NetFoss (PCMicro).

## Common Code

```
fossil/common/
├── m_fossil_socket.pas    socket FOSSIL backend (sysop/0, 189 lines)
└── serial_ext.pas         6 extended serial functions (wrench)
```

### m_fossil_socket.pas

Cross-platform socket backend used by all FOSSIL drivers.
Three init modes matching async_linux.c:
- `InitFromFD(fd)` — use existing file descriptor
- `InitAccept(port)` — listen and accept one connection
- `InitConnect(port)` — connect to existing listener

### serial_ext.pas

Extended serial functions not in FPC's RTL Serial unit:
- `SerGetDCD` — carrier detect (ioctl TIOCMGET on Unix)
- `SerDataAvailable` — non-blocking input check (fpSelect)
- `SerDetectUART` — chip identification (DOS only)
- `SerSetFIFO` — 16550 trigger level (DOS only)
- `SerEnableIRQ` / `SerDisableIRQ` — IRQ control (DOS only)

## FSC-0015 Conformance

All FOSSIL drivers implement FSC-0015 Rev 5 (27 functions).
The D4 conformance test suite (`tests/test_d4_fossil.pas`)
validates all functions. 37 tests, 0 failures is the bar.

Reference: `docs/FOSSIL_FSC0015_Reference.txt`

## Credits

| Who | What |
|-----|------|
| sysop/0 | serial.pas (UART), m_fossil_socket.pas (socket backend) |
| kiddo | serial_irq.pas (ISR + ring buffer) |
| wrench | serial_ext.pas, async_linux.c, D4 test suite |
| evga | SIO2K OS/2 driver rebuild (13,371 lines, GPLv3) |
| hexadecimal | LINUX_SERIAL_PLAN.md (23 ASYNC function mapping) |
