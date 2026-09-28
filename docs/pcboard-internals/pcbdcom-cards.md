# COMMDRV / pcbdcom — Card support: shipped, stubbed, and hidden

**Scope**: full inventory of the multiport serial cards WCSC's `COMMDRV`
driver system claims to support versus what was actually implemented in
`COMMDRV.EXE` / `COMMTSR.EXE` / the `COMMDV*.DRV` backend modules. Cross-
referenced against Clark's shipped installer disks and the reverse-
engineered `pcb154/pcbdcom/` re-implementation.

**Sources analyzed**:
- `pcb1541/install/dist/target/COMMDRV/COMMDRV.EXE` — the loader
- `pcb1541/install/dist/target/COMMDRV/COMMTSR.EXE` — the TSR runtime
- `pcb1541/install/dist/target/COMMDRV/DRVSETUP.EXE` — the config UI
- `pcb1541/install/dist/target/COMMDRV/COMMDV{00..08}.DRV` — 9 backend modules
- `pcb1541/install/dist/target/COMMDRV/{XABIOS,XACOMX,XACOOK,BOCA1610}.BIN` — firmware
- `pcb1541/install/dist/target/COMMDRV/{ARNETSP4,ARNETSP8,DIGI4E,DIGI8E}.DAT` — sample configs
- `pcb1541/install/INSTALL.zip → INSTALL.DAT` — installer script (deploys the above)

## Headline findings

1. **9 backend modules ship, 3 UI stubs never shipped.** `DRVSETUP.EXE`
   offers 11 card types in its configuration menu; `COMMDV*.DRV` implements
   only 8 of them plus a Windows VxD passthrough. Three cards are UI-only
   options with no backend, no firmware, no sample config — pure stubs.

2. **The TSR knows about 3 additional cards the UI doesn't expose.** Sub-
   variants like `ARNETSPLUS`, `DIGIPCXE`, and `DIGI2PORT` are named in
   `COMMTSR.EXE`'s runtime switch but `DRVSETUP.EXE` doesn't offer them
   as menu choices — you'd have to hand-edit the config file to select them.

3. **The 9th backend is a hidden Windows 95 VxD.** `COMMDV08.DRV` has no
   signature string of the usual `NAME  1.00` form; instead it identifies
   as `"COMMDRV VxD 1.00"`. Not exposed in any UI, not documented anywhere
   in the installer materials — an internal path for the Windows 95 build
   of PCBoard/M.

## The 9 shipped backend modules

Each `COMMDV*.DRV` is a discrete backend implementation loaded by
`COMMDRV.EXE`. Backend signatures come from a fixed `NAME  1.00` byte
sequence near the start of each `.DRV`.

| File | Signature | Card / target hardware |
|---|:---|:---|
| `COMMDV00.DRV` | `GENERIC 1.01`     | Standard 8250/16450/16550 UART (COM1-COM4 style) |
| `COMMDV01.DRV` | `HUB6 1.00`        | HUB-6 multiport card (a specific ISA design) |
| `COMMDV02.DRV` | `DIGI-COMXI 1.00`  | DigiBoard COM/Xi (EISA / MCA bus intelligent multiport) |
| `COMMDV03.DRV` | `ARNET-SPORT 1.00` | Arnet SmartPort (ISA 4-port / 8-port) |
| `COMMDV04.DRV` | `BOCA(1610) 1.00`  | Boca BB1610 / BB2016 (ISA 4-port / 8-port) |
| `COMMDV05.DRV` | *no signature*     | DigiBoard PC/Xe, PC/Xi, PC/8i family (FEP-based intelligent) |
| `COMMDV06.DRV` | *no signature*     | GTEK BBS-8 / BBS-16 (dumb multi-UART cards) |
| `COMMDV07.DRV` | `INT14H 1.00`      | INT 14h software gateway (network / DOS emulator use) |
| `COMMDV08.DRV` | `COMMDRV VxD 1.00` | Windows 95 VxD passthrough (internal — no UI) |

## The 3 UI stubs (planned, never shipped)

`DRVSETUP.EXE`'s card-selection menu offers these three, but there is:
- No `COMMDV*.DRV` implementing them,
- No firmware `.BIN` for them,
- No sample `.DAT` config for them,
- No mention in TSR runtime strings.

Trying to use them would fail at driver load time.

| UI label | What it was | Status |
|:---|:---|:---|
| `AST` | AST 4-port serial card family (AST FourPort was a well-known 80s multiport) | **STUB** — UI accepts, no backend |
| `BOCA-DMB` | Boca "Dual Mode Board" — a hybrid ISA card | **STUB** — UI accepts, no backend |
| `PC-COM` | PC-COM 8-port serial (specific vendor's ISA card) | **STUB** — UI accepts, no backend |

The presence of these in the UI without implementation is consistent with
Clark's usual pattern: reserve the option slot early so sysops don't have
to reconfigure when the backend eventually ships. In this case the backends
never did.

## The 3 TSR-only variants (implemented but not menu-selectable)

`COMMTSR.EXE`'s runtime dispatch recognizes these card-type IDs, but
`DRVSETUP.EXE` doesn't offer them as choices. To use them you'd manually
edit the generated config file.

| TSR ID | Variant of | Difference | Why hidden |
|:---|:---|:---|:---|
| `ARNETSPLUS`  | ARNET SmartPort | "Plus" model — different chip revision or firmware | UI collapses both under `ARNET` |
| `DIGIPCXE`    | DIGI-PCX family | PC/Xe — 2-port variant of the FEP family | UI collapses into `PC/XI` |
| `DIGI2PORT`   | DIGI-PCX family | 2-port variant (predecessor to PC/Xe) | UI collapses into `PC/XI` |

The UI's `PC/XI` selection probably auto-detects which specific variant is
installed at runtime and routes to the right TSR handler; sub-selecting
manually would only matter for edge cases.

## Firmware inventory

Some backends load firmware blobs into their card's on-board processor
at TSR init time. Others are pure ISA I/O and need none.

| Firmware | Size | Loaded by | For |
|---|---:|:---|:---|
| `XABIOS.BIN`   | 2,048 | `COMMDV05.DRV` (DIGI-PCX*) | DigiBoard PC/Xe/Xi/8i BIOS |
| `XACOOK.BIN`   | 6,144 | `COMMDV05.DRV` (DIGI-PCX*) | DigiBoard PC/Xe FEP "cook" |
| `XACOMX.BIN`   | 6,144 | `COMMDV02.DRV` (DIGI-COMXI) | DigiBoard COM/Xi firmware |
| `BOCA1610.BIN` | 3,228 | `COMMDV04.DRV` (BOCA-1610)  | Boca BB1610 IUART firmware |

No firmware ships for GTEK (`COMMDV06`) — those are dumb 16550-based cards.
No firmware for the AST / BOCA-DMB / PC-COM stubs (further confirmation
they were never implemented).

## Sample configs (INSTALL.DAT's optional block)

`INSTALL.DAT` at lines 608-612 offers to install these config files:

- `ARNETSP4.DAT` (2,053 B) — Arnet SmartPort 4-port sample
- `ARNETSP8.DAT` (3,397 B) — Arnet SmartPort 8-port sample
- `DIGI4E.DAT`   (2,053 B) — DigiBoard PC/Xe 4-port sample
- `DIGI8E.DAT`   (3,397 B) — DigiBoard PC/Xe 8-port sample

The presence of DIGI4E/8E samples but no BOCA sample suggests DigiBoard
was Clark's reference target. Boca had a smaller sysop install base at
the time and DigiBoard's XA/XE line was the industry standard for BBS
multiport support.

## The Windows VxD path

`COMMDV08.DRV` = `COMMDRV VxD 1.00`. This backend translates COMMDRV API
calls into calls against a Windows 95 VxD (probably one bundled with the
PCBoard/W port that was in progress but never fully shipped). It's the
odd one out:

- Not exposed in `DRVSETUP.EXE`'s menu.
- Not mentioned in `INSTALL.DAT`.
- Not documented in any shipped `.DOC` file.
- Runtime string `"New cardseg"` suggests it dynamically remaps the card
  segment on the fly, which is a Windows-specific concern.

This lines up with the PCBoard 15.4 release notes' hints of an "internal
Windows testbed" — this was the plumbing.

## Cross-references

- `docs/pcboard-internals/PLANNED-FEATURES.md` — MCI / PPL / prompt gaps
- `pcb1541/install/RUNTIME-DEPS.md` — firmware + FOSSIL / archiver deps
- `pcb154/pcbdcom/` — reverse-engineered replacement (all 8 backends re-implemented)
- `pcb154/pcbdcom/GAP-ANALYSIS.md` — implementation status of the rewrite

## Bottom line

Clark shipped 9 backends covering 8 UI-exposed card types plus a hidden
Windows VxD. Three UI options (`AST`, `BOCA-DMB`, `PC-COM`) are pure
stubs — reserved menu slots for backends that never made it to release.
The TSR runtime also knows about 3 sub-variants (`ARNETSPLUS`, `DIGIPCXE`,
`DIGI2PORT`) that the UI intentionally collapses into their parent
choices.

No documentation anywhere calls these out; the stub status is only visible
by cross-referencing the DRVSETUP menu list against the shipped
`COMMDV*.DRV` inventory and the TSR runtime dispatch table.
