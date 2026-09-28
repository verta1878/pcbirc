# PCBCOMM Linkage Architecture

## Two linking modes

PCBoard has two linking modes — SDK programs get STUBS, PCBOARD.EXE
gets REAL CODE.

### PCBOARD.EXE (real code)

PCBOARD.MAK links the full serial chain:

    ASYNC.ASM          bare-metal ISR-driven UART (30+ exports)
    MODEM.C            modem init, vtable dispatch
    MODEMFOS.C         FOSSIL backend (INT 14h)
    MODEMASY.C         ASYNC function pointer table
    MODEMDRV.C         COMM-DRV .DRV loader
    MODEMOS2.C         OS/2 serial path
    commdrbl.lib       13 ser_rs232_* functions (cdecl)
    libsbl.lib         6 utility functions

PCBOARD.EXE talks to hardware directly.

### SDK doors and utilities (stubs)

Door developers link override OBJs from the toolkit:

    FOSSIL.OBJ         3K stub (Toolkit3.ZIP, 1994-11-23)
    COMMDRV.OBJ        3K stub (Toolkit3.ZIP, 1994-11-23)
    NO*.OBJ            NOCHAT, NODISP, NOINPUT, NOLOG, etc.

Doors don't talk to hardware — they call PCBoard's API at runtime.
The stubs just satisfy the linker.

That's why UUXFER links FOSSIL.OBJ + NO*.OBJ stubs while PCBOARD.MAK
links ASYNC.ASM + the full modem chain.

## Two sets of OBJs

    pcbcbase/COMMDRV/OBJ/    Full 10K implementations (PCBOARD.EXE)
    toolkit/pwa153/bc31/obj/ 3K stubs (door developers)

Clark shipped stubs to door developers because COMMDRV was a commercial
WCSC product. The 10K OBJs were used internally. Clark never replaced
the stubs before the bank closed Clark Development.

## pwa153/154 vs delta154

pwa153/154 preserves Clark's world as-is — stubs in toolkit, full OBJs
in pcbcbase. Nothing changed.

delta154 finishes what Clark started — door developers get real
functional OBJs from the crew's clean-room source, GPLv3. No stubs,
no WCSC dependency.

## MAKE CLEAN

MAKE CLEAN must NOT delete:

- pcbcbase/COMMDRV/OBJ/COMMDRV.OBJ
- pcbcbase/COMMDRV/OBJ/FOSSIL.OBJ
- pcbcbase/COMMDRV/LIB/commdrbl.lib
- pcbcbase/COMMDRV/LIB/libsbl.lib

These are pre-built dependencies, not build output. Clark compiled
the OBJs manually — no MAK rebuilt them. COMMDRV.MAK (sysop/0,
2026-09-26) now provides a rebuild path, but MAKE CLEAN only removes
temp files.

## COMMDRV.MAK builds

    COMMDRV.OBJ    from MODEM.C + MODEMASY.C + MODEMDRV.C (one unit)
    FOSSIL.OBJ     from FOSSIL.C (sysop/0's clean-room, 790L)
    commdrbl.lib   from commdrbl.c (wrench, 346L)
    libsbl.lib     from libsbl.c (wrench, 136L)

## Calling convention

cdecl (C), NOT pascal. Clark's COMMDRV.OBJ imports `_ser_rs232_init`
(lowercase + underscore). COMM.H LIBENTRY is empty for all compilers.

## Clark's own Watcom port

Found at reference/pcball/pcboard/pcb-main/WATCOM/. wpp386, flat model,
OS/2 target. Compiles modem.c + modemos2.c but NOT modemfos.c,
modemasy.c, modemdrv.c — OS/2 doesn't use FOSSIL. No -DCOMMDRV,
no -DFOSSIL. delta154 fills this gap: DOS flat OW2 WITH FOSSIL.

## Credits

- wrench: commdrbl.c, libsbl.c, stub vs real-code finding
- sysop/0: FOSSIL.C, COMMDRV.MAK, mangling fix, OBJ reconstruction
- hexadecimal: WATCOMPAT.H, pcb153 builds, DOSBox-X environment
- verta1878: clean-room decisions, architecture direction
