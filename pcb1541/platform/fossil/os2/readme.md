# netfosol — OS/2 FOSSIL Driver

OS/2 native FOSSIL driver for netmodem2irc.
Two backends: socket (TCP via m_fossil_socket) and serial (DosDevIOCtl).

## Files
- netfosol.pas — main driver (310 lines)
- Uses fossil/common/m_fossil_socket.pas for socket backend

## Serial Mode (OS/2 DosDevIOCtl)
- DosOpen('COM1') for port access
- DosDevIOCtl IOCTL_ASYNC for baud, modem control, queue status
- DosRead/DosWrite for data I/O

## Socket Mode
- InitSocket(port) — listen and accept
- InitSocketFD(fd) — fd from netmodem2irc
- InitSocketConnect(port) — connect to netmodem2irc

## Build
```
ppc386 -Tos2 netfosol.pas
```

## See Also
- evga's SIO2K rebuild (13,371 lines, GPLv3) — OS/2 PDD/VDD/FOSSIL
- docs/OS2_PORT_PLAN.md — 10-phase OS/2 port plan
