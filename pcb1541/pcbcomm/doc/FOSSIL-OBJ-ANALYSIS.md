# FOSSIL.OBJ + COMMDRV.OBJ — OMF Symbol Analysis

## Source

Both objects extracted from `TOOLKIT2.ZIP` → `TOOLKIT/BC/PCBKIT_L.EXE` →
`FOSSIL.OBJ` (9,157 bytes) and `COMMDRV.OBJ` (10,024 bytes).
Dated Oct 11, 1993. OMF source path: `y:\modem.c`.

## Key Finding

**FOSSIL.OBJ is NOT just MODEMFOS.C.** It's `MODEM.C` (the backend selector)
compiled with `-DCOMM -DMULTIPORT -DLIB`, which `#include`s MODEMFOS.C
and MODEMASY.C. The source path `y:\modem.c` proves it.

Same for COMMDRV.OBJ — it's `MODEM.C` compiled with `-DCOMM -DMULTIPORT
-DLIB -DCOMMDRV`, which pulls in MODEMDRV.C instead of MODEMFOS.C.

## FOSSIL.OBJ — 79 Exports, 48 Imports

### Exports (30 FOSSIL_* — the FOSSIL backend)

```
FOSSIL_BAUDDIVISOR      FOSSIL_CDSTILLUP        FOSSIL_CGETBUF
FOSSIL_CGETSTR          FOSSIL_CHECKCOMM        FOSSIL_CLEARINBUF
FOSSIL_CLEAROUTBUF      FOSSIL_COMMGO           FOSSIL_COMMINKEY
FOSSIL_COMMPAUSE        FOSSIL_COMMSTOP         FOSSIL_CSENDBYTE
FOSSIL_CSENDSTR         FOSSIL_CTSOKAY          FOSSIL_DISCONNECTMODEM
FOSSIL_FRAMINGERRORS    FOSSIL_INBYTES          FOSSIL_ONLINE
FOSSIL_OPENMODEM        FOSSIL_OUTBYTES         FOSSIL_OVERRUNERRORS
FOSSIL_PARITYERRORS     FOSSIL_REOPENPORT       FOSSIL_RINGDETECT
FOSSIL_SETPORT          FOSSIL_TURNOFFDTR       FOSSIL_TURNOFFRTS
FOSSIL_TURNONDTR        FOSSIL_TURNONRTS        FOSSIL_TURNONXMIT
```

### Exports (11 ASYNC_* — stubs/fallbacks from MODEMASY.C path)

```
ASYNC_BAUDDIVISOR       ASYNC_CTSOKAY           ASYNC_DISCONNECTMODEM
ASYNC_FRAMINGERRORS     ASYNC_INBYTES           ASYNC_OPENMODEM
ASYNC_OUTBYTES          ASYNC_OVERRUNERRORS      ASYNC_PARITYERRORS
ASYNC_REOPENPORT        ASYNC_RINGDETECT
```

### Exports (30 _lowercase — function pointer vtable from MODEM.C)

```
_bauddivisor  _cdstillup  _cgetbuf  _cgetstr  _checkcomm
_clearinbuf   _clearoutbuf  _commgo  _comminkey  _commpause
_commstop     _csendbyte  _csendstr  _ctsokay  _disconnectmodem
_framingerrors  _inbytes  _online  _outbytes  _overrunerrors
_parityerrors  _reopenport  _ringdetect  _setport  _turnoffdtr
_turnoffrts   _turnondtr  _turnonrts  _turnonxmit
```

### Exports (8 helpers from MODEM.C)

```
OPENMODEM               CLOSEMODEM
SENDBYTE                SENDSTR
WAITFOREMPTY            CLEARANDWAITFORMODEMEMPTY
_Fossil                 _ModemOpened
_ModemOffHook
```

### Imports (23 ASYNC_* — from ASYNC.ASM, linked separately)

```
ASYNC_CDSTILLUP   ASYNC_CGETBUF     ASYNC_CGETSTR     ASYNC_CHECKCOMM
ASYNC_CLEARINBUF  ASYNC_CLEAROUTBUF ASYNC_CLOSECOM    ASYNC_COMMGO
ASYNC_COMMINKEY   ASYNC_COMMPAUSE   ASYNC_COMMSTOP    ASYNC_CSENDBYTE
ASYNC_CSENDSTR    ASYNC_INIT        ASYNC_ONLINE      ASYNC_OPENCOM
ASYNC_SETPORT     ASYNC_TURNOFFDTR  ASYNC_TURNOFFRTS  ASYNC_TURNONDTR
ASYNC_TURNONFIFO  ASYNC_TURNONRTS   ASYNC_TURNONXMIT
```

### Imports (PCBoard internals + runtime)

```
ERROREXITTODOS    GIVEUP            GETTIMER          SETTIMER
TICKDELAY         WRITELOG          LOGUSEROFF        WATCHSYSTEMFUNCTIONS
_Asy              _CDokay           _PcbData          _VerifyCDLoss
__CTSokay         __InBytes         __OutBytes        __FramingErrors
__OverrunErrors   __ParityErrors    __RingDetect
_farmalloc        _farfree          _farcoreleft      _sprintf
F_LXMUL@          F_LDIV@
```

## COMMDRV.OBJ — 79 Exports, 50 Imports

Same structure as FOSSIL.OBJ but with COMMDRV_* instead of FOSSIL_*.
Additional imports: `_ser_rs232`, `_ser_rs232_init`, `_pcb` (the port_param
struct instance).

## Build Recipe (corrected)

```
FOSSIL.OBJ:
  bcc -ml -DCOMM -DMULTIPORT -DLIB -c -oFOSSIL.OBJ MODEM.C

COMMDRV.OBJ:
  bcc -ml -DCOMM -DMULTIPORT -DLIB -DCOMMDRV -c -oCOMMDRV.OBJ MODEM.C

Both need:
  - project.h / model.h (or stubs for -DLIB path)
  - MODEM.C #includes MODEMFOS.C / MODEMDRV.C / MODEMASY.C
  - ASYNC.ASM linked separately (provides 23 ASYNC_* imports)
```

## What This Means for pcbcomm

The toolkit FOSSIL.OBJ was never a standalone FOSSIL driver. It's the
full PCBoard modem selector compiled in library mode. SDK programs that
link against it get:
- The FOSSIL_* functions (INT 14h FOSSIL calls)
- The ASYNC_* fallback (bare UART via ASYNC.ASM)
- The function pointer vtable (so they call online(), inbytes(), etc)
- The selector logic (FOSSIL_dofixups sets up the vtable)
