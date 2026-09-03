# PPL Samples — Clark Development Corp

**20 sample PPL scripts** shipped with PCBoard 15.41 as pedagogical
examples of the PPL 3.0 language. Extracted byte-perfect from
`PPLC.RED` during install v1.6.0 and living at
`pcb1541/install/dist/target/PPL/` as the installer parity target.

Also copied here (`toolkit/pplc/samples/`) so PPL toolchain work
(compiler, decompiler, debugger under `toolkit/pplc/`) has them
readily at hand without cross-tree access.

Each sample is a pair: `NAME.PPS` (source) + `NAME.PPE` (compiled by
Clark's original PPLC 3.0). Two `.BAT` helpers (`RUN1.BAT`, `RUN2.BAT`)
show how to invoke them from the PCBoard command line.

## The 20 samples

### Hello-World tutorials (progressive complexity)

| Sample | What it teaches |
|--------|-----------------|
| **HELLO1.PPS** | `PRINTLN` — the simplest program |
| **HELLO2.PPS** | String variables, concatenation |
| **HELLO3.PPS** | User input via `INPUT` |
| **HELLO4.PPS** | Conditionals (`IF`/`ELSE`) |
| **HELLO5.PPS** | Loops (`FOR`/`WHILE`) |
| **HELLO6.PPS** | Functions/procedures |
| **HELLO7.PPS** | File I/O |

Written by Clark Dev as the introductory PPL tutorial series. Read in
order for a working ramp-up of the language.

### Sysop utilities

| Sample | Purpose |
|--------|---------|
| **OPPAGE.PPS**   | O-command (operator page) replacement with time-of-day gating and multi-attempt paging |
| **START.PPS**    | "New command" that gates BETA testing access by security level |
| **PWRDWARN.PPS** | Warn users about impending password expiration |
| **LANGUAGE.PPS** | Prompt for desired language with 20-second timeout |
| **MORE.PPS**     | MORE? prompt replacement that accepts either standard response OR a complete follow-up command |
| **NODEFILE.PPS** | Display-file dispatch by security level, graphics mode, node number |
| **DOORS.PPS**    | Hotkey-driven door launcher menu |
| **ORDER.PPS**    | Questionnaire-style product order script (template for user surveys) |
| **WELFIRST.PPS** | ANSI-animated welcome for first-time callers (skippable on repeat visits) |

### DBase / accounting integration

| Sample | Purpose |
|--------|---------|
| **DBASE.PPS**    | Sample DBase application — create, open, add-to, and search a DBase file via PPL's DBase primitives |
| **ACCNTDBF.PPS** | Access the Accounting DBF tracking file (usage/time/cost stats) |

Both explicitly labeled "PPL 3.0 Sample" with a 1994 Clark
Development, Inc. copyright header.

### Recreational / demo

| Sample | What it does |
|--------|--------------|
| **KAL.PPS**       | Colorful animated kaleidoscope (Scott Dale Robison) |
| **HAMURABI.PPS**  | Text-mode simulation port of the classic BASIC game — buy/sell land, feed population, manage grain (Scott Dale Robison, port of People's Computer Company BASIC) |

## Authorship

Most sysop utilities and the recreational demos credit **Scott Dale
Robison** — Clark Development's in-house PPL evangelist and author of
the PPL 3.0 language reference.

Business / accounting samples (ACCNTDBF, DBASE) are Clark Development
Corp copyright without individual attribution.

## Compilation

The `.PPE` files here are Clark's original PPLC 3.0 output — the
byte-perfect binaries that shipped on disk. When our `toolkit/pplc/`
compiler is complete, round-trip parity (recompile `.PPS` → compare
`.PPE` byte-for-byte against these) is the acceptance test.

Current PPLC status: scaffolding under `toolkit/pplc/3.00/`,
`3.10/`, `3.20/` (one dir per PPL language version). Compiler not
yet written. See `toolkit/pplc/README.md` for the roadmap.

## Provenance

- Source: `PPLC.RED` archive from PCBoard 15.41 install disk set
  (WCSC, 1994)
- Extraction: install v1.6.0 (2026-09-02) via
  `pcb1541/install/archivers/redx/`
- Documentation: install v1.8.0 (2026-09-03)
