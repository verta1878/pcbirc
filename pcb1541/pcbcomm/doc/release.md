# pcbcomm release — wrench, 2026-09-26

Extract at repo root. Paths use pcb1541/pcbdcom/ (before verta's
directory rename to pcb1541/pcbcomm/).

## New source files (pcb1541/pcbdcom/src/)
- pcbdtsr.c (409L) — PCBDTSR.EXE, resident TSR
- drvsetup.c (457L) — DRVSETUP.EXE, config editor
- test.c (341L) — TEST.EXE, port diagnostics
- PROGRAMS-README.md — documents all three programs

## Updated source files
- int14.c (701→731L) — AH=FFh admin handler for PCBDTSR -d

## Updated docs (pcb1541/pcbdcom/)
- BUILD-STATUS.md — delta 15.4 tightening + piece inventory table
- README.md — two-tree relationship rewrite + delta 15.4 checklist
- ATTIC-CLEANUP.BAT — moves int14-r1.c to attic (move-only, no deletes)
- SPEC.md, GAP-ANALYSIS.md — pcbcomm naming throughout

## Updated docs (pcb1541/pcbdcom/doc/)
- COMM-H-MERGE.md, FOSSIL-OBJ-ANALYSIS.md, INT14-FOSSIL5C-STATUS.md

## Toolkit docs (toolkit/pwa154/pcbdcom/)
- README.md, src/README.md, docs/LINKOUT.md, docs/SDK.md, lib/README.md

## Standalone reference docs (repo root or project folder)
- PCBDCOM-INVENTORY.md — full piece inventory
- PCBDCOM-PLAN.md — consolidated plan
- PCBDCOM-PLANS-INDEX.md — index of ~13 plan docs
- PCBDCOM-REASSEMBLY-START.md — "read this first" recovery doc

## Completed: items 3, 5, 6, 7, 12, 13 + int14.c AH=FFh
## Remaining: items 1, 2, 4, 8, 10, 11
