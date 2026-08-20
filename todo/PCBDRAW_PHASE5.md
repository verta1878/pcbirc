# Phase 5: PCBDraw — ANSI Art Editor with Teleconference

## What Is PCBDraw?

PCBDraw combines CIADraw (local ANSI editor) with PabloDraw's TCP
teleconference. Multiple users draw on the same canvas simultaneously
over a network — like Google Docs for ANSI art.

Ships as a standalone tool on the 15.4 main branch. Uses PCBoard's
own UI libraries (screen.h, getkey, gotoxy, box, window, menu) for
all display and input.

## Source Lineage

| From | Lines | What We Take |
|------|-------|-------------|
| CIADraw (CIA art group) | 2,572 | Drawing engine, font editor, palette, mouse, file I/O |
| PabloDraw Pascal (sysop/0) | 4,226 | TCP client/server, wire protocol, format parsers, SAUCE |
| pcbdraw C port (pcbirc) | 1,826 | OpenWatcom C port of parsers + networking |
| PCBoard LIB (Clark) | ~3,000 | Screen I/O, keyboard, windows, menus, scrolling, cursor |

## Current State

```
tools/pcbdraw/
  pcbdraw.h      209 lines   Core types, canvas, net protocol, prototypes
  pdcanvas.c     175 lines   Canvas, palette, SAUCE reader
  pdansi.c       464 lines   ANSI X3.64 parser + PCBoard @X + Binary
  pdnet.c        724 lines   TCP server + client (wire protocol, 11 commands)
  pcbdraw.c      254 lines   Main EXE: view, sauce, server, client
                ─────────
  Total         1,826 lines

  ciadraw_ref/                CIADraw Pascal source (reference)

Binaries:
  PCBDRAW.EXE      32KB   OS/2
  PCBDRAW_W.EXE    47KB   Windows NT
```

---

## Sub-phase 5a: Editor Core (CIADraw port using PCBoard UI)

New file: `pdeditor.c` (~1,050 lines estimated)

### CIADraw → PCBoard LIB mapping

| CIADraw (Pascal) | Becomes (C + PCB LIB) |
|-------------------|----------------------|
| GotoXY(X, Y) | gotoxy(x, y) |
| TextAttr := Color | setatt(color) |
| Write(Ch) | print(ch) |
| ClrScr | cls() |
| ReadKey | getkey() / bgetkey() |
| Window(x1,y1,x2,y2) | window() + saverest() |
| Buff^[offset] := byte | canvas_set(x, y, element) |
| Direct VGA B800h writes | canvas_set() → print() redraw |
| Inline ASM (Set8X16) | setfont() |

### Checklist

- [ ] Canvas ↔ screen renderer (canvas_get → gotoxy+setatt+print) (~150 lines)
- [ ] Character drawing (type char at cursor) (~50 lines)
- [ ] Cursor movement (arrows, home, end, pgup/pgdn) (~80 lines)
- [ ] Color selection (foreground/background picker using box()) (~100 lines)
- [ ] Function key sets (10 sets of F1-F10 characters) (~60 lines)
- [ ] Block operations (copy, move, fill) (~150 lines)
- [ ] Insert/delete row and column (~60 lines)
- [ ] Undo (single level — save/restore canvas snapshot) (~40 lines)
- [ ] File save (ANSI with escape sequences) (~100 lines)
- [ ] Status bar (position, color, filename) (~40 lines)
- [ ] File browser (lightbar using menu()) (~80 lines)
- [ ] Save As: ANSI or PCBoard @X only (~40 lines)
- [ ] Load: ANSI + PCBoard @X codes (~30 lines, wire to existing parsers)
- [ ] SAUCE auto-append on save (~30 lines)
- [ ] Compiles and links against PCBoard screen/scrnio libs

---

## Sub-phase 5b: Network Integration

### Checklist

- [ ] Connect editor to PDClient on startup (--client flag) (~50 lines)
- [ ] Send CMD_UPDATE on every canvas change (~30 lines)
- [ ] Receive CMD_UPDATE, redraw affected region (~40 lines)
- [ ] Chat overlay (bottom 2 lines, using box()) (~60 lines)
- [ ] Remote cursor ghosts (show where others draw) (~40 lines)
- [ ] User list popup (using box() + menu()) (~30 lines)

### Wire Protocol (11 commands)

| Command | ID | Direction | Purpose |
|---------|----|-----------|---------|-
| CMD_CHAT | 01 | C→S→C | Chat message |
| CMD_UPDATE | 02 | C→S→C | Canvas region changed |
| CMD_LOADDOC | 03 | S→C | Full canvas sync on join |
| CMD_USERLIST | 04 | S→C | Who's connected |
| CMD_USERSTATUS | 05 | S→C | Access level change |
| CMD_CURSOR | 06 | C→S→C | Cursor position broadcast |
| CMD_SETATTR | 07 | C→S→C | Current draw attribute |
| CMD_KICK | 08 | S→C | Remove user |
| CMD_AUTH | 09 | C→S | Join with alias + password |
| CMD_WELCOME | 0A | S→C | Session accepted, canvas dims |
| CMD_BYE | 0B | either | Disconnect |

Access levels: Viewer (watch only) → Editor (draw) → Operator (kick/promote)

---

## Sub-phase 5c: Serial/FOSSIL/Modem Transport

New file: `pdserial.c` (~300 lines estimated)

### Checklist

- [ ] Abstract transport layer (send/recv function pointers) (~60 lines)
- [ ] FOSSIL INT 14h interface (DOS) (~80 lines)
- [ ] SIO interface (OS/2) (~60 lines)
- [ ] Modem control: ATZ, ATDT (dial out), ATA (answer) (~60 lines)
- [ ] Carrier detect (DCD monitoring) (~20 lines)
- [ ] Ring detect (RI monitoring for auto-answer) (~20 lines)
- [ ] --serial COM1 CLI flag (~20 lines)
- [ ] DigiBoard/Cyclades multi-port card support (memory-mapped I/O) (~80 lines)

### How dialup works

**Direct serial (modem-to-modem):**
```
Machine A:  pcbdraw server --serial COM1
Machine B:  pcbdraw client --serial COM1

COM1 ←── modem ←── phone line ──→ modem ──→ COM1
```
Same [LEN:4][CMD:1][DATA] framing over serial. At 19200 baud:
single char update = ~4ms, full canvas sync = ~3 seconds.

**Netmodem2irc bridge (wrench's FOSSIL stack):**
```
DOS machine              Linux server
  pcbdraw client           pcbdraw server
      │                         │
  FOSSIL ──→ netmodem2irc ──→ TCP:3693
```
Zero code changes in pcbdraw.

---

## Sub-phase 5d: Door Mode (BBS Integration)

New file: `pddoor.c` (~220 lines estimated)

### Checklist

- [ ] DOOR.SYS / DORINFOx.DEF reader (~80 lines)
- [ ] Remote I/O via FOSSIL (read/write to caller) (~60 lines)
- [ ] Idle timeout / carrier detect (~30 lines)
- [ ] --door CLI flag (~20 lines)
- [ ] Multi-node: connect door to pcbdraw server (~30 lines)

### How BBS teleconference works

```
Caller A ──telnet──→ PCBoard node 1 ──→ pcbdraw --door ──→┐
                                                           │ pcbdraw
Caller B ──telnet──→ PCBoard node 2 ──→ pcbdraw --door ──→┤ server
                                                           │ (shared
Caller C ──telnet──→ PCBoard node 3 ──→ pcbdraw --door ──→┘  canvas)
```

---

## Sub-phase 5e: ANSI Animation

DuNoDraw-style frame sequencing and playback.

New file: `pdanim.c` (~400 lines estimated)

### Checklist

- [ ] Frame buffer (array of canvas snapshots) (~60 lines)
- [ ] Add frame / delete frame / reorder frames (~40 lines)
- [ ] Frame timing (per-frame delay in ms, default 100ms) (~20 lines)
- [ ] Playback engine (render frames in sequence with timing) (~80 lines)
- [ ] Playback controls (play, pause, stop, step fwd/back, loop) (~60 lines)
- [ ] Onion skin (ghost of previous frame while editing) (~40 lines)
- [ ] Export: ANSI animation file (frames with cursor positioning + delays) (~60 lines)
- [ ] Export: animated GIF (render canvas to pixel buffer → GIF frames) (~80 lines)
- [ ] Timeline bar (frame strip at bottom of screen using box()) (~40 lines)
- [ ] Network: broadcast frame changes to teleconference clients (~30 lines)

---

## Sub-phase 5f: PCBoard Integration

Reuse PCBoard's teleconference/node management code.

### Checklist

- [ ] Who's online display (read PCBOARD.SYS node records) (~60 lines)
- [ ] Last callers list (~40 lines)
- [ ] Activity log (writes to CALLERS log) (~30 lines)
- [ ] Node status (busy/available/drawing) (~30 lines)
- [ ] User auth from PCBoard USERS file (15.4: separate pcbdraw user) (~60 lines)
- [ ] 15.41: tie saved ANSIs into PCBoard file area for download (~80 lines)
- [ ] 15.41: pcbdraw access levels integrated with PCBoard security (~40 lines)

### User Auth

**15.4:** Standalone user — pcbdraw has its own user/password
(configured in pcbdraw.cfg). No PCBoard user file dependency.

**15.41:** User comes from PCBoard's USERS file. Caller is already
authenticated by PCBoard before entering the door. Access level
maps from PCBoard security level.

### File Save

**15.4:** Saves ANSI/.PCB files to a configured directory.
Load and save ANSI or PCBoard @X codes only.

**15.41:** Saved ANSIs go into a PCBoard file area (configurable
conference/directory). Appears in file listings for download.
FILE_ID.DIZ auto-generated from SAUCE metadata.

---

## File Layout (target)

```
tools/pcbdraw/
  pcbdraw.h        Core types, canvas, net, editor prototypes
  pdcanvas.c       Canvas, palette, SAUCE                     ✅ done
  pdansi.c         ANSI parser + PCBoard @X + Binary          ✅ done
  pdnet.c          TCP server + client (11 commands)          ✅ done
  pdeditor.c       Editor engine (CIADraw port, PCB UI)       5a
  pdanim.c         ANSI animation (DuNoDraw-style)            5e
  pdserial.c       Serial/FOSSIL/modem transport              5c
  pddoor.c         Door mode (BBS integration)                5d
  pcbdraw.c        Main EXE entry point                      ✅ done
  ciadraw_ref/     CIADraw Pascal source (reference)

Reference (1541/pcbdraw/):
  pablodraw/       sysop/0's Pascal port (20 units, 4,226 lines)
  ciadraw/         Original CIADraw source (2,572 lines)
```

---

## Summary: All Sub-phases

| Phase | What | New File | Est. Lines | Status |
|-------|------|----------|-----------|--------|
| 5a | Editor core (CIADraw port, PCB UI) | pdeditor.c | ~1,050 | TODO |
| 5b | Network integration (teleconference) | (in pdeditor.c) | ~250 | TODO |
| 5c | Serial/FOSSIL/modem + DigiBoard | pdserial.c | ~300 | TODO |
| 5d | Door mode (BBS integration) | pddoor.c | ~220 | TODO |
| 5e | ANSI animation (DuNoDraw-style) | pdanim.c | ~400 | TODO |
| 5f | PCBoard integration (nodes, users, files) | (in pcbdraw.c) | ~340 | TODO |
| — | Parsers + networking (done) | pdcanvas/pdansi/pdnet/pcbdraw | 1,826 | ✅ DONE |
| | | **Total projected** | **~4,386** | |

---

## Comparison

| Editor | Transport | Multi-user | Animation | UI toolkit |
|--------|-----------|-----------|-----------|------------|
| TheDraw | None | No | No | Custom |
| CIADraw | None | No | No | Crt/VGA |
| DuNoDraw | None | No | **Yes** | Custom |
| PabloDraw C# | TCP | Yes | No | .NET WinForms |
| PabloDraw FPC | TCP | Yes | No | Free Vision |
| ACiDDraw | None | No | No | Custom |
| **PCBDraw** | **TCP + Serial + Modem** | **Yes** | **Yes** | **PCBoard LIB** |

PCBDraw is unique:
1. Uses PCBoard's own UI libraries — looks and feels like PCBoard
2. TCP and serial/modem transport — draw together over a phone call
3. ANSI animation with teleconference broadcast
4. BBS door mode with PCBoard node integration
5. Loads @X and ANSI, saves ANSI or @X only

---

## Credits

- CiA / Strider — original CIADraw (1994-96)
- Curtis Wensley — PabloDraw (C# original, MIT)
- sysop/0 — CIADraw FPC port, PabloDraw Pascal port, PCBDraw integration plan
- hexadecimal — CIADraw preservation, project lead
- wrench — netmodem2irc FOSSIL stack
- evga — SIO driver, RIPView
- kiddo — marc-lib, PabloDraw crash finder

## License

GPLv3 (our code). CIADraw: public domain. PabloDraw parsers: MIT origin.
PCBoard LIB: proprietary (Clark, licensed).
