# 15.3 to 15.4 — delta specification

Working document for Phase 1b.

## There is no diff

Checked directly. The source we hold is **15.3**:

```c
#define PCBVERSION    "15.3"
#define VERSION_MAJOR "15"
#define VERSION_MINOR "3"
```

None of the 15.4 PPL additions are present:

| Addition | Occurrences in 15.3 source |
|---|---|
| `GetBankBal`, `SetBankBal` | 0 |
| `GetMsgHdr`, `SetMsgHdr` | 0 |
| `U_BIRTHDATE`, `U_EMAIL`, `U_GENDER`, `U_WEB` | 0 |
| `MoveMsg` | present in `NEWDATA.H`, `PCBTOOLS.H`, `DATAFIL2.C` — a PCBOARD.DAT config field, **not** the PPL statement |
| `ShortDesc` | present in `USRMAINT.C`, `PCBTEXT.C`, `LOGIN.C` — the file short-description display, **not** the PPL function |

Zero of twelve. Clark never released 15.4 source, and no patch exists.

**Phase 1b is reimplementation from specification, not patching.** Three
inputs: `WHATSNEW` (604 lines, written by Clark) as the spec, `HISTORY` for
the bug fixes, and the 15.4 beta binaries as the oracle.

---

## 1. PPL 3.40 — twelve additions

### GetBankBal(field) — function, returns STRING

Time-bank field accessor. Backed by a 15.4 PSA; PCBSysMgr installs it.

| Field | Meaning | Units |
|---:|---|---|
| 0 | Last deposit date | minutes |
| 1 | Last withdrawal date | minutes |
| 2 | Last transaction amount | minutes |
| 3 | Amount saved | minutes |
| 4 | Max withdrawal per day | minutes |
| 5 | Max stored amount | minutes |
| 6 | Last deposit date | K bytes |
| 7 | Last withdrawal date | K bytes |
| 8 | Last transaction amount | K bytes |
| 9 | Amount saved | K bytes |
| 10 | Max withdrawal per day | K bytes |
| 11 | Max stored amount | K bytes |

```
INTEGER amt_saved
amt_saved = GetBankBal(3)
```

Note the declared return type is STRING while the documented example assigns
to INTEGER. Check the 15.4 binary for which it actually is before
implementing — Clark's own documentation is inconsistent here.

### SetBankBal field, value — statement

Same twelve fields.

```
SetBankBal 10, 10
```

### GetMsgHdr(conf, message, field) — function, returns STRING

`conf` integer, `message` **double**, `field` integer 0-15. Field numbers are
the same set used by `SCANMSGHDR()` (PPL 3.00), so that table is the
reference.

```
msgToName = GETMSGHDR(0, HIMSGNUM(), HDR_TO)
```

### SetMsgHdr(conf, message, field, fieldinfo) — function, returns INTEGER

Five writable fields, a **different and much smaller set** than GetMsgHdr's
0-15:

| Field | Meaning |
|---:|---|
| 1 | To |
| 2 | From |
| 3 | Subject |
| 4 | Password |
| 5 | Echo flag |

Returns the message number. **If the modified header still fits in place the
number is unchanged; if it does not, the message is appended to the end of the
base and a new number returned.** Any caller must use the returned number
rather than assuming the original still resolves.

```
NewMessage = SETMSGHDR(0, HighMessage, 1, "SYSOP")
```

### MoveMsg conf, message, movetype — statement

Moves a message to the end of the message base. `movetype` boolean: TRUE
moves, FALSE copies.

The summary section of WHATSNEW documents this as `MoveMsg(conf, message)` —
two parameters — while the reference section gives three. **Three is
correct**; verify against the binary.

### ShortDesc() — function, returns BOOLEAN

TRUE if the user has short file descriptions enabled.

### ShortDesc value — statement

Sets it. Boolean.

### Five user variables

`U_BIRTHDATE`, `U_EMAIL`, `U_GENDER`, `U_SHORTDESC`, `U_WEB` — all backed by
the 15.4 PSA.

---

## 2. Message header access

`GetMsgHdr` and `SetMsgHdr` read and write the header documented in
`PCBXDOT/DOCDEV/MSGS.TXT`. Two things that will bite:

**Message and reference numbers are `bsreal`** — Microsoft Binary Format
single-precision floats, not integers. Reading them as `uint32_t` yields
garbage. Clark's converters exist already:

```
Pcb-libs/SOURCE/MISC/BS_LONG.C    bsreal -> long
Pcb-libs/SOURCE/MISC/LONG_BS.C    long   -> bsreal
Pcb-libs/SOURCE/MISC/BD_DBLE.C    bdreal -> double
Pcb-libs/SOURCE/MISC/DBLE_BD.C    double -> bdreal
```

This is also why `GetMsgHdr`'s `message` parameter is a **double** rather than
an integer.

**The status flag at offset 0 encodes privacy and read-state together** —
eleven values, per MSGS.TXT note 2:

| Flag | Meaning |
|---|---|
| (space) | readable by anyone |
| `*` | private, addressee has NOT read |
| `+` | private, addressee HAS read |
| `-` | to a person, public, has been read |
| `~` | comment to sysop, NOT read |
| `` ` `` | comment to sysop, HAS been read |
| `%` | sender-password protected, unread |
| `^` | sender-password protected, read |
| `!` | group-password protected, unread |
| `#` | group-password protected, read |
| `$` | group-password, addressed to ALL |

Relevant to Phase 1a: Clark's private-message state machine is already
specified in the format. Phase 1a completes behaviour around an existing
design rather than inventing one.

---

## 3. CHAT enhancements

- **@X colour codes** accepted in user text during group chat
- **Action commands**, sysop-definable
- **COLOR command** — changes the user's foreground colour

| Option | Colour |
|---|---|
| `B` `G` `C` `R` `M` `Y` `W` | blue, green, cyan, red, magenta, yellow, white |
| `+` | brighten |
| `-` | darken |

`W+` is bright white, `R-` dark red. A bare `+` or `-` adjusts the current
colour **and sets the default intensity**.

Bug from HISTORY: actions sharing a common prefix matched the wrong action.
Whatever matching the original used, longest-match is the fix.

---

## 4. User record — 15.4 PSA

Birthday, gender, email address, personal web page. Installed as a PSA;
PCBSysMgr updated to support it.

Bugs from HISTORY:
- W command showed the EMAIL prompt for the WEB field
- NEWUSER field spacing wrong for EMAIL and WEB
- Gender defaulted to `'M'` instead of empty

---

## 5. File flagging

- Flag by **screen display number** as well as by name
- View **full descriptions** while in short-description view

Bug from HISTORY: L command wildcard search was broken.

---

## 6. UUIN — reject by name

A `REJECTS` file in the UUCP base path. ASCII, one entry per line:

```
name,%          bounce
name,           discard
```

Second field beginning with `%` bounces; anything else discards. **Maximum 16
entries.**

Also from HISTORY: multipart/alternative MIME support added.

---

## 7. PPL compiler

- Smaller, faster output than 15.3
- **New PPE security algorithm** — 15.4-compiled PPEs use enhanced encryption
  "to help thwart PPE busters"

That last point matters for compatibility: a 15.4 PPE will not decode under a
15.3 runtime. Whether 15.3 PPEs still run under 15.4 needs checking against
the binary — Clark does not say.

---

## 8. PCBIC

FTP **MGET** — wildcard multiple gets.

---

## 9. Bug fixes from HISTORY

| Fix |
|---|
| L command wildcard search |
| W command showing EMAIL prompt for WEB field |
| NEWUSER field spacing for EMAIL and WEB |
| PPL crash: `USELMRS FALSE` + `GETALTUSER` + `ADDUSER` combined |
| Chat actions with common prefixes matching wrong |
| Gender defaulting to `'M'` instead of empty |
| UUIN multipart/alternative MIME |

Each needs a regression test, because each is evidence of a real failure mode
someone hit.

---

## 10. Compatibility, from README.1ST

- **USERS.SYS is dynamic and versioned**
- **DOORS.LST can specify a version number** — 3 selects v15.2-style
  behaviour, so pre-existing doors keep working

Both must be preserved. They are the mechanism by which 15.4 stayed compatible
with the door ecosystem, and the same mechanism carries 15.41.

---

## Do this first: dump the PPLC opcode table

PPL bytecode is a numbered function table. The twelve additions each took a
specific opcode number in Clark's 15.4 PPLC.EXE.

**Extract those numbers from the binary before implementing.** Guess them and
PPEs compiled by our PPLC will not run under Clark's, which collapses the
two-way verification below and is not discoverable until someone tries it.

## Verification — two directions

We hold Clark's 15.4 binaries, so this is checkable rather than arguable:

1. Our PPLC output runs under Clark's `PCBOARD.EXE`
2. Clark's PPLC output runs under our `PCBOARD.EXE`
3. `USERS.SYS` written by ours reads in Clark's, and the reverse
4. A v15.2-era door still runs via the DOORS.LST version field
5. A PPE using `GetMsgHdr`/`SetMsgHdr` produces headers Clark's 15.4 accepts

Both directions passing means the PPL additions are provably right, not
plausible.
