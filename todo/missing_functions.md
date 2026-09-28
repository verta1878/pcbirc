# Missing Functions — OpenWatcom Port Exceptions

Status: 551 / 557 source files compile under OpenWatcom 2.0
(549 with default flags, +2 PCBSETUP files with `-dVMDATA`).

The remaining 6 files cannot compile without either:
- Phase 3 TASM→WASM assembler port (3 ASM files)
- Source reconstruction (2 fragments)
- OS/2 SDK (1 test driver)

This document catalogs the functions these files provide so
they can be implemented as C stubs or WASM rewrites later.

---

## ANSI.C — ANSI Terminal Output (1 error)

**Location:** LIB/SOURCE/SCREEN/ANSI.C
**Issue:** One inline ASM `Push` instruction in ansi_print()
**Binary:** Part of SCREEN.LIB

The ANSI functions ARE mostly C — only ansi_print() has one ASM
instruction. The rest of the file is pure C and could compile
with a trivial rewrite of that one line.

### Functions

```c
void pascal ansi_print(char *str);
    /* Output ANSI escape string to console.
       Contains: asm { Push ds } — needs rewrite to C putchar() loop.
       Rest of function is pure C. */

void pascal ansi_save(void);
    /* Send ESC[s (save cursor position) */

void pascal ansi_rest(void);
    /* Send ESC[u (restore cursor position) */

void pascal ansi_clear(void);
    /* Send ESC[2J (clear screen) */

void pascal ansi_color(int Color);
    /* Convert PCBoard color attribute to ANSI escape sequence.
       Maps high nibble (background) and low nibble (foreground)
       to SGR parameters. Tracks last color to minimize output. */

void pascal ansi_clearbox(char X1, char Y1, char X2, char Y2, char Color);
    /* Clear rectangular region using ANSI cursor positioning
       and space fill. Sets color first via ansi_color(). */

void pascal ansi_clearcolor(char Color);
    /* Clear entire screen with specified color attribute */
```

### Global Data

```c
char ansicolors[] = "04261537";
    /* Maps PCBoard color index to ANSI SGR color number */
```

### Rewrite Difficulty: TRIVIAL

Only one ASM instruction needs replacement. The `Push ds` in
ansi_print() protects DS during INT 21h output — in 32-bit flat
model this is unnecessary. Replace the entire ASM block with a
simple `while (*str) putchar(*str++);` loop.


## CPUTYPE.C — CPU Detection (4 errors)

**Location:** LIB/SOURCE/MISC/CPUTYPE.C
**Issue:** Entire function is inline ASM (PUSHF/POPF/FLAGS manipulation)
**Binary:** Part of MISC.LIB

### Functions

```c
int pascal cputype(void);
    /* Detect CPU type by testing FLAGS register behavior.
       Returns: 0=8088, 1=8086, 2=V20/V30, 3=80186, 4=80286,
                5=80386, 6=80486, 7=Pentium+
       Uses: PUSHF, POPF, flags bit testing, CPUID (486+)
       Result cached in static CPUvalue. */
```

### Rewrite Difficulty: EASY

In 32-bit flat model under DOSBox, always return 5 (80386) or
higher. Could also use Watcom's `__cpuid` intrinsic if available,
or simply hardcode 5 since we target 386+.

### Stub

```c
int pascal cputype(void) {
    return 5;  /* 80386 — minimum for flat model */
}
```


## GETMODE.C — Video Mode Detection and Screen Buffer (5 errors)

**Location:** LIB/SOURCE/SCREEN/GETMODE.C
**Issue:** Multiple inline ASM blocks (INT 10h calls, port I/O)
**Binary:** Part of SCREEN.LIB

This is the most critical of the ASM files — it initializes the
screen subsystem that BOX.C, CLS.C, GOTOXY.C, PRINT.C, etc.
all depend on.

### Functions

```c
void near pascal getvideotype(void);
    /* Detect video adapter type via INT 10h AH=1Ah.
       Sets Scrn_EGA, Scrn_ColorCard flags.
       Uses: INT 10h with AX=1A00h, BX register inspection. */

void LIBENTRY getmode(void);
    /* Main video initialization. Calls getvideotype(), then:
       - Detects current video mode via INT 10h AH=0Fh
       - Sets Scrn_Mode (color/mono), Scrn_Addr (video base),
         Scrn_BottomRow, Scrn_Size, Scrn_Rtrc
       - Allocates Scrn_Buf (shadow screen buffer)
       Uses: INT 10h, BIOS data area reads, port 3D4h/3D5h
       for retrace detection. */

void LIBENTRY setscreenupdateinterval(int Interval);
    /* Set minimum milliseconds between screen updates */

void LIBENTRY updatelines(scrnupdttype ScrnUpdate, int StartLine, int EndLine);
    /* Copy lines from shadow buffer to video memory.
       Handles retrace wait if Scrn_Rtrc is set. */

void LIBENTRY updatelinesnow(void);
    /* Force immediate update of all dirty screen lines */

void LIBENTRY hidescreen(void);
    /* Increment hide counter — suppresses screen updates */

void LIBENTRY unhidescreen(void);
    /* Decrement hide counter — resumes screen updates */

void LIBENTRY setviolines(int NumLines);
    /* Set number of screen lines (25/43/50) via INT 10h.
       Reallocates Scrn_Buf. For EGA/VGA only. */
```

### Global Data

```c
void  *Scrn_Buf;          /* Shadow screen buffer (malloc'd) */
void  *Scrn_Addr;         /* Video memory base (B800:0 or B000:0) */
char   Scrn_Rtrc;         /* True: wait for retrace on update */
char   Scrn_Mode;         /* True: color mode, False: B&W */
char   Scrn_ColorCard;    /* True: color card installed */
char   Scrn_EGA;          /* True: EGA/VGA card installed */
char   Scrn_Box;          /* True: box drawing allowed */
char   Scrn_24Hour;       /* True: 24-hour clock display */
char   Scrn_DateSeparator;/* Date separator character */
char   Scrn_BottomRow;    /* Last row on screen (24 for 25-line) */
int    Scrn_SizeBytes;    /* Screen buffer size in bytes */
int    Scrn_Size32;       /* Screen buffer size in 32-bit words */
char   Scrn_X;            /* Current cursor X position */
char   Scrn_Y;            /* Current cursor Y position */
```

### Rewrite Difficulty: MEDIUM

The screen buffer management is straightforward C. The INT 10h
calls and port I/O need replacement with either:
- Watcom `int86()` calls for BIOS interrupts
- Direct video memory access (0xB8000 for color, 0xB0000 for mono)
- DOSBox provides standard VGA at 0xB8000

### Stub (minimal — assumes 80x25 color VGA)

```c
#include <stdlib.h>
#include <string.h>
#include <i86.h>    /* Watcom int86() */

void near pascal getvideotype(void) {
    Scrn_EGA = 1;
    Scrn_ColorCard = 1;
}

void LIBENTRY getmode(void) {
    union REGS regs;

    getvideotype();

    /* Get current video mode */
    regs.h.ah = 0x0F;
    int86(0x10, &regs, &regs);

    Scrn_Mode = 1;                    /* color */
    Scrn_Addr = (void *)0xB8000;      /* color video memory */
    Scrn_BottomRow = 24;
    Scrn_SizeBytes = 80 * 25 * 2;
    Scrn_Size32 = Scrn_SizeBytes / 4;
    Scrn_Rtrc = 0;                    /* no retrace wait */
    Scrn_Box = 1;
    Scrn_X = 0;
    Scrn_Y = 0;

    if (Scrn_Buf) free(Scrn_Buf);
    Scrn_Buf = malloc(Scrn_SizeBytes);
    if (Scrn_Buf) memset(Scrn_Buf, 0, Scrn_SizeBytes);
}

void LIBENTRY setscreenupdateinterval(int Interval) { (void)Interval; }
void LIBENTRY updatelines(scrnupdttype ScrnUpdate, int StartLine, int EndLine) {
    (void)ScrnUpdate; (void)StartLine; (void)EndLine;
    /* Copy shadow buffer to video memory */
    if (Scrn_Buf && Scrn_Addr)
        memcpy(Scrn_Addr, Scrn_Buf, Scrn_SizeBytes);
}
void LIBENTRY updatelinesnow(void) {
    if (Scrn_Buf && Scrn_Addr)
        memcpy(Scrn_Addr, Scrn_Buf, Scrn_SizeBytes);
}
void LIBENTRY hidescreen(void) {}
void LIBENTRY unhidescreen(void) {}

void LIBENTRY setviolines(int NumLines) {
    union REGS regs;
    /* Set 25/43/50 lines via INT 10h */
    if (NumLines == 50 || NumLines == 43) {
        regs.x.ax = 0x1112;  /* Load 8x8 font */
        regs.h.bl = 0;
        int86(0x10, &regs, &regs);
    }
    Scrn_BottomRow = NumLines - 1;
    Scrn_SizeBytes = 80 * NumLines * 2;
    Scrn_Size32 = Scrn_SizeBytes / 4;
    if (Scrn_Buf) free(Scrn_Buf);
    Scrn_Buf = malloc(Scrn_SizeBytes);
    if (Scrn_Buf) memset(Scrn_Buf, 0, Scrn_SizeBytes);
}
```


## TEST.C — OS/2 File I/O Test Driver (4 errors)

**Location:** LIB/SOURCE/DOS/TEST.C
**Issue:** Uses `os2errtype` which is only defined under `__OS2__`
**Binary:** Standalone test executable, not part of any library

### Functions

```c
void _cdecl main(void);
    /* Test driver for DOS file I/O functions:
       doscreate(), dosfopen(), dosfwrite(), dosfclose(),
       dosfread(), fileexist().
       Uses os2errtype for extended error reporting. */
```

### Action: SKIP

This is a standalone test program, not part of any PCBoard
binary. It only compiles under OS/2 where os2errtype is defined.
No stub needed.


## OLDSEEN.C — Old SEEN-BY Processing (7 errors)

**Location:** MAIN/SOURCE/FIDO/OLDSEEN.C
**Issue:** Entire file is commented out (`/*` ... `*/`)
**Binary:** Was part of PCBTOSS

### Functions (all commented out)

```c
bool gen_seenby(char * arr[], char * str);
    /* Generate SEEN-BY line from node array.
       Old implementation replaced by new code in TOSSMISC.C.
       Entire file is inside a /* */ comment block.
       A stray #include <stddef.h> before the comment causes errors. */
```

### Action: SKIP or DELETE

This is dead code — Clark commented it out entirely. The replacement
is in TOSSMISC.C (which compiles clean). Remove the stray
`#include <stddef.h>` to make it a clean empty file, or delete it.

### Fix (if desired)

```c
/* OLDSEEN.C — obsolete, replaced by TOSSMISC.C */
/* Entire original implementation was commented out by Clark. */
```


## MKPCBTXT.C (PCBTEXT version) — PCBTEXT.DAT Generator (19 errors)

**Location:** MAIN/SOURCE/UTIL/PCBTEXT/MKPCBTXT.C
**Issue:** Uses undefined `Array` type, missing includes
**Binary:** Standalone utility MKPCBTXT.EXE

**Note:** A second version exists at MAIN/SOURCE/MKPCBSRC/MKPCBTXT.C
which compiles clean under OpenWatcom (276 lines, 0 errors). This
is the pcbrevival replacement that generates/upgrades PCBTEXT.DAT.

### Functions (PCBTEXT version — broken)

```c
/* This version uses an 'Array' class that is not defined in
   any available header. It was likely part of an internal
   Clark Development library that was not included in the
   source release. */
```

### Action: USE MKPCBSRC VERSION

The MKPCBSRC/MKPCBTXT.C (276 lines) compiles clean and provides
the same functionality. Use that instead.


---

## Summary

| File | Functions | Rewrite Effort | Priority |
|------|-----------|----------------|----------|
| ANSI.C | 7 functions | Trivial (1 ASM line) | High — easy win |
| CPUTYPE.C | 1 function | Trivial (hardcode 386) | Low — stub works |
| GETMODE.C | 7 functions + 14 globals | Medium (INT 10h → int86) | High — screen init |
| TEST.C | main() test | Skip | None |
| OLDSEEN.C | 1 function (dead) | Skip/delete | None |
| MKPCBTXT.C | N/A | Use MKPCBSRC version | None |

### Quick Wins (add to next compile pass)

**ANSI.C** — Replace the single `asm { Push ds }` block with
a C `putchar()` loop. Everything else is already C.

**CPUTYPE.C** — Replace with `return 5;` stub. 32-bit flat
model is always 386+.

### Phase 3 Target

**GETMODE.C** — Full C rewrite using Watcom `int86()` for BIOS
calls and direct video memory access. The stub above provides
a starting point. This is the gate for the SCREEN.LIB to be
fully functional under OpenWatcom.
