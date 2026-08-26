# PCBCP — PCBoard Control Panel for OS/2

OS/2 Presentation Manager GUI for managing PCBoard BBS nodes.
Written by Clark Development Company, distributed as a separate
utility (not part of the licensed source archive).

## Status

- [x] Source recovered from pcball.zip (pcboard.be)
- [x] Ported to OpenWatcom 2.0 (8/8 .C files compile clean)
- [x] Linked as PCBCP_W.EXE (77KB, OS/2 LE format)
- [ ] Test under OS/2/ArcaOS

## Provenance

Source from pcball.zip (pcboard.be). PCBCP was a publicly
distributed OS/2 utility for PCBoard sysops. Not included in
the 15.3 source license purchased by Corey Blake.

## Files

```
PCBCP/
├── SOURCE/          8 .C files + 4 .H files + .RC/.DEF/.ICO/.BMP
│   └── pcbcp_compat.h   OpenWatcom compatibility header
├── HELP/            IPF help source + BMP screenshots
│   ├── PCBCP.IPF    Main help file
│   ├── ACTION.IPF   Action menu help
│   ├── EDIT.IPF     Edit functions help
│   ├── FILE.IPF     File area help
│   ├── OPTION.IPF   Options help
│   └── *.BMP        Node settings, run settings, warning screenshots
├── OBJ/             Compiled resources (MAIN.RES)
├── 1522/            Original 15.22 makefile and config
├── PCBCP.INI        Configuration (106KB)
├── COMPILE.CMD      Original OS/2 build script
├── BUILD_OW.SH      OpenWatcom build script
└── README.md        This file
```

## OpenWatcom Port Changes

6 fixes to compile under OpenWatcom 2.0 (wpp386 -bt=os2):

1. Added `pcbcp_compat.h` — bool typedef, alloc.h→stdlib.h
2. Removed hardcoded `\toolkt21\valapi.h` include path
3. `alloc.h` → handled by compat header
4. `_argv` → `__argv` (Watcom C runtime global)
5. Removed Ctrl-Z (0x1A) DOS EOF markers from all files
6. Include order: PCBoard headers before Watcom headers
   (Watcom has its own dosfunc.h that shadows PCBoard's)

Linked against os2386.lib (OS/2 Toolkit 4.5) + SEMAFORE.CPP
(PCBoard mutex semaphore class). PCBoard library functions
(dosclose, readcheck, etc.) stubbed for initial link.

## Installation (OS/2)

1. Copy PCBCP_W.EXE to PCBoard directory
2. Copy PCBCP.INI alongside it
3. Build PCBCP.HLP from IPF sources: `ipfc HELP\PCBCP.IPF`
4. Create WPS desktop object pointing to PCBCP_W.EXE
5. Configure PCBCP.INI with path to PCBOARD.DAT

## License

Clark Development source: proprietary
Our port changes: GPLv3
