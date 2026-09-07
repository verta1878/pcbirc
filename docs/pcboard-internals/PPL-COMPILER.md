# PPL Compiler — What We Have, What We Know, What's Next

## 1. PPLC Source Code

Clark's C++ codebase, one source tree, version controlled by `#define`:

| Location | Version | Era |
|---|---|---|
| `pcb153/SOURCE/PPL/` | 3.30 | PCBoard 15.3 (pwa153) |
| `pcb153/upd154/SOURCE/PPL/` | 3.40 | PCBoard 15.4 update |
| `pcb154/MAIN/SOURCE/PPL/` | 3.40 | PCBoard 15.4 (pwa154) |

Key file: `NEWSCR.CPP` — contains the version `#define`s:

    #define HDR_TXT "PCBoard Programming Language Executable  3.30\x0D\x0A\x1A"
    #define CUR_PPE_VER  330

To build any PPLC version, change these two defines and compile
with BC 3.1 under DOSBox-X.

Note: `pcb1541/PPL/pplengine/pplc/` is a **Rust rewrite** for the
15.41 IRC fork — not Clark's C++ source.

Written by: Scott Dale Robison.
Copyright: Clark Development Company, 1993-96.

### Key source files

| File | Purpose |
|---|---|
| SCRCOMP.CPP | Script compiler (main compile engine) |
| SCREXEC.CPP | Script executor (runtime) |
| EVALP.CPP | Expression evaluator |
| VAR.CPP | Variable management |
| NEWSCR.CPP | PPE header + version defines |
| LABEL.CPP | Label/goto |
| PCBMISC.CPP | PCBoard misc functions |
| SCRMISC.CPP | Misc script support |
| SCOMP.CPP | Compiler main() (in SOURCE/COMPILER/) |
| H2NAME.C | Header-to-name utility (in SOURCE/COMPILER/) |
| CEH.ASM | Critical error handler (in SOURCE/COMPILER/) |

### Build dependencies

PPLC does NOT build standalone. It links against Clark's full PCBoard
support library chain:

| Library | Source | Purpose |
|---|---|---|
| countryl.386 | SOURCE/MISC/ | Country/locale support |
| dos_l.386 | SOURCE/DOS/ | DOS abstraction layer |
| screen_l.386 | SOURCE/DISPLAY/ | Screen output (print/println/newline) |
| pcb_l.386 | SOURCE/MAIN/ | PCBoard core support |

**Note on pcbkit_l.lib:** Clark's MAKEFILEs reference pcbkit_l.lib,
a mega-library combining all individual libs plus main modules. The
pre-built version in the PWA zip is STALE (built from Clark's dev
machine with different headers). Must be rebuilt from our 8 individual
libs. See `docs/pcboard-internals/PCBKIT-LIB.md`.
| misc_l.386 | SOURCE/MISC/ | Misc utilities (fileexist, etc) |
| system_l.386 | SOURCE/SUPPORT/ | System-level support |
| doscls_l.386 | SOURCE/DOS/ | DOS class wrappers |

Plus BC 3.1 standard (cl.lib, mathl.lib, emu.lib, overlay.lib) and
CodeBase (c4base.lib, dBASE file support).

Building these libraries requires the full PCBoard source tree
from the PWA archive (`PCBoard_15_3_source_code_v0_014.zip`).

**Critical build flag: `-P`** (compile .C files as C++). Clark
compiled everything in C++ mode. Without `-P`, `TYPES.HPP` fails
because it uses C++ constructor-style casts. This is documented in
`todo/pcb-libchain-build.md` (the phased build plan, pcbsrc v0.1).

Compiler config: `LIB/CFG/BC31/PCBOARD.CFG` + `ALL.RES` (Clark's
exact flags). Use response file: `BCC.EXE +COMPILE.CFG <file>.C`.

## 2. Shipped Binaries (Reference)

Three shipped PPLC binaries are in the repo inside
`reference/roysac/PCB1522-CS2BACKUP-Clean.ZIP`
(sub-path `CSBACKUP-Clean/PCB/PPLC3??.EXE`):

| Version | Binary | Size | MD5 |
|---|---|---|---|
| 3.00 | PPLC300.EXE | 121,558 | 86325a0f3ff8006c5a19763fb8e1c267 |
| 3.10 | PPLC310.EXE | 118,328 | 18409677b4a40497001c2848e0616ce2 |
| 3.20 | PPLC320.EXE | 222,176 | 2a23e7686f79ea07bbb3c4d04e064a75 |

Also installed by PCBoard:

| Version | Location | Size |
|---|---|---|
| 1.00 | pcb1541/install/dist/target/PPLC100.EXE | 67,786 |
| 3.30 | pcb1541/install/dist/target/PPLC330.EXE | 191,918 |

**Decision:** build PPLC from source rather than extract shipped
binaries. Requires building the PCBoard support library chain first.

## 3. The Version Story

| Version | PCBoard era | Source location | CUR_PPE_VER |
|---|---|---|---|
| 1.00 | 15.0 | (not in repo as source) | 100 |
| 3.00 | 15.0+ | (not in repo as source) | 300 |
| 3.10 | 15.1 | (not in repo as source) | 310 |
| 3.20 | 15.22 | build from pcb153 + edit | 320 |
| 3.30 | 15.3 | pcb153/SOURCE/PPL/ | 330 |
| 3.40 | 15.4 | pcb153/upd154/SOURCE/PPL/ | 340 |

## 4. The v1.0.1 Problem (RUNINET.PPE byte-exact)

**Target:** `pcb1541/pcbic12/bin/RUNINET.PPE` (1,808 B, PPL 3.20)
**Source:** `pcb1541/pcbic12/src/RUNINET.PPS` (3,895 B, DECOMPILED)

Results so far:

| Compiler | Output | Match? |
|---|---|---|
| PPLC 3.20 binary | 2,261 B | NO |
| PPLC 3.30 binary | 2,261 B | NO (same output) |
| PPLC 3.40 binary | 2,286 B | NO |

**Root cause:** the PPS is a decompilation. PPE header byte 48 =
variable count: ours 0x3F (63), Clark's 0x27 (39). The decompiler
created 24 extra implicit temporaries. This is a SOURCE problem.

**Path forward:**
1. Build PPLC from source (requires lib chain — see `todo/pcb-libchain-build.md`)
2. Refactor RUNINET.PPS (reduce variables from 63 to 39)
3. Alternative: find Clark's original PPS (check reference/roysac/)

## 5. PPLC as a Multiplier

Owning the PPL compiler from source means every PPE in the PCBoard
distribution becomes a one-compile target. Three PPLC binaries closed
by changing #defines. Plus the tool that builds all PPE files.

The PCBoard support lib chain unlocks not just PPLC but PCBOARD.EXE
itself and all 207 utilities in SOURCE/UTIL/.

## 6. Related Tools

| Tool | Location | Purpose |
|---|---|---|
| PPLD | devtools/ppld32.zip | PPL decompiler (32-bit) |
| PPLX | devtools/pplx20.zip | PPL executor v2.0 |
| PPL Dev Kit | devtools/ppldevkit.zip | Developer kit installer |
| PPL Engine | pcb1541/PPL/pplengine/ | Runtime (Rust rewrite, 15.41 IRC) |
| PPLD source | pcb1541/PPL/ppld/ | Decompiler (PPLD.C, third-party) |

---

*hexadecimal, 2026-09-06*
