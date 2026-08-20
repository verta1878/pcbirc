# PCBMODEM Stub Replacement Plan

## Current State
- PCBMODEM_W.EXE links at 524KB with 0 unresolved
- modem_stubs.cpp has ~35 function stubs + 28 global declarations
- ASYNC.C (202-line FOSSIL driver) compiled but not linked yet (OS/2 API cascade)

## Step 1: Compile PCBMISC.CPP → removes 3 stubs ✅ READY
PCBMISC.CPP compiles clean. Provides real implementations of:
- `parsersearch()` — PPE token search
- `tokenscan()` — PPE token scanner  
- `dispString()` — display string with attributes

Action: Add PCBMISC.obj to PCBMAIN.LIB, remove 3 stubs.

## Step 2: Compile SCRMISC.CPP with -d___EXEC___ → removes 2 stubs
SCRMISC.CPP needs one of: ___COMP___, ___EXEC___, ___INFO___, ___SCAN___, ___DCOM___
For PCBMODEM runtime context, ___EXEC___ is correct.

Provides:
- `cleanupScript()` — script cleanup
- `stopsearch()` — stop PPE search

Action: Compile with `-d___EXEC___`, add to lib, remove 2 stubs.

## Step 3: Fix PSEARCH.C (bsearch conflict) → removes 1 stub
Line 40: `static int LIBENTRY (*bsearch)(...)` conflicts with stdlib bsearch.
Fix: rename to `pcb_bsearch` or `psearch_func`.

Provides:
- `searchfirst()` — pattern search

Action: Copy, fix name conflict, compile, remove stub.

## Step 4: Write pure C getrows() → removes 1 stub
SETROWS.C uses Borland inline ASM for VGA mode detection (INT 10h).
Write a 10-line C replacement using Watcom `int386()`.

```c
char getrows(void) {
    union REGS r;
    r.h.ah = 0x0F;  /* get video mode */
    int386(0x10, &r, &r);
    r.w.ax = 0x1130; r.h.bh = 0;
    int386(0x10, &r, &r);
    return (char)(r.h.dl + 1);  /* DL = last row (0-based) */
}
```

## Step 5: Replace modem stubs with ASYNC-backed implementations
The COMM path in MODEM.C pulls in OS/2 APIs. Instead of using MODEM.C,
write standalone implementations that call ASYNC.C directly:

```c
/* closemodem — calls ASYNC_CLOSECOM */
void closemodem(bool TurnOffDTR) {
    if (TurnOffDTR) ASYNC_TURNOFFDTR();
    ASYNC_CLOSECOM();
}

/* sendbyte — calls ASYNC_CSENDBYTE */
void sendbyte(char Byte) {
    ASYNC_CSENDBYTE((int)Byte);
}

/* openmodem — calls ASYNC_OPENCOM */
void openmodem(showtype Show) {
    ASYNC_OPENCOM(0, 0);  /* baud/databits set by initmodem */
}
```

Key: compile these as C++ in modem_stubs.cpp (matching MODEM.C mangling)
but DON'T include project.h (avoids OS/2 header cascade).

ASYNC.C stays compiled as plain C (wcc386). The bridge is in modem_stubs.cpp.

## Step 6: Replace user write stubs with real code from USERS.C
USERS.C already compiled (USERS.obj in PCBMAIN.LIB). These functions exist:
- `writeuserrecord()` — line 1078
- `writeuserinfrecord()` — not found (may be usersys)
- `writeuserspsa()` — USERSYS.C
- `writeusersinfdata()` — USERSYS.C
- `saveusersinfdata()` — USERSYS.C
- `readusersinfdata()` — USERSYS.C
- `readuserspsa()` — USERSYS.C

USERSYS.C already compiled (USERSYS.obj in lib).
Action: Remove stubs, let real objects link. Test for cascade.

## Step 7: Remaining justified stubs (keep as-is)

### OS/2-only (no DOS equivalent):
- `bgetkey2(char)` — Kbd16CharIn (OS/2 keyboard API)
- `begnoscroll()` / `endnoscroll()` — OS/2 VIO scroll lock
- `scrollon()` — OS/2 VIO
- `reinstallhandlers()` — DosError
- `execl()` — OS/2 DosExecPgm variant
- `swapenv()` — OS/2 environment swap

### Unused in PCBMODEM context (no-op correct):
- `setfont()` — VGA font switch, PCBMODEM is text-mode config tool
- `setbankdefaults()` — timebank init, irrelevant
- `init_uppercase()` — UpperCase[] table, needs country data
- `readtextinf()` / `restoretextinf()` — text info system
- `doScript()` — PPL script execution (PCBMODEM doesn't run PPEs)

### Correct implementations:
- `fmemcpy()` → memcpy (flat model, mathematically correct)
- `readaccountrates()` → return 0 (PCBMODEM doesn't use accounting)
- `lastChar()` → real implementation (checks last char of string)
- `getrows()` → return 25 (until Step 4 INT 10h version)

### Globals (28):
All correct — extern declarations satisfied by zero-initialized instances.

## Execution Order
1. PCBMISC.CPP → lib → remove 3 stubs → verify link
2. SCRMISC.CPP → lib → remove 2 stubs → verify link  
3. PSEARCH.C fix → lib → remove 1 stub → verify link
4. getrows() INT 10h → replace stub → verify link
5. ASYNC-backed closemodem/sendbyte/openmodem → verify link
6. User write functions → remove stubs → verify link (watch for cascade)
7. Document remaining justified stubs

## Expected Result
- ~12 stubs replaced with real code
- ~15 stubs remain (justified: OS/2-only or unused-in-context)
- 28 globals remain (correct, required by header chain)
- ASYNC.C linked for actual COM port I/O
- All Phase 0 binaries still link clean
