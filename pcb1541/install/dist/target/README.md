# pcb1541/install/dist/target/

**The fully-installed PCBoard 15.41 directory structure** (`C:\PCB\`) as
the original WCSC installer would produce it. All 481 files across the
original 4-disk install set, tracked in git, with a byte-perfect rebuild
path from `../INSTALL.zip` for anyone who wants to reproduce it from
source.

---

## Layout

```
target/
├── PCBOARD.EXE, PCBOARDM.EXE, PPLC100.EXE, PPLC330.EXE     Main BBS binaries
├── PCBSM.EXE, DOORWAY.EXE, INIT.EXE, USERNET.EXE, and
│   30+ more utility EXEs at root
├── PCBSM.HLP, VIEWARCH.COM                                 System tools
├── PCBSH/PCBSZ/PCBTEST/PCBVIEW/BOARD .BAT                  Launcher batches
├── REMOTE.SYS, PCBSM.CLR, PCBSM.CNF                        System configs
│
├── COMMDRV/          22 files  WCSC CommDrv driver package
├── PCBOS2/           10 files  OS/2 support tree
├── PCBMAIL/           4 files  QWK mailer + Borland runtime
├── PPL/              42 files  Sample PPL programs (.PPE + .PPS)
├── HELP/            63 files  Online help topics
├── MAIN/            24 files  Message-base skeleton (CNAMES.*)
├── DOC/             30 files  Documentation
├── ADMIN/, CUSTSRVC/, EMPLOYEE/, ENGINRNG/, FIDO/, FILES/,
│   GEN/, GRAPHICS/, HMNRSRC/, MIS/, PROD1/, PROD2/,
│   SLSMKTNG/, DL01/            Clark's sample conference/subdir tree
│
├── MANIFEST.txt      Full source-key → target-path map (from INSTALL.DAT)
├── AVAILABLE.md      Full inventory: 481 placements, 8 archives, 22 dirs
├── rebuild.sh        Extract-on-demand: Linux/Mac/WSL
├── rebuild.bat       Extract-on-demand: Windows
├── rebuild_place.py  Shared disk-aware placement logic
└── README.md         This file — includes the full work log below
```

---

## To rebuild from source

The 481 files here are also regeneratable byte-perfect from
`pcb1541/install/INSTALL.zip`. Both paths converge — tracked files match
the rebuild output exactly.

```bash
cd pcb1541/install/dist/target
./rebuild.sh              # Linux/Mac/WSL/Git Bash
```

or on Windows:

```cmd
cd pcb1541\install\dist\target
rebuild.bat               REM needs mingw gcc + python3 + unzip in PATH
```

Requires: C compiler (gcc/cc/mingw), python3, unzip. The scripts locate
`INSTALL.zip` and `redx` source automatically by walking up from their
own location.

---

## Provenance chain

```
Original PCBoard 15.41 install-disk set (4 zipped disks: abpb1531-4.zip)
        │
        │  disks 1-4 payload consolidated into INSTALL.zip
        ▼
pcb1541/install/INSTALL.zip           installer disk image
        │
        ├── INSTALL.EXE               original installer
        ├── INSTALL.DAT               installer script (@File → @Out mappings)
        ├── INSTALL.RES               resource file
        ├── COMMDRV.RED, PCBCFGS.RED, PCBMAIL.RED,
        │   PCBOARD.RED, PCBOARD2.RED, PPLC.RED,
        │   PCBDISK.002, PCBDISK.003
        │           │
        │           │  extracted via `redx extract` (all use .RED format)
        │           ▼
        │   sources placed disk-by-disk per INSTALL.DAT @DefineDisk sections
        │           │
        │           ▼
        └── dist/target/              (also tracked in git, byte-identical)
                (481 file placements, byte-perfect against a real install)
```

---

## Sources with source code available

Some target binaries have known upstream source in this repo:

| Target | Source tree |
|---|---|
| `PCBOARD.EXE`, `PCBOARDM.EXE` | `pcb1541/`, `pcb154/`, `pcb153/` (WATCOM builds) |
| `COMMDRV/COMMDRV.EXE` etc.    | `pcb154/pcbdcom/`, `pcb1541/pcbdcom/` (pcbdcom rewrite) |
| `PPLC100.EXE`, `PPLC330.EXE`  | `toolkit/pplc/` (partial: 3.00/3.10/3.20 present) |
| `PPL/*.PPS` sources           | These ARE the source; `.PPE` files rebuild from them |

Everything else in this tree is a compiled binary or data file with no
source available; it's regenerated from the shipped archives as the
byte-perfect parity target for anyone re-implementing the corresponding
piece.

---

## License

WCSC / Clark Development Corp freely redistributable per post-market-exit
community consensus. Borland DLLs (`BWCC.DLL`, `BC450RTL.DLL` under
`PCBMAIL/`) remain Borland's IP; regenerated here solely to allow the
original `PCBMAIL.EXE` to run.

---

# Work log — install v1.0 → install v1.7.2

Full chronological record of how the installer subsystem (`pcb1541/install/`)
came to be. The version numbers here (install v1.0, v1.1, ... v1.7.2) are
**INSTALL-SUBSYSTEM ONLY** — internal milestones within the installer arc.
They are not:

- **PCBoard 15.4 / 15.41** — the DOS BBS software this project revives.
  `pcb1541/` in the directory name refers to that software version.
- **pcbirc** — the project/repo name (**pcb**oard **irc**-branch).
- **pcbirc project versions** (v0.1, v0.2 … tracked in `HISTORY.md`) —
  the whole-project history, of which the installer arc is one part.
- **pcbdcom versions** (its own v1.1, v1.2 … in `pcb154/pcbdcom/`) — the
  serial-driver rewrite subsystem, unrelated numbering.

These install v1.x labels are internal to the installer subsystem
and don't correspond to project releases or software versions. The log
lives in this canonical README so future contributors can see the
reasoning, dead ends, and fixes — not lose them to per-release notes
that get thrown away.

## install v1.0 (2026-08-20, commit `f8cd1d6` "clean") — installer subsystem scaffold

First landing of `pcb1541/install/`. Established the shell of what would
become the reproduction target, without yet knowing how to unpack any
of the payload.

- `INSTALL.zip` (2.4 MB) — the binary blob copied in from the original
  PWA install-disk set. Contents unknown; treated as opaque.
- `dist/disk1/` … `dist/disk5/` — skeleton for the 5-disk physical
  floppy layout, one dir per disk with `FILE_ID.DIZ` and README
  stubs. Placeholder for what each disk would eventually be shown to
  contain.
- `DATA/default/` — template configuration for a fresh install:
  - `BOARD.BAT`, `PCBDOS.BAT` — DOS launcher scripts
  - `PCBOARD.DAT` (350 lines) — the runtime config file
  - `PCBOARD.SYS` (150 bytes) — the caller-state record layout
  - `GEN/CLOSED`, `GEN/EXPIRED`, `GEN/WARNING`, `GEN/WELCOME` —
    sample sysop-facing bulletins
  - `www/index.htm` (220 lines) — HTML dashboard skeleton (an
    IRC-branch UI experiment)
  - `setup.sh` — Linux-side install helper stub

State: 29 files under `pcb1541/install/`. `INSTALL.zip` present but
unpackable — no `.RED` decoder existed yet.

## install v1.1 (2026-08-20, commit `a858054`) — subsystem README landed

`pcb1541/install/README.md` added (61 lines). Documented the
subsystem's role as the ORIGINAL PWA install disk set, why every
downstream PCBoard variant needs it, and the two-view `dist/`
breakdown (physical disks vs. installed target). Same session added
14 other README.md files across `pcb1541/` — the "every program gets
a README" pass.

## install v1.2 (2026-08-25, commit `e4181e5`) — Phase 27 installer reproduction

`src/install.c` added (518 lines, GPLv3): a C reproduction of Clark's
331 KB `INSTALL.EXE`. Script-driven — reads `INSTALL.DAT`, processes
~40 of the 250+ possible `@Command` directives.

Deliberately incomplete: the file's own header says "DEPENDENCY:
.RED container extraction/creation ... Once archivers/redx and
archivers/redc land (built on archivers/lha/), this installer links
against them instead of shelling out." The installer was written
before we could actually unpack the disks.

Same session did the big toolkit restructure — established the 4-version
model (`pcb153`, `pcb153/upd154`, `pcb154`, `pcb1541`), moved
`build/` → `MAIN/build/`, retired `OUT/pwa154/`. Not installer work
per se but the version-model settle-in that everything else was built
on.

## install v1.3 (2026-08-29, commit `2d51f40` "failer update")

Small config tweak (1 file, 8 insertions). Minor bookkeeping — no
substantive install-subsystem change.

State at end of install v1.3: 29 files under `pcb1541/install/`. Still no
`.RED` decoder.

## install v1.4 → install v1.5 (2026-08-31 → 2026-09-01, commit `f142d07`) — the pcbdcom arc

The "three-day work summary" commit. Primarily a `pcb154/pcbdcom/`
push (v1.1 → v1.2 of pcbdcom itself): all 5 planned features shipped
as GPLv3 clean-room:

1. Arnet SmartPort/Plus backend (8th card, 231 lines)
2. 13-function `COMMDRV.OBJ` replacement (216 lines)
3. INT 14h AH≥0x10 COMM-DRV extensions (+42 lines)
4. Proper `_dos_keep()` TSR install (rewrite)
5. SDK packaging for third-party linkers (32 files)

Totals: 3,829 lines GPLv3 across 15 `.c` + 6 `.h` files, 8 card
family backends, 15/15 clean builds with OpenWatcom + BC 3.1 cross,
`PCBDTSR.EXE` = 37,800 bytes.

**Started the `.RED` decoder work here.** Reverse-engineering the
container format (RR magic, LH5-family compression, per-record
CRC16). By session end: byte-perfect decode on 3/6 archives, decoder
bugs identified but not all squashed.

Install subsystem itself untouched in this commit — the .RED work
lived under `pcb1541/install/archivers/`.

## install v1.6 → install v1.7.2 — the .RED archive arc

Everything from install v1.6.0 onward touches the installer subsystem directly.



Chronological record of how this tree came to exist. This lives in the
canonical README so future contributors can see the reasoning, dead ends,
and fixes — not lose them to per-release notes that get thrown away.

## install v1.6.0 — Cracked the .RED format

Reverse-engineered WCSC's `.RED` container format used by the PCBoard
installer. The 6 `.RED` archives inside `INSTALL.zip` (COMMDRV, PCBCFGS,
PCBMAIL, PCBOARD, PCBOARD2, PPLC) unpack cleanly with our new `redx`
tool at `pcb1541/install/archivers/redx/`.

Three bugs squashed getting to byte-perfect decode:
- **Decoder**: `[0x76]!=0` mode embeds CRC-CCITT bytes every 4094 stream
  bytes. Missing those broke every 4th chunk.
- **Parser**: `payload_offset` is always `pos+41` (fixed header),
  not `fn_end+1` (variable filename end).
- **Decoder**: initial `skip = 0`, not the buggy heuristic
  `(cmp[0]==0)?3:2` that ate the filename slot padding.

Result: **257/257 files decode byte-perfect** across all 6 archives.

## install v1.6.1–install v1.6.5 — Consolidating the tree

- Extracted the 257 files and organized them
- Discovered `INSTALL.DAT` — the rosetta stone. 471 `@File <src> @Out
  <dst>` directives mapping each source to its target install path.
- Consolidated everything under `pcb1541/install/dist/target/` matching
  the real `C:\PCB\` install layout (448 file placements)
- Moved `archivers/` → `pcb1541/install/archivers/` (its natural home
  as a subsystem of the installer)
- Full source release shipped: 10,387 tracked files, 836 MB uncompressed

## install v1.6.3 CRITICAL BUG discovered later

`MANIFEST.txt` LIED. The install v1.6.3 placement script naively mapped every
`INSTALL.DAT @File` directive against ALL `PCBCFGS.RED` records,
producing **18 bogus files** at `target/` root — right filename, wrong
content entirely.

Example: `@File 4 @Out PCBSM.EXE` found a source key `4` in `PCBCFGS.RED`
(a 293-byte text stub) and copied it as `PCBSM.EXE`. The real
`PCBSM.EXE` is a **440 KB DOS executable**. My script produced a
293-byte fake with the right name.

Root cause: `INSTALL.DAT` has 4 `@DefineDisk` sections. The same source
key (`4`, `A`, `U`) resolves to **different files** on different disks.
`PCBCFGS.RED` holds Disk 3's small stubs. The real executables were on
disks whose archives we didn't have yet.

The 18 bogus files: `DOORWAY.EXE`, `ENCRYPT.EXE`, `FIDOUTIL.EXE`,
`FIXTEXT.EXE`, `INIT.EXE`, `PCBSM.EXE`, `PCBSTATS.EXE`, `RDPCBTXT.EXE`,
`TESTFILE.EXE`, `UPGRADE.EXE`, `USERNET.EXE`, `UUIN.EXE`, `UUOUT.EXE`,
`UUUTIL.EXE`, `UUXFER.EXE`, `VIEWZIP.EXE`, `ZMRECV.EXE`, `ZMSEND.EXE`.

Plus one hidden victim: `VIEWARCH.COM` shipped as a bogus 308-byte stub
instead of the real 2280-byte DOS COM.

## install v1.6.6 — CRLF fix (prepared, never applied, obsoleted by install v1.7.2)

Windows Git autocrlf stripped `\r` bytes from 127+ files under
`target/` after the install v1.6.5 push. A repair zip was prepared but never
applied — install v1.7.2's `.gitattributes binary` policy solves this class of
bug wholesale.

## install v1.7.0 — Extract-on-demand experiment (reverted in v1.7.2)

Tried "Option A": remove 383 extracted files from git, add
`rebuild.sh` / `rebuild.bat` / `rebuild_place.py` to regenerate them
on demand from `INSTALL.zip`. Rationale: files not tracked can't be
CRLF-mangled.

Also added a `PENDING` skip list in the placement script so the 18
bogus mappings could never accidentally get regenerated as fakes.

**Reverted in install v1.7.2** — losing tracked files meant losing the audit
trail and losing the browse-on-GitHub experience. The correct fix was
CRLF protection, not untracking.

## install v1.7.1 — Recovered the 18 missing files (PCBDISK.002 + PCBDISK.003)

You uploaded 4 disk zips (`abpb1531.zip` through `abpb1534.zip`).
`abpb1532.zip` had **`PCBDISK.002`** (1.4 MB, Disk 2 archive).
`abpb1534.zip` had **`PCBDISK.003`** (830 KB, Disk 3 archive). Both
`.RED` format — `redx` unpacked them clean.

- Added `PCBDISK.002` + `PCBDISK.003` to `INSTALL.zip` (now 12 files
  inside, was 10)
- Rewrote `rebuild_place.py` with **disk-aware placement**: walks
  INSTALL.DAT's 4 `@DefineDisk` sections separately, resolves each
  `@File` against the correct source archive by `@Size`
- 10 size-verified byte-perfect matches: `DOORWAY.EXE` (27827),
  `ENCRYPT.EXE` (11245), `PCBSM.EXE` (440328), `PCBSTATS.EXE` (12696),
  `RDPCBTXT.EXE` (6911), `TESTFILE.EXE` (2534), `UPGRADE.EXE` (7844),
  `USERNET.EXE` (12844), `UUIN.EXE` (266150), `UUOUT.EXE` (139976)
- Bonus: 14 more EXEs from `PCBDISK.002` we hadn't known were missing
  (`MAKEIDX`, `MKPCBMNU`, `MKPCBTXT`, `OVLSIZE`, `PACKFIDO`, `PCBDESC`,
  `PCBDIAG`, `PCBEDIT`, `PCBFILER`, `PCBMODEM`, `PCBMONI`, `PCBNLC`,
  `PCBPACK`, `PCBSETUP`)
- Removed `PENDING` list from `rebuild_place.py`
- Deleted `NOT_INCLUDED.md` (obsolete)
- Rebuild: **481 files placed, 0 missing, 0 skipped**

## install v1.7.2 — Track back the 481 files + real CRLF protection

Reversed the Option A untracking decision. All 481 rebuilt files are
back in git, tracked, byte-perfect.

Also discovered `VIEWARCH.COM` on origin was a **bogus 308-byte stub**
from the same install v1.6.3 mismapping bug — real file is 2280 bytes from
`PCBDISK.003` key `J`, confirmed by `INSTALL.DAT @Size 2280`.

**Real CRLF protection**: `.gitattributes` marks
`pcb1541/install/dist/target/**` as `binary` wholesale. Windows Git
autocrlf CAN'T strip `\r` from binary files. This is what v1.6.6 was
supposed to be — done for the entire tree, not just extensions git
recognized.

Result:
- **10,460 tracked files** total in the repo
- **487 files under target/** (481 rebuilt + 6 docs/scripts)
- All 22+ binaries verified byte-perfect against PCBDISK sources
- Rebuild scripts still work — they reproduce the tracked bytes exactly
- Both paths (tracked files, rebuild from source) converge

---

## For future contributors

- **The `.RED` archives in `INSTALL.zip` are the canonical source.**
  Every file under `target/` came from there.
- **`INSTALL.DAT` drives the placement.** Read
  `rebuild_place.py` to see the disk-by-disk resolution logic.
- **`MANIFEST.txt` is the human-readable source→target map.**
- **`AVAILABLE.md` is the inventory** (what came from which archive).
- **CRLF corruption is prevented by `.gitattributes`.** Don't remove
  the `pcb1541/install/dist/target/** binary` line.
- **If you need to modify a file under `target/`**, modify the `.RED`
  source instead, then rerun `rebuild.sh` — otherwise the tracked file
  and the rebuild output will diverge and confusion will follow.
