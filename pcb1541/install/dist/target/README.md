# pcb1541/install/dist/target/

The **fully-installed PCBoard 15.41 directory structure** as it would appear
on a target machine (typically `C:\PCB\`) after running the original
`INSTALL.EXE` against `INSTALL.zip`.

Built by extracting all 6 `.RED` archives via `redx extract`, then mapping
each source file to its target location using the `@File ... @Out ...`
directives in `INSTALL.DAT`.

## Layout matches the shipped installer's output

```
target/
├── PCBOARD.EXE, PCBOARDM.EXE   Main BBS binaries
├── PPLC100.EXE, PPLC330.EXE    PPL compilers (installer picks one → PPLC.EXE)
├── PCBSM.EXE, DOORWAY.EXE, ... 26+ utility binaries at root
├── PCBSH.BAT, PCBSZ.BAT, ...   Launcher batch files
├── REMOTE.SYS, PCBSM.CLR/CNF   System config files
│
├── COMMDRV/          22 files  WCSC CommDrv (main driver package)
│   ├── COMMDRV.EXE, COMMTSR.EXE, DRVSETUP.EXE, TEST.EXE
│   ├── COMMDV00-08.DRV         (9 backend drivers)
│   ├── XABIOS/XACOOK/XACOMX/BOCA1610.BIN (board firmware)
│   ├── ARNETSP4/8.DAT, DIGI4/8E.DAT (sample TSR configs, text)
│   └── MONITOR.BAT
│
├── PCBOS2/           10 files  OS/2 support tree (from PCBOARD2.RED)
├── PCBMAIL/           4 files  QWK mailer + Borland runtime
├── PPL/              42 files  PPL sample programs (.PPE + .PPS)
├── HELP/             63 files  Online help topics
├── MAIN/             24 files  Message-base skeleton (CNAMES.*)
├── DOC/              30 files  Documentation
│
├── ADMIN/, CUSTSRVC/, EMPLOYEE/, ENGINRNG/, FIDO/, FILES/, GEN/,
│   GRAPHICS/, HMNRSRC/, MIS/, PROD1/, PROD2/, SLSMKTNG/, DL01/
│                              Clark's sample conference/subdir tree
│
└── MANIFEST.txt       Full source→target mapping (from INSTALL.DAT)
```

Total: 448 file placements resolving from 257 unique sources extracted
from the 6 `.RED` archives in `INSTALL.zip`.

## Provenance chain

```
pcb1541/install/INSTALL.zip           original WCSC installer disk image
        │
        ├── INSTALL.EXE               original installer (redx replicates it)
        ├── INSTALL.DAT               installer script (defines all @File → @Out maps)
        ├── COMMDRV.RED, PCBCFGS.RED, PCBMAIL.RED,
        │   PCBOARD.RED, PCBOARD2.RED, PPLC.RED
        │           │
        │           │  extracted via `redx extract`
        │           ▼
        │   257 unique source files (short-name keys and long names)
        │           │
        │           │  mapped to target paths via INSTALL.DAT
        │           ▼
        └── dist/target/              ← THIS DIRECTORY
                (448 file placements, byte-perfect against a real install)
```

## Why some sources appear multiple times

The installer's `@File` directives can map the same source key to
different paths depending on install group. For example, PCBOARD.EXE
lands at root; small PCBCFGS entries like "M4" (which is `PCBSETUP.EXE`)
also land at root; some PCBCFGS entries appear in multiple places if
Clark shipped the same tiny stub for related sample subdirs.

When we hit duplicates during layout generation, both copies are kept
with a suffix disambiguator (e.g. `CNAMES` and `CNAMES.62`) so nothing
is lost. See `MANIFEST.txt` for the exact source-to-destination map.

## Sources with source code available

Some target binaries have known upstream source in this repo:

| Target | Source tree |
|---|---|
| `PCBOARD.EXE`, `PCBOARDM.EXE` | `pcb1541/`, `pcb154/`, `pcb153/` (WATCOM builds) |
| `COMMDRV/COMMDRV.EXE` etc.    | `pcb154/pcbdcom/`, `pcb1541/pcbdcom/` (pcbdcom rewrite) |
| `PPLC100.EXE`, `PPLC330.EXE`  | `toolkit/pplc/` (partial: 3.00/3.10/3.20 present) |
| `PPL/*.PPS` sources           | These ARE the source; `.PPE` files rebuild from them |

Everything else in this tree is a compiled binary or data file with no
source available; it lives here as archival material and as the byte-perfect
parity target for anyone re-implementing the corresponding piece.

## License

WCSC / Clark Development Corp freely redistributable per post-market-exit
community consensus. Borland DLLs (`BWCC.DLL`, `BC450RTL.DLL` under
`PCBMAIL/`) remain Borland's IP; included solely to allow the original
`PCBMAIL.EXE` to run.
