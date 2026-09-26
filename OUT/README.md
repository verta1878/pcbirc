# OUT/ — build output and reference binaries

What each directory holds, and whether it has anything in it. Check this
file against the filesystem before trusting it — several documents in
this repo have described directories that did not exist.

```
OUT/
  pwa153/            15.3 PWA binaries (Borland C++ 3.1)        BUILT
    upd154/            15.4 PWA upgrade binaries (Borland)      IC only
    bins/              SDK example doors (medium, large)        BUILT
  delta154/          15.4 Delta binaries (OpenWatcom, _W)       BUILT
    bins/              SDK example binaries                     empty
  irc1541/           15.41 IRC binaries (ow2irc)                empty
    bins/              SDK example binaries                     empty
  clark-original/    Clark's own shipped 15.4 beta binaries     REFERENCE
  data/              installer data files, by program           populated
  support/           shared runtime data
```

## Contents

### OUT/pwa153 — 15.3 PWA  ✅ built 2026-09-16

`PCBOARDM.EXE` (961,504 B) — *PCBoard (R) v15.3/M 25*, built from source
with Clark's own BC 3.1 via `MAIN/COMPILE.BAT` → `153/PCBOARD.MAK`.

The 9 category libraries it links against (dos_l, misc_l, screen_l,
scrnio_l, pcb_l, system_l, countryl, doscls_l, toolkitl) were in
`OUT/pwa153/lib/` and **moved to `toolkit/pwa153/bc31/lib/` on
2026-09-17**, beside the PCBKBC SDK. They are built from the toolkit
tree with the same compiler, so they belong with it — Clark kept his in
`LIB/BCDOS/BC31/`, inside the library tree rather than an output
directory.

**Read `BUILD-RECIPE.md` before rebuilding.** It records the two things
that cost the most time when reconstructed instead of read: `PCBOARD.MAK`
does not use `pcbkit_l.lib` at all, and it links some library objects
straight from `lib/bcdos/bc31/<subdir>/large.386/` rather than from the
`.LIB`.

`PCBOARD.EXE` (955,904 B, single-node) built 2026-09-17 from the same
makefile after the v0.3.2 path repoint — byte-identical before and after
that change, which is what proved the repoint touched only where the
linker looks.

**Both were built from the ARCHIVE tree (`PCBSRCV/014`), not from
`pcb153/SOURCE`.** They link and run, but the archive carries a 15.4 leak
in `CALLWAIT.C` and lacks `MD5IMPL.CPP`, which this repo's `PCBOARD.MAK`
builds. Rebuild from `pcb153/SOURCE` before treating either as the 15.3
reference.

Not yet built: the utilities, PPLC.

### The "37 root EXEs" target — what it is really made of

`todo/PCBIC-PWA-BUILD-DRAFT.md` lists 37 root EXEs for `OUT/pwa153/`.
That list comes from the 15.3 **installer manifest** — what Clark
shipped — not from what this repo can build. Checked against every
`.MAK` in the source archive (`PCBSRCV/000/`), 2026-09-17:

**24 have a makefile** — PCBOARD, PCBOARDM, PCBSETUP, PCBSM, PCBSTATS,
PCBPACK, PCBMONI, PCBMODEM, PCBEDIT, PCBFILER, PCBDIAG, PCBNLC,
FIDOUTIL, MAKEIDX, MKPCBTXT, USERNET, UUIN, UUOUT, UUUTIL, UUXFER,
ZMRECV, ZMSEND, PPLC100, PPLC330.

**13 have no makefile anywhere** — PCBDESC, DOORWAY, ENCRYPT, FIXTEXT,
INIT, MKPCBMNU, OVLSIZE, PACKFIDO, RDPCBTXT, TESTFILE, UPGRADE,
VIEWARCH, VIEWZIP. Some of these were never Clark's to begin with:
DOORWAY is Marshall Dudley's third-party door driver, shipped with
PCBoard under licence. Others were small one-file tools kept outside the
product tree — `packfido.c` is the documented case — **resolved 2026-09-24**, reconstructed
from the shipped binary (see `pcb153/SOURCE/MISC/PACKFIDO/README.md`).

So 37 is the shipping target, 24 is the buildable ceiling from what
survives, and 2 is what is built today. Those three numbers should not be
conflated, and earlier drafts did.

### OUT/pwa153/bins — SDK example doors  ✅ built 2026-09-17

Six of Clark's seven sample doors from
`toolkit/pwa153/SOURCE/TOOLKIT/SAMPLES/`, linked against
`PCBKBC{M,L}.LIB` — the proof the SDK builds end to end. No small or
compact: those hit the 64K single-code-segment limit, which is inherent,
not a defect. See the README there.

### OUT/pwa153/upd154 — 15.4 PWA upgrade

Holds the 6 PCBIC v1.2 binaries only (Pcbic, Pcbic2, PCBICCFG, PCBICEVT,
TESTIC, TESTIC2). The rest of the 15.4 upgrade is not built — the
reconstructed source needs its build-fix pass first. See
`pcb153/upd154/README.md`.

### OUT/delta154 — 15.4 Delta

15 OpenWatcom binaries, `_W` suffix. Recovered 2026-09-16 from
`e4181e5^` after being deleted in the 2026-08-25 restructure and never
re-added. See the README there.

### OUT/clark-original — Clark's originals

Clark Development's own shipped 15.4 beta binaries: 12 EXEs (11 DOS,
1 OS/2), newest build stamp 04/22/97, plus the documents as shipped.
**Not our output. Never overwrite.** `beta-patched/` holds three
date-check-patched variants recovered from git history; they are not the
reference.

### OUT/data — installer data

Every data file the installer ships, grouped by program rather than by
install-tree layout: ic12, commdrv, pcbmail, pcbos2, help, doc, ppl,
gen, fido, graphics, files, main, conferences, root, dl01. No EXEs, no
source. Where a program's own layout has name collisions across
subdirectories, the subdirectories are mirrored rather than flattened.

### OUT/support — shared runtime data

PCBOARD.SER, PCBSM.CLR, PCBSM.CNF, ENDPCB. Not version-specific.

## Two conventions worth knowing

**Program EXEs sit at the version top level.** `bins/` inside a version
directory is for *SDK example binaries* — the small buildable sample
add-ons that prove the SDK builds end to end. Shipping binaries do not
go there.

**The SDK is not here.** The per-compiler SDK matrix
(PCBKBC/PCBKIT/PCBKMS, four memory models each) lives under
`toolkit/<branch>/<compiler>/lib/`. `OUT/lib/` was deleted on
2026-09-17; do not recreate it. Status:
`MAIN/build/SDK-BUILD-STATUS.md`.

As of 2026-09-17 one leg exists: `toolkit/pwa153/bc31/lib/` holds
`PCBKBC{S,C,M,L}.LIB` — 152 modules, all four memory models. That is
the door SDK. The 9 category libraries now sit in that same directory
and are a different thing: they link `PCBOARD.EXE`, are large-model
only, and are split by source area rather than merged. The README there
has the full distinction.

## Per-version source → output

| Version | Source | Toolkit | Output |
|---|---|---|---|
| 15.3 PWA | pcb153/SOURCE | toolkit/pwa153 | OUT/pwa153 |
| 15.4 PWA | pcb153/upd154/SOURCE | toolkit/pwa154 | OUT/pwa153/upd154 |
| 15.4 Delta | pcb154/MAIN/SOURCE | toolkit/delta154 | OUT/delta154 |
| 15.41 IRC | pcb1541/ | toolkit/irc1541 | OUT/irc1541 |

`clark-original/` sits at the top level deliberately: Clark's material is
not any one build leg's output.
