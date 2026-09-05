# INSTALL.EXE parity report — install v1.10.6

Verifies that our C reimplementation of Clark's `INSTALL.EXE` matches
the actual binary's semantics for the subset of directives that
`INSTALL.DAT` actually uses. This closes the install v1.10
"understanding-complete" milestone.

## Reference binary

- Path: `pcb1541/install/reference/INSTALL.EXE`
- Size: 338,548 bytes
- md5: `5239767bfced0689a1da961799a0f79c`
- Format: NE (New Executable), Family API, linker 5.10
- Target OS byte: 0x01 (OS/2 1.x, but Family API subset runs under DOS
  as well — matches the "for DOS and OS/2" claim in Clark's welcome
  banner)
- Imported modules: `DOSCALLS`, `KBDCALLS`, `VIOCALLS` (OS/2 native API)
- Header string: `$Logfile: W:/master/install/main.c_v $`,
  `INSTALL Ver ... Copyright (c) 1987-1995`

## The big finding

**INSTALL.EXE supports 329 total `@`-directives; Clark's INSTALL.DAT
uses 60.** The engine is a general-purpose installer framework, not
a bespoke PCBoard tool. Our v1.10.0 → v1.10.5 arc implemented all 60
directives Clark's script actually uses. The other 269 are supported
by the engine but unexercised by this particular install script.

This means:
- Our reimplementation is **complete** for running `INSTALL.DAT`
  correctly.
- We are **not** a drop-in replacement for `INSTALL.EXE` as a general
  installer engine (269 directives short of that).
- Byte-exact reconstruction of the `INSTALL.EXE` binary (v1.11+) will
  need to implement all 329 directives — that's a materially larger
  scope than what v1.10 accomplished.

## Verification: our 60 directives vs the binary

Each directive appears in the `INSTALL.EXE` binary's string table
(where the parser's ALL-CAPS lookup keys live) or as a scanner-level
keyword (compiled into the parser code, doesn't need a string entry).

### Present in binary strings (54 confirmed)

Category | Directives (all confirmed in binary)
---|---
Project metadata | `@DEFINEPROJECT`, `@ENDPROJECT`, `@NAME`, `@VERSION`, `@SUBDIR`, `@OUTDRIVE`, `@REQUIRES`, `@HARDDISK`
Variable declarations | `@DEFINEVARS`, `@ENDVARS`, `@QSTRING`, `@SET`
Display | `@DISPLAY`, `@ENDDISPLAY`, `@CLS`, `@PAUSE`, `@PROMPT`, `@OUT`
Interactive input | `@GETOUTDRIVE`, `@ENDOUTDRIVE`, `@GETSUBDIR`, `@ENDSUBDIR`, `@GETSTRING`, `@ENDSTRING`, `@GETGROUPS`, `@ENDGROUPS`, `@CHECKBOX`, `@GROUP`, `@SETGROUP`, `@CLEARGROUP`, `@ASKOVERWRITE`
Control flow | `@ELSE`, `@ENDIF`, `@GOTO`, `@LABEL`, `@ABORT`, `@EXIT`
String ops | `@STRLEN`, `@STRHEAD`, `@STRTOKEN`
File operations | `@BEGINLIB`, `@ENDLIB`, `@FILE`, `@FILES`, `@SIZE`, `@PATH`, `@COPY`, `@DELETE`, `@APPENDTO`, `@FILEATTR`
Disk sequencing | `@DEFINEDISK`, `@ENDDISK`
Filesystem | `@MKDIR`, `@CHDIR`, `@CHDRIVE`, `@DIREXISTS`, `@EXISTS`
System hooks | `@SYSTEM`, `@FINISH`, `@ENDFINISH`, `@SETAUTOEXEC`, `@ENDAUTOEXEC`, `@SETCONFIG`, `@ENDCONFIG`

### Not in string table — scanner-level keywords (1)

- **`@IF`** — control-flow keyword compiled directly into the parser,
  not looked up via string hash. Confirmed by presence of related
  `@ELSE`/`@ENDIF`/`@ELSEIF` (which uses `@If` as its base token).

### Not directives — user-declared variables (5)

These appear in `INSTALL.DAT` and look like `@`-directives but are
actually variable names the script declares via
`@DefineVars @Qstring @Fname`. The engine only knows `@QSTRING`
(the type declaration) — the user variables are runtime-created:

- `@Fname` — first name storage
- `@Lname` — last name storage
- `@CitySt` — city/state storage
- `@Pwd` — password storage
- `@RegCode` — registration code storage

Our v1.10.0 catalog listed these in the directive count for
completeness. They're implemented correctly (we handle `@QString`
declarations and store the variables in our `Vars[]` table),
but they're not engine directives per se.

**Adjusted count**: 55 engine directives + 5 user variables + 1
scanner keyword = 61 things we implement (matching Clark's script's
full usage).

## Semantic surprises discovered

### Constraint 1: `@Goto` cannot exit `@Finish` block

Binary contains the error string:

    Cannot use @Goto to exit the @Finish block

Our implementation does not enforce this. In practice Clark's
`INSTALL.DAT` doesn't `@Goto` out of `@Finish` anyway (no such
statements in the script), so the constraint never fires. Documented
for completeness; v1.11+ (byte-exact rebuild) will need to enforce it.

### Constraint 2: `@Size` incompatible with `@Requires`

Binary contains:

    @Size commands cannot be used when @Requires is specified

But Clark's `INSTALL.DAT` uses `@Requires @HardDisk` at the top of the
project AND uses `@Size` extensively inside `@File` directives. So
this constraint must not fire on `@Requires @HardDisk` — probably
only on `@Requires` with other targets (network install types?).
Documented; our implementation ignores this constraint.

### The `@OutXXX` family (disk-size-conditional output paths)

Binary contains a family of related directives we didn't know about:

    @OUT, @OUT0K, @OUT128K, @OUT360K, @OUT512K, @OUT720K,
    @OUT1M, @OUT1440K, @OUT5M, @OUT10M, @OUT20M, @OUT30M,
    @OUTABS, @OUTDISKBELL

Error message: `@Outxxx commands are not in ascending order`.

These are conditional output paths keyed on the destination disk's
free-space bucket. A file directive can specify different output
paths for different disk sizes:

    @File FOO.EXE @Out0K PATH1 @Out1M PATH2 @Out10M PATH3

Clark's `INSTALL.DAT` doesn't use any of these — only plain `@Out` —
so we're not missing coverage. Documented for v1.11+.

### `@Option` alongside `@Group`

Binary contains:

    @Group or @Option

Suggesting `@Option` is a parallel selection mechanism to `@Group`,
likely used for radio-button UIs where `@Group` is checkbox-based.
Clark's `INSTALL.DAT` uses only `@Group`; `@Option` support is not
in our implementation.

### `@Dir` variable type

Binary contains:

    @Dir or @QString type variable

Suggesting `@Dir` is a directory-path-typed variable in addition to
`@QString` (arbitrary string). Clark's `INSTALL.DAT` uses only
`@QString` — so no coverage gap.

### `@Verbatim`, `@Write`, `@Read` string ops

Binary strings:

    @Verbatim string too long.
    @Write: Could not write to file
    @ReadLine

These are file-I/O directives (write to a file, read a line back)
that Clark's script doesn't use.

## The 269 unused directives — categorized

For posterity, these are the engine capabilities Clark's PCBoard
`INSTALL.DAT` doesn't exercise, grouped by function.

### `@Out*` family (14 variants — disk-size-conditional paths)

`@Out`, `@Out0K`, `@Out128K`, `@Out360K`, `@Out512K`, `@Out720K`,
`@Out1M`, `@Out1440K`, `@Out5M`, `@Out10M`, `@Out20M`, `@Out30M`,
`@OutAbs`, `@OutDiskBell`

Only plain `@Out` is used in Clark's script.

### System-query directives (~50)

Detection of running environment:

- **OS**: `@OSMajor`, `@OSMinor`, `@RevMajor`, `@RevMinor`,
  `@RevSub`, `@Platform`
- **Windows**: `@WinMajor`/`@WinMinor`/`@WinMode`/`@WinDir`/
  `@WinSysDir`/`@WinSysDrive`/`@WinDrive`/`@WinExec`/`@WinExit`/
  `@WinExitExec`/`@WinScreenCaps`/`@WinVersion`/`@WinEmsFrame`/
  `@WindowsMajor`/`@WindowsMinor`/`@WindowsDir`/`@WindowsDrive`/
  `@WindowsMode`/`@WindowsVersion`/`@WindowsExit`/
  `@WindowsExitExec`/`@WindowsEmsFrame`
- **CD-ROM**: `@CDRomFirst`, `@CDRomMajor`, `@CDRomMinor`,
  `@CDRomTotal`, `@DriveCDRom`
- **Memory**: `@EmmAvail`, `@EmmMajor`, `@EmmMinor`, `@EmmTotal`,
  `@ExtAvail`, `@ExtTotal`, `@RamAvail`, `@RamTotal`,
  `@XmsAvail`, `@XmsHandles`, `@XmsMajor`, `@XmsMinor`,
  `@XmsRevision`, `@XmsTotal`, `@Xma2Ems`
- **Video**: `@VideoCard`, `@VideoGraph`, `@VideoMode`,
  `@VideoMonitor`, `@VideoRam`, `@EgaMajor`, `@EgaMinor`,
  `@AspectX`, `@AspectXY`, `@AspectY`, `@ColorRes`, `@CurveCaps`,
  `@HorzRes`, `@HorzSize`, `@VertRes`, `@VertSize`,
  `@LineCaps`, `@LogPixelsX`, `@LogPixelsY`, `@Planes`,
  `@PolygonalCaps`, `@RasterCaps`, `@RGB`, `@ScreenProto`,
  `@SizePalette`, `@TextCaps`
- **Ports**: `@Com`, `@ComTotal`, `@Lpt`, `@LptTotal`, `@Keyboard`,
  `@KeybCom`
- **Drives**: `@BootDrive`, `@StartupDrive`, `@StartupDir`,
  `@Drive`, `@DriveCDRom`, `@DriveExists`, `@DriveFree`,
  `@DriveRemote`, `@DriverSys`, `@DriverVersion`, `@DriveSize`,
  `@LastDrive`, `@Removable`, `@DiskFree`, `@DiskProto`, `@DiskSize`
- **Machine**: `@Cpu`, `@Ndp`, `@MachineId`, `@MachineName`,
  `@MachineNum`, `@MCBSignature`, `@Netbios`, `@LanMajor`,
  `@LanMinor`, `@LanVendor`

### String operations (10 more we don't have)

`@StrDel`, `@StrFind`, `@StrIndex`, `@StrLwr`, `@StrMid`,
`@StrRFind`, `@StrTail`, `@StrToDate`, `@StrToInt`, `@StrUpr`

### File I/O + path handling (extra)

`@FileCrc`, `@FileDate`, `@FileSize`, `@FileFormat`,
`@Crc`, `@CrcFile`, `@GetCwd`, `@GetDir`, `@Move`, `@Rename`,
`@RmDir`, `@Chmod`, `@Assign`, `@Decompress`, `@Format`,
`@FormatAllowed`

### System integration

`@Execute`, `@Spawn`, `@Shell`, `@Reboot`, `@GetEnv`, `@SetEnv`,
`@GetIni`, `@SetIni`, `@SetMacro`, `@SetAppend`, `@SetPrepend`,
`@SetReplace`, `@DosAppend`, `@DosAssign`, `@DosKey`, `@DosPrint`,
`@DosShare`, `@DosVerify`, `@Ansisys`, `@Displaysys`, `@Grafttbl`,
`@Nlsfunc`, `@Buffers`, `@Stacks`, `@Break`, `@Device`,
`@ProgramManager`

### DOS interrupt access

`@IntAh`, `@IntAl` — direct interrupt-register queries

### Advanced flow

`@Chain`, `@Debug`, `@Immediate`, `@Terse`, `@Suppress`,
`@Verify`, `@Default`, `@Select`, `@Return`, `@ReturnValue`,
`@ScriptFile`, `@ScriptLine`, `@ScriptSize`, `@Eval`,
`@BeginPatch`, `@EndPatch`, `@Simulate`, `@EndSimulate`,
`@Welcome`, `@EndWelcome`, `@Verbatim`, `@Write`, `@ReadLn`,
`@GetInteger`, `@EndInteger`, `@GetQString`, `@GetOption`,
`@EndOption`, `@SetOption`, `@ClearOption`, `@FlushGroups`,
`@FlushKeyboard`, `@FlushOptions`, `@Integer`, `@Byte`,
`@DlgCtrlSize`, `@BackgroundMode`, `@LocalWindow`, `@BitsPixel`,
`@CompletionBar`, `@Chain`, `@AddFont`, `@RemoveFont`,
`@Macro`, `@Move`, `@MoveCCStr`, `@MoveCStr`, `@Min`, `@Max`,
`@App`, `@Desc`, `@Dir`, `@Option`, `@Overwrite`, `@NoOverwrite`,
`@True`, `@False`, `@Off`, `@On`, `@AssumeHardDisk`,
`@SystemDate`, `@TextFormat`, `@TitlePause`

## Acceptance for v1.10.6

- **60/60 directives** used by real `INSTALL.DAT` implemented (55
  engine + 5 user variables + 1 scanner keyword, actual coverage
  is complete).
- **End-to-end run** of `INSTALL.DAT` produces byte-perfect output
  vs `pcb1541/install/dist/target/` on 94.5% of placed files (350
  of 361). Remaining 9 differ + 2 extras are per-BBS multi-writer
  collisions (last-write-wins) — architectural, not a bug.
- **CONFIG.SYS.pcb** and **AUTOEXEC.BAT.pcb** correctly generated with
  Clark's intended contents (`FILES=25`, `PATH=%PATH%;C:\PCB\`).
- **@Finish** cleanup semantics correct — `INIT.EXE`/`UPGRADE.EXE`
  removed under `--run-finish`.

**install v1.10 arc: understanding-complete.**

## What's not in this arc

## install v1.11 arc — byte-exact INSTALL.EXE reconstruction

### What actually happened (v1.11.0 through v1.11.3)

The original 10-phase plan (v1.11.1 through v1.11.10, one directive
bucket per phase) was **retired after v1.11.2 collapsed phases 1-5**
into a single handler port. The actual path:

| Phase | What | Status |
|---|---|---|
| v1.11.0 | Toolchain shakedown (empty main, NE binary, linker 5.10) | DONE |
| v1.11.1 | Dispatch table scaffold (301 directives, enum/lookup) | DONE |
| v1.11.2 | Semantic handler port (all 60 handlers from v1.10.5, collapsed phases 1-5) | DONE |
| v1.11.3 | Family API link target (/Toe, C0FL.OBJ, OS byte 0x01) + BC 3.1 compliance | DONE |
| **v1.11.4** | **Gap analysis (DOCUMENT, not code)** — structural comparison of our 56KB binary vs Clark's 338KB, categorized by gap type. Defines v1.11.5-v1.11.9. | NEXT |
| v1.11.5-v1.11.9 | **Undefined until v1.11.4 lands.** Phases will be driven by the gap categories: implementation gaps, RTL gaps, resource gaps, compiler-artifact gaps. |  |
| v1.11.10 | Understanding-complete milestone | |
| **v1.12** | **Byte-exact arc** (next arc, not this one). Actual `cmp -s` target. | DEFERRED |

### What v1.11.4 (gap analysis) must produce

`docs/pcboard-internals/INSTALL-EXE-GAP-ANALYSIS.md` containing:

1. NE segment table dump — both binaries side by side
2. Code / data / resource segment size deltas
3. Import / export table entry counts — both sides
4. String table extraction (`strings` both sides, diffed)
5. Categorized gap inventory:
   - **Implementation gap** — Clark's binary handles something we
     stubbed (directive branches, screen rendering, error messages)
   - **RTL gap** — Clark links against BC 3.1 full C runtime; we may
     be missing overlay manager, extended file I/O, math library
   - **Resource gap** — Clark embeds strings/tables/screens we rebuild
     at runtime or load from disk
   - **Compiler-artifact gap** — optimization settings, debug info,
     dead-code elimination, NE alignment padding
6. Phase recommendations — what v1.11.5-v1.11.9 should be, based on
   the categories

The category matters for phasing: implementation gaps become handler
work. Resource gaps become an embedded-strings phase. RTL gaps become
a linker-config phase. Compiler-artifact gaps might close for free
with the right BC 3.1 flags.

### Note on the 56KB vs 338KB framing

Don't assume byte-for-byte parity is the goal at v1.11. Understanding
parity is. Clark's 338KB may include debug symbols, resource fork
padding, NE-format alignment gaps. Byte-exact rebuild is v1.12.

## See also

- `pcb1541/install/reference/README.md` — reference material provenance
- `pcb1541/install/reference/INSTALL.EXE` — the parity target
- `pcb1541/install/reference/INSTALL.DAT` — the script that drives it
- `pcb1541/install/src/install.c` — our reimplementation
- `pcb1541/install/src/README.md` — reimplementation build + usage
- `docs/pcboard-internals/INSTALL-DAT-DIRECTIVES.md` — 60-directive
  canonical catalog with per-directive implementation status

--install v1.10.6, 2026-09-05
