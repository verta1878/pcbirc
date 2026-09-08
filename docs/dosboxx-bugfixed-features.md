# DOSBox-X Bug Fixes & Current Bugs — Full Report

## FIX 1: NE2000 Internal Loopback — CONFIRMED WORKING ✅

**File:** `src/hardware/ne2000.cpp`

**Problem:** DOSBox-X's NE2000 emulation has loopback code at line 320
that calls `rx_frame()`, but `rx_frame()` rejects the loopback packet
for two reasons:

- `rx_frame()` checks `CR.stop != 0` and returns early — during
  loopback init the card is still stopped
- `rx_frame()` does MAC address filtering that rejects loopback packets

**Impact:** Artisoft NE3.EXE driver (LANtastic v6) uses internal
loopback for hardware validation during initialization. Without the
fix, NE3 polls ISR forever waiting for the loopback response.

**Change 1 — Save/restore CR.stop (line ~324):**

```cpp
// BEFORE:
        } else {
            rx_frame (& BX_NE2K_THIS s.mem[BX_NE2K_THIS s.tx_page_start*256 -
                BX_NE2K_MEMSTART],
                BX_NE2K_THIS s.tx_bytes);

// AFTER:
        } else {
            int saved_stop = BX_NE2K_THIS s.CR.stop;
            BX_NE2K_THIS s.CR.stop = 0;
            if (BX_NE2K_THIS s.page_start == 0) BX_NE2K_THIS s.page_start = BX_NE2K_THIS s.curr_page;
            rx_frame (& BX_NE2K_THIS s.mem[BX_NE2K_THIS s.tx_page_start*256 -
                BX_NE2K_MEMSTART],
                BX_NE2K_THIS s.tx_bytes);
            BX_NE2K_THIS s.CR.stop = saved_stop;
```

**Change 2 — Skip MAC filtering during loopback (line ~1414):**

```cpp
// BEFORE:
  // Do address filtering if not in promiscuous mode
  if (! BX_NE2K_THIS s.RCR.promisc) {

// AFTER:
  // Do address filtering if not in promiscuous mode and not loopback
  if (! BX_NE2K_THIS s.RCR.promisc && ! BX_NE2K_THIS s.TCR.loop_cntl) {
```

**Verification:** NE3DBG.COM diagnostic reads ISR=0x03 (pkt_rx + pkt_tx
both set) immediately after loopback transmit. Proven working on both
stock and patched DOSBox-X builds.

## FIX 2: DOSBox-X Config for LANtastic ✅

**Problem:** NE3.EXE fails under default DOSBox-X config due to three
issues.

**Fix — Required config settings:**

```ini
[cpu]
cputype=pentium       # NE3 uses ENTER/LEAVE/PUSHA (286+ instructions)

[serial]
serial2=disabled      # COM2 uses IRQ 3, conflicts with NE2000 on IRQ 3

[ne2000]
ne2000=true
nicbase=300
nicirq=3
macaddr=AC:DE:48:88:99:AA
backend=slirp         # Without backend, NIC registers return 0xFF
```

**Notes:**

- `serial2=disabled` is critical — IRQ 3 conflict causes NE2000 to
  not respond
- `backend=slirp` is critical — without it, NE2000 registers return
  all FFs (GitHub issue #815)
- NE3 with `/IOBASE=300 /IRQ=3` command-line args triggers INT 6 in
  command parser — use NO ARGS (NE3 scans default I/O bases)

## FIX 3: Headless Writeback — Image Not Flushing ✅

**Problem:** Headless DOSBox-X image writeback appeared
"non-deterministic" — a FAT hard-disk image would get written once,
then identical retries produced no file on the host. This made
unattended builds impossible.

**Root cause:** DOSBox-X hangs on an interactive prompt:

    Mounting this type of disk images requires a reported DOS version
    of 7.10 or higher. Do you want to auto-change the reported DOS
    version to 7.10 now and mount the disk image?  y/n:

Headless, no keypress arrives — DOSBox-X hangs — the outer timeout
kills it before it flushes the FAT to the host image = "no write".

**Fix — two parts:**

1. Set DOS version in config so no prompt appears:

```ini
[dos]
ver=7.10
```

2. Drive the build from `[autoexec]`, ending with explicit unmount
   to flush, then exit:

```ini
[autoexec]
imgmount c /path/to/build.img -t hdd -fs fat
c:
REM ... build commands ...
z:
imgmount c -u
exit
```

`imgmount c -u` unmounts the image, flushing the cached FAT back
to the host file. `exit` then ends DOSBox-X cleanly.

**Turnkey headless invocation:**

    timeout 600 xvfb-run -a dosbox-x -silent -conf build.conf

- `-silent` runs without UI and exits after AUTOEXEC
- `xvfb-run` provides the virtual display DOSBox-X still initializes
- Give `timeout` enough headroom for the whole build

## BUG 1: Custom DOSBox-X Build — INT 6 on 286+ Instructions — OPEN ❌

**Problem:** Any DOSBox-X built from source with
`./configure --enable-sdl2 && make` produces a binary that throws
Illegal Unhandled Interrupt 6 (invalid opcode) on 286+ instructions:
ENTER (0xC8), LEAVE (0xC9), PUSHA (0x60). The stock Ubuntu package
binary (`apt install dosbox-x`) does NOT have this bug.

**Impact:** NE3.EXE crashes with millions of INT 6 errors before
reaching any NE2000 I/O. INST6.EXE and other LANtastic binaries
using 286+ instructions also affected.

**Root cause — UNKNOWN.** Investigated:

- NOT from LTO flags — builds with `-flto=auto -ffat-lto-objects`
  still have INT 6
- NOT from configure options — `--enable-sdl2 --disable-avcodec`
  matches stock
- NOT from compiler version — same GCC 13.3.0 as stock
- Possibly related to `drive_physfs.o` compiling with ZERO symbols
  (physfsDrive class `#ifdef`'d out because `C_PHYSFS` not defined
  in `config.h`). The missing physfs integration may change code
  paths that affect CPU emulation.

The stock Ubuntu .deb is built with `dpkg-buildpackage` which applies
13 Debian patches including `system-physfs.patch` — this may be the
key difference.

**Workaround:** Use the stock Ubuntu package binary
(`/usr/bin/dosbox-x`) for running LANtastic binaries. Our NE2000
patch must be applied to the Ubuntu source package and rebuilt with
`dpkg-buildpackage` to get both the loopback fix AND correct CPU
emulation.

**Build blocker:** `dpkg-buildpackage` requires 15+ minutes for full
build (LTO link step alone takes 5-10 minutes) and ~2GB disk space
for LTO temp files. Repeatedly times out in our environment.

**Next step:** Find the `./configure` flag or Debian patch that
enables `C_PHYSFS` in `config.h`. Run `grep -i physfs configure.ac`
to find `--enable-physfs` or similar. Once physfs is properly enabled,
the link succeeds and the CPU emulation may be fixed.

## BUG 2: NE3.EXE Still Hangs After Loopback Fix — OPEN ❌

**Problem:** Even with the NE2000 loopback fix confirmed working
(ISR=0x03), NE3.EXE still hangs during initialization. It shows no
output and never reaches the TSR call.

**What works:** ISR returns 0x80 ✅, MAC reads correctly ✅,
loopback TX sets ISR.pkt_tx ✅

**What hangs:** NE3.EXE loads but produces zero output and never
returns to the command prompt.

**Possible causes:**

- INT 6 CPU bug — NE3 crashes during self-relocation before reaching
  NE2000 I/O (confirmed on custom builds, need to test on stock build
  with loopback fix)
- NE3 init sequence issue — some step between MAC read and TSR
  (shared memory test at 0x1097, IRQ handler install at 0x10EF, or
  PIC unmask at 0x1143) may fail silently
- IRQ delivery — NE3's ISR handler may not receive the loopback
  interrupt correctly (PIC masking, IRQ vector not set up, or the
  interrupt fires before the handler is installed)

**NE3 init flow (fully disassembled):**

1. Parse command line → flags
2. INT 2Fh MPX check (AL=0) → passes (AL returns unchanged)
3. Align IOBASE: `AND [0x129],0xFFE0; ADD [0x129],0x10`
4. Read MAC via remote DMA (6 bytes from data port)
5. Checksum verify
6. Shared memory test (write 0x55AA, read back)
7. Install INT 2Fh handler
8. Install IRQ handler, unmask IRQ in PIC
9. Print version banner
10. Close file handles 0-4
11. TSR via INT 21h AX=3100h

**Next step:** Build a DOSBox-X binary that has BOTH the loopback fix
AND correct CPU emulation (no INT 6). Test NE3.EXE on that binary.
If it still hangs, add `fprintf(stderr)` tracing to `write_cr()` and
`page0_write()` to find the exact stuck point.

## Patch File

Saved at `docs/ne2000_loopback.patch` in the repo. Also applied to
`devtools/dosbox-x-src-patched.7z`.

## Diagnostic Tools (need creating)

| Tool | Purpose | Result |
|---|---|---|
| READISR.COM | Read NE2000 ISR register (BASE+7) | Returns 0x80 ✅ |
| READMAC.COM | Read MAC via remote DMA | Returns AC:DE:48:88:99:AA ✅ |
| NE3DBG.COM | ISR before/after loopback TX | B00 A03 C22 — loopback works ✅ |
| NE3TEST.COM | Step-by-step NE2000 init (9 steps) | 1.2.3.4.5.6.7.8.9T — TX timeout |

## Build Environment Requirements (pcbsrc v0.1–v0.2)

These must be set in every DOSBox-X session for PCBoard source builds:

### 1. PATH

TASM, BCC, TLINK, TLIB all in BC31\BIN:

    SET PATH=C:\BC31\BIN;%PATH%

### 2. PPLC.CFG

Compiler config at `LIBSRC/CFG/PPLC.CFG` must include:

    -c
    -P
    -ml
    -3
    -ff
    -Od
    -IC:\LIBSRC\H
    -IC:\MAINSRC\H
    -IC:\PPLC\SRC
    -IC:\BC31\INCLUDE
    -DPCBOARD
    -DPCBCOMM
    -DNDEBUG
    -DPCB152
    -DPCB153
    -D___COMP___
    -DOSDRIVER
    -DFOSSIL
    -DBIGNDX
    -D___USE_VAR___
    -DS4ERROR_HOOK
    -DDBASE
    -DMG
    -DTOSSCLASS
    -d

`-IC:\PPLC\SRC` is for H2NAME.H (needed by SCOMP.CPP).
The 8 extra `-D` defines and `-d` (merge strings) match Clark’s
`PPLC.CFG` from `PCBSRCV/014/MAIN/153/`. Clark also uses `-K`
(unsigned char) and `-f` (no FP emulation) but those require the
entire library chain to be recompiled with matching flags.

### 3. Additional flags for TOOLKIT modules

INIT, PCBINIT, and INITPORT need these extra defines:

    -DPCB_MAXNODES=250   (statustype node count constant)
    -DCOMM                (modem prototypes: cdstillup, online)
    -DLIB                 (statustype with SysLimit in PCBOARD.H)

Without `-DLIB`, PCBOARD.H uses a shorter statustype that lacks
SysLimit and OverrideLimit. Without `-DCOMM`, modem function
prototypes (cdstillup, online, openmodem) are hidden.

### 4. TASM flags for ASM modules

    TASM /MX /D__l__ /iC:\LIBSRC\H source.ASM,output.OBJ

- `/MX` — case-sensitive for public/external symbols
- `/D__l__` — large memory model (selects `.model large` in source)
- `/iC:\LIBSRC\H` — include path for RULES.ASI

### 5. encrypt3 guards in NEWSCR.CPP

All encrypt3/decrypt3 calls in NEWSCR.CPP must be wrapped:

    #if CUR_PPE_VER >= 330
    encrypt3(tmpBuf, size);  // Added for 15.3
    #endif

Without this, PPLC applies 3.30 encryption (encrypt3 + encrypt2)
to 3.20 PPE files. Clark's real PPLC 3.20 only applies encrypt2.
These guards are lost when source is re-extracted from the PWA zip.
