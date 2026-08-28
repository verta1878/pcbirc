# pcbconf — Import FidoNet echo/file lists into PCBoard

Bulk-loads FidoNet **echo-tag** and **file-tag** lists into PCBoard,
starting at a given conference/area number. Saves a sysop the tedium
of setting up feeds one at a time in PCBoard's config UI.

Reads two families of files:

- `*.NA` — **A**ctive echoes/file areas (the normal case)
- `*.NO` — echoes/file areas marked **N**ot active / **N**o longer
  carried (see FTSC notes below)

Message-echo list (BACKBONE.NA / FIDONET.NA) -> PCBoard message
conferences. File-echo list (FILEBONE.NA) -> PCBoard file areas.

## Status

Work in progress. Target compiler is **Borland C++ 3.1** first (the
pwa153 default); the rest of the SDK matrix (TC 2.01, MSC 7.0)
follows once the Borland port is proven.

## The `.NA` / `.NO` file format (FTSC background)

FidoNet echo/file backbone lists are plain ASCII files, one echo per
line, in the format:

```
<TAG>          <description>
MOVIES         Movies & TV Discussion
NET_DEV        FidoNet Network Development
FILEBONE_TEST  Test file echo
```

The `TAG` is short (typically <= 20 chars, uppercase, no spaces --
underscores allowed). The description runs to end of line.

Filename conventions observed on real FidoNet backbones (see
`ftsc.org` and the file indexes on Z1/Filegate, e.g.
`dreamlandbbs.com/filegate/fidonet/backbone/`):

| File | Meaning |
|------|---------|
| `BACKBONE.NA` | **Active** message echoes on the North American Backbone |
| `BACKBONE.NO` | **Inactive** echoes (still known, not currently distributed) |
| `BACKBONE.DST` | Echoes no longer available (DiSTribution list drop) |
| `BACKBONE.INT` | Echoes gated from the internet |
| `BACKSTAT.NA` | Latest changes / status |
| `FIDONET.NA`  | Regional/hub-specific active echo list (what a PCBoard sysop typically receives from their coordinator) |
| `FILEBONE.NA` | Active **file** echoes (file-distribution backbone) |
| `FILEBONE.NO` | Inactive file echoes |

Relevant FTSC documents (fetch from `http://ftsc.org/docs/`):

- **FSC-0061** -- Proposed Guidelines for the FileBone
- **FSC-0087** -- TIC file format (references AREADESC "found in
  FileBone.NA")
- **FTS-0001** -- Basic FidoNet technical standard
- **FSC-0057** -- Echo Area Managers, request specifications

## PCBoard side -- what we need to learn

The tool's shape (parse a .NA/.NO file, walk lines, write one record
per line) is trivial ANSI C. The interesting work is on the PCBoard
side: knowing which on-disk records to write and how to open them
safely.

Two data-structure areas to nail down:

1. **Conference (message-area) records** -- for `BACKBONE.NA` /
   `FIDONET.NA`. PCBoard stores conferences in `CNAMES` (main
   conference file). We need to write name, echo tag, description,
   and the FidoNet flag/settings per conference. Header definitions
   in `toolkit/pwa153/H/PCBDATA.H` and `H/NEWDATA.H`.
2. **File-area records** -- for `FILEBONE.NA`. PCBoard's file-area
   layout is a separate structure (DLPATH.LST for download paths,
   plus per-conference file configs). Need to identify which files
   the sysop-config UI writes when a file area is added.

Locking / reopening safely while PCBoard might be running is a
separate question -- the pwa153 toolkit has helper routines for
opening PCBoard-owned files with the right sharing modes; those
should be used rather than raw `fopen`.

**Learning source in flight:** MUTIL.PAS (Mystic BBS's utility)
is the closest working reference for a bulk `.NA` importer -- its
`[Import_FIDONET.NA]` stanza does exactly this for Mystic's message
bases. Adapting its logic to PCBoard's `CNAMES` layout is the near
path. verta1878 to provide MUTIL.PAS after FTSC docs are read.

## CLI

```
PCBCONF /FILE=path /CONF=nnn /NA [/DESC]
PCBCONF /FILE=path /CONF=nnn /NO [/DESC]
```

| Switch | Meaning |
|--------|---------|
| `/FILE=path` | path to the `.NA` or `.NO` list |
| `/CONF=N` | starting conference (or file-area) number; imports walk upward |
| `/NA` | input is a `.NA` (active) list -- required for the active case |
| `/NO` | input is a `.NO` (inactive) list -- required for the inactive case |
| `/DESC` | use the description field as the conference name (default: echo tag) |

Exactly one of `/NA` or `/NO` must be present.

pcbconf is command-line only -- no interactive UI, no menus, no
prompts, no banner. Missing/invalid input prints usage on stderr
and exits 1. A successful import prints per-entry `[N] TAG DESC`
lines and a summary, then exits 0.

Exit codes:
- **0** -- import completed
- **1** -- bad or missing args (usage printed)

## Files

- `pcbconf/pcbconf.c` -- Clark-style port; portable ANSI C with pwa153
  toolkit helpers (`maxstrcpy`, `dosopencheck`, etc.). Windows/Wildcat
  scaffolding gone; PCBoard config-write layer stubbed with TODO
  comments pointing at `toolkit/pwa153/H/PCBDATA.H` and
  `H/NEWDATA.H`.
- `pcbconf.zip` -- archived original binary + working notes.

## Roadmap

- [x] Strip Windows-only externs; retarget to Borland C++ 3.1 style
      (Clark: 2-space K&R, PascalCase globals, `pascal` calling
      convention, `strupr` + `strncmp` switch pattern)
- [x] `/NA` and `/NO` switch handling
- [x] Command-line only, no UI, no banner
- [ ] Wire `openconfig` / `seekconf` / `writeconf` / `closeconfig` to
      the pwa153 toolkit conference-DB records (`CNAMES` via
      `PCBDATA.H`'s `conftype`)
- [ ] File-area path (`FILEBONE.NA` -> PCBoard file-area records)
- [ ] Build under BC++ 3.1 -> land in `OUT/lib/pwa153/bc31/bin/`
- [ ] Sanity test: feed a real BACKBONE.NA, verify PCBoard sees the
      new conferences in PCBSETUP; verify a `.NO` run doesn't
      double-create existing ones
- [ ] TC 2.01 and MSC 7.0 builds (fill out the 3-compiler matrix)
- [ ] pwa154 (delta154) and 1541 targets (same source, forward
      feature additions)
