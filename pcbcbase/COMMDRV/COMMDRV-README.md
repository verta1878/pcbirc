# COMMDRV SDK — PCBoard 15.3 Serial Driver Libraries

## What This Is

PCBoard's multi-port serial card support. Clark's PCBOARD.MAK links these
when compiled with `-DCOMMDRV` (not the default LOCALONLY build). The SDK
ships both driver paths:

- **FOSSIL** — INT 14h FOSSIL driver (wrench's clean-room, GPLv3)
- **COMMDRV** — WCSC COMM-DRV/DOS multi-port driver (Clark's source, reconstructed)

Both compile from MODEM.C with different defines (`-DFOSSIL` or `-DCOMMDRV`).

## File Inventory

### Headers
| File | Lines | What |
|------|-------|------|
| COMM.H | 155+9 | ser_rs232_* API + BAUD constants. LIBENTRY = cdecl (CORRECTED) |

### Libraries (wrench's clean-room, GPLv3)
| File | Size | What |
|------|------|------|
| commdrbl.lib | 3,072B | 13 ser_rs232_* functions via INT 14h |
| libsbl.lib | 2,560B | 6 utility functions (strerror, detect, baud, etc.) |

### Object Files (compile from source via COMMDRV.MAK)
| File | Publics | What |
|------|---------|------|
| COMMDRV.OBJ | 79 | MODEM.C + MODEMASY.C + MODEMDRV.C with -DCOMMDRV |
| FOSSIL.OBJ | 79 | FOSSIL.C standalone |

### Source
| File | Lines | What |
|------|-------|------|
| FOSSIL.C | 790 | sysop/0's clean-room FOSSIL driver (INT 14h + vtable) |
| commdrbl.c | 346 | wrench's ser_rs232_* via INT 14h FOSSIL |
| libsbl.c | 136 | wrench's utility functions |
| MODEM.C | 553 | Clark's master — #includes MODEMASY.C + MODEMDRV.C |
| MODEMASY.C | 256 | Clark's ASYNC wrappers + vtable pointers |
| MODEMDRV.C | 521 | Clark's COMMDRV backend — ser_rs232_* calls |

### Build
| File | What |
|------|------|
| COMMDRV.MAK | Borland MAKE — builds all 4 targets |

## Repo Placement

```
pcbirc/
├── pcbcbase/
│   └── COMMDRV/
│       ├── H/
│       │   └── COMM.H                    ← API header (cdecl corrected)
│       ├── LIB/
│       │   ├── commdrbl.lib              ← wrench's clean-room
│       │   └── libsbl.lib                ← wrench's clean-room
│       ├── OBJ/
│       │   ├── COMMDRV.OBJ              ← built from MODEM.C -DCOMMDRV
│       │   └── FOSSIL.OBJ               ← built from FOSSIL.C
│       └── SRC/
│           ├── FOSSIL.C                  ← sysop/0's reconstruction
│           ├── commdrbl.c               ← wrench's source
│           └── libsbl.c                 ← wrench's source
├── toolkit/
│   └── pwa153/
│       └── SOURCE/
│           └── TOOLKIT/
│               ├── MODEM.C              ← Clark's master (with #includes)
│               ├── MODEMASY.C           ← Clark's ASYNC wrappers
│               └── MODEMDRV.C           ← Clark's COMMDRV backend
├── pcb153/
│   └── 153/
│       └── COMMDRV.MAK                  ← Borland MAKE build file
```

**pcbcbase/COMMDRV/** is SAFE from `make clean` — OBJs stay put.

## Build

From DOSBox with repo root mounted as C:\:

```
MAKE -fCOMMDRV.MAK all
```

Builds: COMMDRV.OBJ + FOSSIL.OBJ + commdrbl.lib + libsbl.lib

Individual targets: `commdrv`, `fossil`, `libs`

## How PCBOARD.MAK Uses These

```makefile
ifdef COMMDRV
  COMMDRV_LIBS = $(LIBSDIR)\COMMDRV\LIB\commdrbl.lib \
                 $(LIBSDIR)\COMMDRV\LIB\libsbl.lib
  COMMDRV_OBJS = $(LIBSDIR)\COMMDRV\OBJ\COMMDRV.OBJ
  COMMDRV_INC  = -I$(LIBSDIR)\COMMDRV\H
endif
```

LIBSDIR = pcbcbase (set by BLDDOS.BAT). Default build is LOCALONLY (no COMMDRV).

## Calling Convention — The Mangling Fix

Clark's COMMDRV.OBJ imports `_ser_rs232_init` (lowercase + underscore = **cdecl**).
The COMMDRV_* and ASYNC_* exports are **pascal** (uppercase, no underscore).

COMM.H originally had `#define LIBENTRY pascal` which made ser_rs232_*
pascal too — wrong. Fixed to `#define LIBENTRY` (empty = cdecl).

## Symbol Verification

Both OBJs match Clark's 79 public symbols exactly:
- 30 COMMDRV_* or FOSSIL_* (pascal)
- 11 ASYNC_* (pascal)
- 30 vtable function pointers (_bauddivisor, _comminkey, etc. — cdecl)
- 8 helpers (OPENMODEM, CLOSEMODEM, SENDSTR, etc. — pascal)

## 3 Source Fixes Applied to Clark's Code

1. MODEMDRV.C line 23: `//` comment → `/* */` (BCC 3.1 C mode)
2. MODEMDRV.C line 62: `static struct port_param pcb` → `struct port_param pcb` (exports _pcb)
3. MODEM.C: initializemodem() wrapped in `#ifndef LIB` (matches Clark's OBJ)

## Provenance

- FOSSIL.C: sysop/0's clean-room reconstruction (GPLv3)
- commdrbl.c + libsbl.c: wrench's clean-room (GPLv3)
- COMM.H: crew reconstruction, interface only (GPLv3)
- MODEM.C + MODEMASY.C + MODEMDRV.C: Clark Development Company (pwa153 source)
- COMMDRV.MAK: sysop/0 (GPLv3)

## the crew 4free
