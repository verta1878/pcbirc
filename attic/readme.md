# attic â working artifact snapshots

Reference snapshots that let us recover if the live tree gets corrupted.
Not built by CI; refreshed by hand when the live design changes.

| File | Provenance | Kept because |
|------|-----------|--------------|
| `PCBBLDBT.IMG` | 96 MB FAT16 HDD image built from `dosbox-x/pcbirc/BUILDROOT/` on 2026-08-29. Geometry: 195 cyl Ã 16 heads Ã 63 spt = 100,638,720 bytes. Contains full pcbirc build root: BC31 / MSC70 / TC201 compilers; HX / CWSDPMI / D32A DPMI extenders; 386MAX_S / PCB153 / TOOLKIT source trees; BUILD scripts; AUTOEXEC.BAT route dispatcher; six-route CONFIG.SYS menu; FDOS/ FreeDOS runtime. **`KERNEL.SYS` is first in root directory** (MS-DOS boot convention), followed by `COMMAND.COM`. | **Bootable HDD snapshot.** FreeDOS 1.3 FAT16 boot record installed via `ms-sys --fat16free`, drive number byte set to 0x80 for HDD. Boot in any emulator (86Box, PCem, QEMU, VMware, VirtualBox) or on real hardware by attaching as C: and setting BIOS to boot from HDD â lands at build environment with six-route CONFIG.SYS menu. Also mountable directly as C:: `imgmount 2 attic/PCBBLDBT.IMG -t hdd -fs fat -size 512,63,16,195`. ms-sys sourced from https://github.com/pbatard/ms-sys (GPL, by Henrik Carlqvist). |
| `FDBOOT.IMG` | 1.44 MB FreeDOS 1.3 floppy backup, snapshotted 2026-08-29 13:24 right before the `SYS C:` experiment that we learned from. Same FreeDOS 1.3 boot sector as `fdboot-stock.img` below; customized `FDAUTO.BAT` (122 bytes) chains to `C:\AUTOEXEC.BAT` if C: exists, else lands at `A:\>` prompt. | **Rescue floppy for boot-sector repair.** Both `PCBBLDBT.IMG` and `fdboot-stock.img` are bootable in their own right; this one is the recovery tool if either gets damaged. Ships with `BOOTFIX.COM` at `A:\BOOTFIX.COM` â from a live boot on real emulator or hardware, run `A:\> BOOTFIX C:` to reinstall a FreeDOS boot sector on C: without touching data, or `A:\> SYS A: C:` to reinstall boot sector + KERNEL.SYS + COMMAND.COM. Also preserves the pre-damage FDCONFIG.SYS + FDAUTO.BAT for reference. BOOTFIX.COM sourced from https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/repositories/1.3/util/bootfix.zip (GPLv2, by Arkady, v1.4a 2022-02-17). |


## What else is kept here

| Folder | Holds | Added |
|--------|-------|-------|
| `prebuilt-libs/BC31/` | 9 prebuilt PCBoard category libraries (`.386`, large model) found in `pcbcbase/PREBUILT/`, where they did not belong. Origin unknown, not rebuildable yet. Its README records every library's module list and sha256, which is the recipe for checking a future rebuild. | 2026-09-22 |
| `retired-build-files/` | 19 hand-made compiler/linker side files no build in this repo uses — BC 5.0 / TC 3.0 link lists, `INITBUILD.RSP`, the `…154.CFG` copies, and pcb153's `T.BAT` / `T.CMD`. Kept byte-for-byte with Clark's `\PROJ` paths, under their original folder paths. | 2026-09-22 |
| `virtual1/` | The near virtual-memory variant (`VIRTUAL1.C` / `VIRTUAL1.H`) for delta154, irc1541 and pwa153, deleted by commit 37bf4e4 and recovered from `37bf4e4^`. Nothing builds it; the branches that lost it use the huge `virtual.h` or the merged file. `pcb154/LIB/` still holds Clark's complete pair. | 2026-09-22 |
| `lib/pwa153/` | Earlier library-build staging and status notes. | before 2026-09 |

Nothing in `attic/` is built, read or linked by any makefile. Files come
here instead of being deleted, and each folder's README says what the
files are and what would be needed to bring them back.

## Refresh procedure for PCBBLDBT.IMG

When live `dosbox-x/pcbirc/BUILDROOT/` changes materially and the attic
snapshot needs updating, the full procedure is documented in
`MAIN/build/scripts/make-bootable-img.md` under "How the shipped image
was made". Summary:

```bash
# Requires: mtools, ms-sys (github.com/pbatard/ms-sys)
IMG=attic/PCBBLDBT.IMG
BLD=dosbox-x/pcbirc/BUILDROOT

mformat -C -T 196560 -h 16 -s 63 -H 0 -i $IMG ::
mcopy -i $IMG $BLD/KERNEL.SYS ::/          # first
mcopy -i $IMG $BLD/COMMAND.COM ::/         # second
# ... then the rest of BUILDROOT (see make-bootable-img.md)
ms-sys -f -p -H 16 --fat16free $IMG
python3 -c "open('$IMG','r+b').write(bytes([0x80]))"  # patch BPB byte 36
```

The ordering matters: `KERNEL.SYS` must be the first file in root for
MS-DOS boot loader compatibility. See `MAIN/build/scripts/make-bootable-img.md`
for the complete recipe with all subdirectories, verification steps,
and three repair paths if the boot sector ever gets damaged.
