# Version Model — 15.3 PWA, 15.4 PWA, 15.4 Delta

Three distinct things. Keeping them straight avoids confusion.

## 15.3 PWA

- Directory: `pcb153/`
- The base source PWA released (from Corey Blake's licensed copy)
- No 15.4 features
- Builds with Borland C++ 3.1

## 15.4 PWA (Clark's)

- Directory: `pcb153/upd154/`
- Clark's 15.4, released as binary upgrades on top of 15.3,
  reconstructed into source form
- = 15.3 PWA base + the changes recovered from Clark's 15.4 binaries
- Represents Clark's shipped 15.4 (all features present, reconstructed
  from the binaries)
- Builds with Borland C++ 3.1 (same lineage as 15.3 PWA)
- Compile status: NOT yet 100% — needs a build-fix pass (same
  "patched to compile" issues as 15.3: missing STATS OBJ, a few
  header/struct resolutions). See pcb153/upd154/README.md.
- The patch 15.3 PWA -> 15.4 PWA is `patches/15.4-pwa.patch`,
  labeled "15.4 PWA"
- Output: `OUT/pwa153/upd154/` (our rebuild). Clark's original shipped
  15.4 EXEs are kept as the reference in
  `OUT/pwa153/upd154/clark-original/`. (The old standalone OUT/pwa154/
  slot is retired - 15.4 PWA now nests under pwa153, matching the
  source layout.)

## 15.4 Delta

- Directory: `pcb154/`
- The crew's active work: fixing, adding, changing, moving
- Ongoing — NOT complete
- Builds with OpenWatcom 1 (needs the 22 Watcom fixes to reach 100%)
- This is a separate effort from 15.4 PWA

## The key distinction

**15.4 PWA (Clark's)** is authentic, completed, Borland — Clark's own
15.4 as shipped.

**15.4 Delta** is the crew's evolving work — where fixes and additions
happen.

Don't conflate them. 15.4 PWA is done; Delta is in progress.

## Patches

- `patches/15.4-pwa.patch` — 15.3 PWA -> 15.4 PWA (Clark's). Complete.
  416 files (378 new, 38 modified). Applies clean, both ends build
  with Borland.
- Delta patches come later, once the Delta work is complete.

## 15.41 IRC

- The next step beyond Delta (FidoNet, TCP, RIP, crew additions)
- Builds with openwatcom2irc (sysop/0's fork)
- Toolkit work-in-progress in toolkit/irc1541/

## Toolkit

Clark's toolkit is recreated bit-for-bit (all original bugs intact),
pwa153 first as the base. Toolkits under toolkit/: pwa153, pwa154, delta154, irc1541 - each a
self-contained branch with the same structure. toolkit/pwa154 is the
15.3 toolkit plus one enum value (SPACERIGHTAT); editing pwa153 does not
affect it. See toolkit/README.md and todo/toolkit.md.
