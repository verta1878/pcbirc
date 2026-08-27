# PCBKMS Build Setup — MSC 7.0 + 386MAX

Turnkey recipe to build PCBKMS{S,M,C,L}.LIB once you have a stable host.
Everything on our side is validated and ready; this is just the host
setup.

## What you need on the host

- **MSC 7.0** — extract `MSC70BT.ZIP` to `C:\MSC70`.
- **386MAX 8.03** (DOS route) — `devtools/386MAX-803.7z`. Install it so
  its DPMI host is loaded (386MAX.SYS in CONFIG.SYS). OR use an **OS/2**
  host and set CC to the OS/2 compiler (no 386MAX needed).
- **TASM** on PATH — for the 5 ASM modules (from BC31/PCB153BT.ZIP).
  MSC7 ships no standalone assembler; TASM's OMF .OBJ links fine.
- **Both source trees mounted:**
  - `C:\TOOLKIT\PWA153`  (95 of the 119 modules)
  - `C:\PCB153`          (24 modules, incl. the 5 ASM)
- The `OUT\LIB\PWA153\msc70\OBJ\{small,medium,compact,large}` dirs
  (BLDKMS.BAT creates them if missing).

## Host options

**Route A — real DOS + 386MAX** (proven requirement): a full-PC
virtualizer (QEMU / 86Box / PCem / VirtualBox) running MS-DOS with
386MAX loaded. 386MAX.SYS is a CONFIG.SYS memory manager — it needs a
real/virtualized 386, NOT DOSBox-X (which emulates its own DPMI and
can't host a guest memory manager).

**Route B — OS/2** (preferred, no DPMI): run the OS/2 "1616" compiler
(`C:\MSC70\OS2\BINP\CL.EXE`) under an OS/2 host. Set `CC` accordingly in
BLDKMS.BAT.

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
