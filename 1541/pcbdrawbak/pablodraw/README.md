# PabloDraw Pascal

Pure Pascal ANSI/RIP art toolkit. Viewer, editor, teleconference server.
Converted from PabloDraw's 414 C# files. No .NET. No encoding bugs.

## Build

```bash
# Linux
fpc -FUpd_out -FEpd_out src/pdmain.pas
fpc -FUpd_out -FEpd_out src/pdviewfv.pas
fpc -FUpd_out -FEpd_out src/pdserver.pas
fpc -FUpd_out -FEpd_out src/pdclient.pas
fpc -FUpd_out -FEpd_out src/pdtest.pas

# DOS (go32v2) — run build-dos.bat
```

## Binaries

| Binary | What |
|--------|------|
| `pdmain` | CLI: load any format, render to BMP, dump to terminal, show SAUCE |
| `pdviewfv` | Free Vision TUI: file browser, scroll, SAUCE info |
| `pdserver` | FV TUI teleconference server: host, kick, promote |
| `pdclient` | FV TUI teleconference client: connect, draw, chat |
| `pdtest` | Test suite: 33 tests, all pass |

## Usage

```
pdview file.ans              — ANSI color dump to terminal
pdview file.ans -bmp out.bmp — render to 24-bit BMP
pdview file.rip -bmp out.bmp — RIPscrip → 640×350 BMP
pdview file.ans -sauce       — show SAUCE metadata
pdview file.xbin -info       — show format info
```

## Formats (9)

| Extension | Format | Unit |
|-----------|--------|------|
| .ans .ansi .diz .ice | ANSI escape sequences | pdansi |
| .txt .asc .nfo | Plain ASCII | pdascii |
| .bin | Binary char+attr pairs | pdbinary |
| .avt | AVATAR/0+ terminal codes | pdavatar |
| .xb .xbin | XBin (compressed + embedded font/palette) | pdxbin |
| .rip | RIPscrip v1.54 vector graphics | pdrip |
| .tnd | Tundra Draw 24-bit color | pdtundra |
| .msg | PCBoard Ctrl-A color codes | pdpcboard |
| .idf | iCE Draw Format | pdidf |

## Architecture

```
CLI/TUI                          Network
  pdmain.pas    (CLI viewer)       pdserver.pas  (FV TUI server)
  pdviewfv.pas  (FV TUI viewer)   pdclient.pas  (FV TUI client)
  pdtest.pas    (test suite)       pdnet.pas     (TCP protocol)
     │                                │
Format Parsers (no GUI)          Teleconference
  pdansi.pas    ANSI CSI/SGR       CMD_AUTH      join with alias
  pdansiw.pas   ANSI writer        CMD_LOADDOC   full canvas sync
  pdascii.pas   plain text         CMD_UPDATE    region update
  pdbinary.pas  binary pairs       CMD_CHAT      text message
  pdavatar.pas  AVATAR/0+          CMD_CURSOR    position broadcast
  pdxbin.pas    XBin               CMD_USERLIST  who's online
  pdrip.pas     RIPscrip           CMD_KICK      disconnect user
  pdtundra.pas  Tundra Draw
  pdpcboard.pas PCBoard Ctrl-A     Access: Viewer → Editor → Operator
  pdidf.pas     iCE Draw
     │
Core
  pdtypes.pas   canvas, cell, attribute, palette
  pdsauce.pas   SAUCE metadata reader/writer
  pdbitfont.pas bitmap font (253/256 CP437 glyphs) + BMP renderer
  cp437font.inc 8×16 VGA ROM font data (4096 bytes)
```

## DOS Teleconference

Two DOS machines with packet drivers. Server hosts canvas on TCP 3693.
Client connects, draws, chats. See `docs/DOS-TELECONF.md`.

```
pdserver.exe ← TCP:3693 → pdclient.exe
     │                          │
 sockets.pp (pure Pascal)   sockets.pp
     │                          │
 packet driver              packet driver
     │                          │
 NE2000 ──── Ethernet ──── NE2000
```

## PabloDraw RIP Crash Bug

Filed on cwensley/pablodraw. PabloDraw 3.3.14.0 crashes loading RIP
files with CP437 bytes ≥128. Root cause: `BinaryReader.PeekChar()`
defaults to UTF-8. One-line fix. See `docs/PD-RIP-CRASH.md`.

## Tests

```
$ pdtest
33 passed, 0 failed
ALL TESTS PASSED
```

## DOS Verified

Tested in DOSBox 0.74 with CWSDPMI:

```
C:\> PDTEST.EXE
33 passed, 0 failed
ALL TESTS PASSED

C:\> PDMAIN.EXE
PabloDraw Pascal Viewer v0.1
Usage: pdview <file> [options]
```

| Binary | Size | Platform |
|--------|------|----------|
| pdmain | 1.1MB | Linux i386 |
| pdmain.exe | 685KB | DOS (DJGPP) |
| pdviewfv | 1.4MB | Linux i386 |
| pdserver | 1.4MB | Linux i386 |
| pdclient | 1.4MB | Linux i386 |
| pdtest | 1.1MB | Linux i386 |
| pdtest.exe | 672KB | DOS (DJGPP) ✅ 33/33 |

## Stats

| | Count |
|---|---|
| Pascal source files | 20 |
| Lines of code | 4,460 |
| Formats supported | 9 |
| Tests | 33/33 pass |
| C# files replaced | 414 |
| .NET dependencies | 0 |
| Encoding crashes | 0 |

## License

Format parsers derived from PabloDraw (MIT) by Curtis Wensley.
Pascal conversion by FPC264IRC project (GPLv3).

## Credits

| Handle | Role |
|--------|------|
| verta1878 | Project lead |
| sysop/0 | Compiler engineer |
| evga | Display, RIPView |
| kiddo | marc-lib, PD crash finder |
| wrench | tork netmodem2irc, network arch |
