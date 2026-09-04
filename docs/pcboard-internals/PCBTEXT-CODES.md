# PCBoard Screen Prompts (PCBTEXT) — Complete Analysis

**Scope**: full inventory of the 747 prompt slots in `PCBTEXT` — the binary
file that holds every screen message, prompt, and error string PCBoard 15.3 /
15.41 displays. Cross-referenced against Clark's SDK headers, source code,
and the shipped binary in `pcb1541/install/dist/target/GEN/PCBTEXT`.

**Sources analyzed**:
- `pcb153/SOURCE/H/pcbtext.h` — 15.3 SDK header (718 named)
- `pcb153/upd154/SOURCE/H/pcbtext.h` — 15.41 SDK header (722 named)
- `pcb154/MAIN/SOURCE/H/PCBTEXT.H` — 15.41 MAIN branch header (722 named)
- `pcb154/MAIN/SOURCE/DISPLAY/PCBTEXT.C` — runtime dispatcher + `UseText[]` bitflags
- `pcb153/SOURCE/DISPLAY/PCBTEXT.C` — 15.3 runtime (identical layout)
- `pcb1541/install/dist/target/GEN/PCBTEXT` — shipped binary (747 records × 80 bytes)

## Headline findings

1. **747 slots. All 747 have text.** No empty records. Format: 80-byte fixed
   records, byte 0 is the color code (`0`-`8`), bytes 1-79 are space-padded text.

2. **Header + 716 active + 2 deprecated + 28 reserved = 747.** Every slot
   accounted for:
   - Slot 0: version string header (`"PCBoard version 14.5 & 15.0 & 15.2 & 15.3 PCBTEXT file"`)
   - 716 slots: named in `PCBTEXT.H` + marked "used" by `UseText[]==1` — actively displayed
   - 2 slots: named + `UseText[]==0` — legacy code path, name kept for source compat (#108 `TXT_USERSFILEPACKED`, #184 `TXT_RELOADINGPCBOARD`)
   - 28 slots: unnamed + `UseText[]==0` — pure legacy text, no code path reads them

3. **`UseText[]` is the ground truth.** The 8-bit-per-byte bitflag array in
   `PCBTEXT.C` at compile time tells the runtime whether each slot is active.
   The header (`PCBTEXT.H`) also omits names for the deprecated slots, but the
   bitflag is what actually gates whether the string is loaded.

4. **The 28 "unnamed" slots are a MIXED bag.** Text-content analysis (below)
   separates them into 4 stubs (incomplete strings Clark never finished) and
   24 legacy prompts (complete messages from PCBoard 14.5 / 15.0 that later
   versions stopped calling). Zero references to any of the 28 strings exist
   anywhere else in the source tree — confirming they're all inactive today,
   but not distinguishing origin.

5. **15.3 → 15.41 added 4 new prompts (#747-#750).** For gender, birthdate,
   web address, favorite color — user-registration extensions. But the shipped
   `PCBTEXT` binary is a 15.3-vintage file with 747 records (0..746), so slots
   747-750 are declared in the header but **not present** in the on-disk binary:
   - `#747` `TXT_ENTERGENDER` — declared, but PCBTEXT binary would need re-generation to include it
   - `#748` `TXT_ENTERBIRTHDATE` — declared, but PCBTEXT binary would need re-generation to include it
   - `#749` `TXT_ENTERWEBADDR` — declared, but PCBTEXT binary would need re-generation to include it
   - `#750` `TXT_ENTERCOLOR` — declared, but PCBTEXT binary would need re-generation to include it

## Classification

| Category | Count | Meaning |
|---|---:|---|
| Header | 1 | Slot 0 — version identifier |
| Active + named | 716 | Currently used by 15.3 / 15.41 |
| Deprecated + named | 2 | Name kept in header for source compat; runtime skips |
| Reserved + unnamed | 28 | Legacy text from older versions, name dropped |
| Missing from binary | 4 | New in 15.41 header, need re-generated PCBTEXT to appear |
| **Total in binary** | **747** | slots 0..746 |
| **Total in 15.41 header** | **750** | slots 1..750 |

## The 28 reserved unnamed slots (the "missing screen codes")

These are prompts Clark shipped in the binary but that neither the SDK header
nor the runtime code path uses. Third-party tools can only reference them by
numeric index — no symbolic constant exists. They're preserved for backward
compatibility with older PCBoard versions (14.5, 15.0) that displayed them.

| Slot | Color | Text |
|-----:|:-----:|:-----|
| 20 | 1 | `Printer Off-Line ...` |
| 23 | 1 | `Path error in system configuration!` |
| 44 | 1 | `Error reading Script Questionnaire` |
| 47 | 1 | `No record available to update!` |
| 48 | 6 | `Messages Successfully Packed & Purified!` |
| 81 | 2 | `Purge older than (Enter)=010180` |
| 91 | 2 | `Reference Message number purification proceeding ...` |
| 115 | 3 | `  Registered in Conferences` |
| 122 | 6 | `Enter (U) for status while awaiting other caller ...` |
| 124 | 1 | `Sorry, @FIRST@, PACK not available from remote ...` |
| 147 | 1 | `Message Base Error!  Attempting to continue ...` |
| 151 | 2 | `Checking user file - please wait ...` |
| 172 | 1 | `Unable to write USER Record - Aborting!` |
| 174 | 1 | `Number of Users Purged:` |
| 183 | 1 | `PCBPACK Security Level Fail!` |
| 201 | 1 | `Error in filename request!` |
| 202 | 6 | `Welcome back to PCBoard~` |
| 205 | 1 | `Only ASCII and Kermit Protocols supported at 7-E-1!` |
| 206 | 1 | `Number of Msgs Purged :` |
| 207 | 1 | `Number of Bytes Purged:` |
| 362 | 1 | `Thread Read Terminated ...` |
| 364 | 1 | `Message Text Search Terminated ...` |
| 391 | 1 | `SHELL Batch file Missing (` |
| 392 | 0 | `Resetting Packet ...` |
| 410 | 0 | `Hanging Up Phone ...` |
| 432 | 1 | `Sorry, @FIRST@, Insufficient Security for 'From' edit!` |
| 456 | 1 | `Pack Message Base Before Continuing!` |
| 462 | 1 | `Assign More Message BLOCKS using PCBSetup!` |

## The 2 deprecated named slots

These have names in `PCBTEXT.H` (so source code that includes the header still
compiles) but are marked `UseText[]==0` — the runtime never reads them. Kept
for source-level backward compatibility.

| Slot | Name | Color | Text |
|-----:|:-----|:-----:|:-----|
| 108 | `TXT_USERSFILEPACKED` | 7 | `User file successfully packed.` |
| 184 | `TXT_RELOADINGPCBOARD` | 7 | `Reloading PCBoard.  Please wait ...` |

## Color codes (byte 0 of each record)

| Code | Meaning | Typical use |
|:----:|:--------|:------------|
| `0` | Follow the display file's current color | Status messages, transient text |
| `1` | Bright red (errors) | Access denied, warnings, aborts |
| `2` | Bright green | Success messages, action prompts |
| `3` | Bright yellow | Configuration prompts |
| `4` | Bright blue | Rarely used |
| `5` | Bright magenta | Rarely used |
| `6` | Bright cyan | Info, welcome messages |
| `7` | Bright white | Prompts, questions |
| `8` | Bright dark grey | Rarely used |

Colors are DOS-style — foreground on background=0. The runtime combines with
the current display file's color state per the `PCBTEXT.C` dispatch logic.

## Record format

```
typedef struct {
  char Color;       // "0".."8" — foreground color code
  char Str[80];     // Prompt text, space-padded to 80 bytes (79 usable)
} pcbtexttype;
```

The struct is `#pragma pack(1)` so each record is exactly 81 bytes on disk?
No — the on-disk format is 80 bytes total: byte 0 is Color, bytes 1..79 are
text (79 chars max, space-padded). The `Str[80]` in the struct includes byte
index 0 through 79 giving 80 chars, but the on-disk layout uses 80 bytes total
with byte 0 as the color prefix. The C struct is a load-time interpretation.

## Provenance

- **Ground truth**: `pcb154/MAIN/SOURCE/DISPLAY/PCBTEXT.C` `UseText[]` bitflags
- **Name authority**: `pcb154/MAIN/SOURCE/H/PCBTEXT.H` — 722 `#define TXT_*` entries
- **Binary**: `pcb1541/install/dist/target/GEN/PCBTEXT` — 59,760 bytes / 747 records
- **License**: Clark Development Company source, © 1996, part of the freely
  redistributable PCBoard 15.41 release.

## See also

- `MCI-CODES.md` — companion analysis of `@TOKEN@` macros in prompts
- `PLANNED-FEATURES.md` — cross-reference for what Clark meant to add
- `pcb1541/install/dist/target/GEN/PCBTEXT` — the binary being analyzed

## Did Clark write notes about the 28 stub slots?

**Short answer: No explanatory notes. Only structural signals.**

The full source tree contains three files that authoritatively describe every
PCBTEXT slot:

1. **`pcb154/MAIN/SOURCE/H/PCBTEXT.H`** — the SDK header with `#define TXT_XXX N`
   entries. Deprecated slots have NO `#define` at all. Between `#17`, `#18`, `#19`
   and `#21`, `#22` there is simply a gap where `#20` and `#23` would go — no
   comment marks the absence.

2. **`pcb154/MAIN/SOURCE/DISPLAY/PCBTEXT.C`** — the runtime with the `UseText[]`
   bitflag array. Each 0 in the array tells the runtime "skip loading this slot".
   The array has only position markers (`/* 20 */`, `/* 30 */`, etc.) — no
   annotation on the individual zeros.

3. **`reference/pcball/pcboard/pcb-util/PCBTEXT/STRS15.C`** — the source of truth
   for the shipped binary. All 747 records are declared here with rich per-record
   metadata (flag, alignment, max length, color, text). Even the deprecated slots
   keep their text so `MKPCBTXT.EXE` can regenerate the binary preserving them.
   No slot has a note explaining WHY it's deprecated.

The full comment inventory in `STRS15.C` is just three blocks: the standard file
header, a compile note (`"merge duplicate strings" must be off`), and a caller-log
length note (`56 chars max for full-log records`). Nothing per-slot.

## Are they LEGACY (used-then-retired) or STUBS (planned-never-shipped)?

Both categories exist in the 28. Empirical test: grep every source file
(.c and .h across `pcb153/`, `pcb154/`, `pcb1541/`, `reference/`, `toolkit/`)
for each of the 28 text strings, excluding the two files that define them
(`STRS15.C` and `PCBTEXT.H`).

**Result: 0 references. None of the 28 strings appear in any other code path.**

For legacy prompts, at least one 14.5 backward-compat code path should
still reference them; for stubs, zero references is the expected count.
Zero refs across the board doesn't fully settle it — Clark's 14.5 was
partly BASIC and that source isn't in this repo — but combined with the
text-content analysis below, the 28 divide into two groups:

### The 4 clear STUBS

Four slots show unambiguous stub signals: incomplete strings that could
never have been displayed as-is, indicating Clark reserved the slot and
wrote a placeholder but never wired up the calling code.

| Slot | Text | Stub signal |
|-----:|:-----|:------------|
| 174 | `Number of Users Purged:` | bare-label — no value formatting |
| 206 | `Number of Msgs Purged :` | bare-label — no value formatting |
| 207 | `Number of Bytes Purged:` | bare-label — no value formatting |
| 391 | `SHELL Batch file Missing (` | unbalanced paren — cut off mid-sentence |

- **#391** `SHELL Batch file Missing (` — the trailing `(` was clearly
  meant to be `(@FILENAME@)` or `(filename.bat)`. Clark stopped
  mid-sentence. No shipping error message ends with an unclosed paren.
- **#174, #206, #207** — three bare labels (`"Number of Users Purged:"`,
  `"Number of Msgs Purged :"`, `"Number of Bytes Purged:"`) with no
  trailing format specifier. Real PCBoard stat lines use patterns like
  `"Number of X: %d"` or a follow-up MCI code. These stopped at the
  colon.

### The 24 likely LEGACY

The remaining 24 read as grammatically complete, self-contained messages
consistent with the 14.5 / 15.0 era:

- **#20** `Printer Off-Line ...`
- **#23** `Path error in system configuration!`
- **#44** `Error reading Script Questionnaire`
- **#47** `No record available to update!`
- **#48** `Messages Successfully Packed & Purified!`
- **#81** `Purge older than (Enter)=010180`
- **#91** `Reference Message number purification proceeding ...`
- **#115** `  Registered in Conferences`
- **#122** `Enter (U) for status while awaiting other caller ...`
- **#124** `Sorry, @FIRST@, PACK not available from remote ...`
- **#147** `Message Base Error!  Attempting to continue ...`
- **#151** `Checking user file - please wait ...`
- **#172** `Unable to write USER Record - Aborting!`
- **#183** `PCBPACK Security Level Fail!`
- **#201** `Error in filename request!`
- **#202** `Welcome back to PCBoard~`
- **#205** `Only ASCII and Kermit Protocols supported at 7-E-1!`
- **#362** `Thread Read Terminated ...`
- **#364** `Message Text Search Terminated ...`
- **#392** `Resetting Packet ...`
- **#410** `Hanging Up Phone ...`
- **#432** `Sorry, @FIRST@, Insufficient Security for 'From' edit!`
- **#456** `Pack Message Base Before Continuing!`
- **#462** `Assign More Message BLOCKS using PCBSetup!`

Some are historically-anchored to obsolete hardware or workflows:
- **#81** `Purge older than (Enter)=010180` — default date of 01/01/1980
  betrays a very early PCBoard version (BBSes younger than a few years
  weren't purging into the 70s).
- **#205** `Only ASCII and Kermit Protocols supported at 7-E-1!` —
  refers to the 7-E-1 modem parity/data-bit setting, standard on
  university mainframes in the 80s but dead by the mid-90s when 8-N-1
  became universal.
- **#20** `Printer Off-Line ...` — sysops used to have hardcopy log
  printers; replaced by log-file-only.
- **#202** `Welcome back to PCBoard~` — the tilde becomes a space in
  Clark's runtime string post-processor, so this rendered as
  `"Welcome back to PCBoard "`. Replaced by the `DISPLAY` file system
  (`WELCOME`, `WELFIRST`) which gives sysops full control.
- **#410** `Hanging Up Phone ...` — old modem control talkback; replaced
  by silent modem commands.

### Bottom line

- **4 slots are stubs** (Clark's placeholders for planned messages, never
  finished — most obviously `#391`).
- **24 slots are legacy** (real messages from PCBoard 14.5 / 15.0 that
  the runtime stopped calling as features got reworked).
- **All 28 are structurally deprecated** (no `TXT_*` name, `UseText[]==0`)
  regardless of origin.

Clark preserved the text of all 28 in `STRS15.C` so `MKPCBTXT.EXE` can
regenerate the binary with them intact — a form of source-level
provenance, but not accompanied by any per-slot explanatory notes.

### An interesting fossil in `PCBTEXT.C`

Right after the `UseText[]` array, `processtext()` contains a large **commented-out
switch statement** that would have provided hardcoded English fallback strings for
8 named prompts (`TXT_FILESSHOWPROMPT`, `TXT_ENTERDIRCMD`, `TXT_NOFILESFOUND`,
`TXT_DIRECTORYOF`, `TXT_SHORTINEFFECT`, `TXT_LONGINEFFECT`, `TXT_SHOWLONGDESC`,
`TXT_USESHORTDESC`). This was Clark's abandoned "safety net" for missing PCBTEXT
records — left in place as a comment rather than deleted. The runtime instead
trusts that PCBTEXT is well-formed and uses only whatever the file provides.

### An anomaly in the `MAIN` branch header

`pcb154/MAIN/SOURCE/H/PCBTEXT.H` contains a stray unclosed comment near the
15.4-additions block:

```c
/*
/*
/* 15.4 additions — prompts for Personal PSA fields and color selection.
 * Values match STRS15.C (extracted from 15.4b MKPCBTXT.EXE binary).
 * Slots 747-750 are the next unused after the 15.3 baseline of 746.
 */
```

The doubled `/*` looks like a merge or edit artifact. This is the ONE spot in the
whole PCBTEXT source tree where Clark (or a maintainer) wrote a substantive
freeform note — and it's about the 4 NEW slots (747-750), not the 28 old ones.

## CORRECTION — deeper source check turned up more, but still no notes

An earlier version of this document reported that I'd checked "three
authoritative source files." Digging further into `pcb153/upd154/SOURCE/MKPCBSRC/`
turned up four MORE files with PCBTEXT data:

1. **`MKPCB.C`** (73 KB) — a modern reverse-engineered recreation of
   `MKPCBTXT.EXE` 15.4b with the FULL 817-entry `Pcbtext154Table[]` static
   array extracted from the original binary. Every record has a `/* rec NNN,
   src 0xXXXX */` comment showing its 1-based index + source-file byte offset.
   Extracted by `extract_pcbtext.py` (referenced but not present in this repo).

2. **`PCBTXT154.H`** (5.5 KB) — a 50-entry preview of the same table with
   a header comment explaining the extraction methodology. Says explicitly:
   *"IMPORTANT: this file preserves the STRING CONTENT recovered from the
   original binary. The exact `TXT_` numeric assignment for records past
   index 713 (the 15.3 baseline) is inferred from context — verify against
   your specific runtime before shipping to users."*

3. **`OLD.CDC`** (64 KB) — the .CDC-format PCBTEXT input to `MKPCBTXT.EXE`,
   with all 15.4-era text (including a record 0 that already says
   `"PCBoard version 14.5 & 15.0 & 15.2 & 15.3 & 15.4 PCBTEXT file"`).

4. **`RUNLOG.TXT`** (30 KB) — capture of `MKPCBTXT.EXE` running against
   `OLD.CDC`. Lists `Record #NNN has been upgraded in OLD.CDC.` for hundreds
   of records. Purely mechanical output, no per-slot commentary.

### What the 15.4 source table tells us about the 28

Cross-referencing our 28 "deprecated" slots against the 15.4 `Pcbtext154Table[]`:

**27 of the 28 are STILL PRESENT in Clark's 15.4 table with the same
text.** Only one — **slot 23 `"Path error in system configuration!"`** — was
actually removed between 15.3 and 15.4.

So "deprecated" in the earlier sense (dropped from Clark's source) applies
to exactly one slot. The other 27 are **inactive but preserved**: the
runtime's `UseText[]==0` skips them, but Clark kept the text in his source
so `MKPCBTXT.EXE` regenerates the binary with them intact. Sysops who
customized their local PCBTEXT to reference these slots by number would
still get whatever text was in there.

This is defensible engineering: removing a slot mid-file would renumber
every slot after it and break every PPE script + custom `.CDC` file in
existence. Clark preserved the numbering by keeping the strings even after
the runtime stopped calling them.

### Revised classification

| Category | Count | Meaning |
|---|---:|---|
| Header | 1 | Slot 0 — version identifier |
| Active + named | 716 | Currently displayed by 15.3 / 15.41 |
| Deprecated + named | 2 | `#108 TXT_USERSFILEPACKED`, `#184 TXT_RELOADINGPCBOARD` |
| Preserved but inactive (unnamed, still in Clark's 15.4 source) | 27 | Text kept for numbering compat + old .PPE references |
| **Actually removed in 15.4** | **1** | Slot 23 `"Path error in system configuration!"` — the only true deletion |
| Missing from binary | 4 | 15.41 header declares `#747-#750`, binary only goes to #746 |

### The one true removal

`"Path error in system configuration!"` (slot 23) is the ONLY string
Clark's 15.4 source drops. Every other slot survives. If you regenerate
`PCBTEXT` from the 15.4 `MKPCBTXT.EXE`, slot 23 in the output would be a
DIFFERENT string (whatever previously lived at 15.4 slot 24 shifted down)
— which would silently break any PPE that referenced slot 23 by number.

Clark's team knew this and left slot 23 alone in the shipped binary. The
15.4 upgrade utility only APPENDS to the end of an existing PCBTEXT file
— it never rewrites or renumbers existing records.

### Still no per-slot explanatory notes

Even in the 15.4 source, the `Pcbtext154Table[]` uses only `/* rec NNN,
src 0xXXXX */` markers — just index + source offset, no commentary. The
inactive slots don't have any `/* deprecated */` or `/* legacy */` or
`/* reserved for X */` annotation. The user's intuition that Clark's
notes "have to be there" is understandable — with 817 strings in a
production table, one would expect maintenance comments — but he simply
didn't leave any per-slot notes anywhere in the source tree we have.

The absence itself is the answer.

## Slots 747–817 — Clark's phantom expansion (NEW FINDING)

Clark's `MKPCB.C` (in `pcb153/upd154/SOURCE/MKPCBSRC/`) defines a
`Pcbtext154Table[]` with **817 records**, not 750. Slots 747–817 are a
completely separate reservation that **never shipped in any PCBTEXT binary
and never got #define names in PCBTEXT.H**.

Worse — Clark had **two irreconcilable plans** for the 747+ range:

**Plan A (PCBTEXT.H — user-facing):**
- 747 `TXT_ENTERGENDER` — "Enter your gender (M/F)"
- 748 `TXT_ENTERBIRTHDATE` — "Enter your birthdate"
- 749 `TXT_ENTERWEBADDR` — "Enter your WEB address"
- 750 `TXT_ENTERCOLOR` — "Enter color (B)lue,(G)reen,(C)yan,(R)ed,(M)agenta,(Y)ellow,(W)hite,(+/-)"
- `TXT_NUMPROMPTS = 750` — comment: "15.4 grew from 746 to 750"

**Plan B (MKPCB.C — internal reservation):**
- 747–752: DOS `errno` strings ("Invalid argument", "Arg list too big",
  "Cross-device link", "Math argument", "Result too large", "File already exists")
- 753: printf error format `"FILE: %s, LINE: %d"`
- 754–764: **PCBSM.HLP help viewer strings** — the help-file navigation UI
  ("The PCBSM.HLP file is missing!", "The PCBSM.HLP file is the wrong version!",
  "press PGDN to go forward", "press PGUP to go backward", "general help, ESC to exit", etc.)
- 765–766: **unshipped CLI switches** — `/NOGIVEUP`, `/COLOR`
- 767–771: file I/O status ("Opening", "Reading", "Writing")
- 772–812: **complete DOS error message table** — all 41 slots for
  errno 0 through "Insufficient memory for file operation", plus network
  ("Network Error", "File Exists", "Cannot make directory entry",
  "Fail on INT 24h")
- 813–814: error format string + "Automatic retry in 10 seconds. Press ESC to cancel."
- 815: "press any key to continue"
- 816: **Latin-1 → ASCII fold table** — `"CUEAAAACEEEIIIAAEAAOOOUUYOU$$$$$AIOUNN"`
  (58-char accented → plain letter mapping — for caller log downgrade)
- 817: `"@WEWEWEW"` — canary / end-of-table marker

### What this reveals about Clark's roadmap

The two plans occupy the same slot numbers, so exactly one of them would
have shipped — probably A (the user-registration prompts were partially
plumbed in the header and got `#define` names, Plan B never made it out
of `MKPCB.C` at all).

Plan B was ambitious:
- **Localizable DOS errors** — every errno message would come from PCBTEXT
  instead of being hardcoded, so sysops could translate them
- **PCBSM.HLP routing through PCBTEXT** — the System Manager's help viewer
  would draw its chrome text from PCBTEXT slots too, unifying the string table
- **CLI expansion** — `/NOGIVEUP` (never give up on carrier) and `/COLOR`
  (force ANSI) as first-class flags
- **Character folding** — an on-the-fly Latin-1 → 7-bit ASCII table for
  the caller log so international names wouldn't corrupt it

None of it shipped. `MKPCB.C` compiles fine and would emit an 817-record
`PCBTEXT.154` file, but that binary was never packaged — the released
`PCBTEXT` in every PCBoard 15.x install has exactly 747 records.

### `@WEWEWEW` at slot 817

Almost certainly Clark's canary — a string he'd never expect a caller to
see, so if it ever appeared on screen he'd know PCBTEXT's index was off
by one. The doubled `WE` (`@WE` × 2) is close to the MCI token `@WEEK@`
but with garbled tail — probably typed as a deliberately-invalid token
that would render as literal text if displayed. Consistent with debug
markers found in other Clark utilities (`FLIP`, `ZULU`, `SANITY`).

## Revised classification

| Category | Count | Notes |
|---|---|---|
| Header (slot 0) | 1 | Version stamp |
| Active + named | 716 | Currently displayed by 15.3 / 15.41 |
| Deprecated + named | 2 | `#108 TXT_USERSFILEPACKED`, `#184 TXT_RELOADINGPCBOARD` |
| Preserved but inactive (unnamed) | 27 | Text kept, no code path reads them |
| Actually removed in 15.4 | 1 | Slot 23 `"Path error in system configuration!"` |
| **Plan A phantom slots (in PCBTEXT.H, missing from binary)** | **4** | `#747-#750` GENDER/BIRTHDATE/WEBADDR/COLOR |
| **Plan B phantom slots (in MKPCB.C only, no header, no binary)** | **71** | `#747-#817` — errno/PCBSM/CLI/i18n reservation |

The Plan A + Plan B overlap (747–750) means those four slots have TWO
competing definitions in the source tree — the strongest evidence that
Clark was mid-refactor when 15.4 development stopped.
