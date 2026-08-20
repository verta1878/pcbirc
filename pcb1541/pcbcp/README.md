# PCBCP — PCBoard Control Program (OS/2)

Clark's OS/2 Presentation Manager GUI for PCBoard node control.
Found in `reference/pcball/pcboard/pcb-util/PCBCP/`.

Copyright (C) 1996 Clark Development Company. Version 15.22 era.

8 C source files in `SOURCE/`:
- `MAIN.C` — PM window procedure, message loop
- `DLG.C` — dialog boxes
- `FILE.C` — file operations
- `HELP.C` — online help
- `INIT.C` — initialization
- `PNT.C` — paint/display
- `THRD.C` — worker threads (native OS/2 threading)
- `USER.C` — user management

Build: `PCBCP.MAK` + `COMPILE.CMD` (Borland C++ for OS/2)
Resources: `MAIN.RC`, `HELP.RC`, `PCBCP.DLG`, `PCBCP.DEF`
