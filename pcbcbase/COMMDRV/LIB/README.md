# pcbcbase/COMMDRV/LIB — empty on purpose

`reference/pcball/pcboard/pcb-main/153/PCBOARD.MAK` lines 824-825 link:

    /o+ $(LIBSDIR)\commdrv\lib\commdrbl.lib+
    /o+ $(LIBSDIR)\commdrv\lib\libsbl.lib+

and line 45 puts `$(LIBSDIR)\COMMDRV\H` on the include path. `LIBSDIR` is
this folder's parent, the same place `CODEBASE` sits. So this is where
those two libraries have to end up for PCBOARD to link.

Neither is in this repo and neither is obtainable. WCSC's only free
offering is a Win32 library with a different API — `OpenComPort`,
`GetByte`, `PutPacket`, no `ser_rs232_*` at all — and COMM-DRV/DOS is
still a paid product. What we do have is the **v15.0b runtime** in
`pcb1541/install/dist/target/COMMDRV/` (`COMMDRV.EXE`, `COMMTSR.EXE`,
nine `.DRV` files), which is the TSR, not the toolkit.

## What fills it

`pcb1541/pcbdcom/src/commdrbl.c` and `libsbl.c` — sysop/0's clean-room
replacements, landed 2026-09-22. Build:

    bcc -ml -c -DCOMMDRV_DRIVER commdrbl.c
    tlib commdrbl.lib +commdrbl.obj

against `../../pcbcbase/COMMDRV/H/COMM.H`. Read
`pcb1541/pcbdcom/doc/COMM-H-MERGE.md` first — it records which of the two
`comm.h` reconstructions this tree uses, and an open struct-layout defect
in `ser_rs232_shim.c` that will silently corrupt the port block if it is
built before being fixed.

`LIBSBL.LIB`'s scope is still unsettled: `libsbl.c` covers what PCBOARD
links, but whether Clark's original held more than that is unknown.
