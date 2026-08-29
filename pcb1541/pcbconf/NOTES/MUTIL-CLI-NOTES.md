# MUTIL CLI reference notes (for pcbconf)

Command-line design lessons from Mystic BBS's `mutil.pas` (main program
only — INI reading, task execution, and file-parsing internals
deliberately not studied). Source read: `mysticbbsirc/mystic/mutil.pas`
(James Coyle, GPLv3, ~1997-2013).

Purpose: use MUTIL's flag conventions as a guide for pcbconf's CLI
surface, since MUTIL is the closest-shape working analog and its
switch style is familiar to BBS sysops.

## MUTIL invocation shapes

```
MUTIL                           Execute using mutil.ini
MUTIL [IniFile]                 Execute using a custom INI file
MUTIL [IniFile] -RUN [Command]  Execute specific functions from custom INI
MUTIL -RUN [Command]            Execute one or more (comma separated)
MUTIL -LIST                     List all MUTIL functions
MUTIL -VER                      Show version information
MUTIL -NOSCREEN                 Execute without screen output
MUTIL -HELP    (also -H)        Show usage help
```

## Flag rules observed in the source

- **Dash prefix**, case-insensitive (source uses `strUpper` on
  `ParamStr` before comparing).
- **`-VER` / `-LIST` / `-HELP` / `-H`** must be at `ParamStr(1)` —
  they short-circuit startup and `Halt(0)`.
- **`-NOSCREEN`** can appear in any position (scans all
  ParamStr on startup).
- **`-RUN`** consumes the next argument as a comma-separated task
  list. INI-file positional arg parsing skips the token immediately
  after `-RUN` so it doesn't get mistaken for an INI filename.
- **Positional arg** (INI filename) is any argument not starting with
  `-`. Falls back to `<name>.ini` if no extension. Defaults to
  `mutil.ini`.

## Task-selection pattern

MUTIL has ~25 tasks in a hardcoded string array (`TaskNames`). Each
task is a symbolic name like `Import_FIDONET.NA`, `Import_FILEBONE.NA`,
`Export_FILEBONE.NA`, `ImportEchoMail`, `PackMessageBases`, etc. In
`-RUN` mode, task selection is a substring match against the
comma-separated `-RUN` list:

```
Result := Pos(strUpper(pName), strUpper(RunList)) > 0
```

Simple, no regex, no glob. Sysop-friendly.

## What pcbconf should adopt

pcbconf is single-purpose (import .NA / .NO into PCBoard), so the
task-list dispatch is overkill — but three MUTIL conventions are
worth taking:

1. **Standard flag set**: add `/HELP` (or `-H`), `/VER`, `/NOSCREEN`.
   Sysops familiar with MUTIL will find these instantly.
2. **Case-insensitive flag parsing**. DOS convention is `/FLAG`,
   MUTIL uses `-FLAG`, but the comparison is `strUpper`-ed either way.
   pcbconf's existing `/NA`, `/CONF=`, `/FILE=`, `/DESC` fit right in.
3. **`-NOSCREEN` (equivalently `/QUIET` or `/Q`)**: important for
   headless / scripted runs — no console output, only exit code +
   optional log file. Matters because the golden build image
   (`PCBBLDBT.IMG`) runs BUILD.BAT unattended.

## What NOT to copy from MUTIL

- The INI-file config layer — pcbconf is a single-run tool, all
  behavior on the command line, no config file.
- The screen/status console (`TOutput`, status bars, `DrawStatusScreen`)
  — nice for MUTIL's long multi-task runs, unnecessary for a single
  .NA import.
- The task-selection Array + substring-match dispatch — irrelevant
  for a one-command tool.

## Final pcbconf CLI (as implemented)

```
PCBCONF /FILE=path /CONF=nnn /NA|/NO [/DESC]
```

| Switch | Meaning |
|--------|---------|
| `/FILE=path` | path to `.NA` / `.NO` list |
| `/CONF=N` | starting conference/area number |
| `/NA` | active-list mode (mutually exclusive with `/NO`) |
| `/NO` | inactive-list mode |
| `/DESC` | use description as conference name |

Deliberately spare — no `/RUN`, no `/NOSCREEN`, no `/QUIET`, no
`/HELP`, no `/VER`, no banner. pcbconf is one-purpose, all-CLI, no
UI. Missing/invalid args → usage + exit 1. Valid args → do the
import and exit 0.

The MUTIL flags studied here (`-RUN`, `-NOSCREEN`, `-LIST`) don't
apply because pcbconf isn't a multi-task runner and has no console
UI to suppress. Notes kept in case a future pcb1541 tool with
MUTIL shape (many tasks selected from INI or CLI) needs the same
conventions.

## Provenance

`mutil.pas` fetched transiently from
`github.com/verta1878/mysticbbsirc/tree/main/mystic` for CLI-pattern
study only. File not vendored into pcbirc — only these notes are kept.
Deleted from local scratch after reading.
