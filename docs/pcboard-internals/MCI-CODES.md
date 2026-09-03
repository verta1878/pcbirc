# PCBoard MCI Codes — Complete Analysis

**Scope**: full inventory of PCBoard 15.3 / 15.41 MCI codes (the `@TOKEN@`
macros processed by `PCBMACRO.C` in the display subsystem). Rebuilt from
source after the previous list was lost.

**Sources analyzed**:
- `pcb153/SOURCE/H/PCBMACRO.H` — 15.3 enum
- `pcb153/upd154/SOURCE/H/PCBMACRO.H` — 15.41 enum
- `pcb153/SOURCE/DISPLAY/PCBMACRO.C` — 15.3 string table
- `pcb153/upd154/SOURCE/DISPLAY/PCBMACRO.C` — 15.41 string table
- `docs/PCBoard_154_Reference.txt` — sysop-facing docs
- `pcb1541/install/dist/target/PPL/*.PPS` — sample scripts
- `pcb1541/install/dist/target/GEN/*` — sample display files

## Headline findings

1. **The enum is FROZEN between 15.3 and 15.41.** 107 tokens in both;
   zero added, zero removed. No new MCI codes shipped in the 15.41
   update.
2. **5 aliases exist in the string table** — same enum slot reachable
   via two spellings (short user-facing + long symbolic).
3. **4 MCI codes appear in docs but have NO enum entry** — these are
   the planned-but-never-added codes. Documented as if they exist;
   PCBMACRO would reject them as unknown.
4. **2 MCI codes appear as documented but are handled as SPECIAL CASES**
   outside the enum dispatch — hardcoded string matches in other
   modules.

## The 107 canonical enum tokens (15.3 = 15.41)

Grouped by domain for readability. Slot number = enum position.

### Session control & I/O (12)

| Slot | Token       | String   | Purpose |
|-----:|:------------|:---------|:--------|
| 0    | SKIP        | ""       | Sentinel (empty slot) |
| 2    | AUTOMORE    | AUTOMORE | Auto-scroll toggle |
| 3    | BEEP        | BEEP     | Terminal bell |
| 13   | CLREOL      | CLREOL   | ANSI clear-to-end-of-line |
| 14   | CLS         | CLS      | Clear screen |
| 24   | DELAY       | DELAY    | Pause N seconds |
| 58   | MORE        | MORE     | Show "More? (Y/n)" prompt |
| 69   | PAUSE       | PAUSE    | Wait for keypress |
| 70   | POFF        | POFF     | Turn OFF page-pause |
| 71   | PON         | PON      | Turn ON page-pause |
| 100  | WAIT        | WAIT     | Wait for user keypress |
| 105  | XOFF        | XOFF     | Turn OFF processing |

### User identity (11)

USERNAME, REALNAME, INAME, FIRST, FIRSTU, ALIAS, CITY, HOMEPHONE,
DATAPHONE, SECLEVEL, EXPDATE

### Session statistics (14)

MINLEFT, TIMELEFT, TIMEUSED, TIMELIMIT, TOTALTIME, LOGDATE, LOGTIME,
LASTDATEON, LASTTIMEON, NUMTIMESON, NUMCALLS, KBLEFT, KBLIMIT,
MAXBYTES

### File transfer stats (14)

DLBYTES, DLFILES, UPBYTES, UPFILES, DAYBYTES, FBYTES, FFILES, RBYTES,
RFILES, SBYTES, SFILES, BYTELIMIT, BYTESLEFT, MAXFILES

### Ratios & credits (11)

BYTECREDIT, FILECREDIT, BYTERATIO, FILERATIO, RATIOBYTES, RATIOFILES,
CREDLEFT, CREDNOW, CREDSTART, CREDUSED, PWXDATE

### Conference / directory (7)

CONFNAME, CONFNUM, INCONF, NUMCONF, DIRNAME, DIRNUM, NUMDIR

### Messages (7)

CURMSGNUM, HIGHMSGNUM, LOWMSGNUM, MSGLEFT, MSGREAD, LMR, FNUM

### System / node (14)

BOARDNAME, NODE, EVENT, OFFHOURS, SYSDATE, SYSTIME, SYSOPIN, SYSOPOUT,
FREESPACE, NUMBLT, LASTCALLERNODE, LASTCALLERSYSTEM, HIGHMSGNUM,
POS

### Modem / connection (5)

BPS, BICPS, RCPS, SCPS, CARRIER

### Protocol descriptors (2)

PROLTR, PRODESC

### Environment (3)

ENV (table string `ENV=` — takes variable name), OPTEXT, XCOLORS

### Y/N prompt characters (2)

YESCHAR, NOCHAR

### Utility (7)

FBYTES, EXPDAYS, PWXDAYS, POS, QOFF, QON, WHO

## The 5 aliases (dual-name entries in Table[])

The string table has 5 entries whose names differ from their enum
symbolic name. Both spellings work in display files.

| Enum symbol   | Table string | Notes |
|:--------------|:-------------|:------|
| USERNAME      | USER         | `@USER@` is documented; enum uses long name |
| REALNAME      | REAL         | `@REAL@` = full real name |
| SECLEVEL      | SECURITY     | `@SECURITY@` = user security level |
| ENV           | `ENV=`       | Takes variable name: `@ENV=PATH@` |
| (sentinel)    | LASTENTRY    | End-of-table marker, not an MCI code |

## The 4 documented-but-never-implemented codes

These appear in `docs/PCBoard_154_Reference.txt` and/or elsewhere as
if they were real MCI codes, but **have no enum entry and no
PCBMACRO dispatch**. Best guess: planned features that Clark
documented ahead of implementation and never shipped.

| Code         | Where documented | Best-guess intent |
|:-------------|:-----------------|:------------------|
| `@LOGIN@`    | Reference guide  | Insert user's login name (distinct from USERNAME/ALIAS) — likely superseded by USERNAME before implementation |
| `@FILENAME@` | Reference guide  | Insert the current file's name in file-list displays — likely intended for `FILE_ID.DIZ` / description contexts |
| `@VARIABLE@` | Reference guide  | Generic named-variable substitution (like `@ENV=` but for internal state) |
| `@GK@`       | Display files (`GEN/` samples) | "Get Key" — pause and read a single keypress. Referenced in display markup but no dispatcher slot |

**Not-a-real-code confirmation**: none of these appears in
`PCBMACRO.H` enum, none in `PCBMACRO.C` string table, none in
`findtoken()` dispatch. If a display file contains one of these,
PCBMACRO returns the SKIP token (0) — the macro passes through as
literal `@LOGIN@` text on screen.

## The 2 special-case tokens (bypass the enum)

These are documented `@X@`-style codes but handled by string match in
other subsystems, not routed through PCBMACRO's enum dispatch:

| Code       | Handler location | Purpose |
|:-----------|:-----------------|:--------|
| `@HANGUP@` | `SOURCE/MAIN/SCRIPT.C:211` + `SOURCE/DISPLAY/FILES.C:1253` | Terminate session immediately. Two independent hardcoded matches. |
| `@ECHO@`   | `SOURCE/NODE/NEWCHAT.C:150` (chat command table, slot O_ECHO) | Chat subsystem echo toggle, not a display macro |

`@HANGUP@` handling is duplicated across two modules — likely because
it needs to trigger from both display-file processing (mid-screen)
and script execution (out-of-band). The macro subsystem's
`findtoken()` never sees it.

## Not counted here — the `@X` color codes

`@X<HH>@` where `<HH>` is two hex digits (background nibble, foreground
nibble) is PCBoard's ANSI color prefix. Handled separately from the
MCI enum by the ANSI translator. 256 possible codes (`@X00@` through
`@XFF@`). Not part of the "planned but missing" analysis — they're a
distinct namespace.

## IRC-branch chat commands (unrelated to MCI)

`toolkit/irc1541/` and `SOURCE/NODE/NEWCHAT.C` define 18 chat
commands (BROADCAST, BYE, CALL, CHANNEL, ECHO, IGNORE, HANDLE, MENU,
MONITOR, NOECHO, QUIT, PRIVATE, PUBLIC, SEND, SHOW, SILENT, TOPIC,
WHO). These are runtime chat verbs, not display-file macros. Naming
overlap with MCI codes (`ECHO`, `WHO`) is coincidental.

## What "Clark was going to add" actually means

Best interpretation, based on evidence:

- **4 real gaps**: `@LOGIN@`, `@FILENAME@`, `@VARIABLE@`, `@GK@` —
  documented as MCI codes but no dispatch. Either shipped-then-removed
  or documented-ahead-of-implementation-then-forgotten.
- **NOT** "codes 15.41 added over 15.3" — the enums are byte-identical.
  There's no version-delta signal on the MCI namespace.
- **NOT** "codes hidden behind #ifdef" — no future-feature guards
  found in PCBMACRO source.

The 4 planned codes are candidates for our own implementation if we
want to complete Clark's documented spec — they're low-risk (one enum
slot + one table entry + one dispatch clause each). Non-goal until
after byte-exact reconstruction of the shipped binary is achieved.

## See also

For the full inventory of 15.4 additions across all subsystems (PPL
tokens, PPL statements, PPL user variables, PCBTEXT prompts, chat
commands, PCBIC verbs, and the ListServ PPE that never shipped), see
[`PLANNED-FEATURES.md`](PLANNED-FEATURES.md).

## Provenance

Analysis performed 2026-09-03 during install v1.9+ planning after
prior "missing MCI codes" list was lost from earlier session. Full
enum extraction + docs cross-reference + special-case search done
programmatically; findings verified manually against source.
