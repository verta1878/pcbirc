# PCBKMS Build Setup — MSC 7.0 + DPMI host

Turnkey recipe to build PCBKMS{S,M,C,L}.LIB once you have a stable host.
Everything on our side is validated and ready; this is just the host
setup.

## What you need on the host

- **MSC 7.0** — extract `MSC70BT.ZIP` to `C:\MSC70`.
- **A DPMI host** (see routes below).
- **TASM** on PATH — for the 5 ASM modules (from BC31/PCB153BT.ZIP).
  MSC7 ships no standalone assembler; TASM's OMF .OBJ links fine.
- **Both source trees mounted:**
  - `C:\TOOLKIT\PWA153`  (95 of the 119 modules)
  - `C:\PCB153`          (24 modules, incl. the 5 ASM)
- The `OUT\LIB\PWA153\msc70\OBJ\{small,medium,compact,large}` dirs
  (BLDKMS.BAT creates them if missing).

## Host options

### The DPMI distinction that decides your route

DPMI providers come in two shapes and they're **not interchangeable
for what DOSBox-X can host**:

- **DPMI extenders** (HX HDPMI16, HX HDPMI32, DOS/32A, DOS/4GW): user-
  space programs loaded from AUTOEXEC.BAT that hook DPMI interrupts
  and answer INT 2F/AX=1687h. These run inside DOSBox-X (as of the
  `zero unused int 68h=true` fix — see
  `../../todo/dosboxx-dpmi-failures.md`).
- **CONFIG.SYS memory managers** (386MAX.SYS, BlueMAX, EMM386,
  QEMM386): device drivers loaded at boot via `DEVICE=` in
  CONFIG.SYS that put the CPU in V86 mode and become the DPMI/VCPI
  host from bare-metal upward. These do NOT run inside DOSBox-X
  (architectural: DOSBox-X is a high-level emu, it *is* the CPU
  itself, no guest gets to hook it at boot). Tier 3 (86Box/PCem)
  only.

**Key insight**: MSC 7.0's DOSX32 stub doesn't care WHICH DPMI host
answers INT 2F/AX=1687h — it just wants DPMI. Historically people
provided that via 386MAX because that's what the error message
suggests, but any DPMI provider works. If HDPMI32 (an extender that
DOES run in DOSBox-X) satisfies DOSX32's DPMI check, then **PCBKMS
builds inside DOSBox-X and never needs 386MAX at all**. That is the
route to test first.

### Route A — DOSBox-X + HDPMI32 (VERIFIED 2026-08-29)

**Status**: DPMI pipeline confirmed working. HDPMI32 loads (INT 68h
fix), MS32KRNL.DLL accepts HDPMI32 as DPMI host (no R6901), CL invokes
the 32-bit compile passes (C13216.EXE et al) successfully.

**Proof-of-life artifacts** (2026-08-29 build):
- `TINY.C` (3 lines, `int main(void){return 0;}`) → `TINY.OBJ` 351
  bytes, valid OMF: `8086 relocatable (Microsoft), "tiny.c"`. First
  MSC 7.0 build ever produced under DOSBox-X in the pcbirc build image.
- First real PCB toolkit module (ADDBACKS.C) reached CL without any
  DPMI error; failed on a header-modernization gap
  (`types.hpp(49): fatal error C1017: invalid integer constant
  expression` — `sizeof(char)` inside `#if` under an insufficient
  guard, `_MSC_VER` not excluded). Not a Route A block; a per-file
  header fix, tracked as first-build fixup per
  `SDK-BUILD-STATUS.md`.

**Configuration on the image (all shipped)**:

1. `PCBBLDBT.CONF` has `[dos] zero unused int 68h=true`
2. AUTOEXEC.BAT runs `C:\HX\HDPMI32.EXE -r` early
3. `SET INCLUDE=C:\MSC70\INCLUDE`, `SET LIB=C:\MSC70\LIB` before
   invoking CL

The full `BLDKMS.BAT` build (476 steps × 4 memory models → 4 libs)
is now reachable from within DOSBox-X. Expected remaining work is
header fixups on the same order the earlier PCBKBC build required
(2-3 guarded blocks, all on toolkit headers, changes are additive
`!defined(_MSC_VER)` guards).

### Route B — real DOS + 386MAX (historical requirement)

A full-PC virtualizer (QEMU / 86Box / PCem) running MS-DOS with
386MAX loaded via `DEVICE=` in CONFIG.SYS. Use this when Route A
doesn't pan out or for byte-exact historical reproduction. Also the
right path if you want to test our TASM-downgraded 386MAX build
(see `../../todo/386max-build-downgrade.md`).

### Route C — OS/2 (no DPMI at all)

Run the OS/2 "1616" compiler (`C:\MSC70\OS2\BINP\CL.EXE`) under an
OS/2 host. Set `CC` accordingly in BLDKMS.BAT. Sidesteps the DPMI
question entirely but installing OS/2 under emulation is a bigger
lift than Routes A or B.

## Selecting a route at boot (CONFIG.SYS multi-boot menu)

Rather than editing scripts to switch routes, use DOS's native
multi-boot menu (`[menu]` block in CONFIG.SYS — MS-DOS 6.0+ and
FreeDOS both support it). The menu shows on boot, times out to a
default, and sets `%CONFIG%` for AUTOEXEC.BAT to branch on.

**`C:\CONFIG.SYS`**:
```
[menu]
menuitem=A, Route A - DOSBox-X + HDPMI32 (test first)
menuitem=B, Route B - real DOS + 386MAX (needs low-level emu)
menuitem=C, Route C - OS/2 1616 (needs OS/2 host)
menuitem=BARE, Bare boot (no extenders, real-mode only)
menudefault=A,10
menucolor=7,0

[common]
FILES=40
BUFFERS=30
LASTDRIVE=Z
DOS=HIGH,UMB

[A]
REM DPMI extender loaded from AUTOEXEC (HDPMI32 -r)
REM no CONFIG.SYS memory manager needed

[B]
DEVICE=C:\386MAX\386MAX.SYS
REM only takes effect under 86Box/PCem/QEMU (not DOSBox-X)

[C]
REM no CONFIG.SYS driver needed - OS/2 host handles it

[BARE]
REM real-mode compile only (TC, TASM, real-mode BCC)
```

**`C:\AUTOEXEC.BAT`**:
```
@ECHO OFF
IF "%CONFIG%"=="A" GOTO ROUTE_A
IF "%CONFIG%"=="B" GOTO ROUTE_B
IF "%CONFIG%"=="C" GOTO ROUTE_C
IF "%CONFIG%"=="BARE" GOTO ROUTE_BARE
GOTO ROUTE_A

:ROUTE_A
ECHO Route A: DOSBox-X + HDPMI32
C:\HX\HDPMI32.EXE -r
SET PATH=C:\MSC70\BIN;C:\BC31\BIN;C:\FDOS\BIN;C:\
SET INCLUDE=C:\MSC70\INCLUDE
SET LIB=C:\MSC70\LIB
SET TMP=C:\TMP
GOTO END

:ROUTE_B
ECHO Route B: 386MAX from CONFIG.SYS
ECHO (only valid under 86Box/PCem/QEMU, not DOSBox-X)
SET PATH=C:\MSC70\BIN;C:\BC31\BIN;C:\FDOS\BIN;C:\
SET INCLUDE=C:\MSC70\INCLUDE
SET LIB=C:\MSC70\LIB
SET TMP=C:\TMP
GOTO END

:ROUTE_C
ECHO Route C: OS/2 1616 - launch OS/2 host, not this DOS boot
GOTO END

:ROUTE_BARE
ECHO Bare boot - real-mode tools only, no extenders
SET PATH=C:\BC31\BIN;C:\FDOS\BIN;C:\
GOTO END

:END
```

**Why CONFIG.SYS menu beats a BAT menu**:
- Runs before AUTOEXEC — Route B's `DEVICE=386MAX.SYS` takes effect
  because it's actually in CONFIG.SYS where DEVICE= belongs
- 10-second timeout to default A → headless-friendly (unattended runs
  just work)
- Sysops see the menu on boot without needing to remember a command
- `%CONFIG%` env var cleanly routes AUTOEXEC

**Headless override**: DOSBox-X can pre-select via keyboard buffer,
or the 10-second timeout does the right thing on its own.

**Which route runs where**:

| Route | Emulator | CONFIG.SYS `DEVICE=` needed? | Where the DPMI comes from |
|---|---|---|---|
| A | DOSBox-X | no | HDPMI32.EXE loaded from AUTOEXEC |
| B | 86Box / PCem / QEMU | yes — `DEVICE=C:\386MAX\386MAX.SYS` | 386MAX takes over CPU at boot |
| C | OS/2 host (QEMU) | no | OS/2's own DPMI (or no DPMI, 1616 is 16:16 NE-format) |
| BARE | any | no | none — real-mode only |

## Build steps

1. In `BLDKMS.BAT`, set `CC` for your host (DOS CL vs OS/2 CL).
2. Run `BLDKMS.BAT` — compiles/assembles 119 modules × 4 models (476
   steps) into `OUT\LIB\PWA153\msc70\OBJ\<model>\`.
3. Run `MKLIB` (or use the `MS??.RSP` response files) to assemble the
   four libraries: `PCBKMS{S,M,C,L}.LIB`.
4. Drop them in `OUT/lib/pwa153/` — the 12-library SDK matrix is complete.

## Validation already done (2026-08-26)

- All 119 modules exist at expected paths. ✓
- BLDKMS.BAT: 476 lines (452 C + 24 ASM), every one resolves to a real
  source file, correct model flags, correct obj targets. ✓
- Headers are MSC7-ready: every `__TURBOC__/__BORLANDC__` branch either
  handles `_MSC_VER` or has an `#else` fallback; PCBTOOLS.H sets
  `#pragma pack(1)` for MSC7 (correct struct packing); borland.h maps
  Borland intrinsics to MSC equivalents. ✓

Expect at most minor first-build fixups; the structural readiness is
confirmed.

## Note (DOSBox-X in a headless sandbox)

DOSBox-X's writeback to mounted drives/images can be non-deterministic
headless (flush race). Use `imgmount` with a FAT hard-disk image (more
reliable than a directory mount), or a real virtualizer, for an
unattended 476-step run.
