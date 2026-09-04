# pcb1541/install/src/ — installer reimplementation source

C reimplementation of Clark's `INSTALL.EXE` (Phase 3 of the install
arc — the installer side, complementing the archiver work in
`../archivers/`). Reads Clark's `INSTALL.DAT` and executes its
`@`-directives to place the 481 files of a PCBoard install from the
8 `.RED` archives on Disk 1.

## Files

    src/
    ├── README.md      (this file)
    ├── Makefile       cross-platform build (gcc default)
    └── install.c      the whole thing — single translation unit

## Build

Linux / macOS:

    make

Produces `./install` binary.

Windows / DOS / OpenWatcom variants coming as needed. Source uses
`#ifdef __WATCOMC__` / `#ifdef _WIN32` guards; should cross-compile
with minimal adjustment.

## Runtime dependency

**`redx` binary** — the `.RED` archive extractor at
`pcb1541/install/archivers/redx/`. Build it separately:

    cd ../archivers/redx && make

Then either put `redx` in your `$PATH` or pass its path via
`install --redx <path>`.

This matches Clark's original shell-out pattern (INSTALL.EXE also
shells out to external archivers/protocols). See `../RUNTIME-DEPS.md`
for the full list of external tools the installer relies on.

## Usage

    install [options] [INSTALL.DAT]

    Options:
      -a, --archives DIR   Where the .RED archives live (default: cwd)
      -t, --target DIR     Where installed files go (default: cwd)
      -r, --redx PATH      Path to redx binary (default: `redx` in PATH)
      -h, --help           Show usage

If `INSTALL.DAT` is omitted, defaults to `./INSTALL.DAT`.

## Progress

Version labels track how much of Clark's INSTALL.DAT we can execute:

| Version | Directives added | Cumulative | State |
|---|---:|---:|---|
| v1.10.0 | 12 | 12 | Project metadata, display, basic control (@DefineProject/@Display/@Cls/@Pause/@Abort/@Exit + @DefineVars declarations). Scaffold only — nothing gets *installed*. |
| **v1.10.1** | **6** | **18** | **File operations — installer actually places files now**. `@BeginLib`/`@EndLib` (shell out to redx for archive extraction), `@File` with `@Out`/`@Size`/`@AppendTo` subclauses, `@Copy`, `@Delete`, `@FileAttr`. Verified byte-perfect against `../dist/target/` on the file-op subset. |
| **v1.10.2** | **11** | **29** | **Variables + control flow + string ops shipped.** `@If`/`@Else`/`@EndIf`/`@Endif`/`@Goto`/`@Label`/`@Set`/`@StrLen`/`@StrHead`/`@StrToken`/`@Exists`-as-predicate. Full recursive-descent expression evaluator (`[= [! == != > < >= <= && \|\| ()` on int and string; `@Func(...)` inline in string literals). Trailing `@Group X` on `@File` as per-directive filter. Real INSTALL.DAT runs end-to-end: 471 successful @File ops, 348 byte-perfect (94.8%). |
| **v1.10.3** | **10** | **39** | **Filesystem + disk sequencing shipped.** Real `@MkDir`/`@Mkdir` (actual mkdir); `@ChDir`/`@ChDrive` with state tracking; `@DirExists` predicate; `@DefineDisk`/`@EndDisk` semantics; `@Requires`/`@HardDisk`/`@Version` metadata. Also fixed `@Out DIR\*.*` glob-syntax + missing-`@Out` fallback. Full 481 file ops succeed vs real INSTALL.DAT (matches reference tree), 394 byte-perfect (94.0%). |
| **v1.10.4** | **10** | **49** | **Interactive menu shipped.** Real `@GetGroups`/`@CheckBox`/`@GetString` (TTY prompt + headless fallback); `@SetGroup`/`@ClearGroup` state mutators; `@AskOverwrite`/`@Prompt`/`@Qstring`/`@RegCode` accepted. Single-flavor install (`-g abcdefp`): 371 files placed, 350 byte-perfect (94.5%). |
| **v1.10.5** | **7** | **56** | **System hooks + finish shipped.** `@System(cmd)` (headless stub / `--exec-system` opts in); `@SetConfig`/`@SetAutoexec` generate `CONFIG.SYS.pcb` + `AUTOEXEC.BAT.pcb`; `@Files=N`/`@Path=...` emit inside; `@Finish`/`@EndFinish` executes inline (or `--skip-finish` default). File placement unchanged (350/361 byte-perfect); adds 2 system-config files. |
| **v1.10.6** | **0** | **60** | **install v1.10 arc COMPLETE.** Disassembly parity vs INSTALL.EXE binary. No new code — verified our 60 directives all present in the binary. Found INSTALL.EXE supports 329 total directives (Clark's INSTALL.DAT uses 60 = our coverage). See `docs/pcboard-internals/INSTALL-EXE-PARITY.md`. Understanding-complete. Byte-exact INSTALL.EXE rebuild is v1.11+. |

Full directive catalog: `docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md`.

## Acceptance test (v1.10.1)

The v1.10.1 acceptance bar: given a minimal `.DAT` exercising only
the shipped file-op directives, produce output byte-matching the
tracked `../dist/target/` reference tree.

Example minimal `.DAT`:

    @DefineProject
      @Name = "PCBoard"
      @Version = "15.3"
      @Subdir = "\\PCB\\"
      @OutDrive = C
    @EndProject

    @BeginLib PCBOARD.RED
      @File PCBOARD.EXE  @Out PCBOARD.EXE
      @File PCBOARDM.EXE @Out PCBOARDM.EXE
      @File PPLC330.EXE  @Out PPLC.EXE
    @EndLib

    @BeginLib COMMDRV.RED
      @File COMMDRV.EXE  @Out COMMDRV\COMMDRV.EXE
      @File COMMTSR.EXE  @Out COMMDRV\COMMTSR.EXE
      @File DRVSETUP.EXE @Out COMMDRV\DRVSETUP.EXE
    @EndLib

    @Exit

Verified against `../dist/target/`:
`PCBOARD.EXE`, `PCBOARDM.EXE`, `PPLC.EXE`, `COMMDRV/COMMDRV.EXE`,
`COMMDRV/COMMTSR.EXE`, `COMMDRV/DRVSETUP.EXE` — all md5-identical.

Real INSTALL.DAT can't run end-to-end yet (blocks on `@If`/`@Group`
control flow, which is v1.10.2 territory). That's the arc.

## References

- `../reference/INSTALL.DAT` — Clark's real installer script
- `../reference/INSTALL.EXE` — Clark's compiled installer (disassembly
  parity target for v1.10.6)
- `../reference/README.md` — reference material provenance
- `../dist/target/` — byte-verified 481-file install tree (acceptance
  target for later v1.10.x releases)
- `../dist/target/rebuild_place.py` — Python reference impl of the
  file-placement logic (was the model for the v1.10.1 redx wire-up)
- `../archivers/redx/` — the .RED archive extractor
- `../RUNTIME-DEPS.md` — external dependencies (redx, FOSSIL, DSZ/GSZ,
  archivers)
- `../../../docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md` —
  canonical @-directive reference
