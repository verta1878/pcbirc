# pcbdcom compatibility plan

## Two-phase, two-team approach

**Phase 1 — Feature discovery (internal, unrestricted).**
We study COMM-DRV to answer *what does it do that pcbdcom doesn't
yet*. This is a private read for gap identification, not code
production. Any team member can look at COMM-DRV binaries,
COMMDRV.RED contents, DRVSETUP behavior, live traces — whatever
tells us the feature list.

Deliverable: `pcb154/pcbdcom/GAP-ANALYSIS.md` — a plain-English
list of features COMM-DRV has that pcbdcom is missing, plus
notes on which cards / INT 14h functions / config options each
feature touches. No code, no register values from COMM-DRV, no
byte sequences — just *what features exist*.

**Phase 2 — Implementation (clean room, strict).**
A second reader takes the GAP-ANALYSIS document and implements
each missing feature using only:

- Public FOSSIL spec (INT 14h)
- Public hardware datasheets (16550, CD1400, SC26198, Digi FEP,
  Comtrol AIOP, Arnet register specs where published)
- GPL Linux driver source (already staged in `ref/linux/`)
- PCBoard's own source for what it expects on the interface

The implementation reader does not look at COMM-DRV, its
disassembly, its config file bytes, or anything derived from
COMM-DRV internals. This is the wall that keeps our
implementation legally clean regardless of what Phase 1 found.

## Practical workflow in this repo

Phase 1 output lives in `GAP-ANALYSIS.md` (or a private note if
we want to be extra careful — not committed to public repo).

Phase 2 commits reference only public sources in file headers:
```
/* Ported from Linux drivers/char/cyclades.c v2.6.32 (GPLv2).
 * Public datasheet: Cirrus Logic CD1400 Register Reference. */
```
Never:
```
/* From COMM-DRV COMMDRV.EXE +0x2A40 */    /* ← would poison it */
```

## Where firmware / BIOS blobs come from

Card manufacturer firmware (Digi FEPCODE, Comtrol microcode,
Arnet XABIOS, etc.) is owned by the card vendor, not WCSC. Our
model matches Linux:
- pcbdcom (GPLv3) knows how to talk to the card once firmware
  is loaded.
- Sysop supplies firmware from vendor disk at load time via
  `PCBDCOM.CFG` (`FIRMWARE=path/FEPCODE.BIN` line).
- We do not redistribute vendor firmware unless the vendor
  explicitly permits it.

## Missing-feature candidates (starter list to expand in Phase 1)

Educated guesses at what pcbdcom v1.1 might not yet cover:
- 8th card: Arnet SmartPort Plus (PCBoard docs list it)
- Cold-boot BIOS upload for Digi cards (v1.1 assumes warm boot)
- Comtrol MUDBAC IRQ routing (v1.1 uses IRQ-disabled mode)
- Cyclades multi-chip (16Y/32Y) — v1.1 has structure, needs test
- COMM-DRV extended INT 14h functions beyond standard FOSSIL
  (functions 0x10+; PCBoard source will show which it uses)
- Hardware flow control config nuances per card
- 15.41 TCP socket backend (deferred by design; port later)

Phase 1 replaces this list with real findings.

# File naming + architecture (locked in 2026-08-30)

## Ship files match WCSC 1:1 for sysop familiarity

Sysops know the COMM-DRV layout. Ours mirrors it — same slots, same
purpose, only the COMM/DRV prefix swaps to PCB/PCBD.

| WCSC file    | Our file      | Purpose                             |
|--------------|---------------|-------------------------------------|
| COMMTSR.EXE  | PCBDTSR.EXE    | Resident TSR (INT 14h + IRQ hooks) |
| COMMDRV.EXE  | PCBDCOM.EXE   | Main control utility                |
| DRVSETUP.EXE | PCBDSET.EXE   | Setup editor (visual config)        |
| TEST.EXE     | PCBDTEST.EXE  | Hardware tester                     |
| MONITOR.BAT  | MONITOR.BAT   | Startup helper                      |
| COMMDV00-08.DRV | PCBDV00-08.DRV | Loadable per-card drivers (v2)  |
| COMMDRV.OBJ  | PCBDCOM.OBJ   | Link-time static library            |

## Dropped: PCBDCOM.SYS (DOS device driver mode)

Serial cards don't need to load before DOS finishes booting. No file
access at boot time, no chicken-and-egg with disk drivers. The .SYS
device-driver path exists in v1.1's pcbdcom.c but is being removed:

- COMM-DRV was TSR-only for good reason
- Sysops don't expect a .SYS for serial (unfamiliar shape)
- CONFIG.SYS space is precious (counts against FILES= etc.)
- Adds test surface, no benefit

Delete device_entry() from pcbdcom.c in v1.2 pass. Keep main() +
_dos_keep for TSR path only.

## Architecture: monolithic (v1) → modular loadable (v2)

WCSC's design: small TSR skeleton (COMMTSR.EXE) + on-demand loading
of only the card drivers the sysop's PCBDCOM.CFG references. Sysop
with just a Boca 8-port pays ~1 KB resident, not ~15 KB for all
7 backends.

v1.x (now): monolithic PCBDTSR.EXE with all backends compiled in.
  Ship this first — it works, sysops can use it, gets pcbdcom in
  people's hands.

v2 (later): split each backend into loadable PCBDV00-08.DRV files.
  PCBDTSR.EXE becomes a small skeleton that reads PCBDCOM.CFG, EXECs
  or overlay-loads only the .DRV files needed, dispatches ISR + INT
  14h calls to loaded drivers. Same source, different link target.

  Enables:
    - Small resident footprint (matches WCSC's efficiency)
    - Sysop can update one card driver without rebuilding TSR
    - New card support drops in as a new .DRV, no rebuild

## Concrete deliverables

v1.2 (next work): 7 files
  PCBDTSR.EXE       ← renamed from PCBDCOM.EXE (main TSR)
  PCBDCOM.EXE      ← control utility (new)
  PCBDSET.EXE      ← setup editor (new)
  PCBDTEST.EXE     ← hardware tester (new)
  PCBDCOM.OBJ      ← link-time static library (new)
  PCBDCOM.CFG.sample  ← config template
  firmware/*.BIN   ← extracted from COMMDRV.RED (Phase 1 output)

v2: 18 files (matches WCSC's 22 minus 4 .DAT templates we don't need)

Removed from earlier plans:
  PCBDCOM.SYS      ← dropped, not needed

# PCBDCOM.OBJ SDK packaging (v1.2 first-class goal)

Not just a build artifact — the deliverable that closes the gap
Clark left open in Feb 1994 when they added COMMDRV.OBJ + FOSSIL.OBJ
to Toolkit3/PCBKIT_S.ZIP as a link-time-choice pair. That slot has
been waiting for a third, source-available, free option ever since.

## Ships in the toolkit tree

`toolkit/pwa154/pcbdcom/` (new SDK section):

    src/                 full pcbdcom source (GPLv3)
    lib/                 pre-built .OBJ variants:
      PCBDCOM_BC31_S.OBJ  Borland C++ 3.1, small model
      PCBDCOM_BC31_M.OBJ  medium
      PCBDCOM_BC31_C.OBJ  compact
      PCBDCOM_BC31_L.OBJ  large
      PCBDCOM_MSC70_S.OBJ Microsoft C 7.0, small
      PCBDCOM_MSC70_M.OBJ medium
      PCBDCOM_MSC70_C.OBJ compact
      PCBDCOM_MSC70_L.OBJ large
      PCBDCOM_MSC80_S.OBJ (through L)
      PCBDCOM_OWC_F.OBJ   OpenWatcom flat
      = 13 variants matching Clark's compiler+memory-model matrix
    inc/
      PCBDCOM.H          the ser_rs232_* API header
    docs/
      SDK.md             how to link + examples
      LINKOUT.md         stub-out pattern (per Toolkit3 Appendix D)
    examples/
      simple.c           open port, send string, read reply
      multiport.c        use all 8 sub-ports of a Boca card
      tsrless.c          run without PCBDTSR — call init at startup

## Constraints (per pcbcomm README, now enforced)

- **Calling convention:** Pascal, callee cleans stack (chosen for
  code size — matches COMMDRV.OBJ). Names case-fold to uppercase.
  Must match exactly or nothing links.
- **Memory models:** S/M/C/L across three compilers = 12 variants;
  OpenWatcom flat = 13th target, not a replacement.
- **Symbol names:** SER_RS232_INIT, SER_RS232_SETUP, SER_RS232_PUTBYTE
  etc. — the 13 entry points MODEMDRV.C already knows.

## Who uses it

- **PCBoard**: `#ifdef PCBDCOM` in MODEMDRV.C, links PCBDCOM.OBJ
- **Doors** (FrontDoor, BinkleyTerm, T-Mail): multiport support
- **PPL/PPE authors**: serial hardware access from door scripts
- **Terminal programs**: Telix-style apps, procomm-like
- **Fax/data software**: modem-pool serial mux
- **Any DOS dev** who needs a modern, source-available serial library

## Deliverable slot in v1.2

`toolkit/pwa154/pcbdcom/` gets built and populated alongside the
pcb154/pcbdcom/ source work. Both trees ship together.

# Related toolkit gap — NO*.OBJ stubs (tracked, not v1.2)

Clark's Toolkit3 (PCBKIT_S.ZIP) ships a set of empty-body .OBJ files
that satisfy PCBoard symbol imports. A door lists the stubs for
subsystems it doesn't use before the .LIB — linker takes the stubs,
real subsystem code doesn't get pulled in, executable shrinks.
Appendix D of the toolkit docs describes the pattern:

> "A hello-world door against the full library is 49K, so Clark
> shipped stub objects... that satisfy symbols with empty bodies.
> List the ones you don't need before the .LIB and the linker takes
> the stubs."

## Full stub inventory (from MAIN/build/IC-TOOLKIT-SDK-BUILD-PLAN.md)

17 NO* stubs + 1 override + 1 error variant:

| Stub     | Subsystem       | What that subsystem does                                                                                       |
|----------|-----------------|----------------------------------------------------------------------------------------------------------------|
| NODISP   | display engine  | Formats text output: colour codes, @-macros, page-pause (More? prompt), word-wrap for the caller's line width. |
| NOANSI   | ANSI renderer   | Turns ANSI/PCBoard @X codes into terminal escape sequences (`ESC[...m`) for callers on ANSI terminals.        |
| NOCHAT   | sysop chat      | Split-screen interactive chat — sysop presses a key mid-call, chat window opens, sysop and user type live.    |
| NOHELP   | help system     | Loads and displays PCBHELP.LST context help when caller types `H` at any prompt.                              |
| NOINPUT  | line input      | Reads a line from the caller — handles backspace, character filters, idle timeout, control-key trapping.      |
| NOLANG   | languages       | Loads alternate PCBTEXT.<lang> and PCBML.LST files so caller sees prompts in their chosen language.           |
| NOLOG    | CALLERS log     | Writes CALLERS log entries (login, logout, uploads, downloads, doors, security violations).                   |
| NOMEMORY | memory tracker  | Debug allocator that tracks every malloc/free with file+line for finding leaks. Off in production builds.     |
| NOPCBSYS | PCBOARD.SYS in  | Reads the per-node state file PCBoard drops in `PCBOARDx.SYS` — caller info, connect speed, node number.      |
| NOPRINT  | printer output  | Log-to-LPT and sysop print-current-screen (Alt-P).                                                            |
| NOSCREEN | local screen    | Draws to sysop's local console — the physical monitor at the PCBoard machine.                                 |
| NOSHELL  | DOS shell       | F6-drop-to-DOS for sysop, `SHELL` command for privileged users. Runs COMMAND.COM as a child.                  |
| NOSTATUS | status line     | Bottom-of-screen 1-line status showing caller name, node, time-left, current activity — for local sysop.      |
| NOSYS    | sysop functions | Alt-key menu: see all nodes, kick a user, view chat request, monitor another node's screen, drop to DOS.      |
| NOTXT    | PCBTEXT prompts | Reads PCBTEXT.DAT — every prompt string PCBoard displays. Change PCBTEXT and PCBoard talks differently.       |
| NOUPDSYS | PCBOARD.SYS out | Writes PCBOARDx.SYS back after a door exits — updates caller time-remaining, byte counts, upload stats.       |
| NOXLATE  | translation     | Character remapping — swap 8-bit accented chars for 7-bit ASCII, or apply national character overlays.        |
| PCBDAT   | PCBOARD.DAT     | Reads the sysop's master config file (paths, security levels, event schedule). OVERRIDE = full replacement.   |
| SMALLERR | error table     | Smaller subset of error strings — for utilities that only ever hit a handful of errors, saves ~2 KB.          |

Real implementations live in `pcb153/SOURCE/DISPLAY/`, `pcb153/SOURCE/
MAIN/`, `pcb153/SOURCE/SUPPORT/` — Clark's proprietary code, ships
with pcbirc under PCBoard source license terms. Free-source
replacements would be Phase 2+ work per subsystem.

## Scope for pcbirc

### v1.2 (with pcbdcom SDK)

- **NOPCBDCOM.OBJ** — empty serial stub (~30 lines), 13 `ser_rs232_*`
  functions returning `RS232ERR_NONE`. Ships in `toolkit/pwa154/
  pcbdcom/lib/` alongside `PCBDCOM.OBJ`. Small enough to add naturally.

### Later (separate toolkit-stubs session, per-subsystem)

- Free replacements for NODISP, NOANSI, NOCHAT, NOHELP, NOINPUT,
  NOLANG, NOLOG, NOMEMORY, NOPCBSYS, NOPRINT, NOSCREEN, NOSHELL,
  NOSTATUS, NOSYS, NOTXT, NOUPDSYS, NOXLATE, SMALLERR.

  Each needs the correct symbol imports understood for its
  subsystem. Not pcbdcom work — belongs in a dedicated session
  after v1.2 ships.

  Priority order (most-linked-out first for typical doors):
    1. NODISP + NOANSI (utility programs skip display entirely)
    2. NOCHAT + NOHELP (games don't need chat/help)
    3. NOLOG + NOSTATUS (headless utilities)
    4. NOSHELL + NOSYS (locked-down doors)
    5. NOTXT + NOXLATE + NOLANG (single-language doors)
    6. NOSCREEN + NOINPUT + NOMEMORY + NOPRINT (rare skips)
    7. NOPCBSYS + NOUPDSYS + SMALLERR (edge cases)

## End goal

Clark shipped the toolkit library as PROPRIETARY. Sysops with
PCBoard licenses have full rights to build with it, but sharing
built utilities to users without licenses was a grey area (the
utility carries library code). Free NO* replacements make it
possible to build doors that ship freely with no linked-in Clark
code — the last piece for pcbirc to be truly buildable without
any Clark-original binaries.

# Next session (2026-08-31) — build all three driver versions

Locked with lead. Tomorrow's session covers the full pcbdcom
implementation roadmap end-to-end:

## v1 — verify already-shipped v1.1 monolithic build

Already code-complete as of 2026-08-30 (this session). Tomorrow:
build it, test-load it, confirm all 7 backends compile clean under
BC31 / MSC70 / OpenWatcom. Fix any compilation issues that surface.

## v1.2 — SDK + Phase 2 gap fills

Files to create:
  pcb154/pcbdcom/src/
    arnet_backend.c        — 8th backend (Arnet SmartPort Plus)
    ser_rs232_shim.c       — 13-function API wrapper for COMMDRV.OBJ
                             replacement (Pascal calling, uppercase)
  pcb154/pcbdcom/int14.c   — extend with AH=0x10 commgo + AH=0x12 commstop
  pcb154/pcbdcom/pcbdcom.c — add _dos_keep() TSR-resident fix,
                             remove device_entry() (drop .SYS path)
  pcb154/pcbdcom/firmware/ — extract COMMDRV.RED, place XABIOS.BIN,
                             XACOOK.BIN, XACOMX.BIN, BOCA1610.BIN
  pcb154/pcbdcom/PCBDCOM.MAK — new targets: PCBDCOM.OBJ, PCBDSET.EXE,
                                PCBDTEST.EXE. Rename output PCBDCOM.EXE
                                → PCBDTSR.EXE.
  toolkit/pwa154/pcbdcom/
    src/                   — copy of pcb154/pcbdcom/src
    lib/                   — 13 built PCBDCOM_*_*.OBJ variants
    inc/PCBDCOM.H          — ser_rs232_* API header
    docs/SDK.md            — how to link + example
    docs/LINKOUT.md        — stub-linkout pattern
    examples/simple.c
    examples/multiport.c
    examples/tsrless.c
    NOPCBDCOM.OBJ          — empty stub for doors that skip serial

Phase 1 prerequisite for arnet_backend.c: extract COMMDRV.RED,
identify the 9th card if any, read ARNETSP4/8.DAT for register maps.

## v2 — modular loadable drivers

Split monolithic PCBDTSR.EXE into small skeleton + 9 loadable
PCBDV00-08.DRV files loaded on-demand from PCBDCOM.CFG. Sysop
with just a Boca 8-port pays ~1 KB resident instead of ~15 KB.

Requires DOS overlay loader or child EXEC in the skeleton TSR.
More invasive than v1.2 — do it last.

## Order of tomorrow's work

  1. Verify v1.1 builds clean (all 7 backends × 3 compilers)
  2. Extract COMMDRV.RED (Phase 1 completion — get the 9th card
     identity + BIOS blobs + .DAT register maps)
  3. Add v1.2 features on top of v1.1 (Arnet + shim + INT 14h ext
     + _dos_keep)
  4. Build SDK tree (toolkit/pwa154/pcbdcom/)
  5. Start v2 modular refactor (if time permits)

If v2 refactor doesn't finish, ship v1.2 + document v2 as next
session pickup.


# Full PCB DOS install requires .RED compression crack

Not just for pcbdcom — the ENTIRE PCBoard install disk set uses
the same 0x000B compression method:

    COMMDRV.RED    22 records — pcbdcom's target
    PCBOARD.RED    448 records — PCBoard main
    PCBOARD2.RED   ? records — PCBoard extras
    PCBMAIL.RED    ? records — PCBMail
    PCBCFGS.RED    ? records — Config templates
    PPLC.RED       ? records — PPL compiler

Container format is fully reversed (see pcb154/pcbdcom/GAP-ANALYSIS.md
Phase 1 Step 2 section). Records enumerable in all .RED files. The
BLOCKER for a self-hosted PCBoard install from original disks is
the compression algorithm.

## Path forward

**Ghidra reverse of COMMDRV.EXE's decompress routine** — will yield
the exact 0x000B algorithm. Once implemented in redx (pcb1541/install/archivers/redx/),
we can extract every .RED file WCSC ever shipped and rebuild the
install disk set with our own binaries substituted (or added).

That unlocks:
- Full pcbirc-native PCBoard install from the original disks
- Ability to ship UPDATED install disks with pcbdcom.OBJ replacing
  COMMDRV.OBJ in the toolkit .RED
- Ability to ship a modern install path that uses our pcbirc
  installer instead of WCSC's INSTALL.EXE (via re-packing .RED)

Priority: not v1.2 blocker (v1.2 code paths use public sources
only). But required BEFORE we can ship a complete pcbirc PCBoard
install package.

Suggested target session: after pcbdcom v1.2 lands and stabilizes.
Ghidra 11.1.2 is already installed at /opt/ghidra/ (per prior
private notes). Reverse effort estimate: 4-8 hours for a competent
reader focused on the decompress routine.
