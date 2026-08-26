# Disk 2 of 5 — UUCP, FidoNet & Utilities

Per the original FILE_ID.DIZ, this disk held:
- UUIN/UUOUT — UUCP mail stack
- UUUTIL/UUXFER — UUCP file transfer
- PCBFU.EXE — FidoNet utility

## Reconstruction sources (present in this tree)

The source for these components exists and can be rebuilt into the disk:

| Disk component | Source location |
|---|---|
| UUIN / UUOUT | `pcb153/upd154/SOURCE/UUCP/UUIN/`, `.../UUOUT/` |
| UUUTIL / UUXFER | `pcb153/upd154/SOURCE/UUCP/UUUTIL/`, `.../UUXFER/` |
| UUCP core | `pcb153/upd154/SOURCE/UUCP/UUCP.CPP` and siblings |
| PCBFU (FidoNet util) | `pcb1541/pcbfido/` (pcbfido.c, pcbfcfg.c) |

## Rebuild target

Compile the UUCP suite (Borland, 15.4 PWA toolkit) and the FidoNet
utility, then package into the disk image once the INSTALL.DAT / .RED
packaging format is worked out. The binaries are the byte-exact targets.
