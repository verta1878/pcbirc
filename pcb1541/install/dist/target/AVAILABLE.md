# AVAILABLE — files extractable from INSTALL.zip

The full inventory of files that `rebuild.sh` / `rebuild.bat` regenerate
on demand from `pcb1541/install/INSTALL.zip`.

**481 file placements** across **22 target directories**
resolved from 8 source archives.

## Source archives (all inside INSTALL.zip)

| Archive | Kind | Provides |
|---|---|---|
| `COMMDRV.RED`  | .RED (LHA-lh5) | `COMMDRV/` tree (22 files) |
| `PCBCFGS.RED`  | .RED (LHA-lh5) | Disk 3 stubs (171 files across ADMIN/GEN/etc.) |
| `PCBMAIL.RED`  | .RED (LHA-lh5) | `PCBMAIL/` tree (4 files) |
| `PCBOARD.RED`  | .RED (LHA-lh5) | Root binaries (4 files: PCBOARD, PCBOARDM, PPLC100, PPLC330) |
| `PCBOARD2.RED` | .RED (LHA-lh5) | `PCBOS2/` tree (10 files) |
| `PPLC.RED`     | .RED (LHA-lh5) | `PPL/` tree (46 files) |
| `PCBDISK.002`  | .RED (LHA-lh5) | Disk 2 archive (203 files, inc. DOORWAY/ENCRYPT/PCBSM prep) |
| `PCBDISK.003`  | .RED (LHA-lh5) | Disk 3 binaries (24 files, inc. UUIN/UUOUT/ZMSEND) |

## By target directory

| Directory | Files |
|---|---:|
| `(root)/` | 55 |
| `ADMIN/` | 11 |
| `COMMDRV/` | 22 |
| `CUSTSRVC/` | 9 |
| `DL01/` | 2 |
| `DOC/` | 30 |
| `EMPLOYEE/` | 10 |
| `ENGINRNG/` | 9 |
| `FIDO/` | 18 |
| `FILES/` | 11 |
| `GEN/` | 95 |
| `GRAPHICS/` | 7 |
| `HELP/` | 63 |
| `HMNRSRC/` | 9 |
| `MAIN/` | 33 |
| `MIS/` | 9 |
| `PCBMAIL/` | 4 |
| `PCBOS2/` | 10 |
| `PPL/` | 42 |
| `PROD1/` | 11 |
| `PROD2/` | 11 |
| `SLSMKTNG/` | 10 |

## Files at target/ root (all executables + configs)

| File | Size |
|---|---:|
| `BOARD.BAT` | 325 |
| `DOORWAY.EXE` | 27,827 |
| `ENCRYPT.EXE` | 11,245 |
| `FIDOUTIL.EXE` | 214,586 |
| `FIXTEXT.EXE` | 34,886 |
| `INIT.EXE` | 25,092 |
| `MAKEIDX.EXE` | 78,452 |
| `MKPCBMNU.EXE` | 32,800 |
| `MKPCBTXT.EXE` | 62,668 |
| `MODEMS.DAT` | 69,264 |
| `OVLSIZE.EXE` | 9,318 |
| `PACKFIDO.EXE` | 23,214 |
| `PCBCMPRS.BAT` | 18 |
| `PCBDESC.EXE` | 17,214 |
| `PCBDIAG.EXE` | 121,802 |
| `PCBEDIT.EXE` | 119,824 |
| `PCBFILER.DEF` | 618 |
| `PCBFILER.EXE` | 318,992 |
| `PCBMODEM.EXE` | 159,462 |
| `PCBMONI.EXE` | 34,130 |
| `PCBNLC.EXE` | 301,258 |
| `PCBOARD.DAT` | 2,024 |
| `PCBOARD.EXE` | 1,034,096 |
| `PCBOARDM.EXE` | 990,944 |
| `PCBPACK.EXE` | 67,098 |
| `PCBQWK.BAT` | 72 |
| `PCBRB.BAT` | 154 |
| `PCBRH.BAT` | 108 |
| `PCBRZ.BAT` | 23 |
| `PCBSB.BAT` | 160 |
| `PCBSETUP.EXE` | 409,424 |
| `PCBSH.BAT` | 119 |
| `PCBSM.CLR` | 46 |
| `PCBSM.CNF` | 108 |
| `PCBSM.EXE` | 440,328 |
| `PCBSM.HLP` | 284,807 |
| `PCBSTATS.EXE` | 12,696 |
| `PCBSZ.BAT` | 23 |
| `PCBTEST.BAT` | 487 |
| `PCBVIEW.BAT` | 507 |
| `PPLC100.EXE` | 67,786 |
| `PPLC330.EXE` | 191,918 |
| `RDPCBTXT.EXE` | 6,911 |
| `REMOTE.SYS` | 122 |
| `TESTFILE.EXE` | 2,534 |
| `UPGRADE.EXE` | 7,844 |
| `USERNET.EXE` | 12,844 |
| `UUIN.EXE` | 266,150 |
| `UUOUT.EXE` | 139,976 |
| `UUUTIL.EXE` | 140,194 |
| `UUXFER.EXE` | 175,478 |
| `VIEWARCH.COM` | 2,280 |
| `VIEWZIP.EXE` | 8,006 |
| `ZMRECV.EXE` | 138,398 |
| `ZMSEND.EXE` | 129,310 |

## Related documents

- `MANIFEST.txt`      — full source key → target path map from INSTALL.DAT
- `README.md`         — target/ layout overview
- `rebuild.sh` / `rebuild.bat` — extract-on-demand scripts
- `rebuild_place.py`  — disk-aware placement logic
- `../archivers/redx/` — the .RED (un)packer used by rebuild scripts