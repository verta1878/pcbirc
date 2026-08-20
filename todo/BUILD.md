# Building PCBoard 15.4 (pcbirc)

## Compiler

PCBoard 15.4 is compiled with OpenWatcom 2.0 (openwatcom2irc fork).
Borland C++ 3.1 (Clark's original compiler) is no longer used.

Download: https://github.com/verta1878/openwatcom2irc
Targets: DOS16, DOS32, DOS4G, OS/2, WoW64

Stock OpenWatcom 2.0 also works for most builds:
https://github.com/open-watcom/open-watcom-v2

## Environment

```
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl64:$WATCOM/binl:$PATH
export INCLUDE=$WATCOM/h:$WATCOM/lh
```

## Building Clark Utilities (Phase 0)

The 12 Phase 0 utilities are in PCBSRC/MAIN/SOURCE/UTIL/:

```
wcc386 -bt=dos -mf -3r -s -zq -IPCBSRC/LIB/H <source.c> -fo=<output.obj>
wlink system dos4g name <output.exe> file <obj1.obj> file <obj2.obj> ...
```

Each utility has its own source directory with a .MAK or .PRJ file
from the original Borland build. These need conversion to Watcom
wmake format (Phase 1 work).

## Building PCBISO

```
cd PCBSRC/MAIN/SOURCE/UTIL/PCBISO
wcc386 -bt=dos -mf -3r -s -zq pcbiso.c -fo=pcbiso.obj
wlink system dos4g name pcbiso.exe file pcbiso.obj
```

Also compiles with GCC for Linux testing:
```
gcc -Wall -Wextra -o pcbiso pcbiso.c
```

## Building QFront

Requires owh/ POSIX shim headers for stock OpenWatcom:

```
cd tools/qfront
wcc386 -bt=dos -mf -3r -s -zq -I./owh -I./src src/<file>.c -fo=out/<file>.obj
wlink system dos4g name bin/dos/qfront.exe file out/qfront.obj file out/serial.obj ...
```

See tools/qfront/INSTALL.TXT for the full build and PCBoard setup.

## Cross-Platform Notes

- `long` is 8 bytes on 64-bit Linux, 4 bytes on DOS/Watcom.
  Use `int32_t` for PCBoard data structures (CNAMES.ADD, USERS, etc.)
- `#pragma pack(1)` required for all on-disk structures
- `#ifdef __WATCOMC__` for DOS-specific code (int386, _dos_findfirst)
- `#else` for POSIX (opendir, readdir)
- Watcom is C89 — no mixed declarations, no VLAs, no // comments
  in headers

## Data Structure Sizes (must match Clark's specs)

| Structure | Size | File | Reference |
|-----------|------|------|-----------|
| addconftype | 256 bytes | CNAMES.ADD | docs/devkit/CNAMES.DOC |
| oldconftype | varies | CNAMES.@@@ | 2-byte header has size |
| PCBOARD.DAT | text file | PCBOARD.DAT | docs/devkit/PCBDAT.DOC |
| DirListType | 99 bytes | DIR.LST | docs/devkit/DIR.DOC |

## DOS4GW

DOS4G executables need DOS4GW.EXE in the same directory at runtime.
It ships with OpenWatcom at $WATCOM/binw/dos4gw.exe.
