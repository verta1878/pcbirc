# PCBoard Display-File Lookup

**Source of truth:** `pcb154/MAIN/SOURCE/DISPLAY/FILES.C` (function `getalternatename`, lines 866–1055)
and `pcb154/MAIN/SOURCE/H/PCBOARD.H` line 168 (`displayfiletype` enum).

Answers the question: **what filename does PCBoard actually open when the user
"loads the ANSI"?** Short answer: the base name gets a `G` (or `R`) suffix, plus
optional security-level number, plus optional language extension. The most
specific match wins.

---

## 1. The `displayfiletype` bitflag

From `PCBOARD.H`:

```c
typedef enum {
    NOALTERNATE      = 0,
    GRAPHICS         = 1,     //  user has ANSI/color enabled
    SECURITY         = 2,     //  append user's security level
    LANGUAGE         = 4,     //  append the current language extension
    WELCOME          = 8,     //  layout tweaks for WELCOME file
    RUNMENU          = 16,    //  .MNU (menu) variant
    RUNPPL           = 32,    //  PPE execution context
    CMDFILE          = 64,    //  command-list file context
    DISABLESUBFILES  = 128    //  skip %file / !ppe / $menu / @HANGUP@
} displayfiletype;
```

Only 4 of these 9 bits (`GRAPHICS`, `SECURITY`, `LANGUAGE`, `RUNMENU`) actually
drive the filename decoration. The other 5 change *behavior* around the display,
not the lookup.

**RIP mode is missing from this enum.** It is read directly from the global
`Control.RipMode` inside `getalternatename`. That's an inconsistency in the API:
callers cannot pass "force RIP" or "force non-RIP" — they get whatever mode the
user's session is in.

---

## 2. The 19-variant priority table (Clark's own comments)

`FILES.C` lines 38–57 — one of the very few enums in Clark's tree with
explanatory comments on every slot:

| Priority | Enum        | Example filename | Bits needed |
|:--------:|-------------|------------------|-------------|
|  1 (most specific) | `ALT_SLR` | `BRDM10R.FRE` | SECURITY + LANGUAGE + RipMode |
|  2 | `ALT_SLG` | `BRDM10G.FRE` | SECURITY + LANGUAGE + GRAPHICS |
|  3 | `ALT_SMR` | `BRDM10R.MNU` | SECURITY + RUNMENU + RipMode |
|  4 | `ALT_SMG` | `BRDM10G.MNU` | SECURITY + RUNMENU + GRAPHICS |
|  5 | `ALT_SL`  | `BRDM10.FRE`  | SECURITY + LANGUAGE |
|  6 | `ALT_SM`  | `BRDM10.MNU`  | SECURITY + RUNMENU |
|  7 | `ALT_SR`  | `BRDM10R`     | SECURITY + RipMode |
|  8 | `ALT_SG`  | `BRDM10G`     | SECURITY + GRAPHICS |
|  9 | `ALT_S`   | `BRDM10`      | SECURITY |
| 10 | `ALT_LR`  | `BRDMR.FRE`   | LANGUAGE + RipMode |
| 11 | `ALT_LG`  | `BRDMG.FRE`   | LANGUAGE + GRAPHICS |
| 12 | `ALT_MR`  | `BRDMR.MNU`   | RUNMENU + RipMode |
| 13 | `ALT_MG`  | `BRDMG.MNU`   | RUNMENU + GRAPHICS |
| 14 | `ALT_L`   | `BRDM.FRE`    | LANGUAGE |
| 15 | `ALT_M`   | `BRDM.MNU`    | RUNMENU |
| 16 | `ALT_R`   | `BRDMR`       | RipMode |
| 17 | `ALT_G`   | `BRDMG`       | GRAPHICS |
| 18 (fallback) | `ALT_DEFAULT` | `BRDM` | none |

The `SECURITY` variant uses the *numeric* security level (`ascii(Sec,
Status.CurSecLevel)` → e.g. `10`, `50`, `100`). Language uses whatever is in
`Status.MultiLangExt` — a dotted 3-char extension like `.FRE`, `.ENG`.

### Lookup algorithm (lines 982–1041)

1. Only one `dosfindfirst` / `dosfindnext` sweep of the target directory.
2. For each file found, walk masks `0 .. First-1` and record the *first* match.
3. `First` shrinks as more-specific matches are found, so more-specific masks
   short-circuit less-specific ones without a second directory pass.
4. Optional `RecordSize` check rules out `BLT20.LST` when a `.LST` was expected
   but the size isn't a multiple of the record.

Result: **one pass, most-specific-wins**, no extra `stat` calls.

---

## 3. PPL script access (`DISPFILE` statement)

`pcb154/MAIN/SOURCE/PPL/SCREXEC.CPP` lines 1453–1466:

```c
case TOK_DISPFILE:
    EVAL(tmpString,  r, nullErrBreak);   // filename
    EVAL(tmpInteger, r, nullErrBreak);   // flags integer

    l1 = (tmpInteger.values.vINTEGER & 0x00000007);   // <-- masked to 3 bits!

    i1 = NOALTERNATE;
    if (l1 & 0x01) i1 |= GRAPHICS;
    if (l1 & 0x02) i1 |= SECURITY;
    if (l1 & 0x04) i1 |= LANGUAGE;

    displayfile(nul2mty(tmpString), (displayfiletype)i1);
    break;
```

The `& 0x00000007` mask means **PPL scripts can only pass GRAPHICS, SECURITY,
LANGUAGE**. All six of the remaining `displayfiletype` bits are inaccessible
from PPE code:

| Flag | Available to PPE? | Notes |
|------|:-:|-------|
| `GRAPHICS` (1) | yes | `DISPFILE name, 1` |
| `SECURITY` (2) | yes | `DISPFILE name, 2` |
| `LANGUAGE` (4) | yes | `DISPFILE name, 4` |
| `WELCOME` (8) | **no** | Reserved for PCBoard core (WELCOME file only) |
| `RUNMENU` (16) | **no** | Reserved for menu dispatcher |
| `RUNPPL` (32) | **no** | Set by PPE launcher itself |
| `CMDFILE` (64) | **no** | Command-list file context |
| `DISABLESUBFILES` (128) | **no** | No PPE-facing knob to disable `%`/`!`/`$`/`@HANGUP@` |
| RipMode | **no** | Not a flag; read from global `Control.RipMode` |

Meaning a PPE cannot: display a menu file with menu-variant lookup, force RIP,
or safely display an untrusted file with sub-file processing disabled.
These would be reasonable SDK additions Clark never made.

---

## 4. `WELCOME` flag — what it actually does

Grep pins it to three callsites only:

- `MAIN/COMMAND.C:47` — the top-of-session welcome display
- `NODE/LOGIN.C:1178` — login-time welcome
- `DISPLAY/FILES.C:1234` — recursive `%file` inside a WELCOME (inherits the flag)

Inside `displayfile` (`FILES.C:1198`) the flag triggers a special layout:

```c
if (Type & WELCOME) {
    Display.Break = FALSE;
    startdisplay(FORCENONSTOP);          // no MORE? prompt
    clsbox(0, 23, 79, 24, 0);            // clear bottom 2 rows
    if (Control.GraphicsMode)
        setlimits(25);                    // cap at 25 rows in ANSI mode
}
```

So `WELCOME` is *display behavior*, not filename decoration — the file itself
is still resolved through the normal `G`/`R`/security/language ladder.

---

## 5. `DISABLESUBFILES` — the untrusted-content flag

When set (`FILES.C:1222`), the whole `switch` on the first character of each
line is bypassed and every line goes through `printxlated()`. Otherwise
PCBoard interprets:

| Prefix | Action |
|--------|--------|
| `%FILE` | Recurse into `displayfile(FILE, GRAPHICS\|SECURITY\|LANGUAGE\|(Type & WELCOME))` |
| `!PPE`  | Run the PPE via `runscriptwithparams` |
| `$MENU` | Dispatch to menu `MENU` via `doMenu` |
| `@HANGUP@` | Set `AutoLogoff = TRUE`, drop carrier after display |

The recursive `%file` call inherits `WELCOME` only — not `SECURITY`/`LANGUAGE`
— so nested display files still lookup with security/language decoration but
with default (top-level) mode. Recursion is guarded by `fileisopen(FileName)`
to prevent infinite loops from `%SELF`.

---

## 6. What's missing / would-have-been

Reserved-but-unused variant slots or capabilities that Clark's code hints at:

- **No `_P` (PPE) variant.** `RUNPPL` is defined as a `displayfiletype` bit but
  never contributes to filename decoration. There is no `BRDMP` or `BRDM.PPE`
  lookup in `getalternatename`. A PPE that wants a per-file `.PPE` variant
  must implement it manually.
- **No RIP flag in the enum.** As above, RIP was retrofitted through
  `Control.RipMode` rather than into `displayfiletype`. A caller cannot force
  RIP mode for one call.
- **PPE cannot pass `RUNMENU`.** So a PPE-driven menu system can't ride the
  `.MNU` decoration ladder.
- **No `CMDFILE`-decorated variant either.** The `CMDFILE` bit exists but is
  behavioral (marks the file as coming from a `CMD.LST`); no `BRDMC` mask.
- **Language extension is a single global (`Status.MultiLangExt`).** There is
  no per-conference or per-call override path visible in `getalternatename` —
  the file either matches the current language or it doesn't.

None of these are stubs in the strict sense (no half-written code), but they
are cleanly-labeled bits that never grew filename semantics — the kind of
"reserved for future use" API surface Clark left throughout PCBoard.

---

## 7. See also

- [`PCBTEXT-CODES.md`](PCBTEXT-CODES.md) — 747-slot screen-text catalog
- [`MCI-CODES.md`](MCI-CODES.md) — `@TOKEN@` macro reference (used inside these files)
- [`PLANNED-FEATURES.md`](PLANNED-FEATURES.md) — reserved-but-unshipped features across PCBoard
- [`PCBDCOM-CARDS.md`](PCBDCOM-CARDS.md) — COMMDRV backend stubs
