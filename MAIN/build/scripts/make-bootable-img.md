# Making attic/PCBBLDBT.IMG bootable

A reference procedure for turning the mountable-only attic snapshot
into a boot-capable HDD image. Not automated because it requires a
real FreeDOS session, which is more reliable on 86Box/PCem than in
DOSBox-X.

## Prerequisites

- Working DOSBox-X or 86Box with FreeDOS boot media (use
  `attic/fdboot_launch.img` as the boot floppy)
- `ms-sys` installed on the Linux host (build from
  https://github.com/pbatard/ms-sys)
- The current mountable `attic/PCBBLDBT.IMG`

## Sequence

1. **Extract files from the image** (mountable state):
   ```bash
   mkdir /tmp/img-files
   for d in $(mdir -/ -i attic/PCBBLDBT.IMG :: | grep '<DIR>' | awk '{print $1}'); do
       # ... use mcopy -s to pull each subtree
   done
   ```
   Or: for a one-liner recursive extract, use `mcopy -s -m -Q -i attic/PCBBLDBT.IMG "::/*" /tmp/img-files/`

2. **Boot into FreeDOS** using `attic/fdboot_launch.img` under DOSBox-X
   with a fresh 96 MB empty image mounted as C:

3. **In the FreeDOS session, run**:
   ```
   FORMAT C: /S
   ```
   The `/S` transfers system files (KERNEL.SYS, COMMAND.COM) and installs
   a proper FreeDOS boot sector on C:.

4. **Exit DOSBox-X**, the C: image now has a working FreeDOS boot sector
   plus KERNEL.SYS + COMMAND.COM at root.

5. **Copy the files back** onto the newly-formatted image using mtools:
   ```bash
   mcopy -s -m -Q -i attic/PCBBLDBT.IMG /tmp/img-files/* ::
   ```
   (Skip KERNEL.SYS/COMMAND.COM — FORMAT already put them there.)

6. **Test boot** in DOSBox-X:
   ```
   imgmount 2 attic/PCBBLDBT.IMG -t hdd -fs none -size 512,63,16,195
   boot -l c
   ```
   Should land at C:\> prompt with CONFIG.SYS multi-boot menu firing.

## Why not just use ms-sys directly?

ms-sys can write a FreeDOS FAT32 boot record to any file:
```bash
ms-sys -f --fat32free attic/PCBBLDBT.IMG
```
This runs and reports success. But the boot record ms-sys writes
expects a filesystem laid out by FreeDOS FORMAT (or a similar tool
that matches FreeDOS's BPB conventions). Our current image is
mformat-generated, which uses slightly different BPB defaults
(media byte, sectors-per-FAT alignment, root cluster location).
After ms-sys, mtools reports "Bad media types 52/f0" and boot -l c
hands off successfully but the FreeDOS loader hangs on the layout
mismatch.

The two paths that work reliably:
- FORMAT-first (as above): let FreeDOS lay out the FS, boot record
  is already right
- ms-sys-first + mkfs.msdos: use Linux `mkfs.msdos -F 32` to lay
  out the filesystem, then ms-sys to install the boot record. Not
  yet tested for our specific case.
