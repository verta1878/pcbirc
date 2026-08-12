# PCBDraw — ANSI Art Editor with Teleconference

## What Is PCBDraw?

PCBDraw combines CIADraw (local ANSI editor) with PabloDraw's
TCP teleconference system. Multiple users draw on the same canvas
simultaneously over a network — like Google Docs for ANSI art.

Builds with both Turbo Pascal 7 (DOS) and FPC 2.6.4irc
(DOS/Linux/Win32/FreeBSD).

## Source Base

| From | Lines | What We Take |
|------|-------|-------------|
| CIADraw | 2,572 | Drawing engine, VGA font editor, palette, mouse, file I/O |
| PabloDraw (Pascal port) | 4,226 | TCP client/server, wire protocol, format parsers, SAUCE |

Combined: ~6,800 lines → PCBDraw

## Architecture

```
                    ┌─────────────────────────┐
                    │     PCBDraw Server      │
                    │  (pcbdserv / pcbdraw -s)│
                    │                         │
                    │  Canvas state (shared)  │
                    │  Client list            │
                    │  Broadcast engine       │
                    └──────┬──────┬───────────┘
                           │      │
              TCP/loopback │      │ TCP/internet
                           │      │
                    ┌──────┴──┐ ┌─┴──────────┐
                    │ Client 1│ │  Client 2   │
                    │ (local) │ │ (remote)    │
                    │         │ │             │
                    │ CIADraw │ │ CIADraw     │
                    │ engine  │ │ engine      │
                    └─────────┘ └─────────────┘
```

## Phase 1: Rename + Refactor (no new features)

**Goal:** CIADraw → PCBDraw rename, clean module boundaries.

- [ ] Copy CIADraw source to `examples/pcbdraw/`
- [ ] Rename CIADRAW.PAS → PCBDRAW.PAS
- [ ] Rename local `SwitchVideoMode` back to `ChangeVideoMode`
- [ ] Extract drawing primitives from CIADRAW.PAS into PCBDRAW_GFX.PAS
- [ ] Extract keyboard handler into PCBDRAW_INPUT.PAS
- [ ] Extract file I/O (load/save) into PCBDRAW_FILE.PAS
- [ ] Verify: compiles with TP7 AND FPC, same binary behavior

Deliverable: PCBDraw compiles, identical to CIADraw, modular source.

## Phase 2: Format Support

**Goal:** Load/save ANSI, PCBoard, Avatar, ASCII, SAUCE.

- [ ] Port pdansi.pas → pcbd_ansi.pas (ANSI parser/writer, 383 lines)
- [ ] Port pdpcboard.pas → pcbd_pcb.pas (PCBoard @X codes, 70 lines)
- [ ] Port pdascii.pas → pcbd_ascii.pas (plain text, 57 lines)
- [ ] Port pdavatar.pas → pcbd_avatar.pas (Avatar codes, 84 lines)
- [ ] Port pdsauce.pas → pcbd_sauce.pas (SAUCE metadata, 331 lines)
- [ ] Port pdbinary.pas → pcbd_bin.pas (Binary/.BIN format, 60 lines)
- [ ] Add format selector to File menu: Save As ANSI/PCB/ASCII/BIN
- [ ] SAUCE auto-append on save (author, group, date, dimensions)

Deliverable: PCBDraw opens and saves all major BBS art formats.

## Phase 3: TCP Networking

**Goal:** Client/server teleconference drawing.

- [ ] Port pdnet.pas → pcbd_net.pas (wire protocol, 907 lines)
  - Binary packet format: opcode + length + payload
  - Opcodes: DRAW, CURSOR, COLOR, CLEAR, CHAT, JOIN, LEAVE
  - Delta compression: only send changed cells
- [ ] Port pdserver.pas → pcbd_serv.pas (server, 177 lines)
  - Accept connections, maintain client list
  - Broadcast draw ops to all clients
  - Canvas state sync on join (full screen dump)
- [ ] Port pdclient.pas → pcbd_client.pas (client, 234 lines)
  - Connect to server, send local draw ops
  - Receive remote draw ops, apply to local canvas
  - Cursor ghost: show where other users are drawing

Deliverable: `pcbdraw -s 9000` starts server,
`pcbdraw -c host:9000` joins session.

## Phase 4: RIP Graphics Mode

**Goal:** Add RIPscrip rendering alongside ANSI editing.

- [ ] Port pdrip.pas → pcbd_rip.pas (RIP parser, 330 lines)
- [ ] Integrate ripview engine for RIP preview
- [ ] Toggle: ANSI mode (text) ↔ RIP mode (640x350 graphics)
- [ ] RIP commands in teleconference: broadcast draw ops as RIP

Deliverable: PCBDraw renders and edits RIP art.

## Phase 5: BBS Integration

**Goal:** Run PCBDraw as a door on Mystic/PCBoard.

- [ ] Add FOSSIL/socket I/O mode (m_fossil_socket backend)
- [ ] Door mode: `pcbdraw -door -fossil` reads/writes via FOSSIL
- [ ] Drop file support: DOOR.SYS, CHAIN.TXT, DORINFOx.DEF
- [ ] MIS integration: sysop launches door, caller edits ANSI art
- [ ] Teleconference via BBS: multiple callers draw simultaneously

Deliverable: Caller telnets into BBS, runs PCBDraw door,
draws ANSI art, saves to file base.

## File Map (target)

```
pcbdraw/
  PCBDRAW.PAS        — main program (from CIADRAW.PAS)
  PCBDRAW_GFX.PAS    — drawing engine (extracted from CIADRAW)
  PCBDRAW_INPUT.PAS  — keyboard/mouse handler
  PCBDRAW_FILE.PAS   — file I/O, format detection
  pcbd_ansi.pas      — ANSI format (from pdansi.pas)
  pcbd_pcb.pas       — PCBoard @X format
  pcbd_ascii.pas     — ASCII format
  pcbd_avatar.pas    — Avatar format
  pcbd_sauce.pas     — SAUCE metadata
  pcbd_bin.pas       — Binary format
  pcbd_rip.pas       — RIPscrip format
  pcbd_net.pas       — TCP wire protocol
  pcbd_serv.pas      — Server
  pcbd_client.pas    — Client
  EXTENSE.PAS        — Extended Pascal utilities
  MOUSE.PAS          — Mouse driver
  PALLETTE.PAS       — VGA palette
  FONTUNIT.PAS       — VGA font I/O
  FONTEDIT.PAS       — Font editor
  LOAD.PAS           — Buffer loader
  EXEC.PAS           — Process exec
  RUNTIME.PAS        — Error handler
  FILELST.PAS        — File selector
  PCBDRAW-PHASES.md  — this document
```

## Build

```bash
# DOS (Turbo Pascal 7):
tpc PCBDRAW.PAS

# DOS (FPC go32v2):
ppc386 -Tgo32v2 PCBDRAW.PAS

# Linux:
ppc386 PCBDRAW.PAS

# Win32:
ppc386 -Twin32 PCBDRAW.PAS
```

## Credits

- CiA / Strider — original CIADraw (1994-96)
- Curtis Wensley — PabloDraw (C# original)
- sysop/0 — CIADraw FPC port, PabloDraw Pascal port, PCBDraw integration
- hexadecimal — CIADraw preservation
- The Crew: verta1878, evga, kiddo, wrench

## License

GPLv3
