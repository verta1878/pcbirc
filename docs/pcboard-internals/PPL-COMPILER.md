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
   but 2,131 bytes differ. Since encryption is deterministic, the
   plaintext must differ — the code generators are different.
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

**Root cause (updated 2026-09-07):**

We built PPLC from Clark’s source code. It compiles PPL programs
correctly — it works. But when we compile the same RUNINET.PPS
with our source-built PPLC and Clark’s shipped PPLC 3.20, the
output files are the same size (2,261 B) with the same variable
count (63), yet the actual bytes inside differ.

The encryption is deterministic — same input always produces the
same output. So the bytes differ because the compilers produce
different internal bytecode from the same source. The part of the
compiler that turns PPL statements into bytecode instructions
(SCRCOMP.CPP — the code generator) was modified between PPL
version 3.20 and 3.30. Our PPLC is built from the 3.30 version
of that code. We changed the version label to say "3.20" but the
code generation logic underneath is still 3.30.

**What byte-exact requires:**
  a. Clark’s ORIGINAL PPS source (39 vars, not decompiled 63)
  b. PPLC built from ACTUAL 3.20 source code (not 3.30 with version
     change) — the code generator changed between versions

**Path forward: Diff pcbsrcv/000 through 014**

The PWA zip has 15 versioned snapshots (PCBSRCV/000 through 014).
Diff SCRCOMP.CPP across all 15 to find:
- Which snapshot changed CUR_PPE_VER from 320 to 330
- What code changed in the code generator at that point
- The last snapshot that still has 3.20-era code generation

That last snapshot is the one we build PPLC from.

Files to diff: SCRCOMP.CPP, NEWSCR.CPP, VAR.CPP, SCRMISC.CPP,
LIB/SOURCE/MISC/CRYPT.C.

After the diff:
1. Build PPLC from the 3.20-era source using our lib chain
2. Compile RUNINET.PPS with the rebuilt PPLC
3. Compile RUNINET.PPS with Clark’s shipped PPLC320.EXE
4. Compare the two PPE outputs — if byte-exact, our source-built
   3.20 code generator matches Clark’s binary
5. Find Clark’s original RUNINET.PPS (39 vars) in reference archives
6. Compile the original PPS with our source-built 3.20 PPLC
7. `cmp -s` against the target bin/RUNINET.PPE
8. If it passes → v1.0.1 DONE

Step 4 is the validation gate. If it fails, the diff missed
something — go back and look for more changes.

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
