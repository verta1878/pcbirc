# OUT\pwa153\SDK\TC201 — PCBKit toolkit libraries (Turbo C 2.01)

The complete PWA 15.3 toolkit compiled with Turbo C 2.01 (TCC.EXE),
all four memory models. 119 modules per model, 476 OBJs total.

Built 2026-09-28 under DOSBox-X headless using BLDKIT.BAT + MKLIB.BAT.

    PCBKITS.LIB   151,552 bytes   small model
    PCBKITM.LIB   156,672 bytes   medium model
    PCBKITC.LIB   163,840 bytes   compact model
    PCBKITL.LIB   168,960 bytes   large model

## Build environment

- **Compiler:** Turbo C 2.01 (`tc201/bin/tcc.exe`)
- **Librarian:** TLIB from Borland C++ 3.1 (`bc31/bin/tlib.exe`)
- **Host:** DOSBox-X headless (`display_type = nondisplay`)
- **Include paths:** `tc201/include`, `toolkit/pwa153/h`, `pcb153/source/h`
- **Defines:** `-dpcb152 -dlib -dcomm`

## Source directories

    pcb153/source/main/       22 modules
    pcb153/source/display/    12 modules
    toolkit/pwa153/source/country/  13 modules
    toolkit/pwa153/source/toolkit/  40 modules
    toolkit/pwa153/source/comm/      7 modules
    toolkit/pwa153/source/misc/     25 modules

## SUBST workaround

TCC 2.01 does not read `turboc.cfg` under DOSBox-X. Include paths
are passed on the command line using SUBST drive letters to stay
under the DOS 127-character limit:

    subst d: c:\tc201\include
    subst e: c:\toolkit\pwa153\h
    subst f: c:\pcb153\source\h

This makes each `-I` flag 4 characters (`-Id:\`) instead of the
full path. Longest command line: 105 characters (compact model).

Fixing TCC to read its config file would eliminate the SUBST
requirement — that investigation is in progress.

## What these are for

These are the **toolkit libraries**, not the final PCBoard binaries.
A door or utility links against the appropriate model's PCBKIT
library. PCBoard itself (PCBOARD.EXE, LOCAL.EXE, etc.) links against
the medium-model lib plus additional object files not in the toolkit.

## Comparison with BC31 SDK

The BC31 libraries in `out/pwa153/sdk/bc31/lib/` are Clark's original
split layout — separate libs per subsystem (dos_l.lib, misc_l.lib,
pcb_l.lib, etc.). The TC201 libraries here are monolithic — one
PCBKIT lib per model containing all 119 modules. Both compile the
same source tree.
