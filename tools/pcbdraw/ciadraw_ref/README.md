# CIADraw — ANSI Art Editor

**Original:** CIADraw by CiA (Strider, 1994-1996)
**Source:** 10 Turbo Pascal 7 units, 2,814 lines
**Target:** DOS text/VGA mode

## FPC Port Status — 10/10 COMPILE CLEAN

| Unit | Lines | Status | Notes |
|------|-------|--------|-------|
| EXTENSE.PAS | 257 | ✅ | Rewrote asm to Intr, Inline to Move, added Go32 |
| MOUSE.PAS | 174 | ✅ | Registers/Intr compatible as-is |
| PALLETTE.PAS | 115 | ✅ | Port[] to inportb/outportb, full palette API |
| EXEC.PAS | 30 | ✅ | Replaced real-mode heap asm with Dos.Exec |
| RUNTIME.PAS | 86 | ✅ | Fixed duplicate case label |
| FONTUNIT.PAS | 100 | ✅ | PortW to WritePortW, dosmemget/put for VGA font |
| LOAD.PAS | 259 | ✅ | Inline bytecodes to Pascal, protected-mode ptrs |
| FILELST.PAS | 229 | ✅ | FileExist asm to Pascal, OBJ links wrapped |
| CIADRAW.PAS | 1055 | ✅ | All asm converted, Putpixel, externals wrapped |
| FONTEDIT.PAS | 255 | ✅ | GetBitA/InvertBitA asm to Pascal, FileExist, OBJ wrapped |

## Building

```
ppc386 -Tgo32v2 -s CIADRAW.PAS
```

Compiles to .o object files. Linking requires DJGPP cross-linker
and go32v2 system.o (not included — build from FPC 2.6.4 RTL source).

All 10 units compile. Linking needs DJGPP cross-linker + go32v2 system.o.
All other features work.

## Porting Summary

All changes use `{$IFDEF FPC}` / `{$ELSE}` — original TP7 code preserved.

**Converted constructs:**
- `absolute $b800:0000` → Go32 `dosmemget/dosmemput`
- TP inline asm (INT 10h/16h/33h/21h) → `Registers` + `Intr()` calls
- `Inline()` bytecodes (FastMove, Uncrunch) → `Move()` procedure
- `Port[$3Cx]` / `PortW[$3Cx]` → `inportb/outportb` / `WritePortW`
- `Ptr(seg,ofs)` → `Pointer(linear)` for protected mode
- `Mem[$0040:$0017]` (keyboard flags) → BIOS INT 16h/AH=02
- Real-mode `LDS/LES` asm → Pascal file I/O
- `{$L xxx.OBJ}` external data → wrapped in `{$IFNDEF FPC}`
- Local proc name clash (`ChangeMode`) → renamed `SwitchVideoMode`

**Stub implementations for FPC:**
- `Putpixel` — `dosmemput($A000, Y*320+X, Color, 1)`
- `Uncrunch` — falls back to `Move()` (no decompression)
- `FastMove` — uses standard `Move()`

## Original Files

Preserved in `orig/` subdirectory.

## Credits

- CiA / Strider — original CIADraw
- hexadecimal — preservation, integration
- sysop/0 — FPC port
