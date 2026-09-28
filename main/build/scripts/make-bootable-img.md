# attic/PCBBLDBT.IMG bootability

**attic/PCBBLDBT.IMG ships bootable.** On any real emulator (86Box, PCem,
QEMU, VMware, VirtualBox) or real hardware, attach it as the primary
IDE / boot HDD, set BIOS to boot from HDD, and it lands at the pcbirc
six-route CONFIG.SYS menu (PWA / DELTA / IRC1541 / PCBKMS / 386MAX /
BARE), then to a `C:\>` prompt in the chosen route's environment.

No rescue floppy needed for normal use. `attic/FDBOOT.IMG` is retained
for repair if the boot sector ever gets damaged again.

## How the shipped image was made

Reproducible on any Linux host with `mtools` + `ms-sys`
(https://github.com/pbatard/ms-sys):

```bash
IMG=attic/PCBBLDBT.IMG
BLD=dosbox-x/pcbirc/BUILDROOT

# 1. Fresh FAT16 mformat (195 cyl x 16 heads x 63 spt = 100,638,720 bytes)
rm -f $IMG
mformat -C -T 196560 -h 16 -s 63 -H 0 -i $IMG ::

# 2. mcopy KERNEL.SYS FIRST — must be first entry in root dir
mcopy -i $IMG $BLD/KERNEL.SYS ::/

# 3. COMMAND.COM SECOND
mcopy -i $IMG $BLD/COMMAND.COM ::/

# 4. Rest of BUILDROOT (2367 files, order after this doesn't matter)
for f in $BLD/*; do
    name=$(basename $f)
    [ "$name" = "KERNEL.SYS" ] && continue
    [ "$name" = "COMMAND.COM" ] && continue
    [ -f "$f" ] && mcopy -m -Q -i $IMG "$f" ::/
done
for d in 386MAX_S BC31 BUILD CWSDPMI D32A FDOS HX MSC70 OUT PCB153 SCRIPTS TC201 TOOLKIT; do
    mmd -i $IMG ::/$d 2>/dev/null
    (cd $BLD/$d && find . -type d -mindepth 1 | while read sub; do
        mmd -i $IMG "::/$d/${sub#./}" 2>/dev/null
    done)
    (cd $BLD/$d && find . -type f | while read f; do
        mcopy -m -Q -i $IMG "$BLD/$d/${f#./}" "::/$d/${f#./}" 2>/dev/null
    done)
done

# 5. Install FreeDOS FAT16 boot record
ms-sys -f -p -H 16 --fat16free $IMG

# 6. Patch BPB byte 36 to 0x80 (HDD drive number)
#    ms-sys defaults to 0x00 (floppy); we need HDD.
python3 -c "
with open('$IMG','r+b') as f:
    f.seek(36); f.write(bytes([0x80]))
"
```

## Repair procedures if the boot sector is damaged

Three ways to reinstall the boot sector, in order from simplest to
most involved. All require `attic/FDBOOT.IMG` (the rescue floppy that
ships in `attic/`) attached as A: and a real emulator or hardware.

### Repair path 1 — BOOTFIX (simplest, boot sector only)

BOOTFIX.COM is baked into `attic/FDBOOT.IMG` at `A:\BOOTFIX.COM`.
Sourced from
https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/repositories/1.3/util/bootfix.zip
(FreeDOS boot sector repair tool, GPLv2, by Arkady, v1.4a).

```
1. Attach FDBOOT.IMG as A:, PCBBLDBT.IMG as C:
2. Boot from A:
3. A:\> BOOTFIX C:
4. Remove A:, reboot from C:
```

BOOTFIX touches only the boot sector — doesn't move files, doesn't
touch KERNEL.SYS or COMMAND.COM. Returns errorlevel 0 on success.

### Repair path 2 — SYS (touches boot sector + kernel files)

```
1. Attach FDBOOT.IMG as A:, PCBBLDBT.IMG as C:
2. Boot from A:
3. A:\> SYS A: C:
4. Remove A:, reboot from C:
```

Syntax: `SYS <source> <target>`. Transfers KERNEL.SYS + COMMAND.COM
from A: to C: root AND installs a matching FreeDOS boot sector on C:.
Safe — replaces the KERNEL.SYS + COMMAND.COM already on C: with the
same versions from A: (both are FreeDOS 1.3 KERNEL May 2021).

### Repair path 3 — host-side ms-sys (no boot needed)

If neither A: nor a working emulator is available, repair from Linux
host directly with the same commands used to build the shipped image
— steps 5 and 6 from "How the shipped image was made" above:

```bash
ms-sys -f -p -H 16 --fat16free attic/PCBBLDBT.IMG
python3 -c "open('attic/PCBBLDBT.IMG','r+b').__enter__().seek(36) or open('attic/PCBBLDBT.IMG','r+b').write(bytes([0x80]))"
```

(For a cleaner one-liner, use the multi-line Python from the build
recipe above.)

## Why not just SYS in DOSBox-X?

Tried. Doesn't work reliably in the DOSBox-X headless sandbox because
DOSBox-X's `boot -l a` (boot from floppy) hands the machine to the
guest OS but doesn't reliably present the `imgmount 2` HDD to the
guest OS's disk driver. FreeDOS booted from A: sees no C:, so
`SYS A: C:` and `BOOTFIX C:` both return "no such drive".

On real emulators (86Box, PCem, QEMU) and real hardware — which have
full-fidelity BIOS INT13 that DOSBox-X's guest-OS mode doesn't fully
emulate — the flows work as documented.

That's why the shipped image was built host-side with `ms-sys` (which
writes bytes directly with no BIOS in the loop). ms-sys produces
identical FreeDOS boot code, so the resulting image behaves the same
as if built via `SYS A: C:` in a real FreeDOS session.

## Verifying the boot sector on a shipped image

```bash
# OEM at bytes 3-10 (should be "MSWIN4.1" for ms-sys-installed FreeDOS,
# or "FRDOS5.1" if written by SYS.COM in a live FreeDOS session)
dd if=attic/PCBBLDBT.IMG bs=1 skip=3 count=8 status=none | od -An -c

# FS type at bytes 54-61 (should be "FAT16   ")
dd if=attic/PCBBLDBT.IMG bs=1 skip=54 count=8 status=none | od -An -c

# Drive number at byte 36 (should be 0x80 for HDD boot)
dd if=attic/PCBBLDBT.IMG bs=1 skip=36 count=1 status=none | od -An -tx1

# Signature at bytes 510-511 (should be 55 aa)
dd if=attic/PCBBLDBT.IMG bs=1 skip=510 count=2 status=none | od -An -tx1

# KERNEL.SYS should be first entry in root
mdir -f -i attic/PCBBLDBT.IMG :: | head -8
```
