# Recovered toolkit objects — FOSSIL.OBJ and COMMDRV.OBJ

These two are **live**, not attic. Landed 2026-09-22.

    FOSSIL.OBJ    9,157 B
    COMMDRV.OBJ  10,024 B

## Why they are not in the attic

`attic/` is for material that has been **superseded** — the nine `.386`
category libraries went there only *after* they were rebuilt from source
and the rebuilds verified against them. Nothing supersedes these two yet.
A reconstruction of the source now exists (below), but nothing has been
compiled from it and compared, so these objects are still the only thing
in the project that is known to be right.

## Provenance

Extracted from `TOOLKIT2.ZIP` → `TOOLKIT/BC/PCBKIT_L.EXE`. Dated
Oct 11 1993. The OMF records carry Clark's own build path, `y:\modem.c`.

## What they actually are

Both are `MODEM.C` — the backend selector — compiled twice with
different defines. That is what `y:\modem.c` in the OMF proves;
`FOSSIL.OBJ` is not `MODEMFOS.C` on its own, as was assumed for most of
this project.

    FOSSIL.OBJ    bcc -ml -c -DCOMM -DMULTIPORT -DLIB -DFOSSIL  modem.c
                  MODEM.C #includes MODEMFOS.C and MODEMASY.C
                  79 exports: 30 FOSSIL_*, 11 ASYNC_*, 30 vtable, 8 helpers
                  48 imports: 23 ASYNC_* from ASYNC.ASM, + PCBoard, + runtime

    COMMDRV.OBJ   same, with -DCOMMDRV in place of -DFOSSIL
                  MODEM.C #includes MODEMDRV.C instead of MODEMFOS.C
                  79 exports, same shape, COMMDRV_* for FOSSIL_*
                  extra imports: _ser_rs232, _ser_rs232_init, _pcb

Full symbol lists: `pcb1541/pcbdcom/doc/FOSSIL-OBJ-ANALYSIS.md`.

## The source exists now — and the test has not been run

`toolkit/pwa153/SOURCE/TOOLKIT/FOSSIL.C` is sysop/0's standalone
reconstruction, 767 lines. It carries its own type stubs rather than
`project.h`/`model.h`, so it needs no `-D` flags and no toolkit config:

    bcc -ml -c -oFOSSIL.OBJ FOSSIL.C

Its 79 exports match this object's symbol for symbol. **That match was
made by a Python OMF parser reading the two symbol tables — not by a
compiler.** Nothing has been built from `FOSSIL.C` yet. So the acceptance
test is still open, and it is the whole reason these objects are kept:

1. Compile `FOSSIL.C` under BC 3.1, large model. Zero errors.
2. Dump the resulting OMF exports.
3. Diff against `FOSSIL.OBJ`'s 79. Byte-level OMF comparison after that.

They move to the attic the day that passes, and not before. Same for
`COMMDRV.OBJ`, which has no reconstruction at all yet.

Three findings from the OMF dump that a reconstruction has to honour,
and which `FOSSIL.C` already does:

* `FOSSIL_dofixups`, `ASYNC_dofixups` and `initializemodem` are **not**
  exported — static near, called only inside the compilation unit.
* `_Fossil` (the `fossilstruct`) is global, not static.
* `InBytes`/`OutBytes` are macros for `inbytes()`/`outbytes()` under
  `MULTIPORT`, not functions.
* The vtable pointers have C linkage (lowercase with a leading
  underscore in the OMF); the functions have pascal linkage (uppercase).
  They do not collide.

## Who wants them

`pcb153\SOURCE\UUCP\UUXFER\UUXFER.MAK` asks for
`toolkit\large\fossil.obj`. UUXFER is the only consumer — PCBOARD does
not link it. That is the gap recorded as "One is missing" in
`MAIN/build/CATEGORY-LIBRARIES.md`, now updated with the answer.
