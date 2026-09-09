# DOSBox-X Patched Build Toolchain
# the crew 4free — GPLv3

## Files
```
dosboxx-toolchain.7z.001     120MB
dosboxx-toolchain.7z.002     104MB
```
Extract: `7z x dosboxx-toolchain.7z.001`

## What's Inside
- MSYS2 Windows installer (76MB)
- 26 offline MinGW64 build packages (78MB)
- DOSBox-X full source with 3 patches (92MB)
- SDL2.dll, config, build docs

## Patches
1. NE2000 internal loopback fix (ne2000.cpp)
2. INT 21h conventional memory stub for PSP[0x0C] (shell_misc.cpp)
3. Top-of-memory COMMAND.COM allocation (shell.cpp)

Fixes DOS network card drivers hanging in DOSBox-X.
COMMAND.COM now loads at top of memory like real MS-DOS.

## Build Steps (Windows)
1. Run `msys2-x86_64-installer.exe` — install to C:\msys64
2. Open MSYS2 MINGW64 terminal (NOT the MSYS2 MSYS terminal)
3. Run: `bash INSTALL-PACKAGES.sh`
   If any deps missing: `pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libpng mingw-w64-x86_64-zlib mingw-w64-x86_64-libslirp autoconf automake make`
4. Extract source: `7z x dosbox-x-src-patched.7z`
5. Build: `cd dosbox-x-full && ./build-mingw-sdl2`
6. Binary at `src/dosbox-x.exe`

## Output
Copy `dosbox-x.exe` + `SDL2.dll` to your DOSBox-X directory.

## Config (dosbox-x.conf)
```ini
[cpu]
cputype=pentium
[serial]
serial2=disabled
[ne2000]
ne2000=true
nicbase=300
nicirq=3
macaddr=AC:DE:48:88:99:AA
backend=slirp
```

## Notes
- Build with LTO recommended: add `CFLAGS="-O2 -flto" CXXFLAGS="-O2 -flto"` to configure
- Without LTO, some 286+ CPU instructions trigger INT 6 in the CPU core
- One-time build — the .exe doesn't change unless patches are updated
