# PPL — PCBoard Programming Language

PPL 3.40 compiler and runtime engine.

- `pplengine/` — PPL executor (interprets compiled .PPE files)
- `ppld/` — PPL decompiler with test data
- `ppe/` — PPE file collection

Clark's PPL lets sysops extend PCBoard with custom scripts.
PPEs are compiled from .PPS source into .PPE bytecode.


## Note: pplengine/ is a third-party Rust project (reference only)

`pplengine/` is **PPLEngine**, a modern from-scratch reimplementation of
the PCBoard PPL toolchain written in **Rust** by an outside author
(vendored here for reference). It is a "just for fun" project and is not
part of our build path — **we are staying with C/C++** for the actual
15.41 work. It's kept because it's a genuinely useful reference:

- `icy_ppe/` — core PPL/PPE library (parser, AST, bytecode format)
- `icy_board_engine/` — the execution engine / VM (src/vm) that runs
  PPE bytecode; the runtime brain behind pplx
- `pplc/` — compiler (PPS -> CP437 PPE)
- `ppld/` — decompiler/disassembler (claims PPE 3.40 support; better
  than the 90s-era tools)
- `pplx/` — console runner
- `ppl-lsp/` — a language server (editor tooltips, highlighting)

Caveat from its own README: the rewritten decompiler's control-structure
reconstruction (if/while/for/select rebuilding) is currently broken
pending an AST rewrite; an older commit has the working version. It
supports 15.4 but with a different AST that needs the reconstruction
redone.

Value to us: it's a reference for what a modern PPL developer experience
looks like (LSP, clean decompiler, headless VM) even though our
implementation language is C/C++. See todo/SDK-1541-IMPROVEMENTS.md.
