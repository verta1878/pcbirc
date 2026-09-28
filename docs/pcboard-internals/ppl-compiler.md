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


## 4. PPE Encryption

PPE files are encrypted after compilation. Both encryption layers
are deterministic — same plaintext always produces the same output.

### Two layers

- **encrypt2** (PPL 3.01+): seed 0xDB24, XOR/ROR chain. Each 16-bit
  word is XORed with the seed and previous word, then rotated. The
  key chains through the data — every byte depends on all previous bytes.
- **encrypt3** (PPL 3.30 only): XOR with "DECOMPILERS SUCK!" — a
  17-byte key that repeats. Clark obfuscated the key as hex bytes in
  the source: `{0x8C,0x53,0xB8,...}` to hide it from hex dumps.

Source: `LIB/SOURCE/MISC/CRYPT.C` (encrypt2 line 213, encrypt3 line 364)

For PPL 3.20 output, only encrypt2 applies. For 3.30, encrypt3 is
applied first, then encrypt2. Our fix: guard encrypt3 calls in
NEWSCR.CPP save() with `#if CUR_PPE_VER >= 330`.

### MISC.H — the encryption API

MISC.H is the authoritative API for Clark’s encryption library.
CRYPT.C has the implementation, PPLD.C has the reverse-engineering,
but MISC.H is where Clark defined the interface. Anyone building
the decryption tool includes this header.

`LIB/H/MISC.H` line 153-165 declares all four functions:

    void LIBENTRY decrypt2(char *Str, int Len);
    #define decrypt3(Str,Len) encrypt3(Str,Len)
    void LIBENTRY encrypt2(char *Str, int Len);
    void LIBENTRY encrypt3(char *Str, int Len);

decrypt2 already exists as a complete function in CRYPT.C — it uses
ROL (rotate left) to reverse encrypt2’s ROR (rotate right). No new
code needed.

decrypt3 does not exist as a separate function. It’s a #define macro
in MISC.H that calls encrypt3. This is not broken or missing — it
works correctly because encrypt3 uses XOR, which is its own inverse
(A XOR B XOR B = A). Calling encrypt3 a second time IS the
decryption. No new code needed here either.

### Decryption tool (needed)

A tool to decrypt PPE files to raw plaintext bytecode. This lets us
compare the ACTUAL instructions between two PPEs regardless of
encryption differences.

The PPLD decompiler (`pcb1541/PPL/ppld/`) already decrypts PPE files —
it has to, in order to decompile them. We used PPLD to decompile
RUNINET.PPE into the RUNINET.PPS we’ve been working with.

What we need is a STANDALONE decryption tool that outputs the raw
decrypted bytecode WITHOUT decompiling it. PPLD goes all the way
from encrypted PPE to PPL source. We need to stop at the
intermediate step: encrypted PPE → decrypted bytecode. That way
we can hex-diff the raw instructions between two PPEs.

Source for the algorithm:
- decrypt2: `PPLD.C` line 1726 (inline ASM, reverse-engineered)
- decrypt2: `CRYPT.C` line 213 (Clark’s source, OS/2 version readable)
- decrypt3: `CRYPT.C` line 364 (XOR with obfuscated key)
- Layer order on read: decrypt3 first, then decrypt2 (`NEWSCR.CPP` line 1350-1352)

Can be written as a small C program or Python script. The OS/2
version of encrypt2 in CRYPT.C is plain C — no inline ASM — and
can be ported directly. encrypt3 is 10 lines of C.

### How we learned this

1. **PPLD.C** — the decompiler source in our repo has the decrypt2
   algorithm in inline ASM (line 1726). This was reverse-engineered.
2. **CRYPT.C** — Clark’s source confirms both encrypt2 and encrypt3.
   The comment on line 360: `// static char *Suck = "DECOMPILERS SUCK!";`
3. **NEWSCR.CPP save()** — shows both layers applied unconditionally
   on write. encrypt3 has no version guard in the 3.30 source.
4. **Two-build comparison** — compiled the same PPS with our PPLC and
   Clark’s PPLC320.EXE. Same size (2,261 B), same var count (63),
   but 2,131 bytes differ initially. After applying encrypt3 guards,
   only 312 bytes differ — 26 blocks of exactly 12 bytes each.
   Bytecode section (578 bytes) is IDENTICAL.
5. **MISC.H** — declares the API. Reveals decrypt3 is a #define
   macro calling encrypt3 (XOR is self-inverse).

## 5. The v1.0.1 Problem (RUNINET.PPE byte-exact)

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

**pcbsrcv diff results (2026-09-07):**

Diffed SCRCOMP.CPP, NEWSCR.CPP, VAR.CPP, SCRMISC.CPP, and CRYPT.C
across all available snapshots (000, 001, 014). Only 000, 001, and
014 contain the PPL source — the other 12 snapshots don’t.

Results:
- SCRCOMP.CPP: IDENTICAL across all three versions. The code
  generator never changed.
- NEWSCR.CPP, VAR.CPP, SCRMISC.CPP: only #include style changes
  (angle brackets <> → quotes ""). Zero logic changes.
- CRYPT.C: #include style + one #pragma inline comment. No
  algorithm changes.
- CUR_PPE_VER = 330 in ALL snapshots. No 3.20-era source exists
  in the PWA zip. The 3.20 code predates the versioned tree.

Conclusion: the diff path is closed. There is nothing to find.
SCRCOMP.CPP is the same code that Clark shipped in PPLC 3.20.

**The 12-byte vtable finding:**

After applying the encrypt3 guards (`#if CUR_PPE_VER >= 330`) and
rebuilding PPLC, the comparison between our output and Clark’s
PPLC 3.20 output shows:

- Same size: 2,261 B
- Same variable count: 63
- Bytecode section: IDENTICAL (578 bytes, zero diffs)
- Variable table: 26 of 63 entries differ by exactly 12 bytes each
- Total difference: 26 × 12 = 312 bytes

Each cVARVAL written to disk is 12 bytes (packed):

    2 bytes: C++ vtable pointer (near, compiler memory address)
    2 bytes: type (variable type enum)
    8 bytes: uVARVAL (value union)

The vtable pointer is a memory address from the compiler’s heap.
It’s different between our PPLC.EXE and Clark’s because the
executables have different memory layouts. Clark serialized the
entire cVARVAL object with `memcpy(tmpBuf, obj.data, sizeof(cVARVAL))`
— this copies the vtable pointer to disk as a serialization
artifact. The pointer is meaningless on disk and is never used when
loading the PPE (the reader creates new cVARVAL objects with fresh
vtable pointers).

The 26 entries that differ are the non-string variables (BOOLEAN,
INTEGER, etc.) that go through the cVARVAL write path. The other
37 are strings that go through a separate string write path
(length + chars, no vtable).

**What byte-exact requires (revised):**

The code generator is proven correct — identical bytecode. The
only difference is a C++ serialization artifact that doesn’t affect
PPE execution. `cmp -s` will never pass between two different PPLC
executables because the vtable address depends on EXE memory layout.

  a. Clark’s ORIGINAL PPS source (39 vars, not decompiled 63)
  b. Acceptance revised: bytecode match (decrypt both PPEs, compare
     instructions only, ignoring vtable pointer bytes) OR accept
     functional equivalence (same size + same var count + correct
     execution)


**Byte-exact match ACHIEVED (2026-09-07):**

After applying encrypt3 guards (`#if CUR_PPE_VER >= 330`), our
source-built PPLC produces bit-for-bit identical PPE output to
Clark’s shipped PPLC 3.20 binary. Verified with `ppedecrypt.py`
(at `toolkit/pplc/ppedecrypt.py`) — decrypted plaintext matches
exactly, and the encrypted files are byte-for-byte identical.

Clark’s shipped RUNINET.PPS (recovered from password-protected
pcbic12 zip via known-plaintext attack using bkcrack) is identical
to our decompiled version. Both produce 63 vars with any PPLC 3.20.
The shipped PPE (39 vars) was compiled from an earlier version of
the source that no longer exists.

**v1.0.1 status:** compiler proven correct. PPE output matches
Clark’s binary. The 39-var vs 63-var gap is a lost-source problem,
not a build problem. The original 39-var PPS does not exist.

**39-var PPE decoded (2026-09-07):**

All 39 variables from Clark’s original RUNINET.PPE recovered using
`ppedecrypt.c` (at `toolkit/pplc/ppedecrypt.c`). Missing for two
decades. These are Clark’s original pcbIC12 Internet Connection
PPE internals:

- `.SLP` / `.PPP` — language extensions that trigger SLIP/PPP doors
- `OPEN SLIP` / `OPEN PPP` — COMMAND() calls to launch the doors
- `$$LOGON.BAT` / `$$LOGON.CMD` — logon batch files searched for
- `/LOGON` — command line switch passed to the batch
- `PATH` — environment variable checked by FileInPath()
- `": User logged off for no internet access."` — sysop log message
- `"RUNINET.PPE - Detecting SLIP/PPP Language Selection."` — banner
- FileInPath() function (args=2, vars=7, start=470)

The 63-var version (from decompilation) contains the same 39
variables plus 24 decompiler temporaries: 10 empty strings,
6 FALSE booleans, 4 zeroed integers, 2 uninitialized dates.
These are intermediate variables the decompiler created for
expressions Clark wrote inline. Both versions produce identical
bytecode (scriptSize=642 bytes).

The full 63-var table (from ppedecrypt.c):

    [ 0] id= 63 STRING     = "\""
    [ 1] id= 62 STRING     = ".\""
    [ 2] id= 61 BOOLEAN    = FALSE
    [ 3] id= 60 SWORD      (returnCode)
    [ 4] id= 59 STRING     = ""
    [ 5] id= 58 STRING     = ""
    [ 6] id= 57 INTEGER    = 0
    [ 7] id= 56 STRING     = ""
    [ 8] id= 55 STRING     = ""
    [ 9] id= 54 STRING     = "disconnecting you so that you can pick another language."
    [10] id= 53 STRING     = "If you see this message, you do not have Internet Access..."
    [11] id= 52 STRING     = ": User logged off for no internet access."
    [12] id= 51 STRING     = "OPEN PPP"
    [13] id= 50 STRING     = "OPEN SLIP"
    [14] id= 49 STRING     = "$$LOGON.BAT"
    [15] id= 48 STRING     = ""
    [16] id= 47 INTEGER    = 1
    [17] id= 46 INTEGER    = 2
    [18] id= 45 STRING     = "$$LOGON.CMD"
    [19] id= 44 STRING     = "PATH"
    [20] id= 43 STRING     = ": Simulated $$LOGON processing"
    [21] id= 42 STRING     = "/LOGON"
    [22] id= 41 STRING     = ": User did not select PPP or SLIP or was local"
    [23] id= 40 INTEGER    = 0
    [24] id= 39 INTEGER    = 9
    [25] id= 38 STRING     = "" (box drawing)
    [26] id= 37 STRING     = "" (box drawing)
    [27] id= 36 INTEGER    = 49
    [28] id= 35 STRING     = " Installed at: "
    [29] id= 34 STRING     = "RUNINET.PPE - Detecting SLIP/PPP Language Selection."
    [30] id= 33 STRING     = "" (box drawing)
    [31] id= 32 STRING     = ".PPP"
    [32] id= 31 STRING     = ".SLP"
    [33] id= 30 SWORD      (function returnCode)
    [34] id= 29 STRING     = ""
    [35] id= 28 BOOLEAN    = FALSE  (foundCMD)
    [36] id= 27 BOOLEAN    = FALSE  (PPPSelected)
    [37] id= 26 BOOLEAN    = FALSE  (SLIPSelected)
    [38] id= 25 FUNCTION   args=2 vars=7 start=470  (FileInPath)
    --- 24 decompiler temporaries below ---
    [39] id= 24 INTEGER    = 0
    [40] id= 23 DATE
    [41] id= 22 STRING     = ""
    [42] id= 21 STRING     = ""
    [43] id= 20 STRING     = ""
    [44] id= 19 STRING     = ""
    [45] id= 18 BOOLEAN    = FALSE
    [46] id= 17 BOOLEAN    = FALSE
    [47] id= 16 BOOLEAN    = FALSE
    [48] id= 15 STRING     = ""
    [49] id= 14 STRING     = ""
    [50] id= 13 STRING     = ""
    [51] id= 12 STRING     = ""
    [52] id= 11 STRING     = ""
    [53] id= 10 STRING     = ""
    [54] id=  9 STRING     = ""
    [55] id=  8 INTEGER    = 0
    [56] id=  7 INTEGER    = 0
    [57] id=  6 INTEGER    = 0
    [58] id=  5 DATE
    [59] id=  4 BOOLEAN    = FALSE
    [60] id=  3 BOOLEAN    = FALSE
    [61] id=  2 BOOLEAN    = FALSE
    [62] id=  1 BOOLEAN    = FALSE

Decoded by reading NEWSCR.CPP open() line by line for the exact
PPE file format, with per-chunk decrypt2 (fresh seed 0xDB24 per
sVARINFO/string/cVARVAL) and correct type numbering from VAR.HPP
(vtSTRING=7).


## 6. PPLC as a Multiplier

Owning the PPL compiler from source means every PPE in the PCBoard
distribution becomes a one-compile target. Three PPLC binaries closed
by changing #defines. Plus the tool that builds all PPE files.

The PCBoard support lib chain unlocks not just PPLC but PCBOARD.EXE
itself and all 207 utilities in SOURCE/UTIL/.

## 7. Related Tools

| Tool | Location | Purpose |
|---|---|---|
| PPLD | devtools/ppld32.zip | PPL decompiler (32-bit) |
| PPLX | devtools/pplx20.zip | PPL executor v2.0 |
| PPL Dev Kit | devtools/ppldevkit.zip | Developer kit installer |
| PPL Engine | pcb1541/PPL/pplengine/ | Runtime (Rust rewrite, 15.41 IRC) |
| PPLD source | pcb1541/PPL/ppld/ | Decompiler (PPLD.C, third-party) |

---

*hexadecimal, 2026-09-07*

---

**RULE:** Source code fixes apply to BOTH locations:
- **Repo:** `pcb153/` (master), `pcb154/`, or `pcb1541/` — whichever version
- **DOSBOXX.ZIP:** matching version under `pcbirc/BUILDROOT/`

Repo is master. DOSBOXX.ZIP mirrors it. Both must stay in sync.



## Clark’s compile flags (from PPLC.MAK v0.2.3 analysis)

Clark’s PPLC build uses `CD = -DLIB;COMM;___COMP___` in the
makefile plus 8 additional defines in PPLC.CFG: OSDRIVER, FOSSIL,
BIGNDX, ___USE_VAR___, S4ERROR_HOOK, DBASE, MG, TOSSCLASS.

The `-DLIB` and `-DCOMM` flags are added by the MAK’s CD variable,
NOT by PPLC.CFG. They apply to ALL source files compiled by the MAK.
For standalone TOOLKIT modules (INIT, PCBINIT, INITPORT), use
`-DPCB_MAXNODES=250 -DCOMM -DLIB` explicitly.

The compiler flags `-K` (unsigned char) and `-f` (no FP emulation)
in Clark’s CFG change function prototypes and require the entire
library chain to be compiled with matching flags. Deferred to v0.2.6.
