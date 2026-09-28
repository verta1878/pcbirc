# DOSBox-X Patched for LANtastic v6 / PCBiRC Revival

## Building (Windows — MSYS2)
See `docs/BUILD_DOSBOXX_MSYS2.md` for full instructions.

1. Extract `dosbox-x-src-patched.7z`
2. Open MSYS2 MINGW64 terminal
3. Run `./build-mingw-sdl2`
4. Copy `src/dosbox-x.exe` here

## Patches Applied
1. **ne2000.cpp** — NE2000 internal loopback fix
2. **shell_misc.cpp** — INT 21h conventional memory stub (PSP[0x0C] fix)
3. **shell.cpp** — Top-of-memory COMMAND.COM allocation (Debian source)

## Status
- `dosbox-x.exe` — **NEEDS BUILDING** (build from dosbox-x-src-patched.7z)
- `SDL2.dll` — Required DLL for Windows
- `dosbox-x.conf` — Config for LANtastic NE2000 networking
