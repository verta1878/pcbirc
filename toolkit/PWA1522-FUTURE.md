# pwa1522 — Pre-15.3 (15.22-era) Toolkit (future goal)

## The idea

Build the older PCBoard 15.22-era toolkit so that someone running an
earlier version has an upgrade path to the 15.3 toolkit. A sysop or
door author on 15.0/15.1/15.2/15.22 could then move forward cleanly
instead of being stranded on a toolkit we don't provide.

Historically this was also tied to the **IC (Internet Connectivity)**
byte-exact reconstruction (Clark's `RUNINET.PPE` was compiled with PPL
3.20, PCBoard 15.22 era). That specific dependency is now closed — see
below.

## Status: still not directly buildable, but leads are open

We do NOT yet have a clean, standalone 15.22 C toolkit source tree
that can be dropped in as `toolkit/pwa1522/`. What we DO now have:

### Closed / in hand

- **PPLC 3.20** binary → `toolkit/pplc/3.20/PPLC320.EXE` (222 KB, md5
  `2a23e7686f79ea07bbb3c4d04e064a75`). Extracted from
  `reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`. This unlocks the IC
  RUNINET.PPE byte-exact reconstruction immediately — that goal no
  longer waits on Carsten or on a 15.22 distribution hunt. See
  `pcb1541/pcbic12/RECONSTRUCTION.md`.
- **PPLC 3.00 and 3.10** binaries also staged in `toolkit/pplc/`
  alongside 3.20 (same provenance) for the version ladder.

### Strong lead (era-tested — see verdict below)

- `reference/pcball/pcboard/pcb-libs/` is a **complete C toolkit source
  tree** — 332 files, matching pwa153's SOURCE/H layout across
  `PCB/MISC/DOS/SCREEN/SCRNIO/TOOLKIT/COUNTRY/DOSCLS/SYSTEM`.
- **Era verdict (tested):** proto-15.3, not pure 15.22.
  - The FIDO structs carry Clark's own `v15.21`/`v15.22` code
    annotations (identical to pwa153's — Clark kept them into 15.3).
  - `H/TYPES.HPP` here is 7,544 B, vs 8,325 B in pwa153. The 781 B gap
    is entirely Turbo C support Turbo C added later.
  - But pcball *already* has `__TURBOC__` guards in `H/DOSFUNC.H`,
    `H/PCBTOOLS.H`, `SOURCE/SCREEN/SETFONT.C`,
    `SOURCE/PCB/DATAFIL2.C`. So the Turbo C port had begun but hadn't
    spread to `TYPES.HPP` yet.
- **How to use:** treat pcball as "15.22 code + early Turbo C porting"
  — the closest to 15.22 we have. Building it with only Borland C++
  (skipping Turbo C entirely) gives us the 15.22 lib as Clark would
  have shipped it. If a pristine 15.22 (pre-Turbo-C-touch) surfaces
  from Carsten later, swap it in without redoing scaffolding.

### Related also in reference/pcball/pcboard/

- `pcb-main/` — full PCBoard main source (SUPPORT, MODEM, USERS, PPL,
  DISPLAY, FIDO, COMPILER, ASM, DOS, H, MSG, NODE)
- `pcb-main/153/` — 15.3-era build configs (PCBOARD.MAK, PPLC.MAK,
  PCBWAT2.WPJ, etc.)
- `pcb-main/SOURCE/COMPILER/SCOMP.CPP` — PPL compiler main module (Scott
  Dale Robison, © 1994)
- `pcb-main/SOURCE/PPL/*.CPP` — PPL runtime + compiler class (SCRCOMP,
  SCREXEC, SCRMISC, VAR, NEWSCR, PCBMISC, ...)
- `pcb-util/PCBCP/1522/` — PCB Country compiler 15.22-specific build
  config (COMPILE.CMD, PCBCP.MAK, PCBCP.CFG)

This is likely Clark's own dev tree preserved by pcball. Enormously
useful; underdocumented in the repo.

### Still open

- Confirmed pre-15.3 (15.0/15.1/15.2/15.22) C toolkit source with era
  markers. Best lead: **Carsten @ pcboard.be** (the largest known
  PCBoard archive). Still worth asking; the `pcball/pcb-libs/` tree
  above may already answer this once its era is nailed down.

## Path forward

1. **Confirm era of `reference/pcball/pcboard/pcb-libs/`.** Diff its
   SOURCE tree systematically vs `toolkit/pwa153/` (case-insensitive).
   Look at copyright dates, version macros, TYPES.HPP evolution,
   PCBTOOLS.H content. If it's 15.22, promote to `toolkit/pwa1522/`.
   If it's early-15.3, note that pwa1522 still needs a distinct
   source and Carsten's lead remains open.
2. **Populate `toolkit/pwa1522/`** with SOURCE/H/CFG following the
   same per-version structure as pwa153, once the source era is
   confirmed.
3. **Build its libraries** using the same three C compilers + four
   memory models (`BUILD pwa1522` — currently stubbed) and the
   period-correct compilers where they matter.
4. **Document the 15.22 -> 15.3 toolkit upgrade** (what a door author
   would need to adjust).

## Relationship to the IC work

Previously a shared blocker — both goals needed PPLC 3.20. That
blocker is now split:

- **IC byte-exact reconstruction** — unblocked. PPLC 3.20 is in
  `toolkit/pplc/3.20/`. Try `RUNINET.PPS` -> compare to 1808 bytes.
- **pwa1522 toolkit** — still needs 15.22-era C toolkit source (or
  confirmation that `pcball/pcb-libs/` is it).

## Naming note

Previously tracked under the working codename `pwa150`. Renamed to
`pwa1522` to reflect the actual era we're chasing (15.22, the last
version before the 15.3 PWA source begins). The `150` name was a
placeholder for "any pre-15.3 15.x" and made the shared-hunt link to
IC (which is specifically 15.22) less obvious.

## Priority

Lower than completing the 15.3 SDK (pwa153: PCBKBC done; PCBKIT done;
PCBKMS pending a DPMI host). Recorded here so the upgrade-path idea
isn't lost. Now that PPLC 3.20 is in hand, the *IC* reconstruction can
be attempted independently of finishing pwa1522.
