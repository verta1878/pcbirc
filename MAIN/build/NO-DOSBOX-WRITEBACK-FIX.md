# NO-DOSBOX-WRITEBACK-FIX.md

How to get reliable file writeback out of DOSBox-X when running
headless (no keyboard, no display) — the problem that blocked the
PCBKMS build.

## The symptoms (two separate failures, often confused)

1. **Non-deterministic writeback.** A directory `mount` (or even an
   `imgmount`) would write files on one run and silently lose them on
   an identical retry. The DOSBox-X log even showed
   `SHELL:Redirect output to <file>` — it *processed* the write — but
   the host file never appeared. A flush race on exit.

2. **The silent interactive hang.** `imgmount` of certain images stops
   on a prompt:
   "Mounting this type of disk images requires a reported DOS version
   of 7.10 or higher... Do you want to change reported DOS version?
   y/n:"
   Headless, there is no keyboard to answer, so DOSBox-X waits
   forever and the run times out with nothing written. This is the
   likely real cause behind most of the "non-deterministic" losses —
   the run wasn't racing, it was **stuck on the prompt**.

## The fix (in order)

### 1. Report DOS 7.1 BEFORE any imgmount
Set the reported DOS version to 7.10+ so the y/n prompt never fires.
Either in the config file:

    [dos]
    ver=7.1

or as an autoexec/`-c` command issued **before** the imgmount:

    ver set 7 10
    imgmount c C:\path\build.img -t hdd -fs fat

With the version already 7.1, DOSBox-X mounts without asking, and the
headless run proceeds instead of hanging.

### 2. Use an imgmount hard-disk IMAGE, not a directory mount
Directory (`mount c /host/dir`) writeback is the least reliable
headless. A FAT hard-disk image mounted with `imgmount ... -t hdd -fs
fat` flushes far more reliably. Build the image with mtools on the
host (mformat / mmd / mcopy), run the build against it, then read the
results back out with mtools. This is the combination that produced a
successful write in testing.

### 3. Force a clean shutdown so buffers flush
Headless DOSBox-X must exit cleanly for the image to flush. Use
`-exit` and end the command sequence with an explicit `exit`. Avoid
`-securemode` (it blocks exit). Give it a moment (`sleep 1`) before
reading the image back on the host.

### 4. Read results back with mtools, not by remounting
After the run, pull files out of the image on the host:

    mdir  -i build.img ::/OUT/LIB/PWA153/msc70/OBJ/medium
    mcopy -i build.img -s "::/OUT/*" /host/out/

No second DOSBox-X launch needed — mtools reads the FAT image
directly.

## Known-good headless invocation (shape)

    xvfb-run -a dosbox-x -silent -exit \
      -c "config -set dos ver=7.1" \
      -c "imgmount c /tmp/build.img -t hdd -fs fat" \
      -c "c:" \
      -c "call C:\BUILD\SCRIPTS\BLDKMS.BAT" \
      -c "exit"

(Then read OBJs/LIBs back out with mcopy.)

## The deeper point — high-level vs low-level emulation

Even with writeback fixed, DOSBox-X still cannot host **386MAX.SYS**.
DOSBox-X is a **high-level** emulator: it fakes DOS and provides its
own DPMI, and it will not let a guest CONFIG.SYS memory manager take
over the CPU. The MSC 7.0 DOS "3216" compiler hard-requires a real
32-bit DPMI host (386MAX / BlueMAX).

So writeback-fixed DOSBox-X is enough for compilers that DON'T need a
guest memory manager (Borland/Turbo C — PCBKBC/PCBKIT built fine), but
the MSC7 DOS route needs a **low-level** full-PC emulator (86Box,
PCem, or QEMU) running real MS-DOS + 386MAX. The OS/2 "1616" route
avoids DPMI entirely and needs an OS/2 host.

Rule of thumb:
- High-level emu (DOSBox-X): fast, good for straight compiles; cannot
  host guest memory managers / real-mode drivers.
- Low-level emu (86Box/PCem/QEMU): emulates the actual 386, boots real
  DOS, loads 386MAX — required for PCBKMS Route A.

## Status when this was written

- Writeback fix (DOS 7.1 + imgmount image + clean exit): identified,
  the DOS-7.1 prompt being the root cause of the headless hang.
- PCBKMS build: validated and ready (BLDKMS.BAT correct, 476 steps,
  headers MSC7-ready). Needs a low-level host for Route A, or OS/2 for
  Route B.

---

## ADDENDUM (WIP, 2026-08-26 late) — tested against DOSBox-X 2024.03.01 (Ubuntu)

**Read this before trusting the fix section above.** Findings from
tonight's re-test in a fresh sandbox with DOSBox-X 2024.03.01 (the
version currently in Ubuntu apt) contradict two things above. Leaving
the original text intact so we can reconcile once we know why the
behavior differs across DOSBox-X versions.

1. **Writeback inversion.** In this DOSBox-X, `mount c /host/dir`
   (directory mount) writes back reliably — every redirect and every
   file created lands on the host, even with a plain `exit`. In
   contrast, `imgmount c build.img -t hdd -fs fat` mounts fine but
   *does not flush to the .img file*, even with `-rw`, `imgmount -u c`
   before exit, `sync`, or clean `-exit`. Image is byte-identical
   before/after (md5 confirmed). This is the reverse of what the fix
   section above records. Possible causes to check: (a) 2024.03.01
   changed imgmount flush semantics, (b) the earlier "successful"
   imgmount was against a different DOSBox-X build, (c) an option like
   `locking disk image mount` that the reference conf mentions.
2. **DOS-7.1 y/n prompt did NOT appear** during imgmount in this
   version, with or without `[dos] ver=7.10`. The prompt fix is still
   correct in principle but wasn't the actual failure mode this time —
   imgmount just silently discarded writes.
3. **PCBKMS is blocked earlier than expected: MSC7 CL.EXE fails at
   load with R6901.** Even with writeback proven via dirmount, the
   very first `CL /?` fails:
   ```
   run-time error R6901
   - DOSX32 : This is a protected-mode application that requires DPMI
   (DOS Protected Mode Interface) services.
   ```
   Confirmed with `[dos] ems=emm386` too (still R6901). DOSBox-X's
   built-in shell does not expose DPMI to guests — you get XMS + EMS +
   HMA but no INT 2Fh/AX=1687h host. So the high-level-vs-low-level
   note in the fix section is right in conclusion but the failure mode
   is "no DPMI host at all", not "386MAX won't load". Loading 386MAX
   under the built-in shell isn't even attempted because there's no
   CONFIG.SYS phase.

**Path forward being tried next:** DOSBox-X `BOOT -l c` command
(boots real DOS off a hard-disk image, runs the guest CONFIG.SYS —
386MAX *can* load in that mode because DOSBox-X hands over to the
booted kernel). If BOOT-mode works, we get DPMI + CL.EXE without
leaving DOSBox-X. If it doesn't, next stop is QEMU + FreeDOS +
386MAX.

**Status:** WIP. Do not rewrite the fix section above until BOOT-mode
is tested; both truths may coexist across DOSBox-X versions and this
addendum protects against a mid-session crash losing the finding.

