# FOSSIL.VXD — Program Recovery Report

**Component:** WinFOSSIL ring-0 driver (Windows 95/98/ME VxD)
**Status:** Recovered, assembles clean, links to valid LE VxD, output
verified functionally equivalent to a genuine MASM 6.11d build.
**Not yet done:** load/run validation on real Win98 hardware.
**Owner:** wrench (FOSSIL) · recovery track: dotmatrix

---

## 1. The problem

`FOSSIL.ASM` (5,714 lines, the complete VxD source) had never assembled.
The cause was not the source — it was the bundled `VMM.INC`, one of the
DDK includes it depends on.

### Root cause: corrupted VMM.INC
The `VMM.INC` in the repo was a reconstruction with **character-level
corruption in its last ~290 lines** — the region that should hold
`BeginProc` / `EndProc` and the data/init segment generators. Instead
it held garbage tokens (`?NOfdf`, `EXITame`, `frc_EXIce`, `ifdf` for
`endif`). Every copy of `VMM.INC` in the tree shared the identical
damage, so it predated all current work. With those macros missing or
mangled, nothing that included `VMM.INC` could assemble.

---

## 2. Recovery of the genuine header

The authoritative fix is Microsoft's own `VMM.INC` from the Win95/98 DDK.

- WinWorld's **Win95 DDK** download is gated against non-interactive
  fetches (returns "invalid download ID" to scripted requests).
- The **Win98 DDK** is freely mirrored and ships the *same* VxD
  headers. Retrieved it, extracted `INCS_DDK.CAB`, and pulled the
  genuine, uncorrupted **VMM.INC (93,146 bytes)** plus VPICD, VCOMM,
  SHELL, VWIN32, REGDEF, VCOMMW32.
- The same DDK also provided **ML.EXE 6.11d** and the DDK **LINK.EXE**,
  used later for reference verification.

The pristine headers are in `src/ddk-genuine/` (unmodified).

---

## 3. Porting the genuine header to JWasm

The genuine Microsoft `VMM.INC` is written for **MASM 6.11**. Building
with JWasm (the maintained MASM-compatible assembler that runs on
Linux) required three dialect ports. Each was confirmed with a minimal
repro before applying:

1. **`&&` -> `&`.** MASM uses double-ampersand token pasting
   (`VxD_&&segname&&_CODE_SEG`) for a macro parameter that must survive
   two expansion levels. JWasm resolves ampersand nesting differently
   and needs a single `&`. (Microsoft's own file uses `&&`, so this is
   a genuine MASM-vs-JWasm difference, not corruption.)

2. **`&macro` / `&endm` -> `macro` / `endm`.** Three argument-marshaling
   macros (`?marg`, `?MKA`, `?LOC`) define a macro *inside* a macro with
   the `&`-prefixed idiom. JWasm rejects the prefixed form but accepts
   plain `macro` / `endm`.

3. **`OPTION NOKEYWORD:<SYSEXIT VMRESUME>`.** Two of the DDK's segment
   names — `SYSEXIT`, `VMRESUME` — are Pentium-era instruction
   mnemonics. JWasm reserves them as keywords and won't allow a segment
   named after one. MASM 6.11 predated those instructions and accepted
   them. `OPTION NOKEYWORD` un-reserves them. (This line is JWasm-only;
   the genuine-MASM build path must omit it — MASM 6.11 errors on it.)

The ported headers are in `src/` (used by `build/build_jwasm.sh`).

---

## 4. Source bugs found in FOSSIL.ASM

With the headers fixed, three real defects surfaced in the driver
source itself:

- **`NODECOUNT` undefined.** `FOSSIL.inc` had `MaxNodes equ NODECOUNT`
  but `NODECOUNT` was never defined and had no default. Added a guarded
  default of 16 (overridable with `-DNODECOUNT=<n>`).
- **Missing `IRQ_Ready` struct field.** The code sets
  `StatusStruct.IRQ_Ready` in two places, but the struct never declared
  it (every sibling flag was present). Added it next to `ICT_Ready`.
- **Out-of-range short jump.** A `jnz Short FIFO_Enabled` went 2 bytes
  out of range after the struct grew; removed the `Short` override so
  the assembler sizes it.

---

## 5. Build

Assembles clean: **0 warnings, 0 errors.** Reproducible — rebuilding
yields bit-identical code and data sections (only the COFF timestamp
and symbol-table ordering vary).

- `build/build_jwasm.sh` — JWasm + Open Watcom `wlink` (Linux). Produces
  `FOSSIL.vxd`, a valid LE-format VxD.
- `build/BUILD_masm.bat` — genuine MASM 6.11d + DDK `LINK.EXE` path,
  using `src/ddk-genuine/`.

---

## 6. Correctness verification (JWasm vs genuine MASM 6.11d)

The concern with any recovered/ported build: does the ported toolchain
produce *correct* output, or just output that assembles? Verified by
building the identical source with genuine **MASM 6.11d** (from the DDK)
and comparing:

- **`_LDATA`** (all initialized data — DDB, tables, config strings):
  **byte-for-byte identical.**
- **`_RCODE`** (real-mode init): **byte-for-byte identical.**
- **`_LTEXT`** (main code): **99.68% identical instruction stream.**
  Every difference is an equivalent encoding:
  * JWasm emits short-form jumps (2 bytes) where MASM emits near-form
    (6 bytes) — same target label, JWasm's is just more compact.
  * `cmp al,0` (JWasm) vs `or al,al` (MASM) — identical flag effects.
  (A handful of apparent diffs are objdump misreading in-code data
  tables, not real code differences.)

Both OBJs link to valid LE-format VxDs. Conclusion: **the JWasm build
is functionally equivalent to a genuine Microsoft MASM 6.11d build.**

`verified/` contains both the JWasm VxD and the MASM reference VxD, plus
both OBJs and `SHA256SUMS.txt`.

---

## 7. Remaining gate

A format-valid, correctly-assembled VxD still has to **load on real
Win98 and hook INT 14h**. That needs actual hardware or a faithful
Win98 VM and is the one step not yet done.

This does **not** block Win98 users: the shipping Win98 FOSSIL driver is
the unified ring-3 DLL (`-DWF_TARGET_WIN98`), which builds clean and
passes 50/50 unit + 12/12 integration tests. The VxD is the optional
ring-0 direct-hardware path.
