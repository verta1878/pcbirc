# attic — working artifact snapshots

Reference snapshots that let us recover if the live tree gets corrupted.
Not built by CI; refreshed by hand when the live design changes.

| File | Provenance | Kept because |
|------|-----------|--------------|
| `PCBBLDBT.IMG` | 96 MB FAT16 HDD image built from `dosbox-x/pcbirc/BUILDROOT/` on 2026-08-29. Geometry: 195 cyl × 16 heads × 63 spt = 100,638,720 bytes. Contains full pcbirc build root: BC31, MSC70, TC201 compilers; HX, CWSDPMI, D32A DPMI extenders; 386MAX_S / PCB153 / TOOLKIT source trees; BUILD scripts; AUTOEXEC.BAT route dispatcher; six-route CONFIG.SYS menu; FDOS/ FreeDOS runtime. | **Standalone mountable snapshot** — if the DOSBOXX.ZIP bundle or the live `dosbox-x/pcbirc/BUILDROOT/` tree ever gets corrupted, this image can be mounted directly (`imgmount 2 attic/PCBBLDBT.IMG -t hdd -fs fat -size 512,63,16,195`) as C: and all build tools + sources are recoverable. **Not currently bootable** — mformat produced a stub boot sector. To make bootable: format the image using FreeDOS `FORMAT.COM` (not `mformat`) in a live FreeDOS session, then write a FreeDOS boot record with `ms-sys --fat32free -f attic/PCBBLDBT.IMG` on the host, then boot-verify under 86Box or real hardware. Attempted here with the mformat-then-ms-sys path; ms-sys wrote the boot record successfully but the BPB layout mismatches what FreeDOS bootloader expects on an mformat-generated filesystem, so boot hangs after handoff. Needs the FORMAT.COM-first sequence to work reliably. |
| `fdboot_launch.img` | 1.44 MB FreeDOS 1.3 floppy backup, snapshotted 2026-08-29 13:24 right before the `SYS C:` experiment that we learned from. | **Known-good bootable floppy** — the only artifact in the repo with a working FreeDOS boot sector. Use as A: in DOSBox-X to boot into FreeDOS at any time. From that session, `SYS C:` can install a boot sector on `PCBBLDBT.IMG` to make it boot-capable, or on any fresh disk image. Also preserves the pre-damage FDCONFIG.SYS + FDAUTO.BAT baseline for reference. |

## Refresh procedure for PCBBLDBT.IMG

When live BUILDROOT/ changes materially and you want the attic snapshot to
reflect the new state:

```bash
# On host (needs mtools):
mformat -C -T 196560 -h 16 -s 63 -H 0 -i attic/PCBBLDBT.IMG ::
mcopy -s -m -Q -i attic/PCBBLDBT.IMG \
      dosbox-x/pcbirc/BUILDROOT/* ::
```

For a bootable image, follow with a live-boot `SYS C:` step (see the .BAK
file's Kept-because column above).
