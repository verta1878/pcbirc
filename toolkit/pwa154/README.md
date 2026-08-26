# toolkit/pwa154 — 15.4 PWA toolkit (self-contained)

The toolkit for **15.4 PWA** (Clark's 15.4, source at pcb153/upd154/).
Lives here under `toolkit/` with the same structure as the other
toolkit branches (pwa153, delta154, irc1541).

## Self-contained by design

This is a **complete, independent copy** of the toolkit — not an overlay
on pwa153. Editing `toolkit/pwa153` has **no effect** on this branch,
and vice versa. Each toolkit stands on its own, exactly like the other
branches.

## What 15.4 adds over 15.3

The only functional difference from the 15.3 toolkit is one enum value:
`SPACERIGHTAT` in `H/PCBTOOLS.H` (the padtype used for @x color codes).
Everything else Clark's 15.4 added lived in the main source, not the
toolkit. The rest of this tree matches the 15.3 toolkit (including the
build-hygiene fixes: C-mode guards, CRLF, /* */ comments).

## Build

The 15.4 PWA build (pcb153/upd154/, Borland C++ 3.1) points its toolkit
include path here:

    -I C:\TOOLKIT\PWA154\H

See pcb153/upd154/build/BLDUPD154.BAT.
