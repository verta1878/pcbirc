# OUT/delta154 — 15.4 Delta binaries (OpenWatcom)

The crew's 15.4 Delta rebuild, compiled with **OpenWatcom** from
`pcb154/MAIN/SOURCE/`. `_W` suffix = Watcom build.

**These are NOT Clark's binaries.** Clark built 15.4 with Borland C++
3.1; his originals are the reference in `OUT/clark-original/`.

## Contents — 15 executables

| Binary | Bytes | What |
|---|---|---|
| PCBOARD_W.EXE | 1,281,492 | Main BBS engine |
| UUIN_W.EXE | 1,404,500 | UUCP inbound |
| UUOUT_W.EXE | 1,404,500 | UUCP outbound |
| UUUTIL_W.EXE | 1,404,500 | UUCP utility |
| UUXFER_W.EXE | 1,404,500 | UUCP transfer |
| PPLC_W.EXE | 1,276,464 | PPL 3.40 compiler |
| LOCAL_W.EXE | 1,275,628 | Local login mode |
| PCBSETUP_W.EXE | 433,986 | Setup utility |
| PCBSM_W.EXE | 226,072 | System Manager |
| PCBCP_W.EXE | 77,952 | OS/2 PM control panel |
| PCBIS_W.EXE | 48,478 | Config TUI |
| MAKEIDX_W.EXE | 37,672 | Index builder |
| USERNET_W.EXE | 27,774 | User network flags |
| MKPCBTXT_W.EXE | 27,506 | Text file builder |
| MAKEHELP_W.EXE | 25,114 | Help file builder |

SHA256 sums in `CHECKSUMS.sha256`.

`bins/` holds SDK example binaries, not program EXEs, and is empty for
this branch — the delta154 SDK has not been built.

## 13 binaries the root README claims that are not here

Checked 2026-09-17 against the filesystem. `README.md` describes **28**
`_W` binaries across two tables; this directory holds **15**. Missing:

- **PCBTEXT_W.EXE** (listed at 39 KB) — was never in `bin/watcom/`
  either, so it was never recovered because it was never committed.
- **The entire "Clark Utilities — Phase 0" table (12).** PCBSTATS_W,
  PCBPACK_W, MSETUP_W, PCBMODEM_W, PCBEDIT_W, PCBMONI_W, PCBDIAG_W,
  PCBFILER_W, PCBNLC_W, OFFLINE_W, WAITBU_W, PCBTITLE_W. None of these
  are in this directory and none were in `bin/watcom/`.

Also **MKPCBTXT_W.EXE** is listed at 86 KB; the file here is 27,506 B.

An earlier revision of this file said "two entries do not match." That
undercounted — it compared against only the first table. The real gap is
13 of 28, and one whole table of the root README describes work whose
output is not in the repo.

What that means is genuinely open and should not be guessed at: either
those 12 utilities were built and the binaries were lost the same way
the other 15 nearly were, or the table describes intended work that was
recorded as done. Resolving it needs the same blob-hash search through
history that recovered these 15 — filename matching is not enough,
because the point is that files were moved and renamed.

## Provenance — recovered 2026-09-16

These were committed to `bin/watcom/` in `7f6e9f3` (2026-08-08,
"PCBoard 15.4 repo update"), with `PCBOARD_W.EXE` refreshed in `11bebe8`
the same day. All 15 were deleted in **`e4181e5`** (2026-08-25, "Session
Summary — Toolkit Restructure + Cleanup") and never re-added anywhere —
verified by blob hash against the current tree, not by filename.

`DOCS/DELTA154-CHANGES.md` has said since that restructure:

> 15 EXEs in OUT/delta154/ (Watcom, _W suffix). Verified executing under
> DOSBox-X.

They never arrived. The restructure moved them out of `bin/watcom/` and
into nothing. Nobody noticed because `.gitignore`'s `*.EXE` rule meant
git could no longer see them — the same failure that lost Clark's 15.4
beta binaries in the same commit (see `OUT/clark-original/README.md`).

Recovered here from `e4181e5^`.

Because the recovery came from the 2026-08-08 commit, these are that
build, not necessarily the newest one. If a later Watcom set exists
outside the repo it should replace these. Until then these are the only
Watcom 15.4 binaries that survive anywhere.

## Rebuilding

See `MAIN/README.md` (Build — OpenWatcom) and
`pcb154/DOCS/DELTA154-CHANGES.md` for the Watcom fixes applied. Build
order is in `pcb154/README.TXT`.
