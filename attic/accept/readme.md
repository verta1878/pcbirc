# ACCEPT — the PACKFIDO acceptance test

Three files. Each does one thing and is named for it.

| File | What it is |
|---|---|
| `MKPFDATA.C` | **makes the PACKFIDO test data** — a version 2 `PCBFIDO.CFG`, an identical `ORIG.CFG`, `CNAMES.@@@`, `CNAMES.ADD` and a `PCBOARD.DAT` |
| `FILEOPS.C` | **copy and byte-compare**, setting `ERRORLEVEL` — a generic replacement for `COPY` and `FC`, which cannot be used here |
| `PFACCEPT.BAT` | **the test** — runs both programs and asks `FILEOPS /D` for the verdict |

`MKPFDATA` follows Clark's own naming: `MKPCBTXT.C`, `MKPCBMNU`, `MKUNIQUE.C`
— `MK` plus what it makes.

## Running it

    mount c <the pcbircrevival folder>      DOSBox-X; or SUBST on real DOS
    BCC -ml -ePACKFIDO.EXE \pcb153\SOURCE\MISC\PACKFIDO\PACKFIDO.C
    BCC -ml -eMKPFDATA.EXE MKPFDATA.C
    BCC -ml -eFILEOPS.EXE  FILEOPS.C
    PFACCEPT

Run `PFACCEPT` from any directory on any drive letter. Every path it uses
starts at `\` — the repo folder mounted as the drive root, the convention
`BLDTK.BAT` already uses — and it checks for `\APPLY.txt` first to prove
the mount is right. There is no `..\..\` walk anywhere, in the batch or in
the C, because a relative walk works from exactly one directory and passes
its own test for exactly that reason.

## What the test actually is

Run our `PACKFIDO.EXE` and Clark's shipped `PACKFIDO.EXE` over identical
copies of the same file and diff the two outputs byte for byte. Anything
less is not an acceptance test — compiling certainly isn't.

    530 areas in, 30 removed, 122,535 bytes out
    sha256 e995b7a8f4f5a3f7aeda0cd2a057dd6db8f2e19e7b9098f4005841860a413fd0
    from both programs

The version 2 layout is written from scratch rather than found, because
the only Fido data in the archive is version 3 and the 1994 binary cannot
read it. It is not guessed: Clark's own converter reads a version 2
`PCBFIDO.CFG` field by field in `MISC/FIDOUTIL/SOURCE/CONVERT.CPP`.

The fixture covers every branch of the pack decision — 500 ordinary areas,
conference 0, conference exactly `NumConf` (kept), one past it (dropped),
65535 (dropped), a conference with no `ConfType` (dropped), a Fido
conference with no message base (dropped), and twelve conferences
appearing three and four times in shuffled order so "first" is not "lowest
numbered". The shuffle is a fixed-seed 16-bit LCG, so the fixture is
identical on every compiler and every run; a byte diff is worthless
otherwise. The trailer is 1,031 arbitrary bytes — not a multiple of 1,024,
so the short final chunk of the copy loop is exercised too.

## Two things that bit us, written down so they don't again

### The test cannot run in this directory

`PcbData.FidoConfig` is `char[33]` and `PcbData.CnfFile` is `char[32]`, so
the paths `PCBOARD.DAT` carries have 32 and 31 usable characters.

    \PCB153\SOURCE\MISC\PACKFIDO\ACCEPT\PCBFIDO.CFG     47 characters

Clark's binary truncates it, opens nothing, prints `done.` and changes no
file — **a silent pass that proves nothing**. That happened once during
this work. So the test runs in a short directory, `\PFTEST` by default,
and `MKPFDATA` refuses with the arithmetic printed rather than write a
`PCBOARD.DAT` that cannot work.

### `COPY` does not work from inside a batch file under DOSBox-X

Measured with `MKDIR` markers, not assumed: `COPY` works from the autoexec
and silently does nothing from a batch — no error, no file, `ERRORLEVEL`
unchanged. The first version of this harness ran Clark's binary, failed to
make the restore copy, and would have compared stale files. `FC` is worse:
an external command that is not on every DOS and is not in DOSBox-X, and
capturing its verdict needs a redirect, which fails for the same reason.

Hence `FILEOPS.EXE`, and hence `PFACCEPT.BAT` using nothing but EXE calls,
`IF ERRORLEVEL`, `IF EXIST`, `CD` and `ECHO`.

## What it leaves behind

`\PFTEST` (or the directory you named), holding the fixture, `CLARK.OUT`
and `OURS.OUT`, so a failure can be examined. Nothing in the source tree is
written to or deleted. The script CDs into the test directory and cannot
come back — Clark's binary reads a bare `PCBOARD.DAT` from the current
directory and `COMMAND.COM` has nowhere to remember the old one.
