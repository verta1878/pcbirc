# 1541 SDK Toolkit — Open-Source Toolchain Migration

Direction, not committed work. The goal: the **15.41 SDK toolkit should
build on tools we control and can ship freely** — our own compiler
(ow2irc) and open DOS-extender / DOS-host tooling (dosxxx) — instead of
depending on proprietary abandonware (MSC 7.0, 386MAX, Borland, Turbo C).

## Why

The 15.3 / 15.4 PWA toolkits are **preservation**: we reproduce Clark's
exact toolchain, proprietary bits and all, to build bit-for-bit what he
shipped. That work legitimately needs the old proprietary compilers
(Borland C++ 3.1, Turbo C 2.01, MSC 7.0) and, for MSC 7.0's DOS route, a
proprietary DPMI host (386MAX). Those are build-time tools we "find,"
not tools we own.

15.41 is **new work**. It shouldn't inherit that dependency chain. A
project we want people to clone, build, and extend should build with an
open, redistributable toolchain — no "go find a copy of 386MAX," no
licensing questions about shipping a 1992 Microsoft compiler.

## The target stack

- **Compiler: ow2irc** — our OpenWatcom 2 fork targeting IRC. Open
  source, we control it, produces the 16-bit code PCBoard needs. This
  replaces MSC 7.0 / Borland / Turbo C for 15.41.
- **DOS host / extender: dosxxx (open)** — the open DOS-extender and
  DOS-host tooling: DOSBox-X for emulation, and an open DPMI host
  (e.g. CWSDPMI) or the OpenWatcom DOS/4GW-class extender path where a
  32-bit host is needed. This replaces 386MAX (the proprietary DPMI
  host) for anything 15.41 builds.
- Assembler: **WASM** (Watcom's assembler), which now fully supports
  MASM syntax — so the standalone .ASM files assemble on the open stack
  too (see todo/OPENWATCOM_PORT_WORKMAP.md Phase 3). Replaces TASM.

## What moves, what stays

| Piece | Preservation (15.3 / 15.4 PWA) | 15.41 (new) |
|---|---|---|
| Compiler | Borland / Turbo C / MSC 7.0 | **ow2irc** |
| DPMI / host | 386MAX (MSC7 DOS route) | **open (dosxxx / CWSDPMI)** |
| Assembler | TASM | **WASM (MASM mode)** |
| Purpose | reproduce Clark exactly | free, cloneable, extensible |

The proprietary chain does **not** go away — it stays for the
preservation builds, which by definition need Clark's actual tools. The
migration is additive: 15.41 gains an open build path; the historical
versions keep theirs.

## Preconditions ("hopefully working by then")

This migration depends on two things maturing:

1. **ow2irc** compiling the toolkit correctly. r0.6.0 is producing
   verified x86-64 output with the codegen-correctness fixes landing
   (instruction-length decoder, frame/stack offsets, 8-byte pointers).
   The 16-bit target path the toolkit needs must be solid.
2. **The open DPMI / DOS-host path** being reliable enough to host the
   build and run the output — DOSBox-X plus an open DPMI host, no
   proprietary extender.

Until both are ready, 15.41 can lean on the proprietary chain as a
bridge. The migration happens **as ow2irc matures**, not as a big-bang
switch.

## Migration steps (rough)

1. Get the 15.41 toolkit compiling under ow2irc for one memory model
   (medium — what PCBoard uses), alongside the existing builds.
2. Assemble the standalone .ASM with WASM (MASM mode) instead of TASM.
3. Host the build + run the artifacts on the open DPMI / DOSBox-X path;
   confirm no proprietary extender is needed.
4. Bring up the remaining memory models.
5. Once green, make the open stack the **default** for 15.41; keep the
   proprietary path documented for anyone reproducing preservation
   builds.
6. Ship a "clone and build with open tools only" path for the 15.41 SDK
   (ties into todo/SDK-1541-IMPROVEMENTS.md — one package, one guide).

## Relationship to the SDK improvements

This is the toolchain half of the SDK story; todo/SDK-1541-IMPROVEMENTS.md
is the developer-experience half. Together: a 15.41 SDK that builds with
free tools (this doc) and is pleasant to develop against (that doc).
