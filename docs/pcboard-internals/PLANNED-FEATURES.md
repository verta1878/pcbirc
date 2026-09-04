# PCBoard — Planned & Late-Added Features (source-derived inventory)

**Scope**: things Clark documented or partially implemented that either
never shipped, shipped without documentation, or shipped in the 15.4
update over the 15.3 baseline. Rebuilt from source diffs and doc
cross-reference after the previous list was lost.

**See also**:
- `MCI-CODES.md` — the display-macro (`@TOKEN@`) namespace
- `pcb153/upd154/docs/WHATSNEW` — Clark's official 15.4 whatsnew
- `docs/SYSOP_154.TXT` — my prior sysop guide covering the 15.4 additions

## Category summary

| Category | 15.4 added (in 15.4, not in 15.3) | Docs-only / never implemented |
|:---------|:----------------------------------:|:------------------------------:|
| MCI display codes  | 0 | 4 (see MCI-CODES.md) |
| PPL functions      | 5 | — |
| PPL statements     | 3 | — |
| PPL user variables | 5 | — |
| PCBTEXT prompts    | 4 | — |
| PCBTEXT Plan A (in `PCBTEXT.H`, missing from binary) | — | 4 (`#747-#750` GENDER/BIRTHDATE/WEBADDR/COLOR) |
| PCBTEXT Plan B (in `MKPCB.C` only) | — | 71 (`#747-#817` DOS errno / PCBSM help / CLI switches / Latin-1 fold) |
| PPE add-ons        | — | 1 (ListServ) |
| CHAT commands      | 1 (COLOR) + N action-cmds | — |
| PCBIC FTP verbs    | 1 (MGET) | — |
| COMMDRV card types | — | 3 (see PCBDCOM-CARDS.md) |

**Cross-reference**: `PCBTEXT-CODES.md` §"Slots 747–817 — Clark's phantom
expansion" documents the two competing 747+ reservations. Plan A and
Plan B collide on slots 747–750 — the strongest single piece of evidence
that 15.4 was abandoned mid-refactor.

## PPL 3.40 additions (15.4 over 15.3)

### 5 new function tokens (expressions)

Extracted from `pcb153/upd154/SOURCE/H/NEWSCR.HPP` diff against 15.3
baseline. All confirmed by RE of compiled `PPLC.EXE` 15.4b token
table (annotations in the header cite the specific file offsets).

| Token           | ID    | Signature                    | Purpose |
|:----------------|:------|:-----------------------------|:--------|
| GETBANKVAL      | -287  | `GETBANKVAL(idx)` → INTEGER  | Read Time Bank PSA field (0-11) |
| GETMSGHDR       | -288  | `GETMSGHDR(msg, field)` → STRING | Read message header field |
| SETMSGHDR       | -289  | `SETMSGHDR(m, f, v)` → (side-effect) | Write message header field |
| SHORTDESC       | -290  | `SHORTDESC(x)` → BOOLEAN     | Read caller's short-desc mode |
| U_PERSONAL      | -291  | `U_PERSONAL(idx)` → STRING   | Indexed Personal-PSA accessor (1=gender, 2=birthdate, 3=email, 4=web) |

`MAX_OP_TOK` bumped from 286 → 291 (+5).

### 3 new statement tokens

| Token             | ID  | Syntax                     | Purpose |
|:------------------|:----|:---------------------------|:--------|
| MOVEMSG           | 227 | `MOVEMSG msgnum destconf`  | Move a message between conferences |
| SETBANKVAL        | 228 | `SETBANKVAL idx value`     | Write Time Bank PSA field |
| SHORTDESCSTMT     | 229 | `SHORTDESC value`          | Set caller's short-desc mode (statement form of the function) |

### 5 new user-variable aliases

Registered by `SCRCOMP.CPP:4964-4965` — direct read/write into the
Personal PSA (silent no-op if PersonalSupport is FALSE):

| Variable      | Type    | Notes |
|:--------------|:--------|:------|
| U_SHORTDESC   | BOOLEAN | Same slot as SHORTDESC() function |
| U_GENDER      | STRING  | = `U_PERSONAL(1)` |
| U_BIRTHDATE   | STRING  | = `U_PERSONAL(2)` |
| U_EMAIL       | STRING  | = `U_PERSONAL(3)` |
| U_WEB         | STRING  | = `U_PERSONAL(4)` |

### 4 new PCBTEXT prompts (records 747-750)

Prompts added to the new-user registration flow for Personal PSA
fields:

| Record | Symbol            | Purpose |
|:-------|:------------------|:--------|
| 747    | TXT_ENTERGENDER   | "Enter your gender" prompt |
| 748    | TXT_ENTERBIRTHDATE| "Enter your birthdate" prompt |
| 749    | TXT_ENTEREMAIL    | "Enter your email address" prompt |
| 750    | TXT_ENTERWEB      | "Enter your web/home page URL" prompt |

Called from `MAIN/SOURCE/NODE/LOGIN.C`. Adding these requires the
PCBTEXT.CDC upgrade in `docs/SYSOP_154.TXT` §3.2.

## Non-PPL additions in 15.4

### CHAT enhancements

Per `pcb153/upd154/docs/WHATSNEW` §3:

- **`@X<HH>@` color codes in chat text** — users can now embed color
  in chat messages.
- **COLOR command** — foreground color change:
  - `B G C R M Y W` (blue/green/cyan/red/magenta/yellow/white)
  - `+` brighten, `-` darken (also alter default intensity)
  - Example: `W+` = bright white, `R-` = dark red
- **Sysop-definable action commands** — the SysOp can now define
  their own set of action commands for the CHAT subsystem, tailored
  to their users. Extension mechanism, not a fixed command list.

### PCBIC (InterCom) — 1 new FTP verb

Per WHATSNEW §4:

- **MGET** — FTP wildcard file retrieval. Extends the FTP command
  set inside Pcbic.exe.

### Time Bank PSA (12 fields)

Not a "code" per se, but the on-disk feature backing GETBANKVAL /
SETBANKVAL. 12 fields covering time (0-5, minutes) and byte-transfer
(6-11, KB) accumulation. Full field breakdown in `docs/SYSOP_154.TXT`
§4.4.

## Documented-but-never-shipped

### ListServ PPE

`pcb153/upd154/docs/WHATSNEW:53`:

> "A ListServ PPE (coming soon) which allows your users to subscribe
>  to mailing lists."

**Status**: never shipped as far as any recovered material shows. Not
in `target/PPL/`, not in `reference/roysac/`, not in `pcball/pcboard/`.
Truly a "planned but not delivered" feature.

**Best-guess spec** (from context clues in WHATSNEW around the entry):
- User-facing subscribe/unsubscribe commands
- Message forwarding to subscribers
- Would have leveraged the new email support (PSA email field)
- Might have relied on GETMSGHDR/SETMSGHDR + MOVEMSG new tokens

### MCI codes (4)

Cross-reference to `MCI-CODES.md` — documented in Reference guide,
no enum entry, no dispatch:

- `@LOGIN@`
- `@FILENAME@`
- `@VARIABLE@`
- `@GK@`

## What is NOT in the "planned" bucket

To avoid future confusion:

- **The `@X<HH>@` color code namespace** — this is 256 codes but
  it's the ANSI color prefix, not a planned-feature list.
- **CHAT's 18 built-in verbs** (BROADCAST/BYE/CALL/CHANNEL/ECHO/
  IGNORE/HANDLE/MENU/MONITOR/NOECHO/QUIT/PRIVATE/PUBLIC/SEND/SHOW/
  SILENT/TOPIC/WHO) — these are runtime chat commands, shipped and
  working since long before 15.4.
- **INSTALL.DAT's 250+ @Commands** — Clark's installer script
  language; ~40 are used by PCBoard's installer, the rest are for
  other Clark products. Not planned-but-unshipped, just unused by
  this product.
- **The 5 aliases in PCBMACRO string table** (USER/REAL/SECURITY/
  ENV=/LASTENTRY) — deliberately dual-named, both spellings work.



## Hidden PPL statements — found via decompiler cross-reference

While reverse-engineering PPL bytecode with the two decompilers we
have (`pcb1541/PPL/ppld/` — Chicken's original T4F-era decompiler,
and `pcb1541/PPL/pplengine/` — the modern Rust reimplementation),
these statement slots exist in the compiled `.PPE` opcode space but
Chicken's PPLD marked as `"???"` in `NAMES.INC` because he couldn't
recover the names.

The modern Rust engine (`icy_ppe`) has since resolved them from
newer analysis of `PPLC.EXE`. These are **compiler-internal
statements** — Clark's PPLC emits them during compilation of
`PROCEDURE`/`FUNCTION` definitions but user-facing docs never
expose them (they'd confuse users, since they exist as bytecode
markers rather than writable statements).

All originate in PPL 2.0 (version marker `200`) — the era when
PPL gained user-defined procedures and functions.

### The 9 previously-nameless statement slots

| Slot | icy_ppe name | PPLD had | Role in the .PPE runtime |
|-----:|:-------------|:---------|:-------------------------|
| 165  | DECLARE      | `???`    | Function/procedure forward declaration marker |
| 166  | FUNCTION     | `???`    | Function definition opener |
| 167  | PROCEDURE    | `???`    | Procedure definition opener |
| 168  | PCALL        | `PROC`   | Procedure call opcode (Chicken guessed "PROC" — actually PCALL) |
| 169  | FPCLR        | `ENDPROC`| Function parameter clear (Chicken guessed "ENDPROC" — actually FPCLR) |
| 170  | BEGIN        | `???`    | Block-begin marker (structured control) |
| 171  | FEND         | `ENDFUNC`| Function end marker (Chicken's guess "ENDFUNC" was close) |
| 172  | STATIC       | `???`    | Static-scope variable declaration |
| 173  | STACKABORT   | `STACKABORT` | Stack-abort — the one PPLD *did* correctly identify at the end of the ??? run |

Chicken guessed 3 of the 9 (PROC, ENDPROC, ENDFUNC) — 2 wrong (should
be PCALL, FPCLR) and 1 close (ENDFUNC vs FEND). Modern `icy_ppe`
resolved all 9 accurately by cross-referencing multiple `PPLC.EXE`
versions.

**Practical consequence**: these are NOT tokens you can write in a
`.PPS` source file — the parser doesn't accept them. They're what
the compiler EMITS when it sees `PROCEDURE foo(...)`, `FUNCTION
bar(...)`, `DECLARE PROCEDURE`, `BEGIN`, and static-scope variables.
A hand-crafted `.PPE` COULD include them and would execute; PPL
source cannot.

**Why this matters for reconstruction**:

1. A byte-exact PPLC clean-room needs to emit these opcodes correctly
   when compiling procedure/function definitions.
2. A byte-exact PPLD needs to recognize them and decompile back to
   `PROCEDURE`/`FUNCTION` source constructs (not literal opcode
   names, which would produce invalid `.PPS`).
3. The RUNINET.PPE byte-exact rebuild effort under
   `pcb1541/pcbic/RECONSTRUCTION.md` benefits from this — some of
   the source-drift gap between our 2,261 byte build and Clark's
   1,808 byte target may be procedure/function opcode emission
   differences.

## PPL opcode version markers — full inventory

Each opcode in the modern `icy_ppe` table carries a version marker
indicating which PPL version introduced it. This gives us a complete
version-added map:

### Statements (229 total)

| Version | Era        | Count | Notable additions |
|:--------|:-----------|------:|:------------------|
| 100     | PPL 1.0    | 115   | Base statements: PRINT, IF, GOTO, GOSUB, INPUT, file I/O, user I/O |
| 200     | PPL 2.0    | 62    | Procedures/functions (DECLARE/FUNCTION/PROCEDURE/PCALL/FPCLR/BEGIN/FEND/STATIC/STACKABORT), file positioning (FSEEK/FFLUSH/FREAD/FWRITE), TPA (thread-private area), sorting, search |
| 300     | PPL 3.0    | 54    | DBase suite (DCREATE/DOPEN/DCLOSE/DLOCK/DNEW/DADD/DAPPEND/DTOP/DSKIP/DFCOPY), FidoNet ops (FDOWRAKA/FDOADDAKA/FDOWRORG/FDOQMOD), account/QWK/EVAL |
| 340     | PPL 3.40 (**15.4**) | 3 | ShortDesc, MoveMsg, SetBankBal |

### Functions / expressions (294 total, ID space -1 to -290)

| Version | Era        | Count | Notable additions |
|:--------|:-----------|------:|:------------------|
| 100     | PPL 1.0    | 165   | Arithmetic, string ops, user info, date/time, DOS interrupts (REGAL..REGES), file inspection |
| 200     | PPL 2.0    | 52    | Type conversion suite (ToBigStr/ToBoolean/ToByte/ToDate/ToDReal/etc.), conference info, chat status, CRC32 |
| 300     | PPL 3.0    | 73    | DBase queries (DBOF/DEOF/DERR/DFIELDS/DLENGTH/DNAME/DRECCOUNT/DRECNO/DSEEK/DGET/DPUT), file search (FindFirst/FindNext), FidoNet queries (FDORDAKA/FDORDOrg/FDORDArea) |
| 340     | PPL 3.40 (**15.4**) | 4 | ShortDesc, GetBankBal, GetMsgHdr, SetMsgHdr |

*Note: `U_PERSONAL` (Clark's slot -291) doesn't appear in icy_ppe's
LAST_FUNC = -290. Either icy_ppe was reverse-engineered against a
pre-release build without U_PERSONAL, or Clark added it later than
the 4 other 15.4 functions.*

## PPLD version mismatch notes

Chicken's PPLD (`pcb1541/PPL/ppld/`) was written for PPL 3.20 / 3.30
era. It has 7 `???` placeholders in its StatNames table
(`ppld-src/NAMES.INC`) because it never saw the resolved token names
for the compiler-internal PPL 2.0 statements. It also has 2 `???` in
FnktNames (function operators it couldn't identify).

If we want a byte-exact PPLD clean-room for the 15.4 era, we need
to:

1. Import the 9 statement-slot resolutions from `icy_ppe`.
2. Import the 15.4 additions (ShortDesc, MoveMsg, SetBankBal
   statements; ShortDesc, GetBankBal, GetMsgHdr, SetMsgHdr
   functions).
3. Verify against decompilation of Clark's shipped `.PPE` files —
   RUNINET.PPE from PCBIC is a natural regression test.

## Reproducibility (extended)

The decompiler cross-reference above was derived from:

    less pcb1541/PPL/ppld/ppld-src/NAMES.INC              # Chicken's PPLD tables (7 ??? in stmt, 2 ??? in func)
    less pcb1541/PPL/pplengine/icy_ppe/src/executable/smt_op_codes.rs   # Modern Rust engine (all 229 stmts resolved)
    less pcb1541/PPL/pplengine/icy_ppe/src/executable/func_op_codes.rs  # Modern Rust engine (all 294 funcs resolved)
    diff pcb153/SOURCE/H/NEWSCR.HPP pcb153/upd154/SOURCE/H/NEWSCR.HPP   # Clark's own PPL 3.30 vs 3.40 diff

Analysis performed 2026-09-03. All findings are source-derived and
reproducible; no speculation.

## Provenance and reproducibility

To regenerate this analysis:

    # PPL 3.40 additions
    diff pcb153/SOURCE/H/NEWSCR.HPP pcb153/upd154/SOURCE/H/NEWSCR.HPP

    # PPL user variables added
    diff pcb153/SOURCE/PPL/SCRCOMP.CPP pcb153/upd154/SOURCE/PPL/SCRCOMP.CPP

    # PCBTEXT prompt additions
    diff pcb153/SOURCE/H/pcbtext.h pcb153/upd154/SOURCE/H/pcbtext.h

    # WHATSNEW official announcement
    less pcb153/upd154/docs/WHATSNEW

    # My prior sysop guide covering the 15.4 additions
    less docs/SYSOP_154.TXT

Analysis performed 2026-09-03 during install v1.9+ planning, prompted
by a lost prior list. Full source diffs done programmatically;
findings verified manually against source and Clark's WHATSNEW.
