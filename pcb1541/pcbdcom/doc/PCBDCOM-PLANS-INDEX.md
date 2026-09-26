# pcbdcom / COMM-DRV — Index of Plan & Status Docs
_A map so we stop rediscovering these. wrench, 2026-09-25._

pcb1541 = the latest work. Docs below are grouped by what they're for.

## Plans / roadmaps
| Doc | What it's for |
|---|---|
| `todo/pcbdcom-clean-room-plan.md` | The Phase 1 / Phase 2 clean-room legal wall — how to build pcbdcom without touching COMM-DRV internals. |
| `todo/toolkit.md` | The toolkit recreated bit-for-bit across four versions (pwa153 / pwa154 / delta154 / etc.) — explains the tree relationships. |
| `todo/SOURCE-RECOVERY.md` | SDK source recovery — tracks what the crash took and what's been reversed/restored. |
| `todo/PCBMODEM_STUB_PLAN.md` | PCBMODEM stub replacement plan. |
| `todo/SDK-1541-IMPROVEMENTS.md` | Ideas for improving the 1541 SDK. |
| `PCBDCOM-PLAN.md` | Current consolidated snapshot: ground-truth state of all trees + ordered plan + open decisions. (The "where are we / what next" doc.) |

## Spec / status / reference
| Doc | What it's for |
|---|---|
| `pcb1541/pcbdcom/SPEC.md` | The v1 interface specification. |
| `pcb1541/pcbdcom/GAP-ANALYSIS.md` | COMMDRV.RED feature manifest from INSTALL.DAT — what COMM-DRV has that pcbdcom must match. |
| `pcb1541/pcbdcom/BUILD-STATUS.md` | Build status / verification log. |
| `pcb1541/pcbdcom/doc/INT14-FOSSIL5C-STATUS.md` | FOSSIL 5 Rev C INT 14h handler status + the 4-backend architecture. |
| `pcb1541/pcbdcom/doc/FOSSIL-OBJ-ANALYSIS.md` | OMF symbol/import analysis of the reversed FOSSIL.OBJ. |
| `docs/pcboard-internals/PCBDCOM-CARDS.md` | Card support: shipped / stubbed / hidden across COMM-DRV & pcbdcom. |
| `docs/pcboard-internals/PLANNED-FEATURES.md` | Source-derived inventory of planned & late-added PCBoard features. |

## Quick "where do I look?" cheat
- **"What's the current state / what's next?"** -> `PCBDCOM-PLAN.md`
- **"How do the toolkit versions relate?"** -> `todo/toolkit.md`
- **"What did the crash take?"** -> `todo/SOURCE-RECOVERY.md`
- **"What must we match in COMM-DRV?"** -> `GAP-ANALYSIS.md` + `PCBDCOM-CARDS.md`
- **"Is it legal / clean-room?"** -> `todo/pcbdcom-clean-room-plan.md`
- **"Does it build?"** -> `BUILD-STATUS.md`
