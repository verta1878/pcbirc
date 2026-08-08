# PCBTIC — TIC File Processor for PCBoard 15.4

## Overview

PCBTIC processes FidoNet .TIC files for file echo distribution with
PCBoard 15.4. PCBoard's built-in PCBTOSS handles echomail (messages)
but has no support for file echoes (TIC). PCBTIC fills this gap.

## What is TIC?

TIC (Tick) is the FidoNet standard for distributing files across the
network. When a file is "hatched" into a file echo area, a .TIC
descriptor file is created alongside it. The mailer (BinkleyTerm XE)
transfers both the file and its .TIC to connected nodes. The receiving
system's TIC processor reads the .TIC, moves the file to the correct
directory, and forwards both to any downlinks.

TIC format is defined in FTS-5006.001 (FTSC standard).

## Integration with PCBoard

```
BinkleyTerm XE (mailer)
  ↓ receives FILE.ZIP + FILE.TIC to inbound/
  ↓
PCBTIC -t (toss)
  ↓ reads .TIC, moves file to PCBoard file directory
  ↓ updates DIR listing (DIR001.LST etc.)
  ↓ forwards TIC+file to downlinks via outbound/
  ↓
PCBoard 15.4
  ↓ users see new file in file listings
  ↓ FILE_ID.DIZ extracted by PCBoard on upload
```

## Installation

1. Compile: `gcc -o pcbtic pcbtic.c -Wall -O2`
   Or Watcom: `wcc386 pcbtic.c -bt=dos -mf -5 -ox`

2. Create config file `pcbtic.cfg`:

```ini
# Global settings — match your pcbis.cfg FidoNet paths
address=1:234/56.0
inbound=/home/pcboard/fido/inbound
outbound=/home/pcboard/fido/outbound

# Map FidoNet file echo areas to PCBoard directories
[area]
name=BBS_UTILS
dir=/home/pcboard/dirs/bbsutil
dirlist=/home/pcboard/dirs/bbsutil.lst
passthrough=no
downlinks=1:2/3 1:2/4

[area]
name=DOORWARE
dir=/home/pcboard/dirs/doors
dirlist=/home/pcboard/dirs/doors.lst
downlinks=1:2/5

[area]
name=NODEDIFF
dir=/home/pcboard/fido/nodelist
dirlist=
passthrough=yes
downlinks=1:2/3
```

3. Add to PCBoard event schedule or cron:
```bash
# Run after each BinkleyTerm session:
pcbtic -t -c /home/pcboard/pcbtic.cfg

# Or via cron every 15 minutes:
*/15 * * * * /home/pcboard/pcbtic -t -c /home/pcboard/pcbtic.cfg -v >> /var/log/pcbtic.log
```

## Usage

### Toss inbound TIC files (default)
```
pcbtic -t [-c pcbtic.cfg] [-v]
```
Scans the inbound directory for .TIC files, processes each one:
- Finds the matching file echo area in pcbtic.cfg
- Copies the described file to the area's directory
- Appends an entry to the DIR listing file
- Forwards TIC + file to configured downlinks
- Removes processed .TIC and file from inbound

### Hatch a new file
```
pcbtic -h FILENAME.ZIP AREANAME [-d "Description"] [-c pcbtic.cfg]
```
Creates a .TIC file and distributes a file to the network:
- Copies the file to the area's local directory
- Updates the DIR listing
- Creates .TIC files in outbound for each downlink

### List configured areas
```
pcbtic -l [-c pcbtic.cfg]
```

## TIC File Format

A .TIC file is plain text with CR/LF line endings:

```
Area BBS_UTILS
Origin 1:234/56
From 1:234/56
To 1:2/3.0
File PCBREV10.ZIP
Size 458752
Date 1722816000
Desc PCBoard 15.4 Revival Kit v1.0
LDesc Complete source port of PCBoard 15.4 to OpenWatcom.
LDesc Includes installer, BinkleyTerm XE, and tools.
CRC A1B2C3D4
Path 1:234/56 1722816000 Mon Aug 05 00:00:00 2026
Seenby 1:234/56
Pw AREAPASSWORD
Replaces PCBREV09.ZIP
```

### Required fields
- **Area** — file echo area tag (must match pcbtic.cfg)
- **File** — filename being distributed

### Optional fields
- **Origin** — originating FTN address
- **From** — sending node address
- **To** — destination node address
- **Size** — file size in bytes
- **Date** — Unix timestamp
- **Desc** — short description (one line)
- **LDesc** — long description (multiple lines)
- **CRC** — CRC-32 of the file (hex)
- **Path** — routing path (one per hop)
- **Seenby** — nodes that have seen this TIC
- **Pw** — area password
- **Replaces** — filename this file supersedes

## PCBoard DIR Listing Format

PCBTIC appends entries in standard PCBoard format:

```
FILENAME.ZIP   458752  08-05-26  PCBoard 15.4 Revival Kit v1.0
```

Format: `%-12s %8ld  %s  %s` (name, size, date MM-DD-YY, description)

PCBoard reads these listings via CNAMES.@@@ conference configuration,
where each file directory has an associated .LST file.

## Passthrough Areas

Setting `passthrough=yes` in an area config means:
- Files are NOT stored locally
- Files are forwarded to all downlinks
- No DIR listing is updated
- Useful for transit nodes

## Relation to Other Tools

| Tool | Purpose |
|------|---------|
| PCBTOSS (built-in) | Echomail tosser — processes .PKT message packets |
| PCBFU.EXE | FidoNet utility — area configuration, maintenance |
| PCBTIC | File echo processor — handles .TIC distribution |
| BinkleyTerm XE | Mailer — transfers files and packets between nodes |
| nlcomp | Nodelist compiler — builds NODELIST.DBF/NDX |

## Source

`tools/pcbtic.c` — standalone C, compiles under gcc (Linux) and
OpenWatcom (DOS). Part of pcbrevival (GPL v3.0).
