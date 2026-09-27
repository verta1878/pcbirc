# STBSTUB — 4 Missing streambuf Source Files for PCBoard UUCP

**Author:** sysop/0 (reconstructed)  
**Date:** 2026-09-27  
**Request from:** hexadecimal  
**License:** GPLv3 (clean-room from BC 3.1 iostream.h public class interface)

## Background

Clark's UUCP makefiles (UUIN.MAK, UUOUT.MAK, UUXFER.MAK) reference 4 individual
OBJ files in their BC 3.1 TLINK response files:

```
..\bc31\stbdsgtn.obj
..\bc31\stbdsptn.obj
..\bc31\stbsgetn.obj
..\bc31\stbsputn.obj
```

Clark never shipped the source files for these. No `.cpp` or `.c` exists in the
PWA distribution, dosboxx.zip, or the pcbirc repo.

## What These Are

These are **out-of-line implementations** of 4 `streambuf` member functions from
Borland C++ 3.1's iostream library. Clark extracted them so they link as standalone
OBJs instead of pulling the entire iostream RTL.

### The 4 Functions

| File | Function | Type | What It Does |
|------|----------|------|-------------|
| `stbdsgtn.cpp` | `streambuf::do_sgetn(char _FAR*, int)` | virtual | Reads n chars one at a time via `sbumpc()` |
| `stbdsptn.cpp` | `streambuf::do_sputn(const char _FAR*, int)` | virtual | Writes n chars one at a time via `sputc()` |
| `stbsgetn.cpp` | `streambuf::sgetn(char _FAR*, int)` | non-virtual | Fast-path `memcpy` from get buffer, fallback to `do_sgetn()` |
| `stbsputn.cpp` | `streambuf::sputn(const char _FAR*, int)` | non-virtual | Fast-path `memcpy` into put buffer, fallback to `do_sputn()` |

### Key Technical Details

- **All 4 signatures use `char _FAR *`** — there is NO near/far pointer split.
  Hex's initial briefing assumed `sgetn`/`sputn` were near-pointer wrappers;
  BC 3.1's iostream.h shows they are all `_FAR`.

- **`sgetn`/`sputn` are NOT wrappers** — they are the out-of-line versions of the
  `_BIG_INLINE_` fast-path code from iostream.h (lines 359-374). When `_BIG_INLINE_`
  is not defined, the compiler needs these as separate translation units.

- **`do_sgetn`/`do_sputn` are the virtual overrides** — character-at-a-time I/O.
  These are short because the complexity lives in `sbumpc()`/`sputc()`/`underflow()`/
  `overflow()`, not here.

- **All use `_Cdecl` calling convention** per the class declaration in iostream.h.

### Source Reference

Reconstructed from BC 3.1's `iostream.h` (from `devtools/BC31.zip` in pcbircrevival):
- Lines 246-247: `sgetn` / `do_sgetn` declarations
- Lines 255-256: `sputn` / `do_sputn` declarations
- Lines 359-374: `_BIG_INLINE_` implementations of `sputn` / `sgetn`

## Compile

Each file compiles identically (large model, C++, unsigned char default, 386 codegen):

```
BCC +<target>.CFG -c -nBC31 stbdsgtn.cpp
BCC +<target>.CFG -c -nBC31 stbdsptn.cpp
BCC +<target>.CFG -c -nBC31 stbsgetn.cpp
BCC +<target>.CFG -c -nBC31 stbsputn.cpp
```

Key flags from CFG: `-ml -K -P -3`

## Placement

- Source: `PCB153/SOURCE/UUCP/stbdsgtn.cpp` (etc.)
- OBJs: `PCB153/SOURCE/UUCP/BC31/stbdsgtn.obj` (etc.)
- LNK reference: `..\bc31\stbdsgtn.obj`

## Replaces

Hexadecimal's combined `STBSTUB.CPP` / `STBSTUB.OBJ` (659 bytes) — a temporary
shim that provided `do_sputn` and `do_sgetn` in one file. That got all 4 UUCP
targets linked but used a single OBJ where Clark's build expects 4 individual ones.

The combined shim can be kept in the attic for reference.

## Verification

After compiling, verify the mangled symbol names in each OBJ match what the UUCP
LNK response files expect. Use `TDUMP -m <file>.obj` to check.

the crew 4free
