# PCBoard Toolkit "NO*" Link-Out Pattern

**Source of truth:** `pcb154/LIB/SOURCE/TOOLKIT/NO*.C` (17 files, 892 lines total)
and `toolkit/delta154/SOURCE/TOOLKIT/NO*.C` (mirror).

Answers a question that comes up constantly when reading PCBoard utility
sources: **why does every subsystem have a `NO<subsystem>` file that does
nothing?** It's not dead code — it's Clark's link-time subsystem-replacement
mechanism. A utility program that doesn't need PCBoard's ANSI engine links
`NOANSI.OBJ` instead, and the linker resolves all the ANSI API symbols with
empty-body versions.

Every PCBoard utility (`PCBSM`, `PCBFILER`, `PCBEDIT`, `USERNET`, `PCBMONI`,
etc.) is built this way — pick the real subsystem OR the NO- stub for each
of the 17 features, and the utility only carries what it actually uses.

---

## 1. The 17 NO* stubs (from `pcb154/LIB/SOURCE/TOOLKIT/`)

| Stub file  | Replaces          | Empty overrides |
|------------|-------------------|----------------:|
| NOANSI.C   | ANSI/color engine |              18 |
| NOCHAT.C   | sysop chat        |               1 |
| NODISP.C   | display / paging  |              21 |
| NOHELP.C   | online help       |               1 |
| NOINPUT.C  | keyboard/input    |               6 |
| NOLANG.C   | multi-language    |               2 |
| NOLOG.C    | callers log       |               5 |
| NOMEMORY.C | memory manager    |               7 |
| NOPCBSYS.C | PCBoard.SYS I/O   |               2 |
| NOPRINT.C  | printer output    |               1 |
| NOSCREEN.C | sysop screen      |               5 |
| NOSHELL.C  | DOS shell         |               5 |
| NOSTATUS.C | status line       |               2 |
| NOSYS.C    | system I/O        |               2 |
| NOTXT.C    | PCBTEXT loader    |               6 |
| NOUPDSYS.C | PCBoard.SYS update|               2 |
| NOXLATE.C  | translation table |               1 |

Each is a minimal replacement: same function names, same
`LIBENTRY`/`_FAR_`/Pascal-calling-convention signatures, but every body is
either empty or returns a zero-equivalent value. Header banner on every one:
`Copyright (C) 1996 Clark Development Company, Inc.` (delta154 mirror is
identical).

**Example** — `NOANSI.C` provides:

```c
void LIBENTRY toggleoff(char _FAR_ *ScrnBuf) {}
void LIBENTRY toggleon(char _FAR_ *ScrnBuf) {}
char LIBENTRY curcolor(void)  { return(0); }
char LIBENTRY awherex(void)   { return(0); }
char LIBENTRY awherey(void)   { return(0); }
/* ... 13 more, all bodies empty or `return(0)`/`return(NULL)` ... */
```

Utility X that never colorizes anything links `NOANSI.OBJ` and the linker is
happy: every `curcolor()` call in the toolkit code resolves, just to a stub
that returns 0.

---

## 2. Two special extra stubs (UUCP tree)

`pcb154/MAIN/SOURCE/UUCP/NOTXT.C` and `pcb154/MAIN/SOURCE/UUCP/UUXFER/NOTXT.C`
are separate copies — the UUCP subsystem re-implements a smaller NOTXT
because it needs different signatures than the toolkit NOTXT. Same pattern,
narrower scope.

---

## 3. Serial backend: the same pattern for hardware

The link-out idiom extends past subsystems to **hardware backends**. In
`MODEMDRV.C`, PCBoard picks a serial layer via `#ifdef`:

```c
#if defined(COMM) && defined(MULTIPORT) && defined(COMMDRV)
    /* link WCSC's COMMDRV.OBJ */
#elif defined(COMM) && defined(FOSSIL)
    /* link FOSSIL.OBJ (INT 14h) */
#endif
```

Clark shipped both `COMMDRV.OBJ` and `FOSSIL.OBJ` in the Toolkit3 SDK as a
matching pair on 1994-02-15 17:53 — same timestamp, same slot, pluggable
choice. Sysops with a multiport card link `COMMDRV.OBJ`; sysops on a
straight FOSSIL driver link `FOSSIL.OBJ`. Both go into the same linker
slot defined by the `ser_rs232_*` API.

**Reserved-but-never-shipped:** the architecture allows a third (or Nth)
backend to plug into the same slot. Clark never shipped one. `pcbdcom`
(GPLv3 replacement) is that third slot finally getting populated — 32
years later.

The pcbdcom analog `NOPCBDCOM.OBJ` — an empty stub exporting the 13
`ser_rs232_*` symbols with empty bodies — would complete the pattern for
utilities that don't need any serial layer at all. Not shipped yet.

---

## 4. Why this matters for the port

- **Every utility's `.MAK` file** picks a set of NO* stubs. When rebuilding,
  the choice must match the original — using the real subsystem where a stub
  was linked bloats the binary and can drag in initialization code that
  isn't safe outside PCBoard's main-loop context. Check the `.LNK` /
  response file in each `.MAK` for which NO* files are listed.
- **New utilities should default to all NO* stubs** and only pull in the
  real subsystem when the utility genuinely needs it. This matches Clark's
  original convention and keeps binary sizes at 1990s-appropriate levels.
- **The stubs are Clark's proprietary code** (per the source banner) and
  ship under his source-license terms in `pcb154/LIB/SOURCE/TOOLKIT/`. Any
  replacement stubs we write for a fully-open toolchain should be
  greenfield — matching signatures only, not Clark's implementations.

---

## 5. See also

- `toolkit/pwa154/pcbdcom/docs/LINKOUT.md` — thin summary aimed at pcbdcom
  users (29 lines, external audience)
- `todo/BINARY-CATALOG.md` — full 550-line PCBoard binary inventory
  ("Have source" vs "Need reversing" table)
- `pcb154/pcbdcom/GAP-ANALYSIS.md` — MODEMDRV.C + INSTALL.DAT crosscheck
  (144 lines) showing what WCSC shipped vs what Clark's build system knows
  about
- [`PCBDCOM-CARDS.md`](PCBDCOM-CARDS.md) — the 9 shipped serial backends
  and 3 UI-only stubs
- [`PLANNED-FEATURES.md`](PLANNED-FEATURES.md) — cross-reference of
  reserved-but-unshipped features across PCBoard
