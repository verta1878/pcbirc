# Runtime Dependencies — PCBoard 15.41

**Scope**: things PCBoard 15.41 needs at runtime that the WCSC install
disks do NOT ship. If you extracted `pcb1541/install/dist/target/` and
tried to run it on a period-appropriate DOS system, this document
lists what else you must obtain to make the BBS actually operate.

**Not runtime deps** (already bundled, do not re-source):

- `COMMDRV.EXE` + `COMMDV0[0-8].DRV` + `COMMTSR.EXE` + `DRVSETUP.EXE` +
  `TEST.EXE` + `MONITOR.BAT` — WCSC COMM-DRV serial-driver suite
- `XABIOS.BIN`, `XACOOK.BIN`, `XACOMX.BIN`, `BOCA1610.BIN` — COMMDRV
  internal driver blobs (BIOS-extender, cooked-mode overlay, COM
  extender, BOCA 16-port multiport card). These live under
  `target/COMMDRV/` and are extracted from `COMMDRV.RED` byte-perfect.
- `ARNETSP4.DAT` / `ARNETSP8.DAT` / `DIGI4E.DAT` / `DIGI8E.DAT` —
  COMMDRV card data files, same story.

The COMMDRV suite covers `C=COMM-DRV` mode in PCBSETUP. FOSSIL mode
(`F=FOSSIL`) needs an external driver — see below.

---

## Serial I/O layer

PCBSETUP's driver selection (per PCBSETUP.EXE strings):

    A=ASYNC     Internal 8250 UART driver (no external dep — pure DOS)
    C=COMM-DRV  WCSC COMMDRV suite (BUNDLED — no external dep)
    F=FOSSIL    Requires external FOSSIL driver
    O=OS/2      Requires OS/2 SIO — see pcb1541/OS2/ subsystem

### FOSSIL driver (required if `F=FOSSIL` selected)

Choose ONE of:

| Driver | Version | Notes |
|--------|---------|-------|
| **BNU** | 1.70 | David Nugent. Most popular. Small, stable. |
| **X00** | 1.53 | Ray Gwinn. More features, larger footprint. |
| **ADF**  | 5.10 | Advanced Digital FOSSIL. High-end multiport support. |

**Sources**:
- BNU 1.70: `archive.org/details/BNU170` or FidoNet software distribution
- X00 1.53: `archive.org/details/X00-153`
- ADF: `archive.org/details/adf510`

**Config**: load in `AUTOEXEC.BAT` before `BOARD.BAT`:

    BNU /L0=57600,8N1 /L1=57600,8N1

---

## Archiver externals

PCBoard uses external archiver programs for four purposes:
1. Building `FILES.BBS` archive-info from uploads (`@` command in file
   listings)
2. FIDO packet compression/decompression (via FIDOUTIL)
3. QWK mail packet handling (via `PCBPACK.EXE`)
4. Sysop maintenance (`PCBFILER.EXE` file operations)

PCBSETUP config keys (from the "Archiver Configuration" screen):

| Index | Format | Ext | Compressor      | Decompressor    | Bundled? |
|-------|--------|-----|-----------------|-----------------|----------|
| 0     | ZIP    | .zip| **PKZIP.EXE**   | **PKUNZIP.EXE** | No       |
| 1     | ARJ    | .arj| **ARJ.EXE**     | **UNARJ.EXE**   | No       |
| 2     | ARC    | .arc| **PAK.EXE** or **PKPAK.EXE** | **PAK.EXE** / **PKUNPAK.EXE** | No |
| 3     | LZH    | .lzh| **LHA.EXE**     | **LHA.EXE**     | No       |

**Recommended sources** (period-appropriate, DOS 16-bit):

- **PKZIP/PKUNZIP 2.04g** (PKWARE): `archive.org/details/PKZ204G.EXE`
  Widely regarded as the last stable "safe" PKZIP for BBS use.
- **ARJ 2.71** (Robert Jung): `archive.org/details/ARJ271`
- **LHA 2.13** (Haruyasu Yoshizaki): `archive.org/details/lha213`
  NOTE: the vendored **lha 1.14i** under
  `pcb1541/install/archivers/lha/` is for `.RED` archive
  compression work — separate purpose. Do not use it as a
  runtime archiver.
- **PAK 2.51** (NoGate Consulting) OR **PKPAK 3.61** — both handle
  legacy .ARC format. PKPAK is more common: `archive.org/details/pkpak361`

**Config**: PCBSETUP > Archiver Configuration. Enter the path to each
executable and any switches (defaults are fine for most).

---

## Transfer protocols

PCBoard 15.41 has some protocols built-in (Ymodem, Xmodem, Ymodem-G)
and expects external drivers for others. Configured via
`PCBPROT.DAT` — the "External Protocols" table in PCBSETUP.

The two you almost certainly want:

| Protocol | Program | Author | Notes |
|----------|---------|--------|-------|
| **Zmodem**       | `DSZ.COM` | Chuck Forsberg | THE Zmodem driver. Shareware. |
| **Zmodem-90**    | `GSZ.EXE` | Chuck Forsberg | Zmodem with resume, streaming. |
| **HS/Link**      | `HSLINK.EXE` | Sam Smith | Bidirectional Zmodem-alike. |
| **Puma**         | `PUMA.EXE` | Matthew Thomas | Fast bidirectional. |

**Sources**:
- DSZ / GSZ: `archive.org/details/dsz`  (Omen Technology)
- HS/Link: `archive.org/details/hslink122`
- Puma: `archive.org/details/puma120`

**Config**: edit `PCBPROT.DAT` via PCBSETUP > Transfer Protocols. Point
each entry at the installed executable and set the correct switches for
send/receive.

DSZ configuration example (in `PCBPROT.DAT`):

    Type: F                                (F = file-per-connection)
    Description: (Z) Zmodem
    Send command: DSZ port %2 speed %3 sz -m %4
    Recv command: DSZ port %2 speed %3 rz -m %5

---

## DOS environment

Nothing exotic. PCBoard 15.41 is a pure real-mode 16-bit DOS BBS.

| Component | Required? | Notes |
|-----------|-----------|-------|
| DOS 3.30+ | Yes       | Any DOS. 5.0 or 6.22 recommended. |
| `SHARE.EXE` | Yes    | File-locking support. Load in `AUTOEXEC.BAT`. |
| Memory manager | No | Runs fine in 640K. EMM386/QEMM helps free conv memory. |
| DPMI host | **No**  | PCBoard.exe is NOT a protected-mode program. |
| DOS extender | **No** | Same. |

If running in DOSBox or a modern DOSBox-X for development/testing,
default configuration works — no special extenders needed.

---

## OS/2 native path

If running via OS/2 SIO (`O=OS/2` in PCBSETUP), the deps shift:

- **OS/2 2.x or Warp 3+** — the OS itself
- **SIO/2 driver** (Ray Gwinn) — under `pcb1541/OS2/` in this repo
- **VSIO** — virtual SIO for DOS boxes under OS/2

See `pcb1541/OS2/README.md` for the OS/2-specific runtime path.

---

## Recommended minimum sysop kit

For a fresh install on period DOS (MS-DOS 6.22 target):

1. **Extract WCSC target/** to `C:\PCB\` (all 481 bundled files)
2. **BNU 1.70** to `C:\FOSSIL\BNU.COM` (if using FOSSIL)
3. **PKZIP/PKUNZIP 2.04g** to `C:\UTIL\` (for archive @-view)
4. **DSZ** to `C:\UTIL\DSZ.COM` (for Zmodem uploads)
5. **SHARE.EXE** loaded in `AUTOEXEC.BAT`

That's the minimum to have a working BBS with file transfer.

---

## What we CAN'T bundle (licensing)

- **PKZIP/PKUNZIP** — PKWARE proprietary, still under active enforcement
- **ARJ** — Robert Jung retains rights; freely redistributable but
  not GPL-compatible
- **DSZ/GSZ** — Omen Technology shareware; separate registration required
- **BNU/X00** — freely redistributable per their license terms; could
  be mirrored but we don't want to fork the maintenance burden

**What we CAN bundle**: nothing on this list, in the current
release. All runtime deps must be sourced by the sysop.

**What we SHOULD document**: known-good archive.org URLs (above), so
this list stays actionable even after the original distribution sites
disappear.

---

## Provenance

- WCSC install disk manifest (COMMDRV.RED + rest): fully bundled,
  see `pcb1541/install/dist/target/MANIFEST.txt`
- PCBSETUP archiver/protocol config schema: reverse-engineered from
  `PCBSETUP.EXE` strings during install v1.6 arc
- External-dep list: cross-referenced against `PCBSETUP.EXE` config
  screens ("Archiver Configuration", "External Protocols") + FidoNet
  SDS software distribution manifests (1994-1996 era)

Last updated: install v1.8.2 (2026-09-03)
