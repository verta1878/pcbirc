# Compilers for the pwa153 SDK

Clark's toolkit SDK shipped in three compiler families. Each maps to a
compiler we reproduce that family's libraries with. Same toolkit source,
compiled three ways.

| SDK prefix | Compiler | Status | Build-tools archive | Source archive |
|---|---|---|---|---|
| PCBKBC | Borland C++ 3.1 | BUILT 4/4 | PCB153BT.ZIP / DOSBOXX.ZIP | (bundled) |
| PCBKIT | Turbo C 2.01 | BUILT 4/4 | TC201BT.ZIP / DOSBOXX.ZIP | devtools/TURBOC201.zip |
| PCBKMS | Microsoft C 7.0 | toolchain + OS/2 host ready | MSC70BT.ZIP | devtools/MSC70-retail.7z + C7OS2.zip |

## Status detail

- **PCBKBC (Borland C++ 3.1)** — DONE. All 4 memory models in
  OUT/lib/pwa153/ (PCBKBC_S/M/C/L.LIB) + loose override OBJs.
- **PCBKIT (Turbo C 2.01)** — DONE. All 4 models (PCBKIT_S/M/C/L.LIB),
  119 modules each. Built via MAIN/build/scripts (BLDKIT + MKLIB), C-mode
  guarded headers, 8.3 response files.
- **PCBKMS (Microsoft C 7.0)** — retail toolchain + OS/2 Hosted
  Add-on both extracted to MSC70/ and packaged as MSC70BT.ZIP. The
  OS/2 add-on gives a real-16-bit, no-DPMI compiler (route B below), so
  the last blocker is cleared - it needs an OS/2 host to run. All
  scaffolding ready (manifest, obj/msc70/ dirs, MS??.RSP files).

## Microsoft C 7.0 — the DPMI situation (and how we get past it)

MS C/C++ 7.0 is the last pure-DOS MS compiler. Its toolchain splits
memory handling:
- LINK / BSCMAKE / CV use a 16-bit extender working with DPMI, VCPI, or
  XMS — fine on plain DOS with HIMEM/EMM386.
- CL (compiler -> C13216/C23216/C33216) uses a 32-bit extender that
  ONLY accepts DPMI. Microsoft's README: on DOS you must install
  386-Max (or run under Windows) to provide DPMI. Plain DOSBox fails
  with R6901 (DOSX32 : DPMI host required).

Two routes to a working PCBKMS build:
- **Route A (DOS + DPMI):** 386-Max under DOSBox, or a DPMI host
  DOSBox-X can present.
- **Route B (OS/2, no DPMI) — preferred, NOW IN REPO:** the MS C/C++ 7.0
  OS/2 add-on (devtools/C7OS2.zip, decompressed to MSC70/OS2/) ships the
  compiler as 16-bit-hosted binaries (C11616/C21616/C31616/C1XX1616) that
  run in native 16-bit protected mode under OS/2 — real 16-bit mode, no
  DPMI. It's an add-on to the base product and uses the base INCLUDE/LIB
  we already have. Host under OS/2 1.x+ (or emulation) and compile the
  119-module manifest -> PCBKMS. Fits our OS/2 targets (OS2TK/).

## Archives in devtools/

- `MSC70-retail.7z` — retail MSC 7.0 (8-18-1992), 12 floppy images.
  Files are KWAJ-compressed; decompress with a libmspack KWAJ extractor
  (the disks' SETUP also decompresses). This produced MSC70/.
- `MSC70-patches.7z` — official update patches (C7pat/C7patb): fix
  LINK 5.31/LIB/PWB/CV. Not needed to build the toolkit libraries.
- Note: the decompressed toolchain is shipped as **MSC70BT.ZIP**
  (extract to C:\MSC70), not as an unpacked MSC70/ dir, to keep the
  repo lean - same pattern as PCB153BT.ZIP / TC201BT.ZIP.
- `C7OS2.zip` — Microsoft C/C++ 7.0 OS/2 Hosted Add-on Kit (2 disks,
  KWAJ-compressed). Provides the OS/2-hosted 16-bit compiler that runs
  without DPMI. Decompressed into MSC70/OS2/.
- `C7OS2.zip` — Microsoft C 7.0 OS/2 Hosted Add-on Kit. Decompressed to
  MSC70/OS2/ (the 16-bit OS/2-hosted compiler - no DPMI). The key to
  building PCBKMS without a DOS DPMI host.
- `TURBOC201.zip` — Turbo C 2.01 install (produced TC201/ -> TC201BT.ZIP).

(The earlier Beta 3 disks were removed in favour of the retail release.)



## install v1.11 toolchain — CONFIRMED (verified 2026-09-05)

The install v1.10 arc reimplemented Clark's INSTALL.DAT semantics in
portable C (see `pcb1541/install/src/install-1010.c`). Install v1.11+
opens the byte-exact reconstruction of Clark's compiled `INSTALL.EXE`
binary (338,548 bytes, NE Family API, linker version bytes 5.10).

**Clark's toolchain: Borland C++ 3.1 (compiler + TLINK 5.1) + API.LIB
from Microsoft OS/2 SDK 1.03.** BC 3.1 is already in the repo as
`devtools/BC31.zip` (also extracted at `PCB153BT.ZIP`). API.LIB
comes from `devtools/OS2SDK103.zip` (at `PMSDK/LIB/API.LIB` inside
`toolkit1.img`).

An earlier draft of this section (pre-verification) speculated
Microsoft C 5.1 was the toolchain. That was wrong. TLINK's *banner*
says "Turbo Link Version 5.1" but the *NE header linker-version bytes*
it writes are `05 0A` = decimal `5.10`. Borland encodes minor-version
1 as byte `0x0A` (10). Microsoft encodes it as `0x01`. Different
conventions, same version-number string.

### Verification chain

Every one of these NE binaries has linker bytes `05 0A` = 5.10:

    file                                        linker bytes
    ------------------------------------------------------------
      Clark's INSTALL.EXE                       5.10
      BWCC.DLL (pcb1541/install/dist/target)    5.10
      BC 3.1's TLINK.EXE (own NE header)        5.10
      Fresh HELLO.EXE built with BC 3.1         5.10
      v1.11.0's install-1011.c -> INSTALL.EXE   5.10

Four independent confirmations plus the target itself.

### Roles of the archives

| Archive | Role | Status |
|---|---|---|
| `BC31.zip` (existing) | **Compiler + TLINK — the toolchain.** | ready |
| `OS2SDK103.zip` (staged) | Provides `API.LIB` for OS/2 Family API imports (DOSCALLS/KBDCALLS/VIOCALLS). Also has PM SDK + Petzold samples for later work. | ready |
| `MSC51.zip` (staged) | Redundant. Has API.LIB binary identical to OS2SDK103's. Keep as backup or drop. | redundant |
| `MSC60A.zip` (staged) | Not needed. Was backup for the "Microsoft toolchain" hypothesis that turned out wrong. Drop or keep. | not needed |

### Build script

`pcb1541/install/build/BLDINS.BAT` — scoped to install/, does NOT
route through `MAIN/build/scripts/BUILD.BAT` (that's the pwa153 SDK
matrix). Run manually from DOSBox-X.

### Family API targeting notes

BC 3.1 ships Family API startup objects (`C0F*.OBJ` — F prefix =
Family) alongside standard DOS (`C0[C|L|M|S|T].OBJ`), DLL
(`C0D*.OBJ`), and Windows (`C0W*.OBJ`) variants. That confirms BC 3.1
CAN target Family API; v1.11.0 uses standard `C0L.OBJ` + Windows
target (`/Twe`) as the toolchain shakedown. Correct Family-target
incantation (`.DEF` file with `EXETYPE OS2`, `C0FL.OBJ` startup,
target OS byte 0x01 or 0x04) is v1.11.1 work.

### md5 provenance

    MSC51.zip      5e6af949652b260bed48f4501acf25cc  (redundant)
    MSC60A.zip     9860b8124ca81a7d52b54d9ed4766dc6  (not needed)
    OS2SDK103.zip  6340f99b9dd50a19b699b0d1c75a244c  (KEEP - API.LIB)

Sourced from:
- OS/2 SDK 1.03: archive.org/details/ms-os2-sdk-1.03
- MSC 5.1: archive.org/details/microsoft-c-5.1-optimizing-compiler-5.25.-7z
- MSC 6.0a: archive.org/details/Microsoft_c_60a


## Notes

- **Assemblers:** the DOS/Borland builds assemble the standalone .ASM
  files with TASM (Borland's assembler, MASM-compatible). For the
  OpenWatcom port, WASM (Watcom's assembler) now **fully supports MASM
  syntax** (as of 2026-08-26), so the 8 standalone TASM files
  (ASYNC/ANSI/CUTIL/C0/NOSCROLL/MEMMOVE/TIMER/BGKEY) can be assembled by
  WASM directly rather than hand-ported. See
  todo/OPENWATCOM_PORT_WORKMAP.md Phase 3.

- The compiler ID is embedded in Clark's .LIB modules — verified:
  PCBKIT="Borland Turbo C 2.0", PCBKBC="Borland C++ 3.1",
  PCBKMS=Microsoft C (path e:\msc\b\...).
- PCBoard itself was built in the MEDIUM memory model — so the _M
  library is the one PCBoard uses.
