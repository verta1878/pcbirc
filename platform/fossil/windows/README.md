# netfoswl — Windows FOSSIL Driver

Windows native FOSSIL driver for netmodem2irc.
Two backends: socket (TCP via m_fossil_socket) and serial (Win32 COM API).
Replaces NetFoss (PCMicro).

## Files
- netfoswl.pas — main driver (340 lines)
- Uses fossil/common/m_fossil_socket.pas for socket backend

## Serial Mode (Win32 API)
- CreateFile('COM1') for port access
- GetCommState/SetCommState for DCB configuration
- ReadFile/WriteFile for data I/O
- GetCommModemStatus for DCD (carrier detect)
- EscapeCommFunction for DTR control
- ClearCommError for queue status
- SetCommTimeouts for non-blocking read (10ms)

## Socket Mode
- InitSocket(port) — listen and accept
- InitSocketFD(fd) — fd from netmodem2irc
- InitSocketConnect(port) — connect to netmodem2irc

## Build
```
ppc386 -Twin32 netfoswl.pas
```
