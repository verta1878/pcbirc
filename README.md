# Patches

## 15.4-pwa.patch (canonical)

Transforms clean 15.3 PWA into 15.4 PWA (Clark's) — **source AND
toolchain together**.

The patch carries both:
- The 15.4 source changes (PSA fields, PPL 3.40 functions, file
  flagging, UUIN reject-by-name, FTP MGET, etc.)
- The 15.4 toolchain change (the single SPACERIGHTAT enum value in the
  toolkit's PCBTOOLS.H, needed for the @x color-code feature)

This means: apply the patch to a clean 15.3 tree (source + toolkit) and
you get a complete, buildable 15.4 PWA. The toolchain rides along
because you can't build 15.4 without it.

- 424 files (source + toolkit)
- Applies 100% cleanly
- Both endpoints build with Borland C++ 3.1

## 386max-tasm/ (Gate 1 complete)

An in-source equivalent of pcbirc's runtime transforms for building
sudleyplace/386MAX 8.03 under TASM 3.1 + TLINK. See
`todo/386max-build-downgrade.md` for context.

**Default pcbirc path**: source stays byte-identical on disk;
`MAIN/build/scripts/xform.awk` and `xform.sed` transform at build
time. Anyone who clones sudleyplace/386MAX still gets Bob's
original source verbatim.

**Alternative for sysops who prefer in-source patching**: this
patch, when applied to sudleyplace source, captures the same
human-readable transforms — TASM+TLINK-buildable source — as a
static in-tree change. Useful when reviewing transforms as a
code artifact, when building outside DOSBox-X, or when gawk/sed
aren't available at build time.

### Files

| File | Purpose |
|---|---|
| `386max-tasm/386max-tasm.patch` | 1.2 MB, 144 files changed, 35,543 lines. Captures 12 sed rules + 6 per-file surgeries. |
| `386max-tasm/README.md` | How to apply + build-time awk caveats. |

### What the patch covers

12 sed rules (from `MAIN/build/scripts/xform.sed`):

1. `.xcref sym,sym,...` lines commented out (TASM misparses the argument list)
2. `@Version` renamed to `?VERSION` (TASM's native reserved-symbol prefix)
3. Absolute-address group definitions dropped (`RGROUP`, `AGROUP`, `PSPGRP`, `CGROUP`)
4. Group references renamed to their sole underlying segment (`ROMSEG`, `ALLMEM`, `PSPSEG`, `CPUID_SEG`)
5. `loop dword ptr X` → `loopd X` (TASM's mnemonic)
6. `loop LABEL.EDD` → `loop LABEL` (strip decorative type hint)
7. `0&&@SYM&&h` → `0&@SYM&h` (MASM 6 double-subst to TASM single)
8. `bt`/`bts`/`btr`/`btc` `byte ptr` → `word ptr` (bt-family needs min word)
9a. bt-family with 32-bit register second operand → `dword ptr`
9b. bt-family with segment override + `[reg]` → `word ptr`
10. `COMMENT<delim>` → `COMMENT <delim>` (TASM needs whitespace)
11. `mov al, es:[bx].OPROG_PCT` in `UTIL_OPD.ASM` → forced `byte ptr` read
12. `push DTE_DPMILDT` → `push word ptr DTE_DPMILDT` (16-bit selector push)

6 per-file surgeries (from `MAXBLD1.BAT` P5):

- P4: prepend `?VERSION equ 510` to `INC/MASM.INC`
- P5a: skip `QMAX_FLX.ASM` (include-only, no `end` directive)
- P5b: prepend `LOADHI equ 1` to `HILO.ASM` (BCF variant defaults to high-load)
- P5c: comment out `UTIL_LOD.ASM` TOPDOS proc/endp block (conflicts with QMAX_I21)
- P5d: add `extrn TOPDOS:near` to `UTIL_LOD.ASM` (used but commented out above)
- P5e: skip `QMAX_DIF.ASM` (100+ catstr/textmacro incompatibilities with TASM)

### What the patch does NOT cover

The macro-scoped `@@:`/`@F`/`@B` label relabeling (from `xform.awk`)
and the bt-family symbol-size annotation (from `xform-bt.awk`) happen
at build time, not in the patch. Those transforms would explode the
diff to unreviewable size with mechanical LBL_N injections that carry
zero review value. If you apply just this patch, you still need to run
the awk passes to get assemblable output — or use the full pcbirc
pipeline (`MAXBLD1.BAT`).

### Apply

```
7z x devtools/386max.7z -o/tmp/386max-src
cd /tmp/386max-src
# KERNEL/ in the patch = 386max-src/386MAX/
# INC/    in the patch = 386max-src/INC/
patch -p1 < path/to/386max-tasm.patch
```

### Build status (Gate 1 complete)

- Gate 1 (all assemblies succeed): ✓ 92/92, 0 fail
- Gate 1 SYS size: 235,224 bytes (102.60% of shipped Qualitas 229,268)
- Gate 1.5 (byte-verified): partial — layout differs due to TASM 3.1
  vs MASM 5.10 encoding, our bt-family word-ptr widening, alignment
- Gate 2 (loads under `DEVICE=`): pending — needs 86Box/PCem/QEMU with
  real DOS/FreeDOS, or DOSBox-X 2026.06+ with `[devices]` section

### What Clark's 15.4 changed

7 feature areas. Only ONE (@x color codes) touched the toolkit — it
added SPACERIGHTAT to the padtype enum. The other 6 are pure source
changes. So the toolkit barely moved; the patch is mostly source plus
that one enum line.

### Apply

The patch expects a combined tree layout (SOURCE/ + toolkit/):

```
# from a clean 15.3 tree containing SOURCE/ and toolkit/
patch -p1 < 15.4-pwa.patch
```

Labeled "15.4 PWA" — Clark's authentic 15.4, NOT the crew's 15.4 Delta
work (pcb154/, ongoing).

## attic/

Superseded patches from earlier phases. See attic/README.md.
