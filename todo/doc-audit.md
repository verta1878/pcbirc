# Doc audit

## Stale — corrections needed

### Phase 1b should not exist

I added it as a gap. It is not one. `patches/153_to_154.patch` already exists —
9 MB, ~1,360 files — and spot-checking it against Clark's WHATSNEW shows all
twelve PPL 3.40 additions present: `GETBANKBAL`, `SETMSGHDR`, `MOVEMSG`,
`SHORTDESC`, `U_BIRTHDATE`, `U_EMAIL`, `U_GENDER`, `U_WEB`, `U_SHORTDESC`.

(I first reported `GetBankBal` missing. My grep was case-sensitive; PPL
identifiers are uppercase.)

**Replace Phase 1b with a verification note**, not a work phase. The work is
done; what is worth doing is checking the patch line by line against WHATSNEW
and HISTORY, and testing against Clark's 15.4 binaries.

### Phase 1 says the wrong baseline

Phase 1 describes porting **15.3** to Watcom. The actual layering is:

```
15.3 source
  + 153_to_154.patch                  -> 15.4 Borland source
  + 154_watcom_phase0_complete.patch  -> 15.4 Watcom + 12 utilities
```

So Phase 1 is finishing a Watcom port of a **15.4** base. Correct the text.

### `154_borland_to_154_watcom.patch` is superseded

Per `patches/README.md`, superseded by `154_watcom_phase0_complete.patch`.
Anything referencing it should point at the newer one.

---

## Duplication

### Three docs cover the PCBDraw port

| Doc | Lines | Contains |
|---|---:|---|
| `sdk/PORT-PCBDRAW.md` | 610 | analysis, corrections, RIP suite scope, testing |
| `sdk/PLACEMENT.md` | 140 | tree layout, cross-repo reference |
| `sdk/PORT-RECORD.md` | 119 | file-by-file mapping |

`PLACEMENT` and `PORT-RECORD` overlap on layout. **Merge PLACEMENT into
PORT-RECORD** — the mapping needs the layout around it anyway, and one file is
easier to keep true than two.

`PORT-PCBDRAW.md` stays separate: it is the reasoning, not the reference. But
see below.

### `PORT-PCBDRAW.md` carries its own history

It contains several sections I marked "Correction" as the understanding
changed — v4 ruled out, then allowed, then ruled out again for a better
reason. Honest, but it means a reader has to work out which paragraph is
current.

**Split it**: a short current-state doc, and an appendix of superseded
reasoning. Keep the appendix — the corrections record *why* decisions landed
where they did, which is the part that gets lost otherwise.

### `sdk/README.md` and `sdk/ARCHITECTURE.md` both explain the toolkit

README explains it for someone writing a door. ARCHITECTURE explains it for
someone building it. That split is defensible — but the vocabulary section
(Pascal calling convention, memory models, classes) is duplicated across both.
Keep it in README, reference it from ARCHITECTURE.

### `phase27/BINARY-CATALOG.md` and the Phase 27 section

The catalog is 550 lines; the phase entry is a summary. That is fine — but the
phase entry has grown to the point where it repeats most of the catalog.
Trim the phase entry to a pointer.

---

## Duplication with the repo's own docs

The repo already has 23 docs. Several overlap with mine:

| Repo doc | Lines | Overlaps |
|---|---:|---|
| `docs/PCB1541-PHASES.md` | 458 | **mine is 1,615** — same filename, different content |
| `docs/PCBMODEM_STUB_PLAN.md` | 130 | superseded: PCBMODEM has full source, no stub needed |
| `docs/PHASE12_PCBWAVE.md` | 164 | Phase 12 is largely built already in `wav/` (13,597 lines) |
| `docs/PCBDRAW_PHASE5.md` | 300 | predates the whole RIP port decision |
| `docs/PCBMAILER-SCOPE.md` | 84 | check against QFront work |
| `docs/REFERENCE_CATALOG.md` | 581 | check against `BINARY-CATALOG.md` |

**`PCB1541-PHASES.md` is the urgent one** — two files, same name, 458 lines
versus 1,615. Whichever lands in the repo silently replaces the other. Decide
which is authoritative before either is committed.

**`PCBMODEM_STUB_PLAN.md` is definitely stale** — it plans a stub for a
program whose complete C++ source is in `Pcb-util/PCBMODEM/`, including
`MODEMS.H` with the exact record layout. Delete or mark superseded.

**`PHASE12_PCBWAVE.md` needs revisiting** against `wav/` — 44 units covering
WAV, MP3, MIDI with FM synthesis, MOD/S3M/XM, FLAC, plus a Sound Blaster DMA
driver. The phase asks for rather less than already exists.

---

## Recommended actions

1. Delete Phase 1b, replace with a verification note under Phase 1
2. Fix Phase 1's baseline: 15.4, not 15.3
3. Merge `PLACEMENT.md` into `PORT-RECORD.md`
4. Split `PORT-PCBDRAW.md`: current state + superseded-reasoning appendix
5. Resolve the two `PCB1541-PHASES.md` files
6. Mark `PCBMODEM_STUB_PLAN.md` superseded
7. Revisit `PHASE12_PCBWAVE.md` against `wav/`
8. Check `REFERENCE_CATALOG.md` against `BINARY-CATALOG.md` for overlap
9. Move the vocabulary section out of `ARCHITECTURE.md`, reference README
10. Trim the Phase 27 entry to a pointer at `BINARY-CATALOG.md`
