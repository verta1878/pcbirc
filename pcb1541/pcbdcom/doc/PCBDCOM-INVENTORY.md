# pcbcomm — Full Piece Inventory (corrected)
_wrench · 2026-09-26 · updated with verta's corrections_
_Project renamed: pcbdcom → pcbcomm (covers DOS + OS/2, not DOS-only)_

## Where the pieces live (7 trees)

| Tree | Role | Files |
|---|---|---|
| A. `pcb1541/pcbdcom/` | Latest code (crashed version) — dirs to be renamed pcbcomm | 23 src, 5 headers, 7 docs, 1 MAK |
| B. `toolkit/pwa154/pcbdcom/` | 154 SDK (redistributable) — dirs to be renamed pcbcomm | 16 src, 5 headers, 4 docs, 3 examples |
| C. `toolkit/pwa153/` | Intact Borland reference | FOSSIL.C + COMMDRV.OBJ + FOSSIL.OBJ + 10 TKLIB.MAK |
| D. `toolkit/delta154/` | OpenWatcom toolkit | 377 files, WATCOMPAT.H bridge — 0 COMMDRV, never wired |
| E. `pcb154/` | 154 base source | MODEMDRV.C + 10 MODEM*.C — NO pcbcomm/ |
| F. `pcbcbase/COMMDRV/` | Original Clark header | COMM.H (reconstructed, 6594B) |
| G. `reference/commdrv/` | COMMDRV reference | 1 file (comm-h-recon-b.h) |

## Naming (corrected by verta)

**Project rename:** pcbdcom → **pcbcomm** (supports OS/2 too, not DOS-only).

**File renames:**
- `ser_rs232_shim.c` → **pcbcomm.c** (compiles to PCBCOMM.OBJ)
- `nopcbdcom_stub.c` → **pcbcomms.c** (compiles to PCBCOMMS.OBJ — S = stub)

**OBJ files — one name, all compilers:**
- **PCBCOMM.OBJ** — the drop-in replacement for Clark's COMMDRV.OBJ (same name regardless of Borland/OW2/MSC)
- **PCBCOMMS.OBJ** — S = stub (empty no-op link-out, satisfies linker
  without pulling in driver code — follows Clark's NO*.C pattern but Clark
  never had a NOCOMMDRV because COMMDRV was a third-party purchase)

**EXE binaries — W/2 suffix = platform (OW2 / OS/2):**

## Programs — what we build vs what Clark shipped

| Clark shipped | Purpose | Our equivalent | Status |
|---|---|---|---|
| COMMTSR.EXE | Resident TSR | **PCBDTSR.EXE** | ❌ NEVER STARTED |
| COMMDRV.EXE | Device driver/loader | **PCBCOMM.EXE** | pcbcomm.c exists (has main + _dos_keep) — status uncertain |
| DRVSETUP.EXE | Card config UI | **DRVSETUP.EXE** (same name) | ❌ NEVER STARTED |
| TEST.EXE | Port test utility | **TEST.EXE** (same name) | ❌ NEVER STARTED |

## Board drivers — ALL 9 card families (COMMDV08 included)

| Clark's .DRV | Card | Our backend | Status |
|---|---|---|---|
| COMMDV00 | Generic 8250/16550 | uart_backend.c (158L) | ✓ done |
| COMMDV01 | Intel Hub6 | hub6_backend.c (152L) | ✓ done |
| COMMDV02 | Digi-ComXi | digi_comxi_backend.c (298L) | ✓ done |
| COMMDV03 | Arnet SmartPort | arnet_backend.c (243L) | ✓ done |
| COMMDV04 | Boca 1610 | boca_backend.c (173L) | ✓ done |
| COMMDV05 | Digi PC/Xe | digi_pcxe_backend.c (58L) | ⚠ NEEDS FINISHING (has probe + FEP framework) |
| COMMDV06 | GTek 8Fx | gtek_backend.c (153L) | ✓ done |
| COMMDV07 | INT14H | int14.c (701L) | ✓ done |
| COMMDV08 | COMMDRV VxD (Win bridge) | — | ❌ NEEDS STARTING ("COMMDRV VxD 1.00", 2284B original) |

Plus 6 post-WCSC card backends (in pcb1541 only):
- chase_iolan (243L) ✓, cyclom (373L) ⚠1, digi_accel (89L) ⚠1,
  equinox_sst (358L) ✓, rocket (340L) ⚠3, stallion_brumby (345L) ✓

## Link-time objects

| Clark shipped | Our replacement | Source file | Status |
|---|---|---|---|
| COMMDRV.OBJ | **PCBCOMM.OBJ** | pcbcomm.c (was ser_rs232_shim.c) | ✅ exists |
| FOSSIL.OBJ | FOSSIL.OBJ | FOSSIL.C | ✅ reversed (sysop/0), 79/79 exports |
| — | **PCBCOMMS.OBJ** | pcbcomms.c (was nopcbdcom_stub.c) | ✅ empty link-out stub |

## Relationship between pcb1541 and pwa154 (clarified)

The two trees are NOT a fork to merge. They are two roles:
- **pcb1541/pcbcomm/** (to rename) = source-code home. Development happens
  here. Has the newer, bigger code (more backends, fuller int14.c, more
  macros in compat.h/backend.h).
- **toolkit/pwa154/pcbcomm/** (to rename) = redistributable SDK. Ships the
  linkable .OBJ variants, examples, drop-in recipe. Consumer-facing.

The relationship is source → built artifact. Don't consolidate.

## What moves between them (not "missing" — separate roles)

The shim source (`pcbcomm.c`) and its header live in BOTH trees by design:
- In the **source tree**: the canonical, development version.
- In the **SDK tree**: possibly a leaner cut for redistribution.

The found pcbdcom-source.zip (pre-crash snapshot) confirms pcb154_pcbdcom
had a SELF-CONTAINED version of the shim (229L, 7398B) with inline RS232ERR
codes + port_param struct — more portable than the toolkit's version (224L,
6834B) which relies on the big pcbdcom.h. The self-contained version is the
recovery piece for the source tree.

The big pcbdcom.h (6,422B in the SDK) vs small (2,307B in pcb1541) is a
ROLE difference, not drift: the SDK header carries full struct declarations
for consumers; the source tree uses the self-contained shim that doesn't
need them.

## The 5 diverged files (clarified — role differences, not drift)

| File | pcb1541 (source) | pwa154 (SDK) | Explanation |
|---|---|---|---|
| backend.h | 2883B / 51L | 2110B / 40L | Source has more backends declared (post-WCSC cards) |
| compat.h | 3949B / 95L | 3186B / 76L | Source has more compiler guards/macros |
| pcbdcom.h | 2307B / 53L | 6422B / 166L | SDK has full struct defs for consumers; source shim is self-contained |
| int14.c | 23454B / 701L | 7448B / 211L | Source is the full 28-fn dispatch; SDK is the older lean cut (STALE — re-cut from source) |
| pcbdcom.c | 9288B / 249L | 8204B / 233L | Source has more loader/TSR logic |

Only int14.c in the SDK is genuinely STALE (the old 211-line cut, not
a deliberate lean version). The rest are legitimate role differences.

## Dead files to attic

- `int14-r1.c` (211L) — superseded earlier revision, nothing references it.

## Firmware/data blobs Clark shipped

ARNETSP4.DAT, ARNETSP8.DAT, BOCA1610.BIN, DIGI4E.DAT, DIGI8E.DAT,
XABIOS.BIN, XACOMX.BIN, XACOOK.BIN

## The finish road (13 items)

TO FINISH:
1. COMMDV05 digi_pcxe (skeleton → complete)
2. 4 backends with TODO markers (cyclom ⚠1, digi_accel ⚠1, easyio ⚠2, rocket ⚠3)
3. irq.c (4 TODO markers)

TO START (new work):
4. COMMDV08 — the COMMDRV VxD bridge (clean-room)
5. PCBDTSR.EXE — the resident TSR
6. DRVSETUP.EXE — card configuration screen
7. TEST.EXE — port test utility

TO WIRE:
8. Land self-contained shim (pcbcomm.c) into pcb1541 source
9. Wire delta154 OpenWatcom build
10. Create pcb154/pcbcomm (or wire include path)

CLEANUP:
11. Attic int14-r1.c
12. Refresh stale docs (README, BUILD-STATUS, reconstruct lost exe/com .md)
13. Rename dirs + files from pcbdcom → pcbcomm throughout
