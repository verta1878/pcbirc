# PCBoard 15.41 FidoNet Sysop Guide

**pcbrevival / pcbirc crew — August 2026**

This document covers how to set up and run FidoNet on PCBoard 15.41,
including echomail, netmail, file requests (FREQ), magic file names,
TIC file echoes, the outbound queue, and troubleshooting stuck mail.

For the original Clark Development FidoNet documentation, see
`docs/fido/FIDO.DOC` (2,358 lines, covers the basics in detail).
This guide extends that with 15.41 additions and modern usage.

---

## 1. Architecture Overview

PCBoard's FidoNet subsystem is self-contained — it handles echomail
tossing, scanning, netmail, EMSI/WaZoo session negotiation, and
file requests internally. What it does NOT do is originate outbound
calls. That requires a frontend mailer.

```
Incoming:
  Remote node → BinkleyTerm XE (or binkd) → inbound packets (.PKT)
                                           → PCBoard PCBTOSS → message bases

Outgoing:
  PCBoard PCBMSG (scan) → outbound packets → outbound queue (PCBDSZ.LST)
                                            → BinkleyTerm XE → remote node

File Echoes:
  Remote node → BinkleyTerm XE → .TIC + files → pcbtic → file directories
```

### Key Files

| File             | Purpose                                    |
|---|---|
| PCBFIDO.CFG      | Master FidoNet configuration (binary)      |
| PCBDSZ.LST       | Outbound queue (files to send)             |
| FREQPATH.DAT     | FREQ directories (where FREQable files are)|
| MAGICNAM.DAT     | Magic name → real file mapping             |
| FREQDENY.DAT     | Nodes denied FREQ access                   |
| FREQ.DAT         | FREQ session limits (time/bytes/baud)      |
| NODELIST.NDX     | Compiled nodelist index                    |
| FIDORECV.LOG     | Inbound session log                        |
| FIDOSEND.LOG     | Outbound session log                       |

### Key Directories (configured in PCBSETUP → FidoNet → File & Directory)

| Directory        | Purpose                                    |
|---|---|
| Incoming Packets | Where inbound .PKT files arrive            |
| Outgoing Packets | Where outbound .PKT files are created      |
| Outgoing Netmail | Where outbound .MSG files are created      |
| Bad Packets      | Where unparseable packets are moved         |
| Nodelist Path    | Where raw NODELIST.### files are stored     |
| Work Directory   | Temporary workspace for packet processing  |
| Passthrough      | Echomail areas forwarded without storing   |
| Secure Mail      | Password-protected inbound area            |


## 2. Initial Setup

### Step 1: Get a FidoNet Node Number

Contact your local network coordinator (NC). You'll need:
- Your BBS name and location
- Your sysop name
- Your phone number or internet hostname
- Which echomail areas you want to carry

You'll receive an address like `1:105/23` (Zone:Net/Node).

### Step 2: Configure Your Address

PCBSETUP → FidoNet Configuration → System Address

Enter your primary address. You can add up to 30 AKAs (alternate
addresses) for other zones/nets you participate in.

### Step 3: Set Up Directories

PCBSETUP → FidoNet Configuration → File & Directory Configuration

Set all paths. Example layout:

```
C:\FIDO\INBOUND\       Incoming packets
C:\FIDO\OUTBOUND\      Outgoing packets
C:\FIDO\NETMAIL\       Outgoing .MSG files
C:\FIDO\BAD\           Bad/unparseable packets
C:\FIDO\NODELIST\      Raw nodelists
C:\FIDO\WORK\          Temp workspace
C:\FIDO\PASS\          Passthrough area
C:\FIDO\SECURE\        Secure inbound
```

### Step 4: Configure Nodes

PCBSETUP → FidoNet Configuration → Node Configuration

Add each FidoNet node you exchange mail with:
- Node address (zone:net/node)
- Packet password (must match both sides)
- Archive type for bundled mail (ZIP recommended)
- Phone number or BinkP hostname

### Step 5: Configure Areas

PCBSETUP → FidoNet Configuration → Tosser Configuration

Map echomail area tags to PCBoard conference numbers:

```
Area Tag          Conference #   Description
BBS_CARNIVAL      10             BBS discussion
FIDO_SYSOP        11             Sysop chat
PCB_ECHO          12             PCBoard support
```

### Step 6: Set Up Nodelist

Copy your NODELIST.### file to the nodelist directory, then
compile it:

```
PCBNLC\NODELIST\NODELIST.218 -o C:\FIDO\NODELIST\
```

This creates NODELIST.DBF and NODELIST.NDX.

Configure in PCBSETUP → FidoNet → Nodelist Configuration.

### Step 7: Install Frontend Mailer

**BinkleyTerm XE** (recommended, included):

Edit BINKLEY.CFG:
```
Address    1:105/23
StatusLog  C:\FIDO\BINKLEY.LOG
Inbound    C:\FIDO\INBOUND
Outbound   C:\FIDO\OUTBOUND
NetMail    C:\FIDO\NETMAIL
Nodelist   C:\FIDO\NODELIST\NODELIST
AfterMail  PCBTOSS.BAT
```

Or **binkd** for TCP/IP only (modern FidoNet):
```
domain fidonet C:\FIDO\OUTBOUND 1
address 1:105/23@fidonet
inbound C:\FIDO\INBOUND
node 1:105/1 your.hub.example.com:24554 password
```


## 3. Echomail Operations

### Tossing Inbound Mail

PCBoard's PCBTOSS processes inbound .PKT files automatically.
It runs via your mailer's AfterMail command:

```batch
@echo off
REM PCBTOSS.BAT — run after receiving mail
CD \PCB
PCBOARD /TOSS
```

PCBTOSS does:
1. Opens each .PKT in the incoming directory
2. Validates packet password against node configuration
3. For each message, matches the area tag to a conference
4. Imports the message into the PCBoard message base
5. Updates SEEN-BY lines (merge, not append)
6. Appends our address to PATH
7. Checks MSGID against dupe database (15.41)
8. Moves processed packets to work directory
9. Bad packets go to the bad packet directory

### Scanning Outbound Mail

PCBMSG scans PCBoard conferences for new echomail:

```batch
@echo off
REM PCBSCAN.BAT — run before calling out
CD \PCB
PCBOARD /SCAN
```

PCBMSG does:
1. Scans each echomail conference for new messages
2. Creates .PKT files with proper Type 2+ headers
3. Generates MSGID kludge for each message (15.41)
4. Sets REPLY kludge linking to parent MSGID (15.41)
5. Adds tearline: `--- PCBoard 15.41/OpenWatcom`
6. Adds origin line from per-area configuration
7. Builds SEEN-BY from node list
8. Initializes PATH with our address
9. Queues packets in outbound for each downlink

### Passthrough Areas (15.41)

Passthrough areas forward echomail to downlinks without storing
locally. Configure in PCBSETUP → Tosser → set Passthrough flag.
Messages are tossed from inbound, repacketed for each downlink,
and placed in the outbound — no conference storage.

### Dupe Detection (15.41)

PCBTOSS checks each message's MSGID kludge against a CRC-32 hash
database (DUPES.DAT). Default: 30,000 entries (~240KB circular
buffer). Configure size in PCBSETUP → Tosser → Dupe DB Size.

If a duplicate is detected, the message is logged and skipped.


## 4. Netmail

### Sending Netmail

A user (or sysop) writes a message in the netmail conference with
the recipient's FTN address in the To field:

```
To: John Smith @ 1:105/1
```

PCBMSG picks this up during scanning, creates a .MSG file in the
outgoing netmail directory, and queues it for the appropriate node.

### Netmail Routing

PCBoard routes netmail through the nodelist:
1. Direct — if you have a direct connection to the destination node
2. Via hub — if the destination is in a net you route through a hub
3. Via zone gate — for inter-zone netmail

Configure routing in PCBSETUP → Node Configuration.

### File Attach

Netmail with file attachments: the file path is stored in the .MSG
header's Subject field (FTS-0001 convention). The mailer picks up
the file and sends it during the next session.


## 5. File Requests (FREQ)

### How FREQ Works

During a FidoNet mail session, the remote node can request files.
The request arrives as a .REQ file containing one filename per line,
optionally followed by a password.

PCBoard's FREQ handler (FIDOFUNC.C) processes requests:

1. Reads the .REQ file
2. Checks if the requesting node is in the deny list (FREQDENY.DAT)
3. Checks baud rate against minimum (FREQ.DAT)
4. For each requested filename:
   a. First checks magic name list (MAGICNAM.DAT)
   b. If not a magic name, searches FREQ path list (FREQPATH.DAT)
   c. Validates password if required
   d. Checks session byte/time limits
   e. Queues matching files for sending

### Configuring FREQ Paths

PCBSETUP → FidoNet → FREQ Path List (menu item J)

Add directories containing files available for FREQ:

```
Path                         Password
C:\PCB\FILES\UTILS\          (none)
C:\PCB\FILES\BBS\            (none)
C:\FIDO\NODELIST\            (none)
C:\PCB\FILES\PRIVATE\        SECRET
```

Files in these directories can be requested by name.
Password-protected paths require the correct password in the
request: `MYFILE.ZIP !SECRET`

### Configuring Magic Names

PCBSETUP → FidoNet → FREQ Magic Names (menu item L)

Magic names are aliases that map to real files. The remote node
requests the magic name, and PCBoard sends the real file:

```
Magic Name    Real File Path                  Password
NODELIST      C:\FIDO\NODELIST\NODELIST.*     (none)
NODEDIFF      C:\FIDO\NODELIST\NODEDIFF.*     (none)
FILES         C:\PCB\GEN\FILES.BBS            (none)
ABOUT         C:\PCB\GEN\ABOUT.ASC            (none)
PCB154        C:\PCB\FILES\PCB154.ZIP         (none)
PRIVATE       C:\PCB\FILES\PRIV\SECRET.ZIP    MYPASS
```

Wildcards are supported — `NODELIST.*` sends the latest matching
file (e.g. NODELIST.218).

### Standard FidoNet Magic Names

Every FidoNet node is expected to support at minimum:

| Magic Name  | What It Returns                           |
|---|---|
| NODELIST    | Current full nodelist (NODELIST.Zcc)       |
| NODEDIFF    | Current nodelist diff (NODEDIFF.Zcc)       |

Common optional magics:
| Magic Name  | What It Returns                           |
|---|---|
| FILES       | Your file listing (FILES.BBS)             |
| ABOUT       | System description                        |
| FREQ        | List of FREQable files                    |
| HELP        | FREQ help/instructions                    |

### FREQ Restrictions

PCBSETUP → FidoNet → FREQ Restrictions (menu item K)

Add node addresses to deny FREQ access entirely.

### FREQ Session Limits

FREQ.DAT stores per-session limits:

| Field          | Meaning                                 |
|---|---|
| Session Time   | Max minutes for FREQ in one session     |
| Daily Time     | Max minutes for FREQ per day            |
| Session Bytes  | Max kilobytes per session               |
| Daily Bytes    | Max kilobytes per day                   |
| Listed Only    | Only allow FREQs from listed nodes      |
| Min Baud       | Minimum connection speed                |

### BinkleyTerm FREQ Support

BinkleyTerm XE has its own FREQ handling via OKFILE (b_frproc.c).
Configure in BINKLEY.CFG:

```
Okfile       C:\FIDO\OKFILE.CFG
MaxReq       10
MaxBytes     5000000
```

OKFILE format (one entry per line):
```
; Path-based: files in these directories are FREQable
C:\PCB\FILES\UTILS\*.*
C:\FIDO\NODELIST\*.*

; Magic names use prefix characters:
; @ = send all matching files
@NODELIST    C:\FIDO\NODELIST\NODELIST.* !PASSWORD
@NODEDIFF    C:\FIDO\NODELIST\NODEDIFF.*

; $ = execute command with net/node/point as args
$UPDATE      C:\FIDO\SCRIPTS\UPDATE.BAT

; + = execute command with request line as arg
+ALLFILES    C:\FIDO\SCRIPTS\ALLFILES.BAT

; * = search in request index file
*            C:\FIDO\BINKLEY.REQ
```

Note: PCBoard and BinkleyTerm both handle FREQs. When BinkleyTerm
is the session mailer, it handles FREQs via OKFILE. When PCBoard
handles the session directly (WaZoo/EMSI), it uses MAGICNAM.DAT
and FREQPATH.DAT. Configure BOTH for complete coverage.


## 6. TIC File Echoes (15.41)

File echo distribution uses TIC files (FTS-5006.001). The pcbtic
utility processes inbound TIC files:

```
pcbtic toss              Process all inbound .TIC files
pcbtic hatch FILE AREA   Create a new TIC for distribution
pcbtic list              Show configured file echo areas
```

### TIC Configuration

Edit pcbtic.cfg:
```
[paths]
inbound = C:\FIDO\INBOUND
outbound = C:\FIDO\OUTBOUND
ticwork = C:\FIDO\WORK

[areas]
BBS_UTILS = C:\PCB\FILES\UTILS     ; area tag = local directory
PCB_FILES = C:\PCB\FILES\PCB
NODELIST  = C:\FIDO\NODELIST

[nodes]
1:105/1 = BBS_UTILS PCB_FILES      ; downlinks per area
1:105/5 = BBS_UTILS
```

See docs/PCBTIC.md for full documentation.


## 7. Outbound Queue

### The Outbound Queue

PCBoard maintains an outbound queue in **PCBDSZ.LST** (defined as
`OUTBOUND_FILE` in DEFINES.H). This is the active queue used by
FIDOFUNC.C for the transfer protocol. A second queue class `cNEWQ`
(FIDOQUE.HPP) references FIDOQUE.DAT but is commented out in the
defines — it appears to be an unreleased replacement that Clark
was working on. The `cNEWQ` class provides richer queue management
(add/remove/poll/FREQ/scan/view) and is what the ALT-F option 5
"View/Modify Queue" uses at runtime.

```c
typedef struct {
    char  filename[80];       /* full path of file to send */
    char  nodestr[25];        /* destination FTN address */
    sint  flag;               /* attributes (see below) */
    sint  failedConnects;     /* failed connection count */
    bool  readOnly;           /* locked by event processing */
    char  reserved[18];
} QUEUE_RECORD;
```

Queue flags (from DEFINES.H):
```
Q_POLL     = 1024    File request queued
Q_FREQED   = 1024    FREQ'd file
```

The mailer reads PCBDSZ.LST to know what to send, and PCBoard
writes to it when scanning outbound mail, processing FREQs, or
when the sysop manually queues files.

### Existing Queue Editor (ALT-F → option 5)

PCBoard 15.4 already includes a "View/Modify Queue" option in
the ALT-F sysop FidoNet menu (option 5). This displays the
outbound queue and allows basic modification. The queue manager
class `cNEWQ` in FIDOQUE.HPP provides:

- `addEntry()` — add a file to the queue
- `removeEntry()` — remove by filename or slot number
- `addPoll()` / `removePoll()` — queue/dequeue node polls
- `addFREQ()` / `removeFREQ()` — queue/dequeue file requests
- `view()` — display the queue
- `scanQueue()` — scan for processable entries
- `forEachMatch()` — iterate entries for a specific node
- `isDupe()` — check for duplicate queue entries

### 15.41 Queue Editor Enhancements

Version 15.41 extends the existing View/Modify Queue with:

| Key | Function                                              |
|---|---|
| P   | Purge all entries where the file no longer exists      |
| B   | Purge stale .BSY lock files (older than 6 hours)      |
| R   | Reset failed connection count to 0 (retry)            |
| F   | Force immediate send (mark as Immediate priority)     |
| S   | Show queue statistics (total bytes, files per node)   |

### When Mail Gets Stuck

1. **Failed connections** — node is down, phone busy, BinkP timeout.
   The `failedConnects` field increments. After N failures, the
   mailer stops trying (configurable in BinkleyTerm via events).

2. **Missing files** — a queued file was deleted or moved.
   The queue record points to a file that no longer exists.

3. **Bad destination** — node number typo, node delisted from
   nodelist, or routing path broken.

4. **Stuck .PKT files** — packets created by PCBMSG but never
   picked up by the mailer, often due to directory mismatch.

5. **Stuck .TIC files** — TIC processing failed partway through,
   leaving orphaned files in the work directory.

6. **Lock files** — .BSY files left behind after a crash prevent
   the mailer from connecting to a node.

### Queue Editor

15.41 adds a sysop-accessible queue editor via the `FIDO STATUS`
command at the main prompt, or from PCBSETUP.

```
┌─── FidoNet Outbound Queue ─────────────────────────────────────────┐
│ #  Node          File                              Flags   Fails   │
│ 1  1:105/1       C:\FIDO\OUT\00690001.MO0          NORM     0     │
│ 2  1:105/1       C:\FIDO\OUT\00690001.PKT          NORM     0     │
│ 3  1:105/5       C:\FIDO\OUT\00690005.PKT          NORM     0     │
│ 4  1:203/0       C:\FIDO\OUT\00CB0000.PKT          NORM     3     │
│ 5  1:105/23      C:\PCB\FILES\UPDATE.ZIP            FREQ     0     │
│                                                                     │
│ [D]elete  [R]etry  [V]iew  [F]orce Send  [P]urge Missing  [Q]uit  │
└─────────────────────────────────────────────────────────────────────┘
```

Queue editor commands:

| Key | Function                                              |
|---|---|
| D   | Delete selected queue entry (with confirmation)       |
| R   | Reset failed connection count to 0 (retry)            |
| V   | View the file contents (for .PKT: show header info)   |
| F   | Force immediate send (mark as Immediate priority)     |
| P   | Purge all entries where the file no longer exists      |
| B   | Purge stale .BSY lock files (older than 6 hours)      |
| A   | Add a file to the queue manually                      |
| S   | Show queue statistics (total bytes, files per node)   |
| Q   | Quit editor                                           |

### Stuck Mail Recovery Procedures

**Procedure 1: Clear stale .BSY files**
```
FIDO STATUS → B (purge stale BSY files)
```
Or manually: delete any .BSY file in the outbound directory
older than 6 hours. These are crash leftovers.

**Procedure 2: Purge missing files**
```
FIDO STATUS → P (purge missing)
```
Removes queue entries pointing to files that no longer exist.

**Procedure 3: Retry failed nodes**
```
FIDO STATUS → highlight entry → R (retry)
```
Resets the failed connection counter so the mailer will try again.

**Procedure 4: Force toss/scan**
```
Command? FIDO TOSS     Force re-toss all inbound packets
Command? FIDO SCAN     Force re-scan all echomail conferences
```
Use when mail arrived but wasn't processed (e.g. after a crash).

**Procedure 5: Rebuild outbound**
If the queue is completely corrupt:
```
1. Stop the mailer
2. Delete PCBDSZ.LST
3. Run PCBOARD /SCAN to rebuild outbound packets
4. Restart the mailer
```

**Procedure 6: Stuck TIC files**
Check the pcbtic work directory for partial processing:
```
pcbtic list           Show area status
pcbtic toss           Re-attempt toss of pending TICs
```
Orphaned files in the work directory can be moved back to inbound
and re-processed.


## 8. Event Processing

PCBoard events can automate FidoNet operations:

```
EVENT 1   02:00   PCBTOSS.BAT      Process inbound mail
EVENT 2   02:05   PCBSCAN.BAT      Scan outbound mail
EVENT 3   02:10   NLCOMP.BAT       Compile new nodelist
EVENT 4   02:15   PCBTIC.BAT       Process TIC files
EVENT 5   06:00   PCBTOSS.BAT      Morning mail run
EVENT 6   18:00   PCBTOSS.BAT      Evening mail run
```

### Continuous Mail (CM nodes)

If your hub is a CM (Continuous Mail) node, you can toss/scan
more frequently or use BinkleyTerm's AfterMail to process
immediately after each session.


## 9. Troubleshooting

### No mail arriving

1. Check FIDORECV.LOG — did a session happen?
2. Check inbound directory — are .PKT files piling up untossed?
3. Check bad packet directory — are packets being rejected?
4. Verify packet passwords match between you and your hub
5. Run `FIDO TOSS` to force toss

### No mail going out

1. Check PCBDSZ.LST — are packets queued?
2. Run `FIDO SCAN` to force scan
3. Check outbound directory — are .PKT files present?
4. Check mailer log — is it connecting?
5. Check for stale .BSY files blocking connections
6. Use `FIDO STATUS` queue editor to check failed counts

### Duplicate messages

1. Check DUPES.DAT exists and is not corrupt
2. Increase dupe DB size if you carry high-volume areas
3. Verify SEEN-BY lines are being merged correctly
4. Check you're not tossing the same packets twice

### FREQ not working

1. Verify MAGICNAM.DAT has your magic name entries
2. Verify FREQPATH.DAT has your FREQ directories
3. Check FREQDENY.DAT isn't blocking the requesting node
4. Check FREQ.DAT session limits aren't too restrictive
5. Also configure BinkleyTerm's OKFILE if using BTXE as mailer
6. Check WZ_FREQ flag is set in EMSI capabilities

### Nodelist issues

1. Recompile: `PCBNLC.### -o C:\FIDO\NODELIST\`
2. Verify nodelist path in PCBSETUP matches
3. For user-facing lookup: `NL` command at main prompt


## 10. Quick Reference: PCBSETUP FidoNet Menu

```
A  Fido Configuration        General FidoNet enable/disable settings
B  Node Configuration        Add/edit FidoNet nodes and passwords
C  System Address            Your FTN address and up to 30 AKAs
D  EMSI Profile              Session negotiation profile
E  File & Directory Config   All FidoNet directory paths
F  Archiver Configuration    ZIP/ARJ/LZH for packet archiving
G  Phone Number Translation  Prefix/suffix for dialing
H  Nodelist Configuration    Nodelist path and compilation
I  FREQ Path List            Directories available for FREQ
J  FREQ Restrictions         Session/daily time/byte limits, baud
K  FREQ Magic Names          Magic name → real file mapping
L  FREQ Deny Nodelist        Deny FREQ by nodelist flags
```

Note: Echomail area tags are configured per-conference from the
third conference configuration screen in PCBSETUP (not from the
FidoNet menu). This includes area tag, AKA selection, origin
line, and high-ASCII handling.

### ALT-F Sysop FidoNet Menu (runtime, at call-waiting screen)

```
1) Poll a Node               Initiate call to a FidoNet node
2) Request a File             Send FREQ to a remote node
3) Transmit a File            Queue a file for sending
4) Force Next Call            Override schedule, call next node
5) View/Modify Queue          View/edit outbound queue (FIDOQUE.DAT)
6) Scan for Outbound Mail     Export new echomail to packets
7) Process Inbound Mail       Toss inbound .PKT files
8) Compile Nodelist           Shell out to nodelist compiler
9) Send Mail to a Node        Compose and send netmail
```

### 15.41 Additions to PCBSETUP

- Conference FidoNet screen: Dupe DB size, passthrough area flag
- Node Config: BinkP hostname, timeout, block size, MD5 auth
- Nodelist: User lookup enable, minimum security level
- General: Tearline text override, version string
