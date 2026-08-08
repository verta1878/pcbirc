# PCBoard Installation System (pcbis)

Menu-driven configuration and management for PCBoard 15.4 BBS.
Full-screen ANSI TUI inspired by Mystic BBS MCFG.

## Quick Start

```bash
pcbis_initv              # First-time directory setup
pcbis -cfg               # Configure via TUI (or pcbis_ui directly)
pcbis_startup            # Start BBS + netmodem2irc
pcbis_shutdown           # Graceful stop
```

## Configuration Screens

| Key | Screen | Fields |
|-----|--------|--------|
| G | General Settings | BBS name, sysop name, telnet port, node count |
| P | Paths & Directories | PCBoard root, data dir, FOSSIL driver, netmodem path |
| W | Web Server | Enabled, HTTP port, web root, page title, ANSI preview, file browser |
| T | FTP Server | Enabled, FTP port, file root, anonymous login, max connections, banner |
| F | FidoNet | FTN address, hub, binkp port, inbound/outbound/nodelist paths |
| D | DOSBox Settings | CPU cycles, video output mode |

## Navigation

- **Arrow keys** — move highlight bar
- **Enter** — edit the selected field inline
- **Letter key** — jump to item by hotkey (G, P, W, T, F, D, S, I, Q)
- **Backspace** — delete character while editing a field
- **Escape / Q** — return to main menu or quit
- **S** — save configuration to pcbis.cfg
- **I** — run first-time setup (pcbis_initv)

## Config File

All settings saved to `pcbis.cfg` in key=value format:

```ini
# BBS Settings
bbs_name=PCBoard BBS
sysop_name=SYSOP
telnet_port=23
nodes=1

# Paths
pcb_root=/home/pcboard
pcb_data=/home/pcboard/data
fossil_driver=ADF
netmodem_path=/home/pcboard/netmodem

# DOSBox
dosbox_cycles=0
dosbox_output=surface

# FidoNet
fido_enabled=0
fido_address=1:1/1.0
fido_hub=
fido_binkp_port=24554
fido_inbound=/home/pcboard/fido/inbound
fido_outbound=/home/pcboard/fido/outbound
fido_nodelist=/home/pcboard/fido/nodelist

# Web Server
web_enabled=0
web_port=8080
web_root=/home/pcboard/DATA/default/www
web_title=PCBoard BBS
web_ansi_preview=1
web_file_browser=1

# FTP Server
ftp_enabled=0
ftp_port=21
ftp_root=/home/pcboard/files
ftp_anonymous=0
ftp_max_connections=10
ftp_banner=PCBoard 15.4 FTP Server
```

## Commands

| Command | Description |
|---------|-------------|
| `pcbis -cfg` | Launch TUI configuration |
| `pcbis_ui --help` | Print help to screen |
| `pcbis_initv` | First-time directory + binary setup |
| `pcbis_startup` | Start PCBoard + netmodem2irc + DOSBox |
| `pcbis_shutdown` | Stop everything (PID cleanup) |
| `pcbtic -t` | Toss inbound TIC files |
| `pcbtic -h file area` | Hatch a file into a file echo |

## Stack

```
Caller (telnet:23)
  → netmodem2irc (TCP → virtual COM)
    → FOSSIL driver (ADF/NetFoss/netfosdl)
      → DOSBox (serial1=nullmodem)
        → PCBOARD.EXE /N:1
          → PCBoard 15.4 BBS
```

## Files

```
installer/
├── README.md            This file
├── pcbis                Main launcher script
├── pcbis_ui.c           TUI source (C, gcc + Watcom)
├── pcbis_ui             Compiled TUI binary
├── pcbis_initv          First-time setup
├── pcbis_startup        Start all services
└── pcbis_shutdown       Stop all services

tools/
├── pcbtic.c             TIC file processor source
├── pcbtic               Compiled TIC processor
└── nlcomp.c             Nodelist compiler source
```

## Requirements

- Linux (Ubuntu/Debian), DOS, or OS/2
- DOSBox 0.74+ (for DOS mode)
- netmodem2irc (from verta1878's repo)
- PCBoard 15.4 binaries (from pcbrevival)
- Terminal with ANSI support (for TUI)
