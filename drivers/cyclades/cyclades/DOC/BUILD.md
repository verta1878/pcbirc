# How to Build CYPORT.SYS

## Two Builds, One Source

The driver builds in two configurations from the SAME source code:

| Build | Directory | DBG | CY_DEBUG | Asserts | Debug Output | Use For |
|-------|-----------|-----|----------|---------|-------------|---------|
| Checked | `build/checked/` | 1 | 1 | Active | Full (levels 0-5) | Development, testing |
| Free | `build/free/` | 0 | 0 | Compiled out | Errors only (level 1) | Production, end users |

## Build Commands

### Checked (Debug) Build
```bat
REM Open "Windows XP Checked Build Environment" from WDK Start menu
cd cyclades\branch_a_kernel\build\checked
build -cef

REM Output: obj\i386\cyport.sys (debug version)
```

### Free (Release) Build
```bat
REM Open "Windows XP Free Build Environment" from WDK Start menu
cd cyclades\branch_a_kernel\build\free
build -cef

REM Output: obj\i386\cyport.sys (production version)
```

## What the Debug Build Adds

### Debug Print Levels (CyDbgPrint)
```
Level 0 (NONE):    Silent
Level 1 (ERROR):   Errors — things that should never happen
Level 2 (WARNING): Warnings — unexpected but recoverable
Level 3 (INFO):    Informational — port open/close, baud changes, chip detect
Level 4 (TRACE):   Detailed — every IOCTL, every IRP, state transitions
Level 5 (VERBOSE): Maximum — ISR-level detail, register values, byte counts
```

### Runtime Level Control
```bat
REM Set debug level via registry (survives reboot):
reg add HKLM\SYSTEM\CurrentControlSet\Services\cyport\Parameters ^
    /v DebugLevel /t REG_DWORD /d 4

REM Level 4 (TRACE) shows every IOCTL and IRP without ISR noise.
REM Level 5 (VERBOSE) adds ISR register dumps — VERY noisy.
```

### ISR Register Tracing (CY_DEBUG_REGS)
The checked SOURCES defines `CY_DEBUG_REGS=0` by default.
Change to `CY_DEBUG_REGS=1` for per-register ISR tracing:
```
C_DEFINES=-DUNICODE -D_UNICODE -DCY_DEBUG=1 -DCY_DEBUG_REGS=1
```
WARNING: This is extremely noisy. At 115200 baud with continuous
data, it produces ~10,000 debug prints per second. Only enable
when debugging specific ISR issues.

### Assert Macros (Checked Build Only)
- `CYPORT_ASSERT(condition)` — bugchecks if condition is FALSE
- `CYPORT_ASSERT_PASSIVE()` — bugchecks if not at PASSIVE_LEVEL
- `CYPORT_ASSERT_DISPATCH()` — bugchecks if above DISPATCH_LEVEL
- `CYPORT_ASSERT_AT_IRQL(n)` — bugchecks if IRQL != n

In the free build, these compile to nothing (zero overhead).

## Viewing Debug Output

### With WinDbg (kernel debugger)
```
REM Set debug filter to show CYPORT messages:
ed nt!Kd_DEFAULT_Mask 0xFFFFFFFF
```

### With DbgView (Sysinternals)
Run DbgView as Administrator. Check "Capture Kernel" in the menu.
All CyDbgPrint output appears with "CYPORT[n]:" prefix where n
is the debug level.

## Source Files

| File | Lines | What |
|------|-------|------|
| cycommon.h | 598 | Shared header — extensions, inline helpers, forward decls |
| cydebug.h | 255 | Debug macros, asserts, IRQL validation |
| cd1400.h | 293 | CD1400 register definitions |
| cyenum.c | 1045 | Bus enumerator — DriverEntry, AddDevice, FDO PnP |
| cypdo.c | 589 | Child PDO PnP — QUERY_ID, capabilities, start/remove |
| cyisr.c | 997 | ISR + DPC + KeSynchronizeExecution wrappers |
| cyserial.c | 692 | Open/Close/Cleanup, IoCsq cancel-safe queues |
| cyread.c | 353 | Read dispatch + DPC completion |
| cywrite.c | 324 | Write dispatch + DPC completion |
| cyioctl.c | 1360 | All 26 serial IOCTLs with full debug output |
| cypower.c | 468 | Power management — sleep/resume state save/restore |
| cylog.c | 400 | Event logging — IoWriteErrorLogEntry wrappers |
| cylog.mc | 215 | Message compiler definitions — 18 event log messages |
