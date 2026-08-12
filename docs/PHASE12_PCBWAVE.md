# Phase 12: Offline Mail — pcbwave

## Overview

pcbwave replaces PWAVE110 (PCBWAVE by Blue Moose Software) as
PCBoard's offline mail door. Supports all major offline mail
packet formats. Ships on both 15.4 and 15.41.

Standalone tool — sysop configures as a door, callers use their
preferred offline mail reader (Blue Wave, OLX, MultiMail, etc.)
to read/reply to messages offline.

## Reference Sources

| Source | License | What we use |
|--------|---------|-------------|
| PWAVE110.ZIP | Shareware (docs only) | Feature target — match all features |
| MultiMail (wmcbrine/MultiMail) | GPLv3 | Packet format reference (struct defs, parsing) |
| BAQWK10.ZIP | Freeware (PPS source) | PCBoard QWK integration patterns |
| QWKBLT12.ZIP | Freeware (PPS source) | QWK bulletin patterns |
| PCB-ATC5.ZIP | (ASM source) | QWK file attach handling |
| QM4_0604.ZIP | Shareware (binary) | QMail 4.00 feature reference |

MultiMail repo: https://github.com/wmcbrine/MultiMail
Key files for reference:
  mmail/qwk.h        QWK packet structures
  mmail/bluewave.h   Blue Wave packet structures
  mmail/soup.h        SOUP packet structures
  mmail/opx.h         OPX packet structures

## Packet Formats Supported

| Format | Read | Write | Notes |
|--------|------|-------|-------|
| QWK | [PLAN] | [PLAN] | 128-byte fixed records, most common |
| Blue Wave | [PLAN] | [PLAN] | Binary structs, file attaches |
| SOUP | [PLAN] | [PLAN] | Internet email/Usenet (RFC 822) |
| OPX | [PLAN] | [PLAN] | Offline Xpress format |
| OMEN | [NO] | [NO] | Rare, skip unless requested |

## PWAVE110 Features to Match

From PWAVE110 documentation:

### Core Mail Features
- [ ] Generate QWK mail packets from PCBoard conferences
- [ ] Generate Blue Wave mail packets from PCBoard conferences
- [ ] Import QWK reply packets into PCBoard message base
- [ ] Import Blue Wave reply packets into PCBoard message base
- [ ] Conference selection (caller picks which conferences to pack)
- [ ] New messages only (since last download)
- [ ] All messages option (full conference dump)
- [ ] Personal mail filtering (messages to/from caller)
- [ ] Message threading preserved in packets
- [ ] Long subject lines (QWK extended)
- [ ] Carbon copy support

### File Attaches
- [ ] QWK file attach import (port PCB-ATC5 CHANGE.ASM to C)
- [ ] Blue Wave file attach support
- [ ] Attach size limits (configurable)

### FidoNet Integration
- [ ] FidoNet echomail in QWK packets
- [ ] FidoNet netmail in QWK packets
- [ ] FidoNet<->UUCP gateway (PWAVE110's signature feature)
- [ ] Kludge line handling (MSGID, REPLY, PATH, SEEN-BY)

### Internet Integration
- [ ] SOUP packet generation (for internet mail/news)
- [ ] RFC 822 headers in SOUP packets
- [ ] Internet email via SOUP (import/export)
- [ ] Usenet articles via SOUP (import/export)

### Door Interface
- [ ] PCBoard door mode (DOOR.SYS)
- [ ] Menu-driven interface (ANSI + ASCII)
- [ ] Caller selects conferences
- [ ] Download packet via transfer protocol (Zmodem, etc.)
- [ ] Upload reply packet via transfer protocol
- [ ] Configurable welcome/goodbye screens
- [ ] Sysop configuration utility
- [ ] Per-user settings (stored in user record)

### Sysop Features
- [ ] Conference mapping (PCBoard conf# <-> QWK conf#)
- [ ] Max messages per conference limit
- [ ] Max packet size limit
- [ ] Message age limit
- [ ] Restricted conferences (security level check)
- [ ] Log file (downloads, uploads, errors)

### OPX Support (added)
- [ ] Generate OPX packets
- [ ] Import OPX reply packets
- [ ] OPX conference mapping

## Architecture

```
Caller connects to BBS
         |
         v
  PCBoard runs pcbwave as door
         |
         v
  pcbwave shows menu:
    [D]ownload mail packet
    [U]pload reply packet
    [S]elect conferences
    [C]onfigure
    [Q]uit
         |
    ┌────┴────┐
    v         v
  PACK      IMPORT
    |         |
    v         v
  Read PCBoard    Read reply
  msg base        packet (.REP/.NEW)
    |              |
    v              v
  Generate        Write to PCBoard
  .QWK/.BW/.SOUP  msg base
    |
    v
  Offer download
  (Zmodem/etc)
```

## File Layout

```
tools/pcbwave/
  pcbwave.h       Core types, packet format structs
  pcbwave.c       Main door: menu, config, door I/O
  qwk.c           QWK pack/unpack (128-byte records)
  bluewave.c      Blue Wave pack/unpack (binary structs)
  soup.c          SOUP pack/unpack (RFC 822)
  opx.c           OPX pack/unpack
  pcbmsg.c        PCBoard message base reader/writer
  pcbconf.c       Conference mapping, selection
  transfer.c      File transfer (Zmodem hook)
```

## Build

```
wcc386 -bt=nt -5r -oxs pcbwave.c qwk.c bluewave.c soup.c opx.c pcbmsg.c pcbconf.c transfer.c
wlink system nt name PCBWAVE_W.EXE file pcbwave,qwk,bluewave,soup,opx,pcbmsg,pcbconf,transfer
```

## Priority

MEDIUM — offline mail is a convenience feature, not critical path.
But it enables the SOUP format which bridges to internet email/Usenet,
tying into Phase 11 (UUEMAIL/NNTP).

## Credits

- Blue Moose Software — original PWAVE110 (feature reference)
- William McBrine — MultiMail (GPL, packet format reference)
- Cutting Edge Computing — Blue Wave struct definitions
- pcbirc crew — clean-room C implementation
