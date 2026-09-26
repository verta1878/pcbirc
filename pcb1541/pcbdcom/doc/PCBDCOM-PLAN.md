# pcbcomm — Consolidated Plan (updated 2026-09-26)
_wrench (transport/FOSSIL) — pcbcomm reassembly project_
_Renamed from pcbdcom: supports DOS + OS/2, not DOS-only_

## 1. Ground truth (verified on disk today)

| Tree | Role | State |
|---|---|---|
| `pcb1541/pcbdcom/` | Latest code (crashed) — dirs to rename pcbcomm | 23 src, 5 headers, 7 docs, 1 MAK |
| `toolkit/pwa154/pcbdcom/` | 154 SDK — dirs to rename pcbcomm | 16 src, 5 headers, 4 docs, 3 examples |
| `toolkit/pwa153/` | Intact Borland reference | FOSSIL.C + COMMDRV.OBJ + FOSSIL.OBJ + 10 TKLIB.MAK |
| `toolkit/delta154/` | OpenWatcom toolkit | 377 files, WATCOMPAT.H — 0 COMMDRV, never wired |
| `pcb154/` | 154 base source | MODEMDRV.C + 10 MODEM*.C — NO pcbcomm/ |
| `pcbcbase/COMMDRV/` | Clark header | COMM.H (reconstructed, 6594B) |

The two trees are NOT a fork — pcb1541 = source-code home, toolkit/pwa154 =
redistributable SDK. Relationship = source → built artifact. Don't merge.

## 2. Naming (settled this session)

**Project:** pcbdcom → **pcbcomm** (covers DOS + OS/2)

**Source files:**
- `ser_rs232_shim.c` → **pcbcomm.c** (compiles to PCBCOMM.OBJ)
- `nopcbdcom_stub.c` → **pcbcomms.c** (PCBCOMMS.OBJ, S = stub — Clark's
  NO*.C pattern; Clark never had a NOCOMMDRV because COMMDRV was third-party)

**OBJ:** one name regardless of compiler — PCBCOMM.OBJ, PCBCOMMS.OBJ

**Binaries (W = OpenWatcom, 2 = OS/2):**
- PCBCOMM.EXE / PCBCOMMW.EXE / PCBCOMM2.EXE (loader/driver)
- PCBDTSR.EXE / PCBDTSRW.EXE / PCBDTSR2.EXE (TSR)
- DRVSETUP.EXE / DRVSETUPW.EXE / DRVSETUP2.EXE (card config)
- TEST.EXE / TESTW.EXE / TEST2.EXE (port tester)
Long names fine as build output; 8.3 renaming at installer-disk time.

## 3. Output structure (from OUT/README.md)

Built binaries go in OUT/ by version. Program EXEs at the top level;
bins/ is for SDK example binaries only. SDK OBJ/LIB is NOT in OUT/ —
goes under toolkit/<branch>/<compiler>/lib/.

| Version | Source | Toolkit | Output |
|---|---|---|---|
| 15.3 PWA | pcb153/SOURCE | toolkit/pwa153 | OUT/pwa153 |
| 15.4 PWA (Borland) | pcb153/upd154/SOURCE | toolkit/pwa154 | OUT/pwa153/upd154 |
| 15.4 Delta (OW2) | pcb154/MAIN/SOURCE | toolkit/delta154 | OUT/delta154 |
| 15.41 IRC | pcb1541/ | toolkit/irc1541 | OUT/irc1541 |

**Note:** OUT/pwa153/upd154 = Borland 15.4 PWA upgrade. NOT OW2.

## 4. Memory model

- Borland / MSC: large model (-ml / /AL)
- OpenWatcom: **FLAT** (WATCOMPAT.H erases far/huge, farmalloc→malloc)
- All SDKs should be ported to OW2
- Borland SDK ships 4 models (S/C/M/L for doors); OW2 is flat only

## 5. What's DONE

- 10 clean card backends (COMMDV00-04, 06-07 + chase_iolan, equinox_sst,
  stallion_brumby) — hexadecimal, code-complete, UNTESTED
- FOSSIL.C reversed (sysop/0, 79/79 exports match Clark's OBJ)
- COMM.H reconstructed
- int14.c full 28-function FOSSIL dispatch
- pcbcomm.c (→ PCBCOMM.OBJ) shim exists (self-contained version in found
  pre-crash zip is the recovery piece)
- pcbcomms.c (→ PCBCOMMS.OBJ) stub exists

## 6. What's LEFT (the finish road)

**TO FINISH:**
1. COMMDV05 digi_pcxe (58L skeleton with probe + FEP framework)
2. 4 backends with TODO markers (cyclom ⚠1, digi_accel ⚠1, easyio ⚠2,
   rocket ⚠3)
3. irq.c (4 TODO markers)

**TO START (never begun):**
4. COMMDV08 — COMMDRV VxD bridge ("COMMDRV VxD 1.00", 2284B original)
5. PCBDTSR.EXE — resident TSR
6. DRVSETUP.EXE — card configuration screen
7. TEST.EXE — port test utility

**TO WIRE:**
8. Land self-contained shim (pcbcomm.c) into pcb1541 source tree
9. Wire delta154 OpenWatcom build (comm.h guards, flat model)
10. Create pcb154/pcbcomm (or wire include path to pcb1541)

**CLEANUP:**
11. Attic int14-r1.c (superseded, nothing references it)
12. Refresh stale docs (README wrong "folded in" framing, BUILD-STATUS
    predates board completion, reconstruct lost exe/com .md)
13. Rename dirs + files from pcbdcom → pcbcomm throughout

## 7. Recovery notes

- hexadecimal's build method: LOST in crash
- The .md documenting exe/com file info: LOST
- pcb1541 = latest work; gets ANALYZED into pcb154/delta (not mechanical
  move), then pcb1541 can be retired. Analysis first.
- Found pcbdcom-source.zip (pre-crash snapshot): superseded on everything
  EXCEPT the self-contained shim (229L, inline RS232ERR + port_param).
  That's the recovery piece for the source tree.

## 8. Open decisions (for verta)

- Priority order for items 1-13
- pcb154: own pcbcomm/, or include path to pcb1541?
- Is pcb1541 promoted to become pcb154's content, or analyzed piecewise?

## 9. Future seam (note only — don't design for it)

pcbcomm's backend vtable and netfossil (netfosdl) could meet someday —
network/FOSSIL backend, or multiport-aware bridge. No need identified.
The vtable is the join point when one appears.

## 10. Related docs (see PCBDCOM-PLANS-INDEX.md for the full map)

- todo/pcbdcom-clean-room-plan.md — the legal clean-room wall
- todo/toolkit.md — the four toolkit versions explained
- todo/SOURCE-RECOVERY.md — crash recovery tracking
- pcb1541/pcbdcom/SPEC.md — v1 interface spec
- pcb1541/pcbdcom/GAP-ANALYSIS.md — COMMDRV.RED feature gaps
- PCBDCOM-INVENTORY.md — full piece-by-piece inventory
- PCBDCOM-REASSEMBLY-START.md — the "read this first" recovery doc
