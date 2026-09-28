# netfosll — Linux FOSSIL / ASYNC Layer

Linux implementation of PCBoard's 23 ASYNC functions.
Replaces FOSSIL INT 14h with socket operations.

## Three Init Modes (via environment variables)

| Env Var | Mode | Description |
|---------|------|-------------|
| PCBFD=N | fd pass | Use file descriptor N (from netmodem2irc) |
| PCBPORT=N | accept | Listen on localhost:N, accept one caller |
| NMPORT=N | connect | Connect to localhost:N (netmodem2irc) |

## CPU Hog Fix

All blocking reads use `select()` with 10ms timeout.
No busy-polling. CPU stays idle when no data.

## Build

```
wcc386 async_linux.c -bt=linux -mf -5 -ox
```

## Design Credits
- hexadecimal — LINUX_SERIAL_PLAN.md (23 function mapping)
- sysop/0 — m_fossil_socket.pas (socket backend design)
- wrench — C implementation
