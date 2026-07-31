# OpenWatcom Port Workmap — pcbrevival

**Goal:** All 12 binaries building under OpenWatcom. No Borland dependency. No DOSBox.
**Result:** pcbrevival becomes **pcbirc**. openwatcomirc becomes the compiler.

---

## Current State

- 12 of 12 binaries built (11 Borland DOS + 1 Watcom OS/2)
- 129 of 273 MAIN source files compile under Watcom (OS/2 target)
- 144 MAIN source files need fixes for Watcom DOS target
- 22 library files with inline Borland ASM
- 7 standalone TASM files (6,649 lines total)
- 185 files reference pascal calling convention
- OS/2 PCBOARD2.EXE already links clean

---

## Phase 1: Foundation — Compiler Compatibility Layer

**What:** Make all C/C++ source compile under both Borland and Watcom.
**Why:** Everything else depends on this. No point touching ASM if the C won't compile.

### 1.1 — WATCOMPAT.H expansion
- [ ] Extend existing WATCOMPAT.H with all Borland→Watcom mappings
- [ ] `farmalloc` → `malloc` (flat model, no far/near distinction)
- [ ] `farfree` → `free`
- [ ] `farcoreleft`/`coreleft` → Watcom equivalents
- [ ] `bioskey` → Watcom keyboard functions
- [ ] `stpcpy` → inline implementation
- [ ] `_AX`/`_BX`/`_CX`/`_DX` pseudo-registers → Watcom `#pragma aux`
- [ ] `geninterrupt()` → Watcom `int86()`
- [ ] `__emit__()` → Watcom inline ASM
- [ ] 30 library files with Borland CRT functions

### 1.2 — Pascal calling convention
- [ ] Audit 185 files with `pascal` keyword
- [ ] Map Borland `pascal` to Watcom `__pascal` or `__stdcall`
- [ ] PROTOTYP.H has 0 pascal declarations — most are in library headers
- [ ] Verify parameter order (pascal = left-to-right, C = right-to-left)

### 1.3 — Borland pragmas and keywords
- [ ] `#pragma option` → Watcom equivalents or remove
- [ ] `#pragma warn` → Watcom `-w` flags
- [ ] `interrupt` keyword → Watcom `__interrupt`
- [ ] `_seg` keyword → remove (flat model)
- [ ] 41 files with Borland-specific pragmas

### 1.4 — types.hpp / dosfunc.h / pcbtools.h
- [ ] Already partially done for OS/2 — extend `__WATCOMC__` guards for DOS target
- [ ] bool/true/false guards (done)
- [ ] minSType/maxSType C-cast macros (done)
- [ ] `#pragma pack` guards (done)
- [ ] Verify all headers compile under Watcom DOS target (`-bt=dos`)

**Deliverable:** All 273 MAIN source files + all library files compile with `wpp386 -bt=dos`.

---

## Phase 2: Inline ASM — The 22 Library Files

**What:** Convert Borland inline ASM to Watcom inline ASM or `#pragma aux`.
**Why:** These 22 files are currently stubbed for OS/2. DOS needs them real.

### Priority order (by impact):

### 2.1 — Screen/Video (critical for all binaries)
- [ ] GETMODE.C — 29 ASM lines — video mode detection, INT 10h
- [ ] SCROLLDN.C — 56 ASM lines — screen scroll, INT 10h
- [ ] SCROLLUP.C — 56 ASM lines — screen scroll, INT 10h
- [ ] ANSI.C — 11 ASM lines — ANSI escape handling
- [ ] TWODIG.C — 12 ASM lines — two-digit number formatting
- [ ] TWODIG0.C — 9 ASM lines — zero-padded two-digit
- [ ] TIMECHNG.C — 7 ASM lines — timer tick via BIOS

### 2.2 — System/CPU (needed for startup)
- [ ] CPUTYPE.C — 63 ASM lines — CPU detection (386/486/Pentium)
- [ ] SHOWERR.C — 0 inline ASM but uses `bioskey()` — Borland CRT issue
- [ ] DATA120.C — 0 inline ASM but uses `validatepath()` + wrong `doscreate()` args

### 2.3 — String/Memory (performance-critical)
- [ ] WILDCARD.C — 25 ASM lines — wildcard pattern matching
- [ ] ZSWAPSTR.C — 25 ASM lines — fast string swap
- [ ] ZSWAPVIR.C — 20 ASM lines — virtual memory string swap
- [ ] CONFFUNC.C — 69 ASM lines — conference configuration

### 2.4 — DOS File I/O
- [ ] DOSFIND.C — lvalue cast issue, not ASM — easy fix
- [ ] VIRTUAL.C — `farmalloc`/`farfree`/wrong `dosread()` args
- [ ] VIRTUAL1.C — `coreleft`/`stat.h` path/pointer casts

### 2.5 — Toolkit Samples (low priority — not linked into any binary)
- [ ] COPYBIN1.C, COPYBIN2.C, COPYTEXT.C, HELLO.C, UPGRADE.C
- [ ] These are sample/demo code, not part of any shipping binary
- [ ] Fix last if time permits, skip otherwise

**Deliverable:** All 22 library files compile under Watcom. OS/2 stubs replaced with real code.

---

## Phase 3: Standalone ASM — TASM to WASM

**What:** Port 7 TASM files (6,649 lines) to WASM syntax.
**Why:** These are core runtime — serial I/O, screen, startup. Can't ship without them.

### 3.1 — C0.ASM (895 lines) — CRT Startup
- [ ] Borland C runtime initialization — stack setup, heap init, file handle allocation
- [ ] Clark modified it (DWT 4/19/93) to remove Borland's `__nfile` hack
- [ ] Has `_OVERLAY_` segment for overlay manager
- [ ] **Decision:** Watcom has its own CRT startup. May not need this at all.
- [ ] If Watcom startup suffices, delete. If not, port segment directives.

### 3.2 — ASYNC.ASM (1,893 lines) — Serial/FOSSIL Driver
- [ ] FOSSIL INT 14h interface — the entire comm layer
- [ ] Uses Borland `pascal` calling convention (`.model large, pascal`)
- [ ] Uses `rules.asi` include for memory model macros
- [ ] **This is the hardest single file.** PCBOARDM and PCBOARD depend on it.
- [ ] LOCAL.EXE does NOT need this (no COMM)
- [ ] Port: change `.model` directive, fix `PROC` declarations, adjust segment names

### 3.3 — ANSI.ASM (1,717 lines) — ANSI Terminal Processing
- [ ] Full ANSI X3.64 escape sequence parser
- [ ] Screen buffer manipulation, cursor control
- [ ] Uses Borland segment naming conventions
- [ ] Port: segment directives + PROC conventions

### 3.4 — CUTIL.ASM (1,101 lines) — C Utility Functions
- [ ] Fast string/memory operations in ASM
- [ ] Mixed C and pascal calling conventions
- [ ] Port: calling convention adjustments

### 3.5 — NOSCROLL.ASM (387 lines) — Screen Buffer
- [ ] Direct video memory access, no-scroll screen update
- [ ] INT 10h BIOS calls
- [ ] Moderate port — mostly standard x86

### 3.6 — MEMMOVE.ASM (340 lines) — Memory Move
- [ ] Optimized memory copy/move
- [ ] May be replaceable with Watcom intrinsics
- [ ] **Decision:** Profile whether Watcom's built-in `memmove` is fast enough

### 3.7 — TIMER.ASM (231 lines) — Timer Tick
- [ ] BIOS timer access, delay loops
- [ ] Straightforward INT 1Ah/port 40h access
- [ ] Easy port

### 3.8 — BGKEY.ASM (85 lines) — Background Keyboard
- [ ] Keyboard buffer polling
- [ ] Smallest file, easy port

### Additional ASM files:
- [ ] INT24HND.ASM (LIB) — DOS critical error handler
- [ ] SWAP.ASM (LIB) — memory swap routines
- [ ] XMODEM.ASM (LIB) — XMODEM protocol fast CRC
- [ ] CEH.ASM (COMPILER) — compiler error handler
- [ ] Our stubs: PCBTHUNK.ASM, SETUPTHK.ASM, INT24STB.ASM — already TASM, need WASM port

**TASM→WASM cheat sheet:**
- `PROC pascal` → `PROC __pascal` or rework to `PROC __watcall`
- `include rules.asi` → rewrite for Watcom memory model macros
- `SEGMENT ... PUBLIC` → same syntax, different default naming
- `ASSUME` directives → mostly same
- `LOCAL` variables → same syntax
- `.model large` → `.model flat` for 32-bit targets

**Deliverable:** All ASM files assemble with WASM. Calling conventions match Watcom C++ OBJs.

---

## Phase 4: Overlay Manager

**What:** Handle PCBOARD.EXE's Borland overlay system.
**Why:** PCBOARD.EXE is the non-386 version that uses overlays to fit in 640K.

- [ ] PCBOARD.EXE uses `__OVERLAY__` define + Borland `overlay.lib`
- [ ] PCBSM.EXE also links `overlay.lib`
- [ ] PCBSETUP.EXE has `#if __OVERLAY__` conditional
- [ ] C0.ASM has `_OVERLAY_` segment

### Options:
1. **Watcom's overlay manager** — Watcom has its own overlay support (`option DYNAMIC` in linker). Different API. Would need adaptation.
2. **Drop overlays, go 32-bit only** — PCBOARDM.EXE is already 386-only. Make PCBOARD.EXE 386 too. Overlays were a 640K workaround — irrelevant on modern DOS (DPMI/DOS4GW).
3. **Keep both** — PCBOARDM (32-bit, no overlay) + PCBOARD (overlay version via Watcom overlay manager)

**Recommendation:** Option 2. Ship PCBOARDM as the DOS binary. PCBOARD.EXE overlay version is a 640K-era artifact. Document the decision.

**Deliverable:** Decision made and documented. If keeping overlays, Watcom overlay manager integrated.

---

## Phase 5: Link and Test — DOS Binaries

**What:** Link all 11 DOS binaries under Watcom and verify against Borland builds.

### 5.1 — Easy targets first (fewer dependencies)
- [ ] MKPCBTXT.EXE — smallest, simplest
- [ ] PPLC.EXE — self-contained compiler
- [ ] PCBSM.EXE — System Manager

### 5.2 — UUCP stack
- [ ] UUIN.EXE
- [ ] UUOUT.EXE
- [ ] UUUTIL.EXE
- [ ] UUXFER.EXE
- [ ] These share PCBTHUNK.ASM — needs WASM port first

### 5.3 — Setup
- [ ] PCBSETUP.EXE — has its own ASM (SETUPTHK, INT24STB, REVTHUNK)

### 5.4 — Main binaries (hardest — need ASYNC.ASM, ANSI.ASM, full library)
- [ ] LOCAL.EXE — no COMM, good test target
- [ ] PCBOARDM.EXE — full 386+COMM build
- [ ] PCBOARD.EXE — overlay version (Phase 4 decision affects this)

### 5.5 — Verification
- [ ] Size comparison against Borland builds
- [ ] Feature string extraction and comparison
- [ ] Run in DOSBox and verify startup/login flow
- [ ] FOSSIL handshake test (ASYNC.ASM port)

**Deliverable:** 11 DOS binaries linking clean under Watcom. Feature-verified.

---

## Phase 6: Unify DOS and OS/2

**What:** Single source tree, single compiler, both platforms.
**Why:** This is the whole point — one `#ifdef` codebase.

- [ ] Merge OS/2 stubs back into real implementations where DOS and OS/2 share code
- [ ] OS/2-only code stays behind `#ifdef __OS2__`
- [ ] DOS-only code stays behind `#ifdef __DOS__`
- [ ] Remove Borland-only code paths (now dead)
- [ ] Single build script: `BUILD_OW.SH` with target parameter
- [ ] Clean up duplicate OBJ naming (LIB_ prefix no longer needed)

**Deliverable:** `wpp386 -bt=dos` builds DOS. `wpp386 -bt=os2v2` builds OS/2. Same source.

---

## Phase 7: openwatcomirc

**What:** sysop/0 forks OpenWatcom, adds GCC backend.
**Why:** Linux/FreeBSD/macOS targets from the same codebase.

- [ ] sysop/0's work — not hexadecimal's phase
- [ ] PCBoard source ready for it after Phase 6
- [ ] GCCCOMPAT.H layer if needed
- [ ] Same `#ifdef` pattern: `__WATCOMC__` / `__GNUC__`
- [ ] pcbrevival renamed to **pcbirc**

**Deliverable:** One codebase → DOS + OS/2 + Linux + FreeBSD + macOS.

---

## Estimated Effort

| Phase | Work | Difficulty |
|---|---|---|
| 1. Compiler compat | WATCOMPAT.H + 273 source files | Medium |
| 2. Inline ASM | 22 library files, ~380 ASM lines | Medium |
| 3. Standalone ASM | 7 files, 6,649 lines (ASYNC.ASM is hard) | **Hard** |
| 4. Overlay manager | Decision + possible Watcom overlay port | Medium |
| 5. Link and test | 11 DOS binaries | Medium |
| 6. Unify DOS/OS2 | Merge ifdef paths | Easy |
| 7. openwatcomirc | sysop/0's GCC backend | sysop/0 |

**Critical path:** Phase 1 → Phase 3.2 (ASYNC.ASM) → Phase 5.4 (PCBOARDM link)

ASYNC.ASM is the single hardest item. 1,893 lines of FOSSIL/serial I/O in TASM pascal convention. Everything else is solvable incrementally. ASYNC.ASM is the gate.

---

## Notes

- Borland C++ 3.1 does not support OS/2 32-bit flat model — confirmed
- Clark had `__WATCOMC__` guards in borland.h — he planned for Watcom
- Watcom C++ name mangling includes type hashes — struct definitions must match across all OBJs
- Watcom OS/2 flat model naming: variables get leading `_`, functions get trailing `_`
- CodeBase 4.x (not 6.5) — S4VERSION 5002, S4NDX index mode
- TOOLKIT/SAMPLES (COPYBIN1/2, COPYTEXT, HELLO, UPGRADE) are demo code — not linked, low priority
