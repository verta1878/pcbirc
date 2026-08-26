# 1541 SDK — Improvement Ideas

Design notes for making the PCBoard 15.41 developer kit genuinely
pleasant to build against — for both PPL (sysop scripting) and native
toolkit (C/C++ add-on) developers. These are ideas and directions, not
committed work; they're here to shape the SDK as 1541 matures.

Our SDK has two faces:
- **PPL** — the scripting layer (PPS → PPE bytecode) sysops use to
  extend the board. Engine, compiler (pplc), decompiler (ppld), and an
  LSP already exist under `pcb1541/PPL/`.
- **Toolkit** — the native C/C++ library (PCBTOOLS.H ~295 exports,
  MISC.H ~122, SCREEN.H ~85, DOSFUNC.H ~70, ...) that add-ons and the
  main program link against.

The goal below: keep 100% compatibility with Clark's SDK (a 15.x PPE and
a toolkit add-on must still build and run), while adding a modern
developer experience around it.

## How an SDK toolkit is supposed to work (the model we aim for)

A good BBS SDK toolkit, stripped to essentials, does this:

- **The board owns its core data; add-ons ask the board, they don't
  poke the files.** The SDK is the sanctioned door to user records,
  message bases, file directories, and config. This is what makes
  multi-access safe and keeps third-party code from corrupting state.
- **A clear session model with access levels.** An add-on connects,
  identifies, logs in, and then can call exactly what its access level
  allows (ordinary user vs trusted system tool vs configuration tool).
  Nothing privileged is assumed from the caller; the board decides.
- **A well-defined, versioned API surface.** Every exported function has
  a documented contract (params, return, errors, which access level it
  needs, which version introduced it). Data structures that cross the
  boundary are explicitly public ABI; everything else is internal.
- **Consistent conventions.** Uniform return/error handling, uniform
  naming, get-vs-search variants for record lookups, const-correct
  record access. A developer learns the pattern once and it holds
  everywhere.
- **Examples that build.** A small set of working sample add-ons
  (a door, a monitor, a front-end, a chat/echo demo) that compile and
  run, kept alongside the SDK so the API is proven end to end.

Our toolkit already has the raw surface (PCBTOOLS.H, MISC.H, SCREEN.H,
the PPL layer). The improvements below are about wrapping that surface in
this model: session clarity, documented contracts, versioning, and
buildable examples.

---

## A. PPL developer experience

1. **Lean into the LSP we already have.** `pcb1541/PPL/pplengine/ppl-lsp`
   is a real asset. Flesh it out to full coverage: hover docs for every
   PPL function/statement, signature help, go-to-definition across
   includes, and live diagnostics from pplc. A sysop writing a door in
   any LSP-aware editor is a huge leap over the old edit/compile/guess
   loop.

2. **A single, versioned function/statement reference — generated, not
   hand-maintained.** `ppl-lsp/data/FUNCS` is a start. Make one machine-
   readable table (name, params, types, return, since-version, security
   notes) that feeds BOTH the LSP and the human docs, so they never drift
   apart. Tag each entry with the PPL version it appeared in (3.20/3.30/
   3.40/…) so cross-version compatibility is visible at a glance.

3. **Deterministic, reproducible PPE builds.** Guarantee that the same
   PPS + same pplc version → byte-identical PPE. This is what makes the
   IC-style byte-exact reconstruction tractable, and it's a real
   developer trust feature (diff two builds, get nothing).

4. **A proper error model.** Line/column, an error code, a one-line
   human message, and a pointer to the reference entry. Consistent codes
   let editors and CI act on them.

5. **A test harness for PPEs.** A way to run a PPE headless against a
   scripted session (fake user, fake inputs, captured output) and assert
   on results. Turns door development from "dial in and poke it" into
   something CI can gate.

## B. Native toolkit (C/C++ add-on) experience

6. **One umbrella header + clear sub-headers.** A dev should `#include`
   one SDK header and get a clean, documented surface, with the
   category headers (SCREEN/SCRNIO/DOSFUNC/ACCOUNT/…) available
   individually. Document the *contract* of each exported function, not
   just its signature.

7. **A stable, documented ABI boundary per memory model.** We already
   ship the matrix (3 compilers × 4 models). Publish exactly which model
   an add-on must use, which structs are part of the public ABI, and
   which are internal — so an add-on built once keeps working across
   point releases.

8. **Header/lib version stamping.** A version macro in the SDK headers
   and a matching stamp in the libs, so a build can assert it linked the
   SDK it expected. Cheap, prevents a whole class of "linked the wrong
   toolkit" bugs.

9. **Sample add-ons that actually build.** A tiny "hello board" native
   add-on and a tiny PPE, each with its build script, kept in CI so the
   SDK is proven to work end-to-end on every change. Examples that
   compile are the best documentation.

## C. Shared / structural

10. **Data-driven configuration over hardcoded slots.** Where the engine
    has fixed-count tables (archivers are the obvious one — 4 slots),
    move toward a small data file the SDK reads. Extends cleanly, and
    third parties can add entries without a rebuild. (Tracked already for
    archivers in PCB1541_DRAFT §8.)

11. **A clean session/IO abstraction the SDK exposes.** As pcbcomm is
    remade, expose one backend-agnostic session handle to add-ons (UART /
    FOSSIL / telnet / SSH all look the same to the developer). New
    transports become backends, not SDK breaks.

12. **One SDK package + one guide.** A single downloadable dev kit
    (headers, libs for each model, pplc/ppld, the reference, the
    samples) plus a "write your first door / first native add-on in 10
    minutes" guide. Discoverability is most of adoption.

13. **Optional modern-host tooling that emits period-correct output.**
    The Rust PPL engine (`icy_board_engine`) can host editor/CI tooling
    on a modern machine while still producing bytecode the real DOS/OS/2
    board runs. Modern DX, retro artifact — the same philosophy as the
    whole 1541 effort.

---

## Priority read

If we do only a few: **1 (LSP), 2 (generated reference), 5 (PPE test
harness)** transform the PPL side; **6 (umbrella header + contracts),
8 (version stamping), 9 (buildable samples)** transform the native side.
Everything else is refinement on top.

## Build output convention — example binaries

The SDK's buildable example add-ons (item 9 / B) must emit their
compiled binaries to a **`bins/` folder under the version's OUT tree**:

    OUT/pwa153/bins/         (15.3 SDK example binaries)
    OUT/pwa153/upd154/bins/  (15.4 PWA)
    OUT/delta154/bins/       (15.4 Delta)
    OUT/irc1541/bins/        (15.41 IRC)

This keeps SDK example outputs beside the version's other build outputs,
separate from the toolkit libraries in OUT/lib/. Each SDK build script
writes its sample .EXE (and any .PPE examples) there, so "did the SDK
still build end to end" is answered by looking in one place per version.

The same applies once MSC7 (PCBKMS) lands: its example builds go to the
same per-version bins/ folder, just compiled with Microsoft C 7.0.

---

## Toolchain direction

The 15.41 SDK should build on an **open toolchain** — ow2irc (our
compiler) + open DOS-host/extender (dosxxx) + WASM — instead of the
proprietary compilers the preservation builds need. Full plan in
todo/SDK-1541-OPENSOURCE-MIGRATION.md.

## Guardrails

- **Compatibility first.** Nothing here may break an existing 15.x PPE or
  a toolkit add-on. New surface is additive; the old surface stays.
- **Preservation first, then improvement.** The toolkit is recreated
  bit-for-bit (bugs intact) before any of this. These are the deliberate
  improvements that come *after* the match is exact — same rule as
  todo/toolkit.md.
