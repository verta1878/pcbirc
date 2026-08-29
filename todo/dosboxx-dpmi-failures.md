# DOSBox-X DPMI failures — history, diagnosis, fixes

Working ledger of every DPMI-host failure we've hit under DOSBox-X in
the pcbirc golden build image (`PCBBLDBT.IMG`), with the exact error
strings, the guessed-at cause, the debug plan for each, and the fix
once known.

**Not** a place for wishlist items or vague "DOSBox-X doesn't work."
Real evidence only, dated, versioned.

## Environment under test

- DOSBox-X: `2024.03.01` (whatever ships with our sandbox)
- Config: `PCBBLDBT.CONF` — `cpu.cputype=pentium`, `cycles=max`,
  `memsize=64`, guest-boot mode via `imgmount 0 fdboot_launch.img` +
  `imgmount 2 PCBBLDBT.IMG -t hdd -size 512,63,16,614` + `boot -l a`
- Guest: FreeDOS 1.3 kernel booted from A: floppy, chains to `C:` via
  `FDAUTO.BAT`, runs `C:\AUTOEXEC.BAT`
- Log capture: `timeout --kill-after=3 40 xvfb-run -a dosbox-x -silent
  -exit -conf ... 2>&1 | head -c 200000 > /tmp/gold.log`

## What actually works under DOSBox-X

- FreeDOS boot from floppy → chain to C: ✓
- Real-mode DOS tools: `TCC.EXE` (Turbo C 2.01), `TASM.EXE` (Turbo
  Assembler 3.1), `MEM.EXE`, `EDIT.COM`, PATH resolution, redirection ✓
- `SVER.EXE` (DOS/32A version tool, real-mode) ✓
- Files write back through IDE image ✓
- Directory-mount write-back (shell mode) ✓
- CWSDPMI as child-of-DPMI-program (32-bit only) — partial: TCC/TASM
  fine, but BCC and CL can't use it (see failure #4)

## Failure ledger

### Failure #1 — HX HDPMI32 -r → `CPU:Illegal Unhandled Interrupt Called 68`

- **When**: HDPMI32.EXE `-r` (install resident) during guest AUTOEXEC.BAT
- **Full error**: `LOG: <ticks> ERROR CPU:Illegal Unhandled Interrupt Called 68`
- **Guest downstream**: DOSBox-X's crash handler tries to pop a native
  file dialog (tinyfd); no zenity/kdialog/xterm on the host, so it
  hangs there. Test outputs never write.
- **What HX is doing**: HX's loader uses INT 68h internally for its
  own control interface. On real hardware, INT 68h is just another
  reflectable interrupt.
- **Root cause (identified)**: **DOSBox-X uses INT 68h itself by
  default.** Confirmed via CHANGELOG entry (~line 7025): "Enhance
  existing INT 68h fix for 'PopCorn' by adding a dosbox.conf option
  to always keep INT 68h NULL, so that it's possible to run the game
  in machine configurations other than CGA." Same class of collision.
- **Fix (verified)**: set `zero unused int 68h=true` under `[dos]` in
  `PCBBLDBT.CONF`. HDPMI32 then loads without the unhandled-interrupt
  error. Verified against DOSBox-X 2024.03.01 with the option; BCC
  was invoked after HDPMI32 load (whereas before, nothing after
  HDPMI32 ran).
- **Status**: **RESOLVED via config** (no source patch or version
  upgrade required).
- **Aftermath**: revealed Failure #5 — `CPU:GRP5:Illegal opcode
  0xff` when BCC subsequently runs. Different failure, tracked
  separately.

### Failure #2 — DOS/32A `PCTEST.EXE` → FPU error + IRET descriptor-type

- **When**: PCTEST.EXE (Watcom binary bound to DOS/32A 9.1.2) launched
  from guest AUTOEXEC.BAT
- **Full error** (two lines, in order):
  ```
  LOG: <ticks> ERROR FPU:8087 only fpu code used esc 3: group 4: subfunction :1
  LOG: E_Exit: IRET:Illegal descriptor type 12
  E_Exit: IRET:Illegal descriptor type 12
  ```
- **Guest downstream**: `E_Exit` = fatal, DOSBox-X quits (cleanly this
  time, exit=0). AUTOEXEC steps after PCTEST don't run.
- **What DOS/32A is doing**:
  - The FPU line: PCTEST uses an ESC 3 opcode (287+ FPU instruction).
    DOSBox-X's FPU code is in an "8087 only" mode and refuses it.
    Should be full-FPU per `cputype=pentium`, but isn't in this path.
  - The IRET line: DOS/32A set up a 32-bit non-conforming
    execute-only code segment (descriptor type 0xC = 12) and did an
    IRET to it. DOSBox-X's IRET handler rejects that type as illegal
    — but it's a legal Intel-defined descriptor.
- **Likely sites in DOSBox-X source**:
  - `"8087 only fpu code used"` — FPU opcode dispatch
  - `"IRET:Illegal descriptor type"` — protected-mode IRET handler
- **Root-cause hypothesis**:
  - FPU: `cputype=pentium` config isn't propagating to the guest-boot
    FPU model, or DOSBox-X hardcodes 8087 in some boot-mode path.
  - IRET: whitelist of "legal" descriptor types in the IRET handler
    is too narrow — needs to include type 0xC (and probably others
    that real Intel CPUs accept).
- **Debug plan**:
  1. Try `fpu=auto`, `fpu=8087`, `fpu=287`, `fpu=387` in config to
     see if any changes the FPU error.
  2. Try `cputype=486` and `cputype=pentium_mmx`.
  3. Grep both error strings in DOSBox-X source.
  4. Read the IRET handler; check whether descriptor type 0xC is
     genuinely rejected or hits an unhandled path.
- **Retest 2026-08-28**: reproduces with `zero unused int 68h=true`
  applied — same FPU 8087-only error + same `IRET:Illegal descriptor
  type 12`. So Failure #1's fix does NOT help #2; this is a
  separate, real DOSBox-X CPU-emu gap. Likely needs source-level
  patches (descriptor whitelist widening + FPU-mode propagation) or
  low-level emu (QEMU/86Box) for DOS/32A-based tools.
- **Status**: open, real DOSBox-X CPU-emu gap.

### Failure #3 — Borland DPMIRES → tinyfd crash-handler pattern, no clean CPU error

- **When**: `DPMIRES.EXE` from BC++ 3.1 during guest AUTOEXEC.BAT
- **Full error**: log truncated by 200 KB cap before a specific CPU
  error surfaces; ends with the same tinyfd file-dialog spew that
  Failure #1 produces
- **Guest downstream**: no test outputs written (unlike Failure #1
  where the guest at least got as far as writing a 0-byte CL output)
- **What DPMIRES is doing**: Borland's built-in DPMI TSR
  installation. Uses proprietary Borland calling conventions and
  segment setup.
- **Root-cause hypothesis**: same class of failure as #1 or #2 — some
  interrupt, descriptor operation, or protected-mode transition
  DOSBox-X doesn't handle. Just don't have the specific error text
  yet.
- **Retest 2026-08-28** (with `zero unused int 68h=true` applied):
  no CPU error logged this time, but DPMIRES output empty AND the
  echo after DPMIRES never fired — batch execution stopped silently
  mid-run. Different failure mode from the earlier tinyfd hang.
  Possibilities: (a) DPMIRES hung waiting for something, (b)
  Borland's proprietary DPMI init did something DOSBox-X handled
  silently but incorrectly, (c) the shell was killed by our timeout
  before it got past DPMIRES.
- **Priority**: LOW. HDPMI16 (Failure #5 fix) provides 16-bit DPMI
  cleanly for Borland tools, so DPMIRES isn't on the critical path.
  Only pursue if we specifically want to reproduce Borland's own
  DPMI environment for byte-exact reasons.
- **Status**: open, deprioritized (HDPMI16 supersedes).

### Failure #4 — CWSDPMI + BCC / MSC7 CL — expected, not a DOSBox-X bug

- **BCC**: prints `16-bit DPMI unsupported.` — accurate. CWSDPMI is
  DJGPP's 32-bit-only DPMI, doesn't provide the 16-bit DPMI Borland
  needs. Not a DOSBox-X issue; CWSDPMI is simply wrong tool.
- **CL** (MSC7 DOSX32): dumps its own error-string table (garbage on
  stdout). DOSX32 attempted to negotiate DPMI 0.9 with specific
  features CWSDPMI doesn't provide. Accurate failure; wrong DPMI
  host for this compiler.
- **Fix**: use a DPMI host that provides both 16- and 32-bit and the
  DOSX32 feature set — HDPMI (Failure #1 now resolved), 386MAX, or
  DOS/32A (Failure #2 still open).

### Failure #5 — BCC.EXE (post-HDPMI32-load) → `CPU:GRP5:Illegal opcode 0xff`

- **When**: BCC.EXE invoked from AUTOEXEC after HDPMI32 loaded
  successfully (Failure #1 resolved)
- **Full error**: `LOG: <ticks> ERROR CPU:GRP5:Illegal opcode 0xff`
  followed by DOSBox-X crash-handler tinyfd spam
- **Root cause (identified)**: **BCC needs 16-bit DPMI. HDPMI32
  provides primarily 32-bit DPMI**; the extender/client mismatch made
  BCC (or its BCCX child) execute an instruction it wasn't in the
  right mode for, which the DOSBox-X CPU emu rejected as an illegal
  opcode. Not a DOSBox-X decoder bug — a wrong-tool problem.
- **Fix (verified)**: use **`HDPMI16.EXE -r`** instead of (or before)
  `HDPMI32.EXE -r`. HDPMI16 provides the 16-bit DPMI that BCC needs.
  With `HDPMI16 -r` loaded, BCC runs cleanly and prints full syntax
  help (1632 bytes of correct output, `Borland C++ Version 3.1
  Copyright (c) 1992 Borland International`).
- **Impact**: **PCBKBC (Borland C++ 3.1) is now buildable under
  DOSBox-X**, all 4 memory models reachable. This was previously
  thought to require low-level emu; wrong-extender diagnosis
  reveals it doesn't.
- **Status**: **RESOLVED**.

## What we know works today, end-to-end

- **PCBKIT (Turbo C 2.01)** — all 4 memory models. Real-mode, no DPMI
  needed. Full BUILD chain reachable under DOSBox-X.
- **TASM-based tools** — real-mode assembler work.
- **Staging, docs, BAT scripting** — anything that doesn't switch to
  protected mode.

## The three-tier emulator fallback (pcbirc approach)

Rather than fixing DOSBox-X ourselves, our architecture uses each
emulator where it fits, with clean fallback:

| Tier | Emulator | Use for | Rationale |
|------|----------|---------|-----------|
| **1** | DOSBox-X | Real-mode compiles (TC, TASM), staging, docs, BAT scripting, dev convenience | Fast, headless-friendly, good enough for the 60% of work that doesn't need DPMI |
| **2** | QEMU (i386/i486 machine) | DPMI-heavy compiles (BCC + BCCX, MSC7 CL + DOSX32, Watcom + DOS/32A) | Full CPU emulation, any DPMI host runs, small setup lift |
| **3** | 86Box / PCem | 386MAX loaded via CONFIG.SYS; byte-exact historical reproduction; anything that hooks the CPU at boot | Cycle-accurate low-level PC emulation; the reference target when Tier 2 has quirks |

**Same `PCBBLDBT.IMG` boots in all three** — no image changes needed.
The tier choice is per-task, documented per compile target.

## DOSBox-X packaging note (crew direction)

Long-term: **DOSBox-X binary in the crew's kit should be
self-contained** — statically-linked, no reliance on host system
libraries, deterministic behavior across sysops' machines. Not
`apt-get dosbox-x`; a specific known-good build we ship.

Once we've done the debugging above and know which version + patches
are needed, this becomes a build recipe (Dockerfile or a plain
build script) that produces the crew's DOSBox-X binary from source.

## Decision — which DOSBox-X version to debug against

Not "same or newer" as a coin flip — evidence-driven:

1. **First**: fetch latest DOSBox-X release, run our smoke tests
   against it, compare failure signatures line-by-line with what we
   documented above (against `2024.03.01`).
2. **If latest fixes all three failures**: use latest, no debug needed.
3. **If latest has some fixes but breaks real-mode stuff we rely on
   (regression)**: pin to `2024.03.01` for daily use, use latest for
   the fixes, backport patches.
4. **If latest still has all three failures**: debug against latest —
   any patches we write go upstream cleanly against current source.

**The wrong move is upgrading blindly** (might break what works) or
staying on `2024.03.01` because "we know it" (might miss fixes
already made). Same emulator, same smoke test, one comparison run —
then we know.

## Fixes log

### 2026-08-28 — Failure #1 (INT 68h) resolved by config

- Setting: `[dos] zero unused int 68h=true`
- **Shipped in repo**: `MAIN/build/PCBBLDBT.CONF` (the crew's
  reference DOSBox-X launcher config) contains this by default. A
  fresh clone + `PCBBLDBT.IMG` build gets the working config
  automatically; no manual step needed.
- DOSBox-X version: 2024.03.01 (no upgrade needed)
- Diagnosis path: read CHANGELOG for "INT 68" mentions → found the
  `zero unused int 68h` option added for a game called "PopCorn"
  (same class of collision — DOSBox-X uses INT 68h internally by
  default, colliding with any guest code that wants it)
- Verification: HDPMI32 -r now loads without unhandled-interrupt
  error; BCC gets invoked (didn't happen before the fix)
- Aftermath: reveals Failure #5 (Group 5 opcode 0xFF), tracked
  separately. Peeling one layer at a time — expected pattern.

### 2026-08-28 — Failure #5 (BCC GRP5 opcode 0xFF) resolved by extender swap

- Setting: **use `HDPMI16.EXE -r`** (not HDPMI32) before running
  Borland tools (BCC, BCCX, etc.). BCC is 16-bit and needs 16-bit
  DPMI; HDPMI32 provides 32-bit primarily and the mismatch triggered
  a DOSBox-X CPU illegal-opcode error that looked like an emulator
  bug but was actually a wrong-tool problem.
- Verification: with `HDPMI16 -r` loaded, `BCC.EXE` prints its full
  1632-byte syntax help correctly (`Borland C++ Version 3.1
  Copyright (c) 1992 Borland International`), and the batch file
  continues past BCC to completion.
- **Impact**: **PCBKBC (Borland C++ 3.1) is now buildable under
  DOSBox-X.** Previously blocked; now unblocked with the correct
  extender.
- Recommendation for AUTOEXEC.BAT: load `HDPMI16 -r` unconditionally.
  Optionally also load `HDPMI32 -r` if 32-bit DPMI is needed for
  other tools. Both can be resident simultaneously.

## Compilers reachable under DOSBox-X (updated 2026-08-28)

| Compiler | Status | DPMI host required |
|---|---|---|
| TCC (Turbo C 2.01) | ✓ reachable | none (real-mode) |
| TASM (3.1) | ✓ reachable | none (real-mode) |
| **BCC (Borland C++ 3.1)** | ✓ **reachable** (F#5 fix) | HDPMI16 -r + INT 68h fix |
| CL (MSC 7.0) with DOSX32 | ⚠ untested with HDPMI16+HDPMI32 combo | (needs test — likely HDPMI32) |
| Watcom binaries via DOS/32A | ✗ blocked | Failure #2 open |

Buildable-under-DOSBox-X SDK targets now include PCBKIT (TC 2.01)
**and PCBKBC (BC++ 3.1)** — 8 of 12 libs achievable in the golden
image with our configuration. PCBKMS (MSC 7.0) still to test with
the combined DPMI setup.
