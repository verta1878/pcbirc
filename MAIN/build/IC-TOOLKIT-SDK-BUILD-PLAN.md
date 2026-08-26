# IC / Toolkit / SDK Build Plan — Priority #2 Linking

## Decisions (from lead, this session)

1. **Category libs stay SEPARATE per compiler** — Clark's way.
   Each compiler (pwa153/Borland, delta154/Watcom, irc1541/ow2irc) gets
   its own set: dos_L, misc_L, screen_L, system_L, scrnio_L, pcb_L,
   country_L, + toolkitl (override). They use separate source versions.

2. **Stubs (NO*) → real code** — DEFERRED to its own phase (see below).
   Leave the toolkit as Clark made it for now; change later if needed.

3. **IC package (Pcbic12)** — rebuild byte-for-byte with the same bugs
   AFTER toolkit/SDK is done. Fix bugs only after full restoration.

4. **OUT/ = binaries only.** Docs → docs/. Runtime data → OUT/support/.

## Category Library Manifests (complete, verified no dupes)

From Clark's makefiles (tlib commands + LIBADD.RSP + obj: targets),
plus objects discovered via linking MAKEIDX/USERNET:

| Category | Objects | Notes |
|---|---|---|
| dos_l | 41 | + SHOWERR (retrycount), SAY |
| misc_l | 51 | + VMFUNCS, VMAVL (VM system) |
| screen_l | 28 | + TIMECHNG, WHEREX |
| scrnio_l | 20 | BGETKEY (real bgetkey) |
| system_l | 4 | KBDSTAT (renamed SYSTEM/BGETKEY) |
| pcb_l | 8 | |
| country_l | 3 | + COUNTRY (getcountryspecs) |
| toolkitl | 3 | ALTMODEM, NODISP, PCBDAT (override) |

Total: 155 main + 3 override = 158 objects.

## Special Build Steps

- DOSCLASS.CPP — C++ (-P flag)
- INT24HND.ASM, SWAP.ASM — tasm /mx /d__l__ (large model)
- SYSTEM/BGETKEY.C — compile to KBDSTAT.OBJ (avoids collision with
  SCRNIO/BGETKEY.C which is the real bgetkey)

## Link Recipe (verified working)

TLINK response file:
```
c0l.obj PROG.OBJ [extra.obj]
PROG.EXE
PROG.MAP
dos_l.lib+misc_l.lib+screen_l.lib+system_l.lib+scrnio_l.lib+pcb_l.lib+country_l.lib+toolkitl.lib+cl.lib+mathl.lib
```

## Gap Binaries Status

| Binary | Compiler | Status |
|---|---|---|
| MAKEIDX | Borland | DONE (built, 0 undefined, executes) |
| USERNET | Borland | DONE (built, 0 undefined, executes) |
| PCBCP | Borland | needs port from Watcom (pcb154) |
| PCBIS | Borland | needs port from pcb154 pcbiso.c |
| PCBOARD2 | Watcom | OS/2 target, PCBOARD2.MAK |
| PCBOARDM | Watcom | multinode variant |

## DEFERRED PHASE: Stubs → Real Code

The NO* files (NODISP, NOANSI, NOCHAT, NOHELP, NOINPUT, NOLANG, NOLOG,
NOMEMORY, NOPCBSYS, NOPRINT, NOSCREEN, NOSHELL, NOSTATUS, NOSYS, NOTXT,
NOUPDSYS, NOXLATE) are empty stub implementations. They let utility
programs link without pulling PCBoard's full display/console engine.

The REAL implementations live in pcb153/SOURCE/DISPLAY/ (DISPLAY.C,
ANSI.C) and pcb153/SOURCE/MAIN/. To "turn stubs into real code":
1. Make DISPLAY/ and console modules linkable as toolkit components
2. Build a "full" library variant that includes real implementations
3. Utilities can then choose: link toolkitl (stubs) OR full display lib

This is a design change to Clark's architecture — deferred until the
byte-for-byte restoration is complete, per decision #2 and #3.

## Rebuild Order (compile ONCE)

1. Compile all toolkit objects per compiler (already done for pwa153)
2. Build category libs from manifests above
3. Link gap binaries (PCBCP, PCBIS for PWA; PCBOARD2, PCBOARDM for Delta)
4. Link full PCBOARD.EXE on both compilers
5. Diff PWA vs Delta → recreate diff patch
