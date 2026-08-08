# PCBoard 15.41 — Draft Changes

## Version Bump: 15.4 → 15.41

### Why
PCBoard 15.4 had no support for:
- FTP file uploads (files arriving via FTP server)
- TIC file echo distribution (FidoNet file echoes)
- Tracking where a file came from (source type)

Version 15.41 extends the file directory system to track file origin,
adds TIC processing support, and adds FTP server configuration.

### What Changes

## 1. File Source Type (new field)

### Location: DIR record extension

PCBoard stores file listings in plain text .LST files:
```
FILENAME.ZIP   458752  08-05-26  Description text here
```

**Problem:** No room for source type in this format without breaking
every existing file listing, PPE, and third-party tool.

**Solution:** Parallel index file. For each `DIRxxx` listing file,
create a companion `DIRxxx.SRC` file with source metadata:

```
DIRxxx        — existing file listing (unchanged)
DIRxxx.SRC    — source type index (new, optional)
```

### DIRxxx.SRC format (fixed-length records, one per file):
```c
#pragma pack(1)
typedef struct {
    char     filename[13];     /* 8.3 filename, null-padded */
    unsigned char source;      /* file source type */
    unsigned long timestamp;   /* when file was added */
    char     origin[40];       /* FTN address, FTP user, or sysop name */
    char     reserved[10];     /* future use */
} filesrcrecord;               /* 64 bytes per record */
#pragma pack()
```

### Source type values:
```c
/* filesourcetype — how a file entered the system */
typedef enum {
    FSRC_LOCAL    = 0,   /* local sysop upload or user upload */
    FSRC_UPLOAD   = 1,   /* remote user upload via modem/telnet */
    FSRC_FTP      = 2,   /* FTP server upload */
    FSRC_TIC      = 3,   /* FidoNet TIC file echo */
    FSRC_FREQ     = 4,   /* FidoNet file request */
    FSRC_IMPORT   = 5,   /* batch import / sysop tool */
} filesourcetype;
```

### Header file: FILESRC.H (new)
```c
#ifndef H_FILESRC
#define H_FILESRC

typedef enum {
    FSRC_LOCAL    = 0,
    FSRC_UPLOAD   = 1,
    FSRC_FTP      = 2,
    FSRC_TIC      = 3,
    FSRC_FREQ     = 4,
    FSRC_IMPORT   = 5,
} filesourcetype;

#pragma pack(1)
typedef struct {
    char           filename[13];
    unsigned char  source;
    unsigned long  timestamp;
    char           origin[40];
    char           reserved[10];
} filesrcrecord;
#pragma pack()

/* API */
int  filesrc_open(const char *dirpath);
int  filesrc_add(const char *dirpath, const char *filename,
                 filesourcetype src, const char *origin);
int  filesrc_find(const char *dirpath, const char *filename,
                  filesrcrecord *rec);
void filesrc_close(void);

#endif
```

## 2. Files That Change

### Headers
| File | Change |
|------|--------|
| FILESRC.H (new) | Source type enum + record struct + API |
| PCBOARD.H | Version string "15.41" |
| NEWDATA.H | Version constant PCB_VERSION 1541 |
| DEFINES.H | Add FSRC_* if not using FILESRC.H |

### Source
| File | Change |
|------|--------|
| pcbtic.c | Call filesrc_add() with FSRC_TIC after updating DIR listing |
| pcbis_ui.c | Version display "15.41" |
| INIT.C | Version check on PCBOARD.DAT (accept 15.4 and 15.41) |
| STRS15.C | Version string update |

### New source
| File | Description |
|------|-------------|
| FILESRC.C | filesrc_open/add/find/close implementation |
| FILESRC.H | Header (see above) |

### Configuration
| File | Change |
|------|--------|
| PCBOARD.DAT | Version field → 1541 |
| pcbis.cfg | Version display |
| PCBSETUP | File directory editor — show source type column (phase 2) |
| PCBSM | File listing display — filter by source (phase 3) |

## 3. Backward Compatibility

### What stays the same:
- DIR listing format (.LST files) — UNCHANGED
- User record format — UNCHANGED
- Message base format — UNCHANGED
- Conference configuration — UNCHANGED
- PCBOARD.DAT layout — UNCHANGED (except version field)
- All 2,757 PPEs continue to work
- All existing doors and utilities work

### What's new:
- DIRxxx.SRC companion files (created on demand, not required)
- FILESRC.H header and FILESRC.C implementation
- pcbtic writes .SRC entries automatically
- FTP server writes .SRC entries when enabled
- If .SRC file doesn't exist, all files show as FSRC_LOCAL

### Migration:
- Existing systems: .SRC files created automatically as new
  files arrive via TIC or FTP
- No conversion needed for existing file areas
- PCBOARD.DAT version bump is cosmetic — 15.4 data works as-is

## 4. Implementation Phases

### Phase 1: 15.4 Source Port ✅ COMPLETE
- 556/556 source files compile under OpenWatcom 2.0
- 13/13 Clark binaries linked (PCBOARD, LOCAL, PPLC, PCBSETUP,
  PCBSM, MKPCBTXT, MAKEHELP, MAKEIDX, USERNET, UUIN-UUXFER)
- Phase 3 ASM→C: 8,251 lines TASM → 309 lines C
- ASYNC.C FOSSIL driver with CPU hog fix (INT 2Fh/1680h)
- CNAMES.C atexit cdecl fix
- pcb.lib complete (275/275 files)
- MESSAGES.H, VAR.HPP, PCBOARD.H synced between H/ and H/H/
- Patches: 153_to_154, 154_borland_to_154_watcom

### Phase 2: 15.41 Core Features (in progress)
- FTP protocol ✅ (FILES.C, SETTINGS.C, TRANSFER.C, PCBOARD.H)
- BinkP mailer — pcbbinkp.c standalone executable (not started)
- FidoNet TIC — pcbtic.c ✅ compiles
- FREQ/magic — pcbfcfg.c ✅ compiles
- Nodelist compiler — nlcomp.c ✅ compiles
- pcbpscan file scanner ✅ compiles
- pcbis_ui installer TUI ✅ compiles, linked (48KB)
- Startup/shutdown scripts ✅ 3 platforms
- PCBTEXT strings: 751-770 (documented), 788 (in source)

### Phase 3: OS/2 Native
- PCBOARD2.EXE ✅ linked
- PCBCP Control Panel ✅ ported, compiled, linked (77KB)
- SIO v1 + v2 driver suite — placed, needs audit
- netfosol — wrench (in progress)

### Phase 4: Standalone Tools
- pcbbinkp.c — BinkP mailer executable
  - Protocol core (port from binkd/protocol.c)
  - CRAM-MD5 auth (port from binkd/crypt.c)
  - BSO outbound scanning
  - Tagged stdout for pcbis_ui status display
  - Called by PCBOARD.EXE on FIDOPOLL/ALT-F
- pcbis_ui FidoNet & Transfer operations console
  - Scrollback buffer with ↑↓ PgUp/PgDn Home/End
  - TAB filter: [B]inkP [T]IC [E]cho [F]REQ [X]fer/FTP [A]ll
  - Verbose debug toggle per component
  - Config screens: address, nodes, BinkP, TIC, echomail, FREQ
  - FTP transfer status: progress, complete, failed, debug
  - Live status + poll trigger
  - All tools write tagged lines: [BINKP] [TIC] [TOSS] [FREQ] [FTP]

### Phase 4a: FOSSIL Socket Layer ✅ DELIVERED (wrench)
- platform/fossil/common/m_fossil_socket.pas (189 lines)
- platform/fossil/linux/async_linux.c (322 lines, C, OpenWatcom/GCC)
- platform/fossil/dos/netfosdl.pas (323 lines) + serial/IRQ units
- platform/fossil/os2/ — netfosol (wrench, in progress)
- platform/fossil/windows/ — planned

### Phase 5: PCBDRAW / PabloDraw Integration
- Blocked on sysop/0's FPC port of cwensley/pablodraw
- Client/server ANSI art editor
- Teleconference mode
- C port from FPC for OpenWatcom
- Section 18 in this document

### Phase 6: Linux Native
- async_linux.c — wrench's FOSSIL socket design (m_fossil_socket.pas)
- Port DOS int86/bdos/inp/outp to POSIX
- OpenWatcom -bt=linux target (no GCC)
- All 15.41 tools compile for Linux

### Phase 7: Multi-Platform
- FreeBSD, Mac (Darwin) targets
- openwatcomirc cross-compiler (sysop/0, on hold)
- Cross-compile testing matrix

### Phase 8: Community & Preservation
- PPE decompiler on 2,757 PPE collection
- PCBoard box/manual scanning (Roy/SAC contacted)
- Package reprint materials
- g00r00 ANSI art for FILE_ID.ANS
- BBS scene outreach

### PCBTEXT Slot Map
```
001-746  PCBoard 15.3 (Clark)
747-750  PCBoard 15.4 (Clark — gender, email, web, birthday)
751-755  Nodelist lookup (section 10)
756-758  FidoNet address display (section 11)
759-760  Message info header (section 11)
761-763  Thread tree display (section 11)
764-765  Move message (section 11)
766      Dupe detected (section 12)
767      Toss stats (section 12)
768      Scan stats (section 12)
769      Passthrough forward (section 12)
770      File source display (section 1)
771-774  (reserved)
775-778  BinkP status (section 20)
779-780  (reserved)
781-787  Queue Editor (section 16)
788      FTP not supported (section 19) ✅ in source
```

## 5. PCBOARD.DAT Version Field

Current: offset 0, 2 bytes, value 0x0F28 (15.40)
New: value 0x0F29 (15.41)

INIT.C checks this on startup. Must accept both 15.40 and 15.41
to allow gradual migration:
```c
if (version != 0x0F28 && version != 0x0F29) {
    printf("Invalid PCBOARD.DAT version\n");
    exit(1);
}
```


## 6. Netmail Support

PCBoard 15.4's built-in FidoNet handles echomail but netmail routing
is basic. Version 15.41 formalizes the netmail and TIC configuration
with proper node records.

### Echomail Node Record (based on Mystic BBS RecEchoMailNode)

Reference: Mystic BBS 1.10a30 records.pas (GPL v3.0, g00r00)

```c
/* Node transport type — how mail is sent/received */
typedef enum {
    MAIL_BINKP   = 0,   /* BinkP protocol (TCP) */
    MAIL_FTP     = 1,   /* FTP transfer */
    MAIL_DIR     = 2,   /* Direct file copy (local/LAN) */
} mailtransporttype;

/* FTN address — zone:net/node.point */
typedef struct {
    unsigned short zone;
    unsigned short net;
    unsigned short node;
    unsigned short point;
} ftnaddress;

/* Echomail/netmail node configuration */
#pragma pack(1)
typedef struct {
    unsigned short index;          /* node index number */
    char           description[36];/* node description */
    unsigned char  active;         /* 0=inactive, 1=active */
    ftnaddress     address;        /* FTN address */
    char           domain[9];      /* FTN domain */
    char           arctype[5];     /* archive type (ZIP/ARJ/LZH/RAR) */
    unsigned char  mailtype;       /* 0=BINKP, 1=FTP, 2=DIR */
    char           binkhost[61];   /* BinkP hostname:port */
    unsigned char  ftppassive;     /* FTP passive mode */
    unsigned char  prottype;       /* protocol type */
    unsigned short binktimeout;    /* BinkP timeout (seconds) */
    unsigned short binkblock;      /* BinkP block size */
    unsigned char  binkmd5;        /* BinkP MD5 auth */
    char           reserved[20];   /* future use */
} echomailnode;
#pragma pack()
```

### File Echo Area Configuration

Extends PCBFIDO.CFG with file echo areas (used by pcbtic):

```c
/* File echo area — maps FTN area tag to PCBoard file directory */
#pragma pack(1)
typedef struct {
    char           areatag[64];    /* FTN area tag (e.g. BBS_UTILS) */
    char           directory[80];  /* PCBoard file directory path */
    char           dirlist[80];    /* DIR listing file path */
    unsigned char  passthrough;    /* 0=store locally, 1=forward only */
    unsigned char  active;         /* 0=inactive, 1=active */
    ftnaddress     uplink;         /* uplink node for this area */
    char           password[20];   /* area password */
    char           reserved[20];   /* future use */
} fileechoarea;
#pragma pack()
```

### Netmail Message Format

PCBoard uses .MSG files for netmail (FTS-0001 Type 2 format):

```
Offset  Size  Field
0       2     Originating node
2       2     Destination node
4       2     Originating net
6       2     Destination net
8       2     Attribute flags
10      2     Cost
12      20    Date/time string
32      36    To field
68      36    From field
104     72    Subject field
176     var   Message text (null-terminated)
```

Attribute flags (FTS-0001):
```c
#define MSG_PRIVATE     0x0001
#define MSG_CRASH       0x0002
#define MSG_RECEIVED    0x0004
#define MSG_SENT        0x0008
#define MSG_FILEATTACH  0x0010
#define MSG_INTRANSIT   0x0020
#define MSG_ORPHAN      0x0040
#define MSG_KILLSENT    0x0080
#define MSG_LOCAL       0x0100
#define MSG_HOLDFORPICK 0x0200
#define MSG_FILEREQUEST 0x0800
```

### TIC File Format (FTS-5006.001)

Full TIC field specification for pcbtic:

```
Required:
  Area      AREANAME           File echo area tag
  File      FILENAME.EXT       Filename being distributed

Optional:
  Origin    zone:net/node      Originating FTN address
  From      zone:net/node      Sending node address  
  To        zone:net/node.pt   Destination node
  Size      bytes              File size in bytes
  Date      timestamp          Unix timestamp
  Desc      text               Short description (1 line)
  LDesc     text               Long description (multi-line)
  CRC       DEADBEEF           CRC-32 of file (hex)
  Path      addr time datestr  Routing path (1 per hop)
  Seenby    addr               Nodes that have seen this
  Pw        PASSWORD           Area password
  Replaces  OLDFILE.EXT        File being superseded
  Magic     name               Magic filename for FREQ
  Created   program version    Creating software
```

### FTSC References

| Spec | Title |
|------|-------|
| FTS-0001 | Basic FidoNet Technical Standard (Type 2 .MSG) |
| FTS-0006 | YOOHOO and YOOHOO/2U2 (session negotiation) |
| FTS-0009 | EMSI/IEMSI Protocol Definition |
| FTS-5006.001 | TIC File Format (file echo distribution) |
| FSC-0039 | FidoNet Registry (zone/net/node allocation) |
| FSC-0048 | Type 2 Packet Format (echomail .PKT) |
| FSC-0056 | EMSI/IEMSI Protocol Definition (extended) |
| BinkP/1.0 | BinkP Protocol Specification |


## 7. FILE_ID.ANS Support

### Background

PCBoard extracts FILE_ID.DIZ from uploaded archives to populate
file descriptions. Version 15.41 adds FILE_ID.ANS support — an
ANSI art version of the file description displayed to callers with
ANSI terminal capability.

### Reference Implementation: Mystic BBS (bbs_filebase.pas)

Mystic's ImportDIZ function (line 626 of bbs_filebase.pas):
1. Searches archive for FILE_ID.DIZ (case-insensitive)
2. Extracts to temp directory via ExecuteArchive()
3. Reads lines with ReadLn(), strips low ASCII
4. Stores in FDir.DescLines / MsgText[] array
5. Limits to bbsCfg.MaxFileDesc lines

Mystic handles ANSI codes in descriptions natively through its
pipe code display engine (|xx color codes). FILE_ID.DIZ files
containing raw ANSI escape codes are displayed as-is when the
caller's terminal supports ANSI.

### PCBoard 15.41 Extension

```c
/* In file upload/import processing: */

/* 1. Try FILE_ID.ANS first (if caller has ANSI) */
if (Status.Graphics >= GRAPHICS) {
    extract_from_archive(archive, "FILE_ID.ANS", temppath);
    if (fileexist(temppath) != 255) {
        import_file_desc(temppath, &DirRec, 1); /* 1 = ANSI */
        goto diz_done;
    }
}

/* 2. Fall back to FILE_ID.DIZ */
extract_from_archive(archive, "FILE_ID.DIZ", temppath);
if (fileexist(temppath) != 255) {
    import_file_desc(temppath, &DirRec, 0); /* 0 = text */
}

diz_done:
```

### Storage

ANSI descriptions stored in the same DIR listing format:
- Short description (first line of DIZ/ANS) → DIR listing
- Full description → stored in extended description file
- ANSI flag bit in .SRC record: `flags |= FSRC_HAS_ANS`

### File Search Priority

When extracting from an uploaded archive:
1. `FILE_ID.ANS` — ANSI art description (preferred for ANSI callers)
2. `FILE_ID.DIZ` — plain text description (fallback)
3. No description — sysop enters manually

### Display Priority

When showing file info to a caller:
1. If caller has ANSI and .ANS description exists → show ANSI version
2. Otherwise → show plain text .DIZ description
3. PCBoard @-codes work in both .DIZ and .ANS

### Repository FILE_ID Files

```
pcbrevival/
├── FILE_ID.DIZ    (plain text, 48 chars wide, BBS-safe ASCII)
├── FILE_ID.ANS    (ANSI art version, for ANSI-capable displays)
```

The repo FILE_ID.ANS is pending crew artwork.
Candidates: g00r00 (Mystic BBS), crew ANSI artist, TheDraw/PabloDraw.


## 8. Extended Archiver Support

### PCBoard 15.4 (current)

PCBoard supports 4 hardcoded archiver slots:
```c
#define ZIP_ARCH  0   /* ZIP Archive */
#define ARJ_ARCH  1   /* ARJ Archive */
#define ARC_ARCH  2   /* ARC Archive */
#define LZH_ARCH  3   /* LZH Archive */
#define MAX_ARCHIVERS 4
```

Each slot has: archiver path, switches, unarchiver path, unswitches.
Configured in PCBSETUP → FidoNet → Archiver Configuration (FIDOARC.C).

### PCBoard 15.41 (extended)

Increase MAX_ARCHIVERS and add modern formats:
```c
#define ZIP_ARCH    0   /* ZIP (PKZIP/Info-ZIP) */
#define ARJ_ARCH    1   /* ARJ */
#define ARC_ARCH    2   /* ARC (SEA) */
#define LZH_ARCH    3   /* LZH/LHA */
#define RAR_ARCH    4   /* RAR (WinRAR) */
#define P7Z_ARCH    5   /* 7Z (7-Zip) */
#define TGZ_ARCH    6   /* TAR.GZ / TGZ */
#define TAR_ARCH    7   /* TAR */
#define MAX_ARCHIVERS 8
```

### Reference Implementation: Mystic BBS (records.pas)

Mystic uses a database-driven approach (ARCHIVE.DAT) with
unlimited archiver records:

```pascal
RecArchive = Record          { ARCHIVE.DAT }
    OSType : Byte;           { 0=Win 1=Linux 2=OSX 3=All 4=OS2 }
    Active : Boolean;
    Desc   : String[30];     { "ZIP Archive" }
    Ext    : String[4];      { "ZIP" }
    Pack   : String[80];     { "zip -j %2 %1" }
    Unpack : String[80];     { "unzip -o %1 -d %2" }
    View   : String[80];     { "unzip -l %1" }
End;
```

Advantages of Mystic's approach:
- Unlimited archivers (not hardcoded to 4)
- OS-specific commands (Win/Linux/OSX/OS2)
- View command for listing archive contents
- Active flag to enable/disable without removing

### PCBoard 15.41 Archiver Record (extended)

```c
/* ARCHIVERS struct — extended from 4 to 8 slots */
/* Backward compatible: first 4 slots unchanged */
#pragma pack(1)
typedef struct {
    char archivers[MAX_ARCHIVERS][MAXFLEN];
    char archiver_switches[MAX_ARCHIVERS][80];
    char unarchivers[MAX_ARCHIVERS][MAXFLEN];
    char unarchiver_switches[MAX_ARCHIVERS][80];
    /* New in 15.41: */
    char archiver_view[MAX_ARCHIVERS][MAXFLEN];    /* view/list command */
    char archiver_ext[MAX_ARCHIVERS][5];            /* file extension */
    unsigned char archiver_active[MAX_ARCHIVERS];   /* active flag */
} ARCHIVERS;
#pragma pack()
```

### Archive Detection

PCBoard detects archive type by file header bytes:
```c
/* Magic bytes for archive type detection */
ZIP:  PK\x03\x04    (offset 0)
ARJ:  \x60\xEA      (offset 0)
ARC:  \x1A           (offset 0)
LZH:  -lh            (offset 2)
RAR:  Rar!           (offset 0)
7Z:   7z\xBC\xAF    (offset 0)
GZ:   \x1F\x8B      (offset 0)
TAR:  ustar          (offset 257)
```

### Media Files (future)

kiddo/evga's work on MP3/MP4 support would add media-aware
description extraction:
- MP3: ID3v2 tag → FILE_ID.DIZ equivalent (TIT2/TALB/TPE1)
- MP4: iTunes metadata atoms
- These are not archivers but the file description import
  pipeline can be extended to read metadata from media files

### PCBSETUP Archiver Screen (FIDOARC.C)

Extended from 16 fields (4 archivers × 4) to 32 fields
(8 archivers × 4). The screen layout adds a second page
or scrolling for archivers 5-8 (RAR/7Z/TGZ/TAR).


## 9. pcbpscan — File Scanner

### Architecture

pcbpscan is a **standalone external tool**, not integrated into
PCBoard. PCBoard's existing PCBTEST.BAT hook provides the interface:

```
PCBoard verifyfile()
  └→ PCBTEST.BAT %1 %2 %3 %4
       └→ pcbpscan <filepath> <type> <descfile> <filename>
            ├→ exit 0: PASS → PCBPASS.TXT
            └→ exit 1: FAIL → PCBFAIL.TXT created by .BAT
```

### Directory Structure

```
pcbrevival/
└── pcbpscan/
    ├── pcbpscan.c     Source (standalone C, no PCBoard deps)
    ├── pcbpscan        Compiled binary
    ├── PCBTEST.BAT       Integration script
    ├── README.md         Documentation
    └── FILE_ID.DIZ       Distribution descriptor
```

### Tests Performed

1. File exists and > 0 bytes
2. ZIP central directory integrity check
3. Zip bomb detection (file count limit)
4. Path traversal in filenames (../ and drive letters)
5. Archive type detection by magic bytes
6. External virus scanner hook (PCBPROSCAN_AV env var)

### PCBoard Integration

Enabled in PCBSETUP → "Test Uploads" = YES.
PCBoard calls PCBTEST.BAT after each upload automatically.
No PCBoard source changes needed — uses existing verifyfile()
in SHELL.C (line 1095).

### Design Decision

pcbpscan stays separate from PCBoard for these reasons:
- Zero risk to PCBoard stability from scanner bugs
- Can be updated independently of PCBoard
- Works with any PCBoard version (15.0+)
- Same scanner usable for FTP uploads and TIC imports
- Can be replaced with any other scanner tool
- Testable without running PCBoard

### Future Integration (deferred)

Deep integration (scanning inside PCBoard process) considered
for a future version. Would allow:
- Real-time scan progress display to caller
- Tighter FILE_ID.DIZ extraction pipeline
- In-process virus scanning without shell overhead

Deferred because: more complexity, more bugs, less flexibility.
The PCBTEST.BAT interface works and has been proven since PCBoard 14.x.

### Credits

Clean room design by evga, kiddo, sysop/0 (pcbirc crew).

## 10. Nodelist Lookup and Search

PCBoard 15.4 compiles nodelists for internal routing (NODELIST.NDX)
but offers no user-facing lookup command. Mystic BBS provides a
TNodeListSearch class that parses raw nodelist text and searches by
address or keyword (sysop name, BBS name, location, phone).

### Reference: Mystic BBS bbs_nodelist.pas (GPL v3.0, g00r00)

Mystic's approach:
- Reads the merged nodelist.txt (text format, FTS-5000 standard)
- Tracks current Zone/Net/Node while scanning (stateful parser)
- Supports two search modes: FTN address (zone:net/node) with
  wildcards (? for any), or keyword match against name/location/phone
- Extracts INA: flag for internet hostname

### PCBoard 15.41 Nodelist Lookup

New command: `NODELIST` (or `NL`) at the main command prompt.

Sysop-configurable via PCBSETUP → FidoNet → Nodelist Settings:
- Nodelist path (where compiled NODELIST.NDX lives)
- Raw nodelist path (where text nodelists live for search)
- Allow user lookup (Y/N, default Y)
- Minimum security level for lookup

User flow:
```
Command? NL
Nodelist Lookup — Enter address or keyword:
> 1:105/*
Zone  Net   Node  BBS Name                 Sysop               Location
1     105   1     The BBS Corner           John Smith           New York NY
1     105   5     Digital Paradise          Jane Doe             Boston MA
1     105   23    Hex Central              hexadecimal          Brooklyn NY
(3 nodes found)
```

Or keyword search:
```
Command? NL
Nodelist Lookup — Enter address or keyword:
> hexadecimal
Zone  Net   Node  BBS Name                 Sysop               Location
1     105   23    Hex Central              hexadecimal          Brooklyn NY
(1 node found)
```

### nlcomp Integration

The nlcomp tool (/pcbrevival/tools/nlcomp.c) compiles raw FTS-5000
nodelists into NODELIST.NDX. PCBoard 15.41 calls nlcomp during
event processing to rebuild the index when diffs arrive.

### Implementation

New source files:
- NODELIST.C — user-facing lookup command, text nodelist parser
- Updates to PCBSETUP FIDOCFG for nodelist path configuration

New PCBTEXT strings (see section 13):
- TXT_NODELISTPROMPT — "Nodelist Lookup — Enter address or keyword:"
- TXT_NODELISTHEADER — column header line
- TXT_NODELISTENTRY — per-node display format
- TXT_NODELISTCOUNT — "(N nodes found)"
- TXT_NODELISTNONE — "No matching nodes found."

### Files Changed

| File         | Change                                         |
|---|---|
| PCBOARD.H   | Add NODELIST command to callfromtype enum       |
| PCBTEXT.H   | Add TXT_NODELIST* string defines (751-755)      |
| COMMAND.C   | Add NL command handler                          |
| FIDOCFG.C   | Add nodelist lookup path to FidoNet config      |
| MKPCBTXT.C  | Add default text for new string slots           |

## 11. Message Reader Updates

PCBoard 15.4's message reader supports basic threading (by reference
number) and single-message display. Version 15.41 extends this with
FidoNet-aware display, improved thread navigation, and new commands
adapted from Mystic BBS patterns.

### Reference: Mystic BBS bbs_msgbase.pas (GPL v3.0, g00r00)

Mystic's message reader features we adopt:
- Message base type awareness (JAM, Squish, local, echomail, netmail)
- Origin line display for echomail
- Configurable reader templates (header display files)
- Group-based area navigation
- Global message search across all bases
- Sent mail review

### Existing PCBoard Reader Commands (MSGREAD.C)

These are already implemented in 15.4:

| Key  | Function                                        |
|---|---|
| +    | Next message                                    |
| -    | Previous message                                |
| A    | Again (re-display)                              |
| C    | Capture to file                                 |
| D    | Download capture                                |
| E    | Enter message / Edit header                     |
| F    | Forward message                                 |
| G    | Goodbye (logoff)                                |
| H    | Help                                            |
| J    | Join conference                                 |
| K    | Kill (delete) message                           |
| L    | Leave reader (back to command prompt)            |
| M    | Memorize message number                         |
| N    | Non-stop reading                                |
| O    | Operator page / Override LMR update             |
| P    | Protect/unprotect message                       |
| Q    | Quick scan                                      |
| S    | Since date                                      |
| T    | Thread reading (toggle)                         |
| U    | Update LMR pointer                              |
| V    | View attached file                              |
| X    | Expert mode toggle                              |
| Y    | Your messages only                              |
| /    | Search text                                     |
| *    | Since date (alias)                              |

### New Commands in 15.41

| Key  | Function                   | Notes                     |
|---|---|---|
| W    | Move message to conference | Was sysop-only internal   |
| I    | Message info (FidoNet)     | Show origin, MSGID, path  |
| B    | Browse thread tree         | Visual thread display     |
| ^    | Jump to thread root        | First message in thread   |

### FidoNet Message Display

When reading echomail or netmail, the reader displays additional
FidoNet information:

```
Msg#: 1234  *ECHOED*  Ref#: 1230
From: hexadecimal                 1:105/23
  To: ALL
Subj: PCBoard Revival Project
Date: 08-05-26 14:30
Area: BBS_DEV
───────────────────────────────────────────────────────
Message text here...

--- PCBoard 15.41/OpenWatcom
 * Origin: Hex Central - Brooklyn NY (1:105/23)
```

New display elements:
- FTN address after From/To names (for echomail/netmail)
- Area tag line (for echomail)
- Origin line passthrough (from message body, not stripped)
- Tearline display (--- Software/version)
- SEEN-BY and PATH hidden by default, shown with `I` command

### Message Info Command (I)

Shows FidoNet routing metadata:

```
──── Message Info ────────────────────────────────────
MSGID: 1:105/23 abcd1234
REPLY: 1:105/5 efgh5678
Origin: Hex Central - Brooklyn NY (1:105/23)
SEEN-BY: 105/1 5 23 203/0 1 2
PATH: 105/23 105/1 203/0
──────────────────────────────────────────────────────
```

### Thread Tree Browser (B)

Visual thread display with navigation:

```
Thread: PCBoard Revival Project
├─ #1230 hexadecimal → ALL (08-04-26)
│  ├─ #1232 verta1878 → hexadecimal (08-04-26)
│  │  └─ #1235 hexadecimal → verta1878 (08-05-26)
│  └─ #1233 wrench → ALL (08-04-26)
└─ #1234 hexadecimal → ALL (08-05-26)  ◄ current

[+] Next  [-] Prev  [Enter] Read  [L] Leave
```

### Implementation

Updates to MSGREAD.C:
- Add case handlers for W, I, B, ^ commands
- Add FTN header display logic (check MsgBase.Header.EchoFlag)
- Add origin/tearline parsing from message body
- Thread tree builder (walk RefNumber chain)

New PCBTEXT strings:
- TXT_FIDOFROMADDR — "@X0E@ADDR@" (FTN address pipe code)
- TXT_FIDOAREATAG — "Area: @AREATAG@"
- TXT_MSGINFOHDR — "──── Message Info ────"
- TXT_THREADTREEHDR — "Thread: @SUBJECT@"
- TXT_MOVEMSGPROMPT — "Move to conference #:"

## 12. Echomail Toss/Scan Updates

PCBoard 15.4 tosses incoming echomail via PCBTOSS.CPP and scans
outbound via PCBMSG.CPP. Version 15.41 updates the toss/scan to
handle modern FidoNet conventions.

### Reference: Mystic BBS mutil_echoimport.pas, mutil_echoexport.pas

Key patterns from Mystic adopted:
- Proper SEEN-BY merging (not just appending)
- PATH line maintenance per FTS-0004
- MSGID/REPLY kludge preservation
- Dupe detection by MSGID
- Strip Down/Pvt nodes from nodelist (configurable)
- PKT file handling with proper Type 2+ headers

### Toss Updates (PCBTOSS.CPP)

| Change                      | Description                          |
|---|---|
| MSGID dupe check            | Hash MSGID, check against dupe DB    |
| SEEN-BY merge               | Merge rather than append SEEN-BY     |
| PATH append                 | Add our address to PATH line         |
| Bad packet logging          | Log reason for reject, not just move |
| Passthrough areas           | Forward without storing locally      |
| Multi-AKA support           | Match incoming to correct AKA        |

### Scan Updates (PCBMSG.CPP)

| Change                      | Description                          |
|---|---|
| Generate MSGID              | Unique MSGID kludge for outbound     |
| Set REPLY                   | Link REPLY to parent MSGID           |
| Proper tearline             | "--- PCBoard 15.41/OpenWatcom"       |
| Origin from config          | Use per-area origin line             |
| SEEN-BY generation          | Build from node list, not hardcode   |
| PATH initialization         | Start PATH with our address          |

### Dupe Database

New file: DUPES.DAT — fixed-size circular buffer of MSGID hashes.

```c
#pragma pack(1)
typedef struct {
    unsigned long crc32;         /* CRC-32 of MSGID string */
    unsigned long timestamp;     /* when seen (DOS packed date) */
} duperecord;
#pragma pack()
```

Default size: 30,000 entries (≈240KB). Configurable in PCBSETUP.

### Files Changed

| File          | Change                                        |
|---|---|
| PCBTOSS.CPP   | MSGID dupe check, SEEN-BY merge, PATH         |
| PCBMSG.CPP    | MSGID/REPLY generation, tearline, origin       |
| FIDOCFG.C     | Dupe DB size, passthrough config               |
| STRUCTS.H     | duperecord typedef                             |
| DEFINES.H     | DUPES_DAT filename define                      |

## 13. New PCBTEXT Strings

PCBoard 15.4 uses text slots 1-750. Version 15.41 adds slots
751-788 for new features. All new strings follow existing
PCBoard pipe code conventions (@X color codes, @-variable
substitution).

### String Assignments

```c
/* ── Nodelist Lookup (section 10) ── */
#define TXT_NODELISTPROMPT     751  /* "Nodelist Lookup — Enter address or keyword: " */
#define TXT_NODELISTHEADER     752  /* "@X0EZone  Net   Node  BBS Name                 Sysop               Location@X07" */
#define TXT_NODELISTENTRY      753  /* "@X07@ZONE@ @NET@  @NODE@ @BBSNAME@ @SYSOP@ @LOCATION@" */
#define TXT_NODELISTCOUNT      754  /* "(@X0F@COUNT@@X07 nodes found)" */
#define TXT_NODELISTNONE       755  /* "No matching nodes found." */

/* ── Message Reader FidoNet (section 11) ── */
#define TXT_FIDOFROMADDR       756  /* "@X0E@FROMADDR@@X07" */
#define TXT_FIDOTOADDRR        757  /* "@X0E@TOADDR@@X07" */
#define TXT_FIDOAREATAG        758  /* "@X03Area: @AREATAG@@X07" */
#define TXT_MSGINFOHDR         759  /* "@X0E──── Message Info ────────────@X07" */
#define TXT_MSGINFOFIELD       760  /* "@X0B@FIELD@: @X07@VALUE@" */
#define TXT_THREADTREEHDR      761  /* "@X0EThread: @X0F@SUBJECT@@X07" */
#define TXT_THREADTREENODE     762  /* "@X07@TREE@ @X0B#@MSGNUM@ @X0A@FROM@@X07 → @X0E@TO@@X08 (@DATE@)@X07" */
#define TXT_THREADTREECUR      763  /* " @X0C◄ current@X07" */
#define TXT_MOVEMSGPROMPT      764  /* "Move to conference #: " */
#define TXT_MSGMOVED           765  /* "Message moved to conference @X0F@CONF@@X07." */

/* ── Echomail Toss/Scan (section 12) ── */
#define TXT_DUPEDETECTED       766  /* "Duplicate message detected (MSGID: @MSGID@), skipped." */
#define TXT_TOSSSTATS          767  /* "Tossed: @X0F@TOSSED@@X07  Dupes: @X0F@DUPES@@X07  Bad: @X0F@BAD@@X07" */
#define TXT_SCANSTATS          768  /* "Scanned: @X0F@SCANNED@@X07 messages in @X0F@AREAS@@X07 areas" */
#define TXT_PASSTHRUFWD        769  /* "Passthrough: @X0F@COUNT@@X07 messages forwarded" */

/* ── File Source Display (section 1 update) ── */
#define TXT_FILESOURCE         770  /* "Source: @X0E@SOURCE@@X07" */
#define TXT_FILESRCFIDO        771  /* "FidoNet TIC (@ADDR@)" */
#define TXT_FILESRCFTP         772  /* "FTP Upload (@USER@)" */
#define TXT_FILESRCLOCAL       773  /* "Local Upload" */
#define TXT_FILESRCSYSOP       774  /* "SysOp Upload" */

/* ── General 15.41 additions ── */
#define TXT_BINKPSTATUS        775  /* "@X03BinkP: @X07@STATUS@" */
#define TXT_BINKPCONNECT       776  /* "Connecting to @X0F@HOST@@X07..." */
#define TXT_BINKPAUTHOK        777  /* "Authenticated with @X0F@ADDR@@X07 (@X0ACRAM-MD5@X07)" */
#define TXT_BINKPXFER          778  /* "@X0B@DIR@ @X0F@FILE@@X07 (@SIZE@ bytes)" */
#define TXT_NLCOMPSTATUS       779  /* "Compiling nodelist: @X0F@NAME@@X07 (@NODES@ nodes)" */
#define TXT_VERSIONSTR1541     780  /* "PCBoard v15.41/OpenWatcom" */

/* ── FTP File Transfer Protocol (section 19) ── */
/* FTP uses ALL existing transfer prompts unchanged (#176, #217, #280,   */
/* #290, #323, #324, #357, #358, #478, #480-482, #500, #620-621).       */
/* The protocol name "FTP (Internet)" comes from PCBPROT.DAT, not here. */
/* Only one new string: guard message for modem (non-TCP) callers.       */
#define TXT_FTPNOTSUPPORTED    788  /* "FTP transfer not available on this connection." */
```

### Pipe Code Variables (new)

| Variable     | Expansion                    | Context            |
|---|---|---|
| @ZONE@       | FTN zone number              | Nodelist, FidoNet  |
| @NET@        | FTN net number               | Nodelist, FidoNet  |
| @NODE@       | FTN node number              | Nodelist, FidoNet  |
| @FROMADDR@   | Sender FTN address           | Message reader     |
| @TOADDR@     | Recipient FTN address        | Message reader     |
| @AREATAG@    | Echomail area tag            | Message reader     |
| @MSGID@      | FTN MSGID kludge             | Message info       |
| @TREE@       | Thread tree drawing chars    | Thread browser     |
| @TOSSED@     | Messages tossed count        | Toss stats         |
| @DUPES@      | Duplicate count              | Toss stats         |
| @SCANNED@    | Messages scanned count       | Scan stats         |
| @AREAS@      | Areas processed count        | Scan stats         |
| @SOURCE@     | File source type text        | File display       |
| @ADDR@       | Generic FTN address          | Various            |
| @HOST@       | BinkP hostname               | BinkP status       |
| @STATUS@     | Connection status text       | BinkP status       |
| @DIR@        | Transfer direction (→/←)     | BinkP transfer     |

### MKPCBTXT Update

MKPCBTXT.EXE must be updated to generate default text for slots
751-788 in PCBTEXT.DAT. The defaults above are compiled into the
MKPCBTXT data section alongside the existing 1-750 defaults.

## 14. Menu Command Updates

PCBoard's command processing uses a tokenizer (tokenize/tokenizestr)
that parses user input into space-separated tokens. Commands are
dispatched through case statements in COMMAND.C (main prompt) and
MSGREAD.C (reader prompt).

### New Main Prompt Commands

| Command      | Function                     | Min Sec | Source        |
|---|---|---|---|
| NL           | Nodelist lookup              | 10      | NODELIST.C    |
| FIDO STATUS  | Show FidoNet mail status     | Sysop   | FIDOCFG.C     |
| FIDO TOSS    | Force echomail toss          | Sysop   | PCBTOSS.CPP   |
| FIDO SCAN    | Force echomail scan          | Sysop   | PCBMSG.CPP    |
| FIDO POLL    | Force BinkP poll             | Sysop   | New           |

### New Message Reader Commands

| Command | Function                     | Notes                    |
|---|---|---|
| I       | Message info (FidoNet)       | MSGID, path, SEEN-BY     |
| B       | Browse thread tree           | Visual tree display      |
| W       | Move message                 | Prompted for conf #      |
| ^       | Jump to thread root          | Walk RefNumber to root   |

### Updated PCBSETUP Screens

FidoNet configuration (FIDOCFG.C) gains new fields:

| Screen       | New Fields                                      |
|---|---|
| Conf FidoNet | Dupe DB size, passthrough flag, MSGID generation |
| Node Config  | BinkP host, timeout, block size, MD5 auth       |
| Nodelist     | Raw nodelist path, user lookup enable, min sec   |
| Fido Config  | Tearline text, version string                    |

### callfromtype Enum Update (PCBOARD.H)

```c
/* Add to callfromtype enum after existing entries */
NODELOOKUP,        /* NL command — nodelist lookup */
FIDOSTATUS,        /* FIDO STATUS — show mail status */
FIDOTOSS,          /* FIDO TOSS — force toss */
FIDOSCAN,          /* FIDO SCAN — force scan */
FIDOPOLL,          /* FIDO POLL — force BinkP poll */
```

### Command Security Mapping

New entries in PcbData.UserLevels[] or PcbData.SysopSec[]:

| Index              | Default Level | Description                 |
|---|---|---|
| SEC_NODELOOKUP     | 10            | Nodelist lookup command     |
| SEC_FIDOSTATUS     | Sysop         | View FidoNet status         |
| SEC_FIDOTOSS       | Sysop         | Force echomail toss         |
| SEC_FIDOSCAN       | Sysop         | Force echomail scan         |
| SEC_FIDOPOLL       | Sysop         | Force BinkP poll            |
| SEC_MSGMOVE        | Sysop         | Move messages between confs |
| SEC_MSGINFO        | 10            | View FidoNet message info   |
| SEC_THREADBROWSE   | 10            | Thread tree browser         |

## 15. FREQ and Magic File Names

PCBoard 15.4 already includes full FidoNet file request (FREQ) and
magic file name support. This section documents the existing system
for the 15.41 reference and notes where pcbis configuration touches
it.

### Existing PCBoard FREQ Infrastructure

PCBoard stores FREQ configuration in three DAT files, managed by
C++ classes in FCONFIG.C/DATA.CPP:

| File           | Class         | Structure    | Purpose                    |
|---|---|---|---|
| FREQPATH.DAT   | cFREQPATHS   | NFREQ_PATH   | Directories available for FREQ |
| MAGICNAM.DAT   | cMAGICNAMES  | NFREQ_MAGIC  | Magic name → real file mapping |
| FREQDENY.DAT   | cFREQDENY    | NADDRESS     | Nodes denied FREQ access       |

### Data Structures (from STRUCTS.H)

```c
/* FREQ path entry — directory available for file requests */
#pragma pack(1)
typedef struct {
    char Path[MAXFLEN];       /* directory path */
    char Password[10];        /* optional password */
    char reserved[10];
} NFREQ_PATH;
#pragma pack()

/* Magic name entry — magic name maps to real file */
#pragma pack(1)
typedef struct {
    char MagicName[20];       /* magic name (e.g. NODELIST, FILES) */
    char RealName[MAXFLEN];   /* actual file path or wildcard */
    char Password[10];        /* optional password */
    char reserved[10];
} NFREQ_MAGIC;
#pragma pack()

/* FREQ session limits */
#pragma pack(1)
typedef struct {
    uint          stime;      /* start time restriction */
    uint          dtime;      /* duration time limit */
    unsigned long sbytes;     /* max bytes per session */
    unsigned long dbytes;     /* max bytes per day */
    char          listed;     /* listed nodes only flag */
    uint          baud;       /* minimum baud rate */
} FREQ_INFO;
#pragma pack()
```

### PCBSETUP FidoNet Menu (existing)

The FidoNet configuration in PCBSETUP already provides these
FREQ/magic menu items (from Clark's FIDO.DOC):

```
A  Fido Configuration
B  Node Configuration
C  System Address
D  EMSI Profile
E  File & Directory Configuration
F  Archiver Configuration
G  Phone Number Translation
H  Nodelist Configuration
I  FREQ Path List          ← directories for file requests
J  FREQ Restrictions       ← session/daily limits, baud, node types
K  FREQ Magic Names        ← magic name → real file mapping
L  FREQ Deny Nodelist
```

Note: Echomail area-to-conference mapping is done per-conference
from the third conference config screen, NOT from this menu.

### BinkleyTerm XE Magic Support

BinkleyTerm XE handles magic names via its OKFILE system
(b_frproc.c). The OKFILE uses prefix characters for special
handling:

| Prefix | Function                                           |
|---|---|
| `*`    | Search in request index (BINKLEY.REQ)              |
| `@`    | Send all files matching the filespec               |
| `+`    | Execute command with remote address as arguments   |
| `$`    | Execute command with formatted net/node/point args |

Standard magic names used in FidoNet:

| Magic Name  | Maps To          | Purpose                         |
|---|---|---|
| NODELIST    | NODELIST.Zcc     | Current full nodelist (weekly)   |
| NODEDIFF    | NODEDIFF.Zcc     | Current nodelist diff (weekly)   |
| FILES       | FILES.BBS        | File listing for the system      |
| ABOUT       | ABOUT.ASC        | System description/advertisement |
| FREQ        | FREQ.DAT         | FREQ-able file list              |

(cc = righthand 2 digits of Julian date)

### pcbis Integration

The pcbis installer TUI should include a FREQ configuration screen
that writes to FREQPATH.DAT and MAGICNAM.DAT, providing a simpler
interface than the full PCBSETUP FidoNet menu for initial setup:

```
┌─── FREQ / Magic Names ────────────────────────────┐
│                                                     │
│  FREQ Paths:                                        │
│    1. C:\PCB\FILES\              Password: ____     │
│    2. C:\PCB\UPLOADS\            Password: ____     │
│    3. ________________________   Password: ____     │
│                                                     │
│  Magic Names:                                       │
│    NODELIST → C:\FIDO\NODELIST\NODELIST.*           │
│    NODEDIFF → C:\FIDO\NODELIST\NODEDIFF.*          │
│    FILES    → C:\PCB\GEN\FILES.BBS                 │
│    ABOUT    → C:\PCB\GEN\ABOUT.ASC                 │
│    ________ → ___________________________________  │
│                                                     │
│  Restrictions:                                      │
│    Max files per session: [10]                      │
│    Max bytes per session: [1000000]                 │
│    Listed nodes only:     [N]                       │
│                                                     │
│  [F1] Help  [ESC] Back  [F10] Save                 │
└─────────────────────────────────────────────────────┘
```

### QFront Compatibility

QFront 1.20a (freeware binary) also supports magic file names
through its own FREQ configuration. When used as an alternative
mailer to BinkleyTerm XE, QFront reads its own FREQ.CFG but the
magic name concept is identical. PCBoard's MAGICNAM.DAT is used
by PCBoard's internal FREQ handler (XMITEMSI.C) which sets
`WZ_FREQ` in the EMSI capabilities during session negotiation.

### FTSC References

| Document      | Title                                          |
|---|---|
| FTS-0006      | YOOHOO and YOOHOO/2U2 (file request protocol) |
| FSC-0013      | File Request (BARK and WaZOO)                  |
| FTS-5005      | BSO flow and control files (section 4: FREQ)   |
| FTS-5000      | Nodelist format (NODELIST/NODEDIFF magic names)|

## 16. Outbound Queue Editor

PCBoard 15.4 maintains an outbound queue in PCBDSZ.LST using
QUEUE_RECORD structures (80-byte filename, 25-byte nodestr,
flags, failed connection count). The existing ALT-F sysop menu
(option 5: "View/Modify Queue") provides basic queue viewing
via the `cNEWQ` class. Version 15.41 extends this with purge,
retry, BSY cleanup, and force-send operations, and exposes it
via the `FIDO STATUS` command at the main prompt.

### QUEUE_RECORD Structure (from STRUCTS.H)

```c
#pragma pack(1)
typedef struct {
    char  filename[80];       /* full path of file to send */
    char  nodestr[25];        /* destination FTN address */
    sint  flag;               /* attributes */
    sint  failedConnects;     /* failed connection count */
    bool  readOnly;           /* locked by event processing */
    char  reserved[18];
} QUEUE_RECORD;
#pragma pack()
```

### Queue Editor Features

| Command | Function                                          |
|---|---|
| D       | Delete queue entry (with confirmation)            |
| R       | Reset failed count to 0 (retry node)              |
| V       | View file contents / .PKT header info             |
| F       | Force send (mark Immediate priority)              |
| P       | Purge entries where file no longer exists          |
| B       | Purge stale .BSY lock files (>6 hours old)        |
| A       | Add a file to queue manually                      |
| S       | Show queue statistics per node                    |

### Stuck Mail Scenarios

The queue editor addresses these common failure modes:

1. **Stale .BSY files** — crash leftovers that prevent the mailer
   from connecting. The B command deletes .BSY files older than
   6 hours in the outbound directory.

2. **Missing files** — queue records pointing to deleted files.
   The P command scans all entries and removes orphaned records.

3. **Failed connections** — nodes that exceeded the retry limit.
   The R command resets the counter so the mailer tries again.

4. **Stuck TIC files** — pcbtic work directory has partial
   processing. View from the editor or re-run `pcbtic toss`.

5. **Priority override** — the F command marks a queue entry
   as Immediate flavour, bypassing all time restrictions.

### Implementation

New source file: FIDOQUE.C (queue editor)

Uses existing cCONFIGBASE<QUEUE_RECORD> template pattern from
DATA.HPP. Reads/writes PCBDSZ.LST directly. BSY file cleanup
scans the outbound directory for *.BSY files and checks mtime.

### PCBTEXT Strings for Queue Editor

```c
#define TXT_QUEUEHDR          781  /* Queue editor header */
#define TXT_QUEUEENTRY         782  /* Per-entry display line */
#define TXT_QUEUEEMPTY         783  /* "Outbound queue is empty." */
#define TXT_QUEUEDELCONF       784  /* "Delete entry? (Y/N)" */
#define TXT_QUEUEPURGED        785  /* "N orphaned entries purged." */
#define TXT_QUEUEBSYPURGE      786  /* "N stale .BSY files removed." */
#define TXT_QUEUESTATS         787  /* "N files, N bytes queued for N nodes" */
```

### Files Changed

| File          | Change                                        |
|---|---|
| FIDOQUE.C     | New — queue editor UI and operations          |
| PCBOARD.H     | Add FIDOSTATUS to callfromtype enum           |
| PCBTEXT.H     | Add TXT_QUEUE* defines (781-787)              |
| COMMAND.C     | Add FIDO STATUS/TOSS/SCAN/POLL handlers       |
| DEFINES.H     | OUTBOUND_FILE defined as PCBDSZ.LST; cNEWQ class also exists for FIDOQUE.DAT   |

## 17. PCBFCFG — Standalone FidoNet Configurator

PCBFCFG.EXE is a standalone FidoNet configuration utility that
reads/writes the same DAT files as PCBSETUP's FidoNet menu (A-L).
It provides focused FidoNet configuration without loading the full
PCBSETUP binary.

### Why

PCBSETUP.EXE is a large binary that configures everything — BBS
settings, conferences, security levels, modem settings, AND FidoNet.
Sysops frequently need to tweak just FidoNet settings (add a magic
name, change FREQ paths, add an AKA). PCBFCFG provides a fast,
focused tool for this.

### Data Files Managed

| File           | Class (DATA.HPP)  | Structure     | Screen |
|---|---|---|---|
| AKAS.DAT       | cAKAS             | NADDRESS      | C      |
| NODELIST.DAT   | cNODELISTS        | NODELIST       | H      |
| FREQPATH.DAT   | cFREQPATHS        | NFREQ_PATH    | I      |
| FREQ.DAT       | —                 | FREQ_INFO      | J      |
| MAGICNAM.DAT   | cMAGICNAMES       | NFREQ_MAGIC   | K      |
| FREQDENY.DAT   | cFREQDENY         | NADDRESS      | L      |
| ORIGINS.DAT    | cORIGINS          | ORIGIN         | O      |

### Screens

```
Main Menu — shows primary address, counts of all configured items

C  System Address (AKAs)    Add/delete FTN addresses
H  Nodelist Configuration   Add/delete nodelist base/diff names
I  FREQ Path List           Add/delete/edit FREQ directories + passwords
J  FREQ Restrictions        Edit session/daily time/byte limits,
                            allowed node types (A/L/N/U), min baud
K  FREQ Magic Names         Add/delete/edit magic → file mappings
L  FREQ Deny List           Add/delete denied FTN addresses
O  Origin Lines             Add/delete/edit origin lines
```

### Not Covered (requires PCBSETUP)

Items A (Fido enable/disable flags), B (Node Configuration with
passwords and packet settings), D (EMSI Profile), E (File &
Directory paths), F (Archiver Configuration), and G (Phone Number
Translation) write to PCBFIDO.CFG (binary config blob managed by
FCONFIG.C). These require PCBSETUP.

### Usage

```
PCBFCFG [fido_data_path]
PCBFCFG C:\PCB\FIDO        Configure FidoNet in C:\PCB\FIDO
PCBFCFG                    Configure in current directory
PCBFCFG -?                 Help
```

### Build

```
wcc386 -bt=dos -mf -5 -ox pcbfcfg.c
wlink sys dos4g name pcbfcfg file pcbfcfg
```

Compiles clean under OpenWatcom 2.0 (0 warnings, 0 errors).
Source: /pcbrevival/tools/pcbfcfg.c (597 lines)

## 18. PCBDRAW — ANSI Art Editor

PCBDRAW.EXE is a PCBoard-native ANSI art editor based on CIADraw
by CiA/Strider (1994-1996), ported to Free Pascal by sysop/0.

### Origin

CIADraw is a full-featured DOS text-mode ANSI art editor:
- 10 Turbo Pascal 7 units, 2,572 lines
- All 10 units ported to FPC by sysop/0 (10/10 compile clean)
- TheDraw-compatible font support with built-in font editor
- VGA palette editor with RGB component control
- Mouse support, block operations, box drawing
- CP437 character set, 16-color CGA palette
- Character stamp sets, screen scrolling

Source: /pcbrevival/tools/fpc264irc/examples/ciadraw/

### PCBDRAW Additions for 15.41

PCBDRAW extends CIADraw with PCBoard-specific features:

**PCBoard @X Color Code Output**

Standard ANSI uses ESC[SGR sequences for color. PCBoard display
files use `@X` codes instead:

```
@X07 = light gray on black (default)
@X0E = yellow on black
@X1F = bright white on blue
@XFC = bright red on light gray
```

PCBDRAW outputs both formats:
- .ANS — standard ANSI escape sequences (for terminals)
- .PCB — PCBoard display file with @X color codes

**PCBoard Display File Tags**

PCBDRAW inserts PCBoard display tags in .PCB output:

| Tag           | Function                              |
|---|---|
| @CLS@         | Clear screen                          |
| @PAUSE@       | Wait for keypress                     |
| @DELAY:n@     | Pause n/10 seconds                    |
| @MORE@        | More prompt                           |
| @POS:x,y@     | Position cursor                       |
| @BEEP@        | Terminal bell                         |

**Animation Support**

PCBDRAW adds frame-based animation to CIADraw:

- Multiple frames stored in a single .PCB file
- Frames separated by @CLS@@DELAY:n@ tags
- Frame editor: add/delete/copy/reorder frames
- Preview mode: plays animation in the editor
- Adjustable frame timing via @DELAY:n@ (n = tenths of second)
- Loop control: one-shot or continuous

Animation workflow:
```
1. Draw frame 1
2. [F6] New Frame — copies current frame as starting point
3. Edit differences for frame 2
4. [F7] Set Delay — enter timing for this frame
5. Repeat for additional frames
6. [F8] Preview — plays animation in editor
7. Save as .PCB — all frames in one file with @DELAY@ tags
```

Example animated .PCB file:
```
@CLS@@X0E
    ___
   /   \        Welcome to
  | o o |       Hex Central BBS
   \ - /        1:105/23
    \_/
@DELAY:10@@CLS@@X0E
    ___
   /   \        Welcome to
  | ^ ^ |       Hex Central BBS
   \ _ /        1:105/23
    \_/
@DELAY:10@@CLS@@X0E
    ___
   /   \        Welcome to
  | o o |       Hex Central BBS
   \ U /        1:105/23
    \_/
@DELAY:20@
```

**File Formats**

| Extension | Format                                    |
|---|---|
| .ANS      | Standard ANSI art (ESC sequences)         |
| .PCB      | PCBoard display file (@X codes + tags)    |
| .BIN      | Raw screen dump (char+attr pairs, 160 bytes/row) |
| .FNT      | TheDraw font file                         |
| .PAL      | VGA palette file (768 bytes, RGB triplets) |
| .PCA      | PCBDRAW animation project (multi-frame)   |

### Implementation

Based on CIADraw's existing unit structure:

| Unit          | Original        | PCBDRAW Changes                     |
|---|---|---|
| CIADRAW.PAS   | Main editor      | Add @X output, animation frames     |
| LOAD.PAS      | ANSI loader      | Add .PCB parser, @X code reader     |
| EXTENSE.PAS   | Screen I/O       | Add .PCB writer, @X code generator  |
| FONTEDIT.PAS  | Font editor      | Unchanged                           |
| FONTUNIT.PAS  | Font renderer    | Unchanged                           |
| FILELST.PAS   | File browser     | Add .PCB/.PCA to filter             |
| PALLETTE.PAS  | Palette editor   | Unchanged                           |
| MOUSE.PAS     | Mouse driver     | Unchanged                           |
| RUNTIME.PAS   | Runtime helpers  | Add animation timer                 |
| EXEC.PAS      | Shell out        | Unchanged                           |

New unit: ANIMATE.PAS — frame management, @DELAY@ timing,
preview playback, .PCA project file I/O.

### Build

```
ppc386 -Tgo32v2 PCBDRAW.PAS
```

Or cross-compile for DOS with FPC 2.6.4+ targeting go32v2.

### Teleconference Mode (Client/Server)

PCBDRAW includes a TCP-based collaborative drawing system based on
sysop/0's Pascal port of PabloDraw's teleconference protocol. Multiple
artists can draw on the same canvas simultaneously over the network.

**Server** (PCBDRAW /SERVER or standalone PDSERVER.EXE):
- Hosts shared canvas on TCP port 3693
- Up to 32 concurrent users
- Three access levels: Viewer → Editor → Operator
- Password-protected sessions
- Operator can kick users, promote/demote
- Full canvas sync on connect (CMD_LOADDOC)
- Real-time region updates (CMD_UPDATE)
- Cursor position broadcasting
- Built-in chat

**Client** (PCBDRAW /CLIENT or standalone PDCLIENT.EXE):
- Connects to server, authenticates with alias/password
- Draws on shared canvas — changes propagated to all users
- Sees other users' cursors in real time
- Chat panel for coordination

**Wire Protocol** (pdnet.pas — 600 lines, pure Pascal sockets):

```
Frame: [LEN:4][CMD:1][DATA:LEN-1]

$01 CMD_CHAT      — text message broadcast
$02 CMD_UPDATE    — canvas region update (x1,y1,x2,y2 + cells)
$03 CMD_LOADDOC   — full canvas sync (w,h + all cells)
$04 CMD_USERLIST  — connected users with access levels
$05 CMD_USERSTATUS— access level change
$06 CMD_CURSOR    — cursor position broadcast
$07 CMD_SETATTR   — attribute change
$08 CMD_KICK      — disconnect user with reason
$09 CMD_AUTH      — join with alias/password
$0A CMD_WELCOME   — server greeting (your index, level, canvas size)
$0B CMD_BYE       — disconnect with reason
```

**Network stack**: Pure Pascal sockets — fpSocket/fpBind/fpListen/
fpAccept/fpConnect/fpSend/fpRecv/fpSelect. Works on Linux (BaseUnix)
and DOS (go32v2 with packet driver + WATTCP or Sockets unit).

**Network detection** (when loaded from pcbis or standalone):

PCBDRAW checks for network availability on startup. If no network
stack is found, teleconference mode is silently disabled — all
local drawing features work normally, and the server/client menu
items are grayed out. No error on startup.

DOS detection order:
1. Check for packet driver at INT 60h-6Fh
2. Check for WATTCP.CFG in %WATTCP_CFG% or current directory
3. If neither found → networking disabled

**pcbis integration**: PCBDRAW is built into pcbis as menu item D.
Launches with PCBoard's display file path pre-configured, .PCB as
default save format. Sysop creates art and saves directly to the
BBS display file directory — no file copying needed.

### Credits

- CiA / Strider — original CIADraw (1994-1996)
- sysop/0 — FPC port (10/10 units), PabloDraw Pascal port (20 units,
  4,460 lines, 9 format parsers, teleconference client/server)
- Curtis Wensley — original PabloDraw (MIT, C#)
- hexadecimal — preservation, pcbrevival integration
- pcbirc crew — PCBoard @X codes, animation, .PCB format

## 19. FTP File Transfer Protocol

### Overview

PCBoard 15.41 adds FTP as a native file transfer protocol, sitting
alongside the existing Xmodem/Ymodem/Zmodem protocols in PCBPROT.DAT.
When a caller connects via telnet (through netmodem2irc), the BBS can
offer FTP as both a single-file and batch transfer option. This mirrors
how Mystic BBS handles FTP — as an integrated server that presents file
bases as virtual directories.

### How It Works

PCBoard's protocol system uses PCBPROT.DAT, a text file where each line
defines one protocol entry:

```
Letter  Type  BlockSize  Description  ErrCorrReq  PortOpen  LockLines
```

The `Type` field determines behavior:
- `I` — Internal (built-in Xmodem/Ymodem/Zmodem)
- `E` — External single file (runs PCBSx.BAT / PCBRx.BAT)
- `D` — External batch (runs PCBSx.BAT / PCBRx.BAT with file list)
- `B` — External bidirectional batch

For FTP, we add two new entries: single-file `F` and batch `T`:

```
F,E,0,FTP (Internet),N,N,N
T,D,0,FTP Batch (Internet),N,N,N
```

The protocol letter `F` is already used internally by PCBoard for
Ymodem-G single file. We reassign it (Ymodem-G was rarely used) or
use an unused letter. The recommended assignment:

```
F,E,0,FTP (Single File),N,N,N
T,D,0,FTP (Batch),N,N,N
```

### Protocol Flow — Single File Download

1. User types `D filename` at the command prompt
2. PCBoard resolves the file, checks security, deducts bytes
3. Protocol selection prompt shows available protocols including
   `(F) FTP (Single File)`
4. User selects `F`
5. PCBoard runs `PCBSF.BAT` with `%3` = full path to file
6. PCBSF.BAT starts the FTP send (mini FTP server or reverse
   connection to caller's FTP client)
7. DSZ-style log written, PCBoard reads transfer result

### Protocol Flow — Batch Download

1. User flags files with `FLAG` command or uses `D` with wildcards
2. When multiple files are flagged, PCBoard prompts for batch protocol
3. User selects `T` for FTP Batch
4. PCBoard writes file list to a text file
5. PCBoard runs `PCBST.BAT` with `%3` = path to file list
6. FTP handler sends all files sequentially
7. DSZ-style log written for each file transferred

### Protocol Flow — Upload

1. User types `U` at file area prompt
2. Protocol selection prompt includes `(F) FTP`
3. PCBoard runs `PCBRF.BAT` with `%3` = upload directory path
4. FTP handler receives file(s) into the upload directory
5. PCBoard processes uploads (FILE_ID.DIZ extraction, virus scan)

### Comparison with Mystic BBS

Mystic handles FTP differently — it has a **built-in FTP server** running
inside MIS2 (Mystic Internet Server) that exposes file bases as virtual
FTP directories. Users connect directly via any FTP client to browse
and download. Key Mystic FTP features (from whatsnew.txt):

- Anonymous FTP supported (file bases individually marked "allow anonymous")
- FTP uploads import FILE_ID.DIZ with ANSI/Pipe/PCBoard/WWIV color support
- Passive mode with per-connection static port (start_port + slot_number)
- IPv4 and IPv6 support, dynamic IP re-resolution every hour
- FTP file deletion if user meets SysOp ACS for that base
- QWK packet available in FTP root directory
- Verbose FTP server logging (loglevel 3)

Mystic's **protocol editor** (from mystic.txt, KALRONG/mysticbbs repo)
uses a simple record format:

```pascal
RecProtocol = Record
  OSType  : Byte;
  Active  : Boolean;
  Batch   : Boolean;      (* single vs batch *)
  Key     : Char;         (* hotkey letter *)
  Desc    : String[40];   (* shown in protocol list *)
  SendCmd : String[60];   (* download command, %0-4 MCI *)
  RecvCmd : String[60];   (* upload command, %0-4 MCI *)
End;
```

Mystic 1.12 added **internal protocols** using `@` prefix commands:
`@zmodem`, `@xmodem`, `@ymodem`, `@ymodemg`. The protocol editor
separates single and batch by having two entries with the same key
letter but different Batch flags. When only one file is selected,
Mystic shows `proto.xxx` display file; for batch, `protob.xxx`.

Mystic's transfer start prompt (prompt 065):
```
; Press ENTER to start file transfer  &1=selected protocol
065 SQ |CR|09Press |01[|15ENTER|01/|15S|01]|09tart or
     |01[|15ESCAPE|01/|15Q|01]|09uit your |15|&1 |09transfer: |XX
```

PCBoard 15.41's approach differs:
- FTP is an **external protocol** (BAT files), not built-in
- The FTP server component lives outside PCBoard (in netmodem2irc or a
  companion daemon)
- PCBoard's role is to invoke the transfer and read the DSZ log
- This keeps the core BBS code unchanged — only PCBPROT.DAT and two
  BAT files are needed

### BiModem Note

BiModem (Erik Labs, 1988-1991) was a bidirectional file transfer
protocol that allowed simultaneous upload and download. PCBoard
supported it as an external protocol with type `B` (bidirectional
batch) in PCBPROT.DAT. BiModem was never open-sourced — only binary
distribution as shareware. The final version was BiModem 1.25 (1991).
Binaries are preserved at archive.org and retroarchive.org.
PCBoard's protocol system already handles bidirectional via the `B`
type flag in `readprotfile()` (FILES.C line 441).

### PCBPROT.DAT Additions

Add these lines to the existing PCBPROT.DAT:

```
F,E,0,FTP (Single File),N,N,N
T,D,0,FTP (Batch),N,N,N
```

### BAT File Templates

**PCBSF.BAT** (FTP single-file send/download):
```batch
@echo off
REM PCBoard FTP single file download
REM %1 = COM port (unused for FTP)
REM %2 = baud rate (unused for FTP)
REM %3 = full path to file
REM DSZLOG env var = path to write transfer log
ftpsend.exe -f %3 -l %DSZLOG%
```

**PCBST.BAT** (FTP batch send/download):
```batch
@echo off
REM PCBoard FTP batch download
REM %3 = path to text file listing files to send
ftpsend.exe -b %3 -l %DSZLOG%
```

**PCBRF.BAT** (FTP receive/upload):
```batch
@echo off
REM PCBoard FTP file receive (upload)
REM %3 = upload directory path
ftprecv.exe -d %3 -l %DSZLOG%
```

### New PCBTEXT Strings

FTP reuses all 20+ existing transfer prompts unchanged. One new string:

| Slot | Define              | Default Text                                        |
|------|---------------------|-----------------------------------------------------|
| 788  | TXT_FTPNOTSUPPORTED | `FTP transfer not available on this connection.`     |

### New Pipe Code Variables

No new pipe code variables needed. Existing `@FILE@` and `@SIZE@`
variables already used by transfer status strings cover FTP.
| @PCT@     | Percentage complete        | FTP progress   |
| @REASON@  | Failure reason text        | FTP error      |

### Protocol Display (PROT display file)

When the user is prompted to select a transfer protocol, PCBoard
displays the PROT command file (or falls back to the built-in list
from `protfile()` in SETTINGS.C). The display shows:

```
   (A) ASCII
   (C) Xmodem CRC
   (O) 1K-Xmodem
   (Y) Ymodem Batch
   (Z) Zmodem Batch
=> (F) FTP (Single File)
   (T) FTP (Batch)
   (N) None

Protocol for Transfer (Enter)=F?
```

The `=>` marker indicates the user's current default protocol.
Users set their default via the `PROT` command or during initial
login settings.

### What Needs to Change in PCBoard for FTP Downloads

SyncTerm and NetRunner both support FTP file transfer from within
a telnet BBS session. When the BBS initiates an FTP transfer, the
terminal handles the out-of-band FTP connection automatically —
the user doesn't need to open a separate FTP client. This means
FTP can work like Zmodem from the user's perspective: select the
protocol, transfer happens, done.

The flow over the wire:
```
caller terminal (SyncTerm/NetRunner)
  ├─ telnet session ──→ netmodem2irc ──→ Wine ──→ PCBOARD.EXE
  └─ FTP data channel ←→ FTP server on BBS host (separate port)
```

The terminal maintains both connections simultaneously. The BBS
tells the terminal to start FTP (via escape sequence or protocol
negotiation), the terminal opens an FTP data connection to the
BBS host, transfers the file, and signals completion back over
the telnet session.

#### Source Changes Required

Since the terminals handle FTP natively, PCBoard's changes are
straightforward — treat FTP like any other external protocol:

1. **PCBPROT.DAT** — add FTP entries with the right BAT files
2. **BAT files** — PCBSF.BAT / PCBRF.BAT invoke an FTP server-side
   handler that works with the terminal's FTP client
3. **TRANSFER.C** — minimal changes, FTP flows through the existing
   external protocol path
4. **Connection detection** — hide FTP from modem callers

**PCBPROT.DAT additions:**
```
F,D,0,FTP (Internet),N,N,N
```

One entry, letter `F`, type `D` (external batch). Handles both single
and batch downloads. PCBoard shows batch protocols for all downloads
(single or multi-file). Only non-batch protocols are hidden when
multiple files are selected — batch protocols are always visible.

The description `FTP (Internet)` is what the user sees in the protocol
list at `protfile()`. No PCBTEXT string needed for the name.

**TRANSFER.C** — `getxferprotocol()` (line ~3185):
No special FTP path needed if the terminal handles it. The existing
external protocol flow works: PCBoard runs the BAT file, the BAT
file starts the FTP server-side handler, the terminal's FTP client
connects, transfer happens, DSZ log is written, PCBoard reads it.

The only addition: detect TCP connections and hide FTP from modem
callers in `protfile()`:

**SETTINGS.C** — `protfile()` (line ~48):
```c
/* Hide FTP/HTTP protocols on modem (non-TCP) connections */
if (q->FtpProtocol && !Status.TcpConnection)
    continue;
```

**pcb.h** — add to `prottype` struct:
```c
bool FtpProtocol;   /* TRUE = requires TCP connection */
```

**FILES.C** — `readprotfile()` (line ~398):
No source change needed. FTP uses existing type `D` (external batch)
which `readprotfile()` already handles. The `FtpProtocol` flag is
set by checking the protocol letter:
```c
/* After parsing, mark FTP protocols for TCP-only filtering */
if (Letter == 'F')
    q->FtpProtocol = TRUE;
```

**INIT.C** — detect TCP vs modem:
```c
Status.TcpConnection = (getenv("PCBTCP") != NULL) ||
                        (Asy.Socket > 0);
```

netmodem2irc sets `PCBTCP=1` before launching PCBoard, or passes
it via PCBOARD.SYS field 66 (currently unused).

#### BAT File Templates

**PCBSF.BAT** (FTP single-file download to caller):
```batch
@echo off
REM %1=COM port  %2=baud  %3=file path
REM DSZLOG set by PCBoard
ftpserve.exe --send %3 --port 2121 --log %DSZLOG%
```

**PCBST.BAT** (FTP batch download):
```batch
@echo off
REM %3=path to file list
ftpserve.exe --batch %3 --port 2121 --log %DSZLOG%
```

**PCBRF.BAT** (FTP upload from caller):
```batch
@echo off
REM %3=upload directory
ftpserve.exe --recv --dir %3 --port 2121 --log %DSZLOG%
```

The `ftpserve.exe` utility is the FTP server-side handler. It:
- Listens on the configured port (or uses the companion FTP daemon)
- Serves the file(s) to the terminal's FTP client
- Writes a DSZ-compatible log entry when complete
- Exits when transfer is done (or times out)

#### DSZ Log Format

```
Z  12345  56000  C:\PCB\DL\FILENAME.ZIP  0  0  0
```
Fields: status (`Z`=ok), bytes, cps, full path, errors, blocksize, flow.

#### PCBSETUP Changes

Add FTP configuration to the PCBSETUP file/protocol editor:

**PCBOARD.DAT** — new fields:
```
FtpHost=bbs.example.com     ; hostname the terminal connects to
FtpPort=2121                ; FTP server port
FtpPassiveStart=60000       ; passive mode port range start
FtpMaxConn=10               ; max simultaneous FTP connections
```

**EDITPROT.C** — add FTP-specific fields when editing an FTP
protocol entry (type F/H): FTP port, passive range, timeout.

#### New PCBTEXT Strings

FTP reuses all existing protocol selection and transfer prompts
unchanged. The protocol name comes from PCBPROT.DAT. Only one new
string at slot 788: `TXT_FTPNOTSUPPORTED` — displayed if a modem
caller somehow selects FTP (fallback guard).

#### Protocol Display

When prompted for protocol, the list shows:
```
   (A) ASCII
   (C) Xmodem CRC
   (O) 1K-Xmodem
   (Y) Ymodem Batch
   (Z) Zmodem Batch
=> (F) FTP (Internet)
   (N) None

Protocol for Transfer (Enter)=F?
```

`(F) FTP (Internet)` appears for both single and batch downloads.
PCBoard always shows batch-capable protocols. The `=>` marks the
user's current default (set via PROT command or user settings).
If the user has a default set, PCBoard skips the prompt entirely.

#### Summary of Files to Modify

| File | Change |
|---|---|
| FILES.C | Mark letter `F` as `FtpProtocol` in `readprotfile()` |
| SETTINGS.C | Hide FTP from modem callers in `protfile()` |
| pcb.h | Add `FtpProtocol` to `prottype` struct |
| PCBTEXT.H | Add TXT_FTPNOTSUPPORTED define (788) |
| PCBOARD.DAT | Add FtpHost, FtpPort, FtpPassiveStart fields |
| PCBPROT.DAT | Add `F` protocol entry (single line) |
| INIT.C | Detect TCP vs modem connection |
| PCBSETUP | Add FTP config fields (EDITPROT.C) |

#### Mystic Reference

Link: `https://github.com/KALRONG/mysticbbs`

Mystic 1.12's FTP is a built-in server inside MIS2, not a selectable
transfer protocol in the protocol editor. The protocol editor handles
Xmodem/Ymodem/Zmodem (internal via `@zmodem` etc. or external via
command line). The protocol selection display uses `proto.ans` for all
protocols and `protob.ans` for batch-only. Prompt 065 is the transfer
start confirmation:

```
; Press ENTER to start file transfer  &1=selected protocol
065 SQ |CR|09Press |01[|15ENTER|01/|15S|01]|09tart or
     |01[|15ESCAPE|01/|15Q|01]|09uit your |15|&1 |09transfer: |XX
```

For PCBoard 15.41 we backport this pattern: display the protocol list
(PROT command file or generated list from `protfile()`), then prompt
for the protocol letter (F for FTP), then show the transfer start
prompt (using existing TXT_ENTERSTARTS / TXT_ABORTSTRANSFER).

#### SyncTerm Source (GPL v2+)

SyncTerm is open source (GPL v2+, Rob Swindell / Stephen Hurd).
Reference source files preserved at `tools/syncterm/` for future use.
Source: `https://gitlab.synchro.net/main/sbbs`
Credit: Rob Swindell (digital man), Stephen Hurd (Deuce)

## 20. BinkP Mailer

### Overview

BinkP (BinkleyTerm Protocol) is the standard TCP/IP mailer protocol
for FidoNet. It replaces dial-up mailers (FrontDoor, BinkleyTerm,
D'Bridge) with a TCP connection that transfers .PKT echomail packets,
file attaches, and TIC file echoes between FidoNet nodes.

PCBoard 15.4 had no mailer — sysops ran a separate mailer
(BinkleyTerm, FrontDoor) alongside PCBoard. Version 15.41 integrates
BinkP directly, removing the need for a third-party mailer for
TCP/IP FidoNet connections. Dial-up mailer support (EMSI/YOOHOO)
is not included — BinkP is TCP-only.

### How BinkP Works

BinkP operates over a single TCP connection (default port 24554):

```
Originating node                    Answering node
     |                                    |
     |──── TCP connect ──────────────────→|
     |←─── M_NUL (banner) ──────────────│
     |──── M_NUL (SYS, ZYZ, LOC) ──────→|
     |──── M_ADR (FTN address) ─────────→|
     |←─── M_ADR (FTN address) ──────────│
     |──── M_PWD (password or CRAM-MD5)─→|
     |←─── M_OK ─────────────────────────│
     |                                    |
     |──── M_FILE (name, size, time) ───→|
     |──── data frames ─────────────────→|
     |←─── M_GOT (file received) ────────│
     |                                    |
     |←─── M_FILE ───────────────────────│
     |←─── data frames ──────────────────│
     |──── M_GOT ───────────────────────→|
     |                                    |
     |──── M_EOB (end of batch) ────────→|
     |←─── M_EOB ────────────────────────│
     |──── TCP close ────────────────────→|
```

Key BinkP features:
- Full-duplex: sends and receives simultaneously
- CRAM-MD5 authentication (password never sent in clear)
- Automatic crash/resume on large file transfers
- Non-destructive: files only deleted after M_GOT confirmation

### Node Configuration

Each echomail node entry in PCBSETUP includes BinkP settings:

```c
typedef struct {
    /* ... existing fields ... */
    unsigned char  mailtype;       /* 0=BINKP, 1=FTP, 2=DIR */
    char           binkhost[61];   /* hostname:port */
    unsigned short binktimeout;    /* timeout seconds (default 60) */
    unsigned short binkblock;      /* block size (default 4096) */
    unsigned char  binkmd5;        /* 1=CRAM-MD5, 0=plain password */
    /* ... */
} echomailnode;
```

PCBSETUP → FidoNet → Node Editor fields:

| Field | Description | Default |
|---|---|---|
| Mail Type | BINKP / FTP / DIR | BINKP |
| BinkP Host | hostname:port | :24554 |
| Timeout | Connection timeout (seconds) | 60 |
| Block Size | Data frame size (bytes) | 4096 |
| MD5 Auth | Use CRAM-MD5 authentication | Yes |
| Password | Session password | (blank) |

### FIDOPOLL Command

The sysop triggers a BinkP poll with the FIDOPOLL menu command
or ALT-F sysop hotkey. This initiates an outbound connection to
all nodes with pending mail in the BSO (Binkley-Style Outbound)
directory:

```
ALT-F → FidoNet Menu → P (Poll)
```

PCBoard scans the outbound directory for:
- `.OUT` / `.OLO` — normal/immediate outbound .PKT files
- `.FLO` — file attach lists (TIC files, file requests)
- `.REQ` — file request lists

For each node with pending mail, PCBoard connects via BinkP,
authenticates, and exchanges files in both directions.

### Inbound Processing

When another node connects inbound (via the BinkP listener),
PCBoard:

1. Authenticates the session (CRAM-MD5 or plain password)
2. Receives .PKT files → places in inbound directory
3. Receives TIC files → processes with pcbtic
4. Sends any pending outbound mail for that node
5. After session ends, auto-tosses received .PKT files

The BinkP listener runs as part of the PCBoard process or as a
companion daemon configured in PCBOARD.DAT.

### PCBOARD.DAT Configuration

```
BinkpEnable=YES              ; enable BinkP listener
BinkpPort=24554              ; listen port
BinkpInbound=C:\PCB\INBOUND  ; received files directory
BinkpOutbound=C:\PCB\OUTBOUND ; BSO outbound directory
BinkpTimeout=60              ; session timeout
BinkpBlockSize=4096          ; data frame size
BinkpMD5=YES                 ; require CRAM-MD5
```

### PCBTEXT Strings

| Slot | Define | Default Text |
|---|---|---|
| 775 | TXT_BINKPSTATUS | `@X03BinkP: @X07@STATUS@` |
| 776 | TXT_BINKPCONNECT | `Connecting to @X0F@HOST@@X07...` |
| 777 | TXT_BINKPAUTHOK | `Authenticated with @X0F@ADDR@@X07 (@X0ACRAM-MD5@X07)` |
| 778 | TXT_BINKPXFER | `@X0B@DIR@ @X0F@FILE@@X07 (@SIZE@ bytes)` |

### Pipe Code Variables

| Variable | Expansion | Context |
|---|---|---|
| @HOST@ | BinkP hostname:port | BinkP status |
| @STATUS@ | Session status text | BinkP status |
| @DIR@ | Transfer direction (→/←) | BinkP transfer |
| @FILE@ | Filename being transferred | BinkP transfer |
| @SIZE@ | File size in bytes | BinkP transfer |
| @ADDR@ | Remote FTN address | BinkP auth |

### BSO Directory Structure

PCBoard uses Binkley-Style Outbound (BSO), compatible with
BinkleyTerm XE, BinkD, and other FidoNet mailers:

```
OUTBOUND/
├── 0001001E.OUT     ← packet for 1:30/0 (normal)
├── 0001001E.FLO     ← file list for 1:30/0
├── 0001001E.REQ     ← file requests for 1:30/0
├── 0001001E.BSY     ← busy flag (session in progress)
└── ...
```

Node address encoding: `NNNNNNNN` = net (hex, 4 digits) + node
(hex, 4 digits). Zone directories for non-default zones.

### Implementation Status

BinkP is a 15.41 addition — no BinkP code exists in Clark's 15.4
source. The implementation uses the echomailnode structure defined
in section 6, the PCBTEXT strings in section 13, and the FIDOPOLL
menu command in section 14.

Source reference: BinkleyTerm XE source at `fido/btxe-source.zip`
(GPL, Thomas Waldmann). BinkP/1.0 protocol spec at
`http://ftsc.org/docs/fts-1026.001`.

### Differences from 15.4

This is a **15.41 feature only**. PCBoard 15.4 (Borland build) has
no BinkP support. The 15.4 source port (OpenWatcom) is a faithful
reproduction of Clark's code. BinkP is added on top as part of the
15.41 revival.

The eventual diff between 15.4-watcom and 15.41 will show BinkP as
entirely new code — no modifications to existing 15.4 functions.

## 21. Startup and Shutdown Scripts

### Overview

PCBoard 15.41 includes platform-specific startup and shutdown
scripts for running the BBS as a service. These handle launching
DOSBox, netmodem2irc, and PCBoard in the correct order, and
shutting them down cleanly.

### Files

| File | Platform | Purpose |
|---|---|---|
| `pcbis_startup` | Linux/Unix (bash) | Start BBS stack |
| `pcbis_startup.bat` | Windows | Start BBS stack |
| `pcbis_startup.cmd` | OS/2 (REXX) | Start PCBoard natively |
| `pcbis_shutdown` | Linux/Unix (bash) | Stop BBS stack |
| `pcbis_shutdown.bat` | Windows | Stop BBS stack |
| `pcbis_shutdown.cmd` | OS/2 (REXX) | Stop PCBoard |
| `pcbis_initv` | Linux/Unix (bash) | First-time directory setup |
| `pcbis_initv.bat` | Windows | First-time directory setup |
| `pcbis_ui.c` | All (OpenWatcom) | Configuration TUI |

### Startup Sequence (Linux/Windows)

```
pcbis_startup
  ├─ 1. Check pcbis.cfg exists (else: run pcbis_initv first)
  ├─ 2. Start netmodem2irc (NMServer.exe)
  │      - Listens on telnet port (default 23)
  │      - Forwards to DOSBox serial port
  ├─ 3. Wait 1-2 seconds for netmodem2irc to bind
  └─ 4. Start DOSBox with PCBOARD.EXE
         - Loads dosbox.conf
         - Mounts PCBoard directory as C:
         - Runs PCBOARD.EXE /N:1
```

### Startup Sequence (OS/2)

```
pcbis_startup.cmd
  └─ 1. Start PCBOARD.EXE /N:1 directly
         - OS/2 runs PCBoard natively (no DOSBox)
         - Serial I/O via OS/2 SIO/VCOM drivers
         - TCP via OS/2 TCP/IP stack
```

OS/2 does not need DOSBox or netmodem2irc. The OS/2 build of
PCBoard handles COM ports through DosDevIOCtl and TCP through
the OS/2 socket API.

### Shutdown Sequence

All platforms:
1. Kill DOSBox process (Linux/Windows) or PCBOARD.EXE (OS/2)
2. Kill netmodem2irc process (Linux/Windows only)
3. Log shutdown timestamp

Linux uses PID files (`logs/dosbox.pid`, `logs/netmodem.pid`).
Windows uses `taskkill /IM`. OS/2 uses `pstat` + `kill` via REXX.

### First-Time Setup (pcbis_initv)

Creates the PCBoard directory structure:

```
$PCBIS_ROOT/
├── bin/          PCBoard executables
├── data/         PCBOARD.DAT, conferences, user files
├── fossil/       FOSSIL driver (ADF.COM, BNU.COM)
├── work/         Temporary/node work directories
├── logs/         BBS logs, PID files
├── nodes/
│   └── node1/    Per-node working directory
├── netmodem/     NMServer.exe and config
├── dosbox.conf   DOSBox configuration
└── pcbis.cfg     pcbis configuration
```

Also creates a minimal WELCOME display file and default
dosbox.conf with serial port forwarding.

### Configuration (pcbis_ui)

`PCBIS_W.EXE` (48KB) — ANSI TUI for editing pcbis.cfg.
Compiles under OpenWatcom (C89, conio.h for keyboard input,
ANSI escape codes for display). Menu items:

- G — General Settings (BBS name, sysop, port, nodes)
- P — Paths & Directories
- W — Web Server
- T — FTP Server
- F — FidoNet Configuration
- D — DOSBox Settings
- S — Save Configuration
- I — Initialize (run pcbis_initv)

### Environment Variables

| Variable | Default | Description |
|---|---|---|
| `PCBIS_ROOT` | `$HOME/pcboard` (Linux) / `%USERPROFILE%\pcboard` (Win) | BBS root directory |
| `PCBIS_PORT` | 23 | Telnet listen port |
| `PCBIS_NODES` | 1 | Number of nodes |

### Running as a Service

**Linux (systemd)**:
```ini
[Unit]
Description=PCBoard BBS
After=network.target

[Service]
Type=forking
ExecStart=/path/to/pcbis_startup
ExecStop=/path/to/pcbis_shutdown
User=pcboard
Environment=PCBIS_ROOT=/home/pcboard/pcboard

[Install]
WantedBy=multi-user.target
```

**Windows (Task Scheduler)**:
Create a task that runs `pcbis_startup.bat` at system startup
with "Run whether user is logged on or not" enabled.

**OS/2 (STARTUP.CMD)**:
Add `call C:\PCBOARD\pcbis_startup.cmd` to STARTUP.CMD.

## 22. PCBCP — OS/2 Control Panel

### Overview

PCBCP is the PCBoard Control Panel for OS/2 Presentation Manager.
It provides a windowed GUI for monitoring and managing PCBoard
nodes, users, file transfers, and events. Originally distributed
as a separate Clark utility (not part of the licensed 15.3 source).

Source recovered from pcball.zip (pcboard.be). Ported to
OpenWatcom 2.0 — compiles and links as PCBCP_W.EXE (77KB).

### Features

- Real-time node status monitoring (all nodes in one window)
- User online display (name, location, activity, time)
- Sysop page alarm with configurable sound
- Node restart and shutdown control
- Event and door warning indicators
- Click-to-run node launch
- Configurable update interval
- Multi-node support (1-250 nodes)
- Online help (IPF format)

### Installation

1. Copy `PCBCP_W.EXE` to PCBoard directory
2. Build help file: `ipfc HELP\PCBCP.IPF` → `PCBCP.HLP`
3. Create WPS (Workplace Shell) desktop object
4. Run PCBCP — first-run will prompt for configuration

### Configuration (PCBCP.INI)

PCBCP.INI is an OS/2 binary INI file (not a text file). It stores:

| Setting | Description |
|---|---|
| FixedFont | Display font for node status |
| NumLines | Number of display lines |
| UpdateInt | Status refresh interval (seconds) |
| FirstNode / LastNode | Node range to monitor |
| ClickToRun | Enable click-to-launch nodes |
| PageAlarm | Sysop page sound enable |
| WarnEvents | Event warning display |
| WarnDoors | Door activity warning |
| WarnXfers | Transfer activity warning |
| Node_N | Per-node launch command and path |

**IMPORTANT:** PCBCP.INI must be configured by the sysop on first
run. The shipped default INI contains paths from the original Clark
development machine (`D:\TC\NEW\`). The sysop must update all
node paths to match their PCBoard installation.

**Future:** PCBSETUP should include a PCBCP configuration screen
that generates or updates PCBCP.INI automatically. This is not yet
implemented — the INI file must be configured through PCBCP's own
Options menu or by using the OS/2 INI editor (`INIED.EXE`).

### Source Files

| File | Lines | Description |
|---|---|---|
| MAIN.C | 600 | PM window procedure, message loop |
| THRD.C | 2,718 | Multi-threaded node monitoring |
| USER.C | 1,771 | User database operations |
| INIT.C | 647 | Configuration and INI read/write |
| FILE.C | 326 | File area management (usernet) |
| HELP.C | 493 | Online help system |
| PNT.C | 104 | Paint/display routines |
| DLG.C | 181 | Dialog box handlers |

### OpenWatcom Port

6 changes from original IBM C Set/2 source:

1. `pcbcp_compat.h` — bool typedef, alloc.h→stdlib.h
2. Removed hardcoded `\toolkt21\valapi.h` path
3. `_argv` → `__argv` (Watcom runtime)
4. Ctrl-Z EOF markers removed
5. Include order: PCBoard headers before Watcom headers
6. Linked against os2386.lib (OS/2 Toolkit 4.5)

### Differences from 15.4

PCBCP is an addon — not present in Clark's licensed 15.3 source
or the 15.4 port. The source was publicly distributed as a
companion utility. Our port compiles it under OpenWatcom for the
first time — the original required IBM C Set/2 for OS/2.

## 23. PCBoard 15.4 FidoNet Stack — External Tools Required

### Overview

PCBoard 15.4 ships **pcbbinkp.exe** as the built-in BinkP/1.1 mailer
(standalone, command-line). This handles TCP/IP transport — connecting
to other FidoNet nodes and transferring .PKT files and file attaches.

However, pcbbinkp is a **transport layer only**. It moves files between
nodes but does not process their contents. A working FidoNet setup on
15.4 requires external tools for mail processing, just as the original
PCBoard 15.3 did with external mailers like BinkleyTerm or FrontDoor.

### What pcbbinkp Does

- Connects to remote FTN nodes via BinkP/1.1 (FTS-1026)
- CRAM-MD5 authentication (FSP-1024)
- Sends files from BSO (Binkley-Style Outbound) directory
- Receives files into inbound directory
- Poll mode: `pcbbinkp poll 1:2320/100`
- Answer mode: `pcbbinkp answer [port]`
- Status: `pcbbinkp status`
- All output tagged `[BINKP]` for log parsing

### What pcbbinkp Does NOT Do

- Unpack mail bundles (.mo0, .tu0, .we0 etc.)
- Parse .PKT files (FTS-0001 Type 2+ packets)
- Import echomail into PCBoard message conferences
- Export outbound echomail from PCBoard into .PKT files
- Process TIC files (file echoes)
- Handle file requests (FREQ)
- Generate SEEN-BY, PATH, MSGID, or other kludge lines
- Maintain a dupe database

### External Tools Needed for 15.4 FidoNet

The sysop must provide their own tools for the mail processing
pipeline. These are well-established FidoNet utilities that work
with any BinkP mailer's BSO directory layout:

| Function | What It Does | Recommended Tools |
|----------|-------------|-------------------|
| **Tosser** | Unpacks bundles, parses .PKT, imports echomail into message bases | FastEcho, Squish, HPT (husky), GoldED tosser |
| **Scanner** | Exports new messages from PCBoard into outbound .PKT files | FastEcho, Squish, HPT |
| **Packer** | Compresses .PKT files into mail bundles for outbound | FastEcho, HPT, built into most tossers |
| **TIC processor** | Handles file echo distribution (.TIC files) | htick (husky), AllFix, TICk |
| **Nodelist compiler** | Compiles raw nodelist into fast-lookup format | FastLst, NLComp, V7+ tools |
| **FREQ handler** | Responds to file requests from other nodes | built into most mailers or standalone |

### Typical 15.4 FidoNet Workflow

```
INBOUND (receiving mail):

  remote node --BinkP--> pcbbinkp answer
                              |
                              v
                     inbound/*.pkt, *.mo0, etc.
                              |
                              v
                    [external tosser] --> PCBoard message bases
                         (e.g. HPT)       (conferences)

OUTBOUND (sending mail):

  PCBoard user writes message in echomail conference
                              |
                              v
                   [external scanner] --> outbound/*.pkt
                        (e.g. HPT)
                              |
                              v
                     pcbbinkp poll 1:2320/100
                              |
                              v
                    remote node receives .PKT
```

### Recommended: Husky Suite (HPT + htick)

For sysops who don't already have a FidoNet toolchain, the
Husky FidoNet Project (https://github.com/huskyproject) provides
a complete open-source stack:

- **HPT** — tosser/scanner, handles .PKT import/export, SEEN-BY/PATH,
  dupe detection, echomail routing. Supports PCBoard-style message
  bases via an adapter or via JAM format with conversion.
- **htick** — TIC file echo processor
- **NLTools** — nodelist compiler
- **fidoconf** — unified configuration for all husky tools

All are open source (GPL), actively maintained, and work with BSO
outbound — the same directory layout pcbbinkp uses.

### Automation

On 15.4, the sysop sets up batch files or cron jobs:

**OS/2 / DOS (poll.cmd):**
```
@echo off
REM Poll hub and toss incoming mail
pcbbinkp poll 1:2320/100
hpt toss
hpt scan
pcbbinkp poll 1:2320/100
```

**Linux (crontab):**
```
# Every 30 minutes: poll, toss, scan, poll again
*/30 * * * * cd /pcboard && ./pcbbinkp poll 1:2320/100 && hpt toss && hpt scan && ./pcbbinkp poll 1:2320/100
```

### 15.41 Difference

Version 15.41 will include **pcbtoss** (built-in tosser/scanner
that writes directly to PCBoard message bases) and **pcbfido**
(FidoNet operations console with scrollback, filtering, and
integrated control of all FidoNet tools). This eliminates the
need for external tossers on 15.41, but 15.4 sysops must use
external tools as described above.

### pcbbinkp Source Files (tools/pcbbinkp/)

| File | Lines | Purpose |
|------|-------|---------|
| binkp.h | 170 | Protocol defines, frame format, state machine, session struct |
| binkp.c | 743 | Frame I/O, command handlers (M_NUL thru M_SKIP), select() loop |
| binkpauth.c | 161 | HMAC-MD5 (RFC 2104), CRAM-MD5 challenge/verify (FSP-1024) |
| bso.c | 283 | BSO outbound scanner — .flo parsing, all mail flavours |
| pcbbinkp.c | 488 | Main EXE: poll/answer/status, pcbis.cfg parser, TCP connect/listen |
| md5.c/h | 163 | RFC 1321 MD5 (public domain, from evga's VMODEM) |
| build.cmd | 50 | OpenWatcom build script: OS/2, DOS4G, NT targets |
| **Total** | **2,058** | |

**Binaries:** PCBBINKP.EXE (47KB, OS/2) / PCBBINKP_W.EXE (62KB, NT)
