# OpenWatcom Port Workmap — pcbrevival

**Goal:** All 12 binaries building under OpenWatcom. No Borland dependency. No DOSBox.
**Result:** pcbrevival becomes **pcbirc**. openwatcomirc becomes the compiler.

---

## Current State (2026-07-31)

> Note (2026-08-26): MASM is now fully supported in WASM — see Phase 3;
> the standalone-ASM phase is substantially de-risked by this.

| Scope | Compiling | Total | Percentage |
|---|---|---|---|
| Core PCBoard | 118 | 135 | **87%** |
| Sub-projects | 87 | 138 | 63% |
| Library | 179 | 288 | 62% |
| **All source** | **384** | **561** | **68%** |

- 12 of 12 binaries built (11 Borland DOS + 1 Watcom OS/2)
- WATCOMPAT.H: 219 lines (Borland→Watcom compatibility layer)
- OS/2 PCBOARD2.EXE: clean link, 0 unresolved symbols

---

## Phase 1: Foundation — Compiler Compatibility Layer ✅ COMPLETE

**Delivered:** WATCOMPAT.H (219 lines) + updated BORLAND.H, TYPES.HPP, DOSFUNC.H, PCBTOOLS.H, SCRNIO.H, SCREEN.H.

What WATCOMPAT.H maps:
- farmalloc/farfree/farcoreleft → malloc/free/0x7FFFFFFF
- bioskey → _bios_keybrd
- stpcpy → inline implementation
- getdisk/setdisk → _dos_getdrive/_dos_setdrive (+1 offset)
- MAXDIR/MAXPATH/MAXDRIVE/MAXFILE/MAXEXT → _MAX_DIR etc.
- interrupt → __interrupt, pascal → __pascal
- int86/int86x → int386/int386x (32-bit flat model)
- _argv/_argc → __argv/__argc
- O_DENYNONE → SH_DENYNO
- setcbrk → no-op, randomize → srand
- _version → (_osminor << 8) | _osmajor
- bool for C mode (wcc386)

Key fixes applied to headers:
- _FAR_/_NEAR_ forced empty under __WATCOMC__ in TYPES.HPP (far/near are Watcom keywords)
- DOSFILE typedef include guard added to DOSFUNC.H and PCBTOOLS.H
- writecheck/readcheck return type fixed (int→unsigned) in PCBTOOLS.H
- externaledit/inputstr changed from enum to int parameters
- intfunctype Watcom branch added (interrupt function pointer typedef)
- qint.hpp: int/long operator overloads guarded by #if !defined(__386__)
- Interrupt handler variadic `(...)` → `()` for Watcom

17 core files still failing:
- 5 inline ASM / pseudo-registers (Phase 2)
- 3 dead code / source fragments
- 4 C++ 32-bit porting (qint, var, oldvar, evalp)
- 5 per-file fixes (H2NAME real-mode, ANSI OS/2 symbols, etc.)

---

## Phase 2: Inline ASM — Borland → Watcom ⬅️ IN PROGRESS

**What:** Convert Borland `asm mov ah,XX` to Watcom `int386()` + REGS struct.
**Why:** 91 library files + 5 core files have Borland inline ASM that Watcom rejects.

### Completed (Phase 2):
- [x] WHEREX.C — INT 10h AH=03h → int386(0x10) (cursor X position)
- [x] WHEREY.C — INT 10h AH=03h → int386(0x10) (cursor Y position)
- [x] GOTOXY.C — INT 10h AH=02h → int386(0x10) (set cursor position)
- [x] DOSCLOSE.C — INT 21h AH=3Eh → int386(0x21) (close file)
- [x] DOSCOMIT.C — INT 21h AH=68h → int386(0x21) (commit/flush file)

### Priority order for remaining:

#### 2.1 — DOS File I/O (INT 21h wrappers) — 11 files
- [ ] DOSOPEN.C (9 asm) — INT 21h AH=3Dh, open file
- [ ] DOSREAD.C (13 asm) — INT 21h AH=3Fh, read file
- [ ] DOSWRITE.C (12 asm) — INT 21h AH=40h, write file
- [ ] DOSCREAT.C (13 asm) — INT 21h AH=3Ch, create file
- [ ] DOSLSEEK.C (6 asm) — INT 21h AH=42h, seek
- [ ] DOSDUP.C (4 asm) — INT 21h AH=45h, duplicate handle
- [ ] CHKLOCK.C (8 asm) — INT 21h AH=5Ch, file locking
- [ ] CHKUNLNK.C (6 asm) — INT 21h AH=41h, delete file
- [ ] EXTENDED.C (10 asm) — INT 21h AH=59h, get extended error
- [ ] STRNCHR.C (15 asm) — string search, no INT call

#### 2.2 — Screen/Video (INT 10h) — 25+ files
- [ ] CURSOR.C (7 asm) — cursor shape
- [ ] CLS.C (17 asm) — clear screen
- [ ] CLSBOX.C (47 asm) — clear box region
- [ ] SCROLLDN.C (56 asm) — scroll down
- [ ] SCROLLUP.C (56 asm) — scroll up
- [ ] GETMODE.C (29 asm) — video mode detection
- [ ] PRINT.C (27 asm) — screen output
- [ ] FASTPUTC.C (24 asm) — fast character output
- [ ] BOX.C (119 asm) — draw box
- [ ] And 15+ more screen files

#### 2.3 — System/Math — 20+ files
- [ ] CPUTYPE.C (63 asm) — CPU detection
- [ ] CRYPT.C (123 asm) — encryption
- [ ] DMATH.C (131 asm) — decimal math
- [ ] BD_LONG.C (93 asm) — BCD to long
- [ ] BMSEARCH.C (186 asm) — Boyer-Moore search
- [ ] And 15+ more

#### 2.4 — MAIN source (5 core files from Phase 1)
- [ ] MODEMFOS.C — FOSSIL INT 14h pseudo-registers → int386()
- [ ] TRANSFER.C — pseudo-registers → int386()
- [ ] FIDOFUNC.C — pseudo-registers → int386()
- [ ] DEVIOCTL.C — DOS IOCTL inline asm
- [ ] DLPATH.C — inline asm operand size

### Conversion pattern:
```c
/* Borland: */
asm mov ah,3Eh
asm mov bx,handle
int21();
asm jnc end
getextendederror();
end:;

/* Watcom: */
#elif defined(__WATCOMC__)
{
  union REGS r;
  r.h.ah = 0x3E;
  r.w.bx = handle;
  int386(0x21, &r, &r);
  if (r.w.cflag)
    getextendederror();
}
```

---

## Phase 3: Standalone ASM — TASM to WASM

> **UPDATE (2026-08-26): MASM is now fully supported in WASM.** Watcom's
> assembler (WASM) now handles MASM syntax completely. This is a major
> unblock for this phase: the standalone TASM files are MASM-syntax
> assembly, so they no longer need hand-porting to WASM's older dialect
> - WASM can assemble them directly (or with minimal adjustment). ASYNC.ASM
> (THE GATE, the FOSSIL serial driver) and the other 7 files below become
> a "point WASM at them and fix what it flags" task rather than a full
> rewrite. Re-scope this phase around WASM's MASM mode before starting.


8 TASM files (7,548 lines total):
- ASYNC.ASM (1,893 lines) — **THE GATE** — FOSSIL serial driver
- ANSI.ASM (1,717 lines) — ANSI terminal
- CUTIL.ASM (1,101 lines) — C utility functions
- C0.ASM (895 lines) — CRT startup (may use Watcom's own)
- NOSCROLL.ASM (387 lines) — screen buffer
- MEMMOVE.ASM (340 lines) — memory move (may use Watcom intrinsics)
- TIMER.ASM (231 lines) — timer tick
- BGKEY.ASM (85 lines) — keyboard polling

---

## Phase 4: Overlay Manager

PCBOARD.EXE overlay version — recommend dropping for 32-bit only.

## Phase 5: Link and Test DOS Binaries

Start with MKPCBTXT, PPLC, PCBSM. End with PCBOARDM.

## Phase 6: Unify DOS and OS/2

Single source tree, single compiler.

## Phase 7: openwatcomirc

sysop/0's GCC backend. pcbrevival → **pcbirc**.

---

## Score Progression

```
Session start:     109 / 273 MAIN (40%)
Phase 1.1:         170 / 273 (62%) — WATCOMPAT.H, header copies, case normalization
Phase 1.2:         186 / 273 (68%) — _FAR_/_NEAR_ fix, DOSFILE guard, prototype fixes
Phase 1.3:         199 / 273 (73%) — intfunctype, int86→int386, _USERENTRY
Phase 1 final:     205 / 273 (75%) — INIT.C, SCOMP.CPP, COPYFILE.C, SHELL.C
Phase 2 start:     384 / 561 (68%) — first inline ASM conversions (WHEREX, GOTOXY, DOS files)
```
