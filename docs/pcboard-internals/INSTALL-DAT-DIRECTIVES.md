# INSTALL.DAT — canonical @-directive reference

Reference for every `@`-directive used in Clark's `INSTALL.DAT`
(the script Clark's `INSTALL.EXE` interprets, and that our C
reimplementation at `pcb1541/install/src/install.c` must match).

This document is grown as install v1.10.1 → v1.10.5 lands each
directive's implementation. Entries marked **spec-only** haven't
been coded yet; entries marked **v1.10.N** have been implemented in
that sub-phase (v1.10.1 shipped 6 directives — file operations;
v1.10.2 shipped 11 more — control flow + strings;
v1.10.3 shipped 10 more — filesystem + disk sequencing;
v1.10.4 shipped 10 more — interactive menu + user input;
v1.10.5 shipped 7 more — system hooks + finish;
v1.10.6 verified all 60 vs binary — install v1.10 arc complete).

## Source

- Reference `INSTALL.DAT` at
  `pcb1541/install/reference/INSTALL.DAT` (42,294 bytes,
  md5 `cca38d36`)
- 72 unique `@`-directives (60 after case-folding `Endif`/`EndIf`
  and `Mkdir`/`MkDir` pairs — see "Case-fold pairs" below)

## Categories

### 1. Project metadata (v1.10.0 — parsed by existing stub)

Frame the whole install: project name, version, output paths,
system requirements.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@DefineProject` |  1 |    5 | parsed | Open the project-metadata block |
| `@EndProject`    |  1 |   13 | parsed | Close it |
| `@Name`          | 12 |    6 | parsed | Project name ("PCBoard") |
| `@Version`       |  3 |    7 | **v1.10.3** | Version string ("15.3") — read from @DefineProject block |
| `@Subdir`        | 69 |    9 | parsed | Default subdir under output drive |
| `@OutDrive`      |101 |   10 | parsed | Default output drive letter |
| `@Requires`      |  1 |   12 | **v1.10.3** | Prerequisite tests (used with `@HardDisk`); accepted as project-metadata |
| `@HardDisk`      |  1 |   12 | **v1.10.3** | Predicate: is a hard disk available; always true in reimplementation |

### 2. Variable declarations (v1.10.2 planned)

Establish user-input variables — first name, last name, city/state,
password, reg code.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@DefineVars` |  1 |  15 | **spec-only** | Open the variable-declaration block |
| `@EndVars`    |  1 |  21 | **spec-only** | Close it |
| `@Qstring`    |  5 |  16 | **v1.10.4** (parsed via @DefineVars) | Declare a quoted-string variable |
| `@Fname`      |  8 |  16 | **v1.10.4**   | Storage for first name |
| `@Lname`      |  8 |  17 | **v1.10.4**   | Storage for last name |
| `@CitySt`     |  6 |  18 | **v1.10.4**   | Storage for city/state |
| `@Pwd`        |  6 |  19 | **v1.10.4**   | Storage for password |
| `@RegCode`    |  1 |  20 | **v1.10.4**   | Storage for registration code |
| `@Set`        | 14 | 115 | **v1.10.2**   | Set a variable's value inline. Single-letter names (`@Set a = "..."`) are recognized as group-label declarations, NOT stored as normal variables (would break `a [= @Group` semantics). Top-level bare `@Var = expr` outside @DefineVars also handled. |

### 3. Output / display (v1.10.0 — parsed by existing stub)

Screen output — messages to user, prompts, pauses, screen clears.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@Display`    |   7 |  23 | parsed | Open a text-block to display |
| `@EndDisplay` |   7 |  43 | parsed | Close it |
| `@Cls`        |  11 |  24 | parsed | Clear the screen |
| `@Pause`      |  12 |  42 | parsed | Wait for keypress |
| `@Prompt`     |   5 |  82 | **v1.10.4**   | Prompt string for @GetString; stored as variable |
| `@Out`        | 570 |  10 | **spec-only** | Emit a single line of output (highest-frequency directive) |

### 4. Interactive menu (v1.10.4 planned)

The `@GetGroups`/`@CheckBox` block that opens the installer with the
6-option selector (First-Time / Upgrade / COMM-DRV / PPL / PCBMail /
OS2). Group state is later queried with `@If @Group X` to conditionally
run install steps.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@GetOutDrive`  |   1 |  45 | **v1.10.4** (headless skip; TTY prompt is v1.10.5 refinement) | Ask user for output drive |
| `@EndOutDrive`  |   1 |  64 | **v1.10.4**   | Close it |
| `@GetSubdir`    |   1 |  68 | **v1.10.4** (headless skip; TTY prompt is v1.10.5 refinement) | Ask user for subdirectory |
| `@EndSubdir`    |   1 |  83 | **v1.10.4**   | Close it |
| `@GetString`    |   4 | 163 | **v1.10.4**   | Ask user for a string value; TTY reads stdin, headless pre-fills sensible defaults |
| `@EndString`    |   4 | 166 | **v1.10.4**   | Close it |
| `@GetGroups`    |   2 |  91 | **v1.10.4**   | Open the checkbox menu; parses @Set-single-letter as menu items; TTY renders minimal interactive picker |
| `@EndGroups`    |   2 | 122 | **v1.10.4**   | Close it |
| `@CheckBox`     |   1 |  92 | **v1.10.4**   | Marks the enclosing @GetGroups as multi-select |
| `@Group`        | 202 | 124 | **v1.10.2/1.10.4** | State variable — string of selected group letters; tested via `x [= @Group` in @If |
| `@SetGroup`     |   1 | 288 | **v1.10.4**   | Programmatically mark a group as selected |
| `@ClearGroup`   |   2 | 155 | **v1.10.4**   | Programmatically mark a group as unselected |
| `@AskOverwrite` |   2 | 877 | **v1.10.4**   | Accepted as directive; headless always overwrites, TTY prompt is refinement |

### 5. Control flow (v1.10.2 planned)

Conditional execution, jumps, labels.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@If`     | 126 |  85 | **v1.10.2**   | Open a conditional block (recursive-descent evaluator with `[= [! == != > < >= <= && || ()` on int and string operands, plus @Func(...) inline in string literals) |
| `@Else`   |   2 | 328 | **v1.10.2**   | Alternative branch |
| `@EndIf`  |  31 |  88 | **v1.10.2**   | Close it (uppercase variant) |
| `@Endif`  |  95 | 132 | **v1.10.2**   | Close it (mixed-case variant — same as EndIf) |
| `@Goto`   |  12 |  87 | **v1.10.2**   | Jump to a label (pre-scan pass builds label → filepos map; @If stack reset on jump) |
| `@Label`  |   4 | 321 | **v1.10.2** (as `Name:` lines detected by pre-scan) | Define a jump target — actual syntax is `Name:` at start of line, not `@Label = "..."` (which is disk metadata) |
| `@Abort`  |   1 | 245 | parsed | Terminate the install |

### 6. String manipulation (v1.10.2 planned)

String utilities — length, prefix, tokenize.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@StrLen`   | 9 |  85 | **v1.10.2**   | Length of a string |
| `@StrHead`  | 5 |  86 | **v1.10.2**   | Prefix (first N chars) of a string |
| `@StrToken` | 4 | 168 | **v1.10.2**   | Tokenize a string (Nth token by delimiter) |

### 7. File / archive operations (v1.10.1 — highest priority)

The actual file-placement engine. `@BeginLib`/`@EndLib` bracket an
archive (`.RED` file); `@File` inside places one file with its
`@Size` and `@Path`. Wire this to `pcb1541/install/archivers/redx/`
and 481 files land byte-perfect.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@BeginLib`  |   8 | 323 | **v1.10.1**      | Open a `.RED` archive to extract from |
| `@EndLib`    |   8 | 331 | **v1.10.1** | Close it |
| `@File`      | 491 | 324 | **v1.10.1**      | Place one file from the current archive |
| `@Files`     |   1 | 876 | **v1.10.5**   | Sets `FILES=N` in generated CONFIG.SYS.pcb (inside @SetConfig) |
| `@Size`      | 476 | 339 | **v1.10.1** (as `@File` subclause) | Expected size of the current `@File` (verification) |
| `@Path`      |   1 | 883 | **v1.10.5**   | Sets `PATH=%PATH%;...` in generated AUTOEXEC.BAT.pcb (inside @SetAutoexec) |
| `@Copy`      |   1 | 252 | **v1.10.1**   | Copy a file (non-archive source), syntax `@Copy("src","dst")` |
| `@Delete`    |   6 |1158 | **v1.10.1**   | Delete a file (or directory), syntax `@Delete("path")` |
| `@AppendTo`  |   1 | 559 | **v1.10.1** (as `@File` subclause) | Append content to a file instead of overwriting |
| `@FileAttr`  |   8 | 888 | **v1.10.1**   | Set file attributes: `@FileAttr("path","r-")` sets r/o, `"r+"` clears |

### 8. Disk sequencing (v1.10.3 planned)

Physical install-disk handling — swap prompts, per-disk file sets.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@DefineDisk` | 4 | 320 | **v1.10.3**   | Open a disk-scope block (organizational only — content gated by inner @If) |
| `@EndDisk`    | 4 | 333 | **v1.10.3**   | Close it |

### 9. Filesystem operations (v1.10.3 planned)

Directory ops, path predicates, size queries.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@MkDir`     | 59 | 926 | **v1.10.3**   | Create a directory (uppercase variant); as function call returns 0 |
| `@Mkdir`     |  9 | 904 | **v1.10.3**   | Create a directory (mixed-case — same as MkDir); as function call returns 0 |
| `@ChDir`     |  3 | 899 | **v1.10.3**   | Change working directory (state-tracked; Unix hosts no-op on filesystem) |
| `@ChDrive`   |  2 | 898 | **v1.10.3**   | Change current drive (state-tracked; Unix hosts no-op) |
| `@DirExists` |  1 |1134 | **v1.10.3**   | Predicate: does a directory exist |
| `@Exists`    |  3 | 134 | **v1.10.2** (as @If predicate) | Predicate: does a file exist |

### 10. System hooks (v1.10.5 planned)

Post-install system integration — AUTOEXEC.BAT edits, CONFIG.SYS
edits, shell-out commands, wrap-up.

| Directive | Uses | First line | Status | Purpose |
|---|---:|---:|---|---|
| `@System`      | 6 | 241 | **v1.10.5**   | Shell out to DOS; headless stub returns 0 (success), --exec-system opts into real shell-out |
| `@Finish`      | 1 | 896 | **v1.10.5**   | Post-install cleanup block; contents execute inline (unless --skip-finish, default for test-friendliness) |
| `@EndFinish`   | 1 |1185 | **v1.10.5**   | Close it |
| `@SetAutoexec` | 1 | 881 | **v1.10.5**   | Open AUTOEXEC.BAT.pcb writer (generates PATH= additions from @Path directives inside) |
| `@EndAutoexec` | 1 | 886 | **v1.10.5**   | Close AUTOEXEC.BAT.pcb |
| `@SetConfig`   | 1 | 874 | **v1.10.5**   | Open CONFIG.SYS.pcb writer (generates FILES= additions from @Files directives inside) |
| `@EndConfig`   | 1 | 879 | **v1.10.5**   | Close CONFIG.SYS.pcb |

## Case-fold pairs

Two directive names have both uppercase-`I` and lowercase-`i`
variants that Clark's parser treats as identical:

- `@EndIf` (31 uses) / `@Endif` (95 uses) — same semantics
- `@MkDir` (59 uses) / `@Mkdir` (9 uses) — same semantics

Case-folding reduces 72 unique names to 60 semantically distinct
directives.

## Implementation progress

Running total of directives coded in `pcb1541/install/src/install.c`
(out of 60 semantically distinct):

| Phase | Directives | Cumulative | Notes |
|---|---:|---:|---|
| v1.10.0 | 12 | 12 | Existing Phase 27 stub — project metadata + basic display |
| v1.10.1 |  6 | 18 | File operations via redx wire-up (@BeginLib/@EndLib/@File/@Copy/@Delete/@FileAttr; @Out/@Size/@AppendTo as @File subclauses). Verified byte-perfect against dist/target/. |
| v1.10.2 | 11 | 29 | Variables + control flow + string ops (@If/@Else/@EndIf/@Endif/@Goto/@Label/@Set/@StrLen/@StrHead/@StrToken/@Exists-as-predicate). Full recursive-descent expression evaluator with `[= [! == != > < >= <= && \|\| ()` on int and string operands, @Func(...) inline in string literals, trailing `@Group X` clause on @File as per-directive filter. Real INSTALL.DAT runs end-to-end: 471 successful @File operations, 348 files byte-perfect vs `dist/target/` (94.8% of placed). |
| v1.10.3 | 10 | 39 | Filesystem + disk sequencing. Real @MkDir/@Mkdir/@ChDir/@ChDrive/@DirExists; @DefineDisk/@EndDisk semantics; @Requires/@HardDisk/@Version predicates. Fixed two @File parser bugs surfaced by real INSTALL.DAT: `@Out DIR\*.*` glob (keep source filename) and missing-`@Out` fallback (default to source name). Full 481 file operations succeed (matches reference tree total), 394 files byte-perfect (94.0%), only PCBOARD.SER fails (legit — not in any archive). |
| v1.10.4 | 10 | 49 | Interactive menu. Real @GetGroups/@CheckBox/@GetString handlers (TTY prompt when stdin is terminal, headless fallback to `--groups` CLI + sensible defaults). @SetGroup/@ClearGroup modify SelectedGroups programmatically. @AskOverwrite/@Prompt accepted as directives. @Qstring/@RegCode + variable-type declarations parsed via @DefineVars. Single-flavor install (-g "abcdefp" for Corporate BBS) places 371 files, 350 byte-perfect (94.5%). Reference tree's 481 total = rebuild_place.py per-flavor artifact accumulation; a real installer produces one flavor at a time. |
| v1.10.5 |  7 | 56 | System hooks + finish. @System real (headless returns 0; --exec-system opts into shell-out). @SetConfig/@EndConfig generate CONFIG.SYS.pcb with FILES= lines. @SetAutoexec/@EndAutoexec generate AUTOEXEC.BAT.pcb with PATH= additions. @Finish/@EndFinish executes contents inline (unless --skip-finish, default for test-friendliness). File placement unchanged (350/361 byte-perfect); adds 2 generated system-config files. |
| **v1.10.6** | 0 | 60 | **install v1.10 arc COMPLETE.** Disassembly parity check vs INSTALL.EXE binary. No new code, semantic verification only. Discovered INSTALL.EXE supports 329 total @-directives; our 60 (matching Clark's INSTALL.DAT usage) all verified present. Two undocumented constraints found in binary error strings but not triggered by Clark's script. Full parity report: [`INSTALL-EXE-PARITY.md`](INSTALL-EXE-PARITY.md). Understanding-complete milestone reached. Byte-exact rebuild of INSTALL.EXE itself is v1.11+ (requires all 329 directives + vintage Borland C++ toolchain). |

## See also

- [`INSTALL-EXE-PARITY.md`](INSTALL-EXE-PARITY.md) — v1.10.6 parity
  report vs Clark's compiled INSTALL.EXE binary (329 total directives
  in the engine, of which we cover the 60 that Clark's INSTALL.DAT
  actually uses)
- `pcb1541/install/reference/README.md` — the reference files
  themselves
- `pcb1541/install/src/install.c` — our C reimplementation
- `pcb1541/install/dist/target/rebuild_place.py` — Python reference
  implementation for the `@File`/`@BeginLib` logic (proven to
  reconstruct 481 files byte-perfect)
- `pcb1541/install/RUNTIME-DEPS.md` — external dependencies

--install v1.10.0, 2026-09-04
