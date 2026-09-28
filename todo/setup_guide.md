# PCBoard 15.4 + NetModem/32 — Setup Guide

## Overview

Run PCBoard 15.4 as a Telnet BBS using NetModem/32 as the
virtual modem. Callers connect with any Telnet client.
PCBoard thinks it's talking to a real modem.

```
Caller (Telnet)
  → NMServer.exe (TCP port 23)
    → FOSSIL driver (ADF or NetFoss)
      → PCBOARD.EXE (COM1, 115200 baud)
```

## Requirements

- PCBoard 15.4 (pcbrevival) — 12 DOS binaries
- NetModem/32 (netmodem2irc) — NMServer.exe
- DOSBox 0.74+ (to run PCBoard on modern OS)
- FOSSIL driver: ADF or X00 (loaded in DOSBox)
- Virtual COM port: com0com (Windows) or DOSBox serial

## Directory Structure

```
C:\PCB\                     ← PCBoard root
├── PCBOARD.EXE               main BBS binary
├── LOCAL.EXE                  local mode (testing)
├── PCBOARD.DAT                main configuration
├── PCBOARD.SYS                system configuration
├── HELP\                      help screens
├── MAIN\                      main data directory
│   ├── INDEX\                 message indexes
│   ├── CLR\                   color files
│   └── USERS                  user database
├── GEN\                       general files
│   └── WELCOME                welcome screen
├── CMD\                       command files
├── PCBTEXT\                   text/language files
└── WORK\                      temporary work directory

C:\FOSSIL\                  ← FOSSIL driver
├── ADF.COM                    ADF FOSSIL driver
└── (or X00.SYS, BNU.COM)

C:\NetModem\                ← NetModem/32
├── NMServer.exe               virtual modem server
└── NMConfig.exe               configuration
```

## Step-by-Step Setup

### 1. Install PCBoard

Extract pcbrevival binaries to `C:\PCB\`:
```
PCBOARD.EXE    PCBFILER.EXE   PCBLOGIN.EXE
PCBMONI.EXE    PCBNODE.EXE    PCBPACK.EXE
PCBQWK.EXE     PCBSETUP.EXE   PCBSM.EXE
PCBUUCP.EXE    PCBZAP.EXE     LOCAL.EXE
```

### 2. Configure PCBoard

Run PCBSETUP.EXE (in DOSBox) or edit PCBOARD.DAT:

Key settings:
```
Modem type:        FOSSIL
COM port:          COM1
Baud rate:         115200
Answer on ring:    YES
Init string:       ATZ
Answer string:     ATA
```

### 3. Set Up DOSBox

Create `pcboard.conf`:
```ini
[sdl]
output=surface
fullscreen=false

[cpu]
cycles=max

[serial]
# Option A: DOSBox virtual modem (built-in)
serial1=modem listenport:23

# Option B: Connect to NMServer via nullmodem
# serial1=nullmodem server:localhost port:2323

[autoexec]
mount C C:\PCB
C:
REM Load FOSSIL driver
LH C:\FOSSIL\ADF.COM COM1 3F8 4 115200 8192 8192 8
REM Start PCBoard
PCBOARD.EXE /N:1
```

### 4. Start NMServer

```
NMServer.exe
```

Or under Linux:
```bash
xvfb-run -a wine NMServer.exe
```

### 5. Start PCBoard in DOSBox

```
dosbox -conf pcboard.conf
```

Or under Linux:
```bash
xvfb-run -a dosbox -conf pcboard.conf
```

### 6. Test

From any machine:
```
telnet your-server-ip 23
```

You should see PCBoard's welcome screen and login prompt.

## Multinode Setup

PCBoard supports multiple simultaneous callers. Each node needs:
- Its own DOSBox instance
- Its own COM port (COM1, COM2, etc.)
- Its own FOSSIL driver instance
- Its own NMServer node

Example for 4 nodes:
```
Node 1: COM1, DOSBox instance 1, NMServer node 0
Node 2: COM2, DOSBox instance 2, NMServer node 1
Node 3: COM3, DOSBox instance 3, NMServer node 2
Node 4: COM4, DOSBox instance 4, NMServer node 3
```

Create separate DOSBox configs for each node:
```
dosbox -conf pcboard-node1.conf
dosbox -conf pcboard-node2.conf
dosbox -conf pcboard-node3.conf
dosbox -conf pcboard-node4.conf
```

## FOSSIL Driver Options

| Driver | Command | Notes |
|--------|---------|-------|
| ADF | `LH ADF COM1 3F8 4 115200 8192 8192 8` | Recommended for DOSBox |
| X00 | `X00 B,0,115200` | Classic, widely compatible |
| BNU | `BNU /L0:115200` | Universal FOSSIL |
| netfosdl | `NETFOSDL /P:1 /B:115200` | Our FOSSIL (netmodem2irc) |

## PCBoard + NetModem/32 Under Linux

Full stack on Linux:
```bash
# Terminal 1: Start NMServer
xvfb-run -a wine NMServer.exe

# Terminal 2: Start PCBoard in DOSBox
xvfb-run -a dosbox -conf pcboard.conf

# Terminal 3: Connect as a caller
telnet localhost 23
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| PCBoard says "No FOSSIL" | Load ADF/X00 before PCBOARD.EXE |
| "Board not found" | Check PCBOARD.DAT paths match your directory |
| Login screen garbled | Caller's terminal must support ANSI |
| Callers drop immediately | Check DOSBox serial config matches NMServer port |
| File transfers fail | Use Zmodem — works through the virtual modem |
| PCBoard exits immediately | Missing data files (USERS, PCBTEXT, etc.) |

## Credits

- Clark Development — original PCBoard (1983-1997)
- hexadecimal — pcbrevival (15.3→15.4 source port)
- verta1878 — netmodem2irc (virtual modem)
- The PCBoard community — 40+ years of BBS history
