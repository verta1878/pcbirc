# packfido — two files, two jobs

`packfido.c` is not in this repo, not in the archive, and not in any Clark
material we have. It lived at `E:\TC\PACKFIDO\PACKFIDO.C` on the
developer's Turbo C scratch drive, outside the product tree the source
archive was made from. That is why no copy survives, and it is not coming
back.

There are two reconstructions, because one file cannot do both jobs:

| File | Reads | Target |
|---|---|---|
| `pcb153\SOURCE\MISC\PACKFIDO\PACKFIDO.C` | `PCBFIDO.CFG`, 15.21 layout | **byte-exact against Clark's shipped `PACKFIDO.EXE`** |
| `pcb154\MAIN\SOURCE\MISC\PACKFIDO\PACKFIDO.C` | `AREAS.DAT`, 15.22+ layout | **the one you actually run** |

Clark's shipped binary is dated `11-10-94` and reads 243-byte
`AREA_STRUCT` records out of `PCBFIDO.CFG`. 15.22 moved the areas into
`AREAS.DAT` with a different record, so **the shipped binary does nothing
useful on a 15.3 or 15.4 board** — and a rebuild that matches it byte for
byte inherits that exactly. Matching the binary and working on a current
board are two different deliverables. Pretending one file is both is how
you end up with neither.

## PACKFIDO was abandoned at 15.21, and Clark's own files say so

The 15.4 file is not a nice-to-have. It is **the upgrade the 15.4 install
set never included**, and three independent pieces of evidence say so.

**1. Clark's own documentation.** `pcb1541/install/dist/target/DOC/PACK.DOC`,
copyright 1995, shipped unchanged in the 15.41 install set, describes a
utility *"written for PCBoard 15.21 to pack the FIDO configuration file
PCBFIDO.CFG."* PCBFIDO.CFG stopped holding the areas at 15.22. (The doc is
even headed `PCBPACK.EXE` — a copy-paste slip; the body is all PACKFIDO.)

**2. Version strings in the shipped binaries.** Every program in that
install set that touches current data carries a 15.3 string:

    PCBOARD  PCBOARDM  PCBSM  PCBPACK  PCBSETUP  MKPCBTXT
    UUIN  UUOUT  UUXFER  PCBMODEM  PPLC330  PCBFILER      → 15.3
    FIDOUTIL  — the 15.21→15.22 converter                 → 15.0, 15.21, 15.22
    PACKFIDO                                              → 15.0, and nothing later

FIDOUTIL knew about the new format. The program whose entire job is packing
the file FIDOUTIL had just converted never learned it.

**3. The converter's own source.** In `MISC/FIDOUTIL/SOURCE/CONVERT.CPP`,
the call to packfido's one exported symbol is commented out:

    //do_pack();

The pack step was disabled at conversion time rather than fixed.

So a 15.3 or 15.4 sysop had a `PACKFIDO.EXE` on disk, documented as a 15.21
tool, that could not read the file it was pointed at. `pcb154\MAIN\SOURCE\MISC\PACKFIDO\PACKFIDO.C`
is the successor that was never written, and `PACKFIDO.DOC` beside it is
the documentation that was never written either.

PACK.DOC also settles the shape of the 15.3 rebuild independently of the
disassembly: *place it in a directory which contains a copy of PCBOARD.DAT,
then type PACKFIDO.* No arguments. That is exactly what the byte-exact
target had to be.

## What PACKFIDO does

    pcb1541\install\dist\target\PACKFIDO.EXE   23,214 bytes
    sha256 ef584fc957a28d051a7c40937ed5eed916dfd043bc9ae7ca3f2996fc6a8f63a8
    Borland C++, banner 1991, internal date string 11-10-94

It compacts the Fido area configuration: it drops area records whose
conference is out of range, whose conference is not a Fido conference, or
that duplicate an area already kept. It is to the Fido config what PCBPACK
is to the message bases. **It is not a FidoNet packet packer** — an early
note here guessed that, and the FTS specs in `docs/fido/` are not what it
implements.

## The 15.3 file — aiming at the bytes

The only test that matters is the sha256 against Clark's binary. If the
bytes match, behaviour matches by construction, and no behavioural harness
is needed.

What the binary itself gave up, measured:

| Evidence | What it means |
|---|---|
| 8 relocations, 512-byte header | **small model** — `MKPCBTXT.EXE`, which Clark built with `MDL=s`, has 10 |
| `"Unable to open PCBOARD.DAT"` | links the kit — `readdatfile()` from `DATAFIL2` |
| `"15.0"` present, `"14.5"` absent | built **without** `-DLIB`, the `#else` arm of the version check |
| the three CNAMES messages | **not in any library we hold** — so they are in `PACKFIDO.OBJ`: the program opens `CNAMES.@@@` / `.ADD` and checks the RecSize header itself |
| `"print scanf : floating point formats not linked"` | `-f-`, no floating point |

So the shape is `readdatfile()`, then its own CNAMES scan, then the pack.
**No argv anywhere** — `PcbData.CnfFile`, `PcbData.FidoConfig` and
`PcbData.NumConf` come from `PCBOARD.DAT` lines 31, 246 and 108.

`PACKFIDO.MAK` is modelled on Clark's own `MKPCBTXT.MAK` — same switches,
same link shape, same small model. `-P` is not optional: the kit libraries
are C++ objects, and a C compile leaves every kit symbol undefined.

### Where it stands: 1,276 bytes short, and we know why

    ours     21,938 bytes    small model, 512-byte header
    Clark's  23,214 bytes

It does not link yet. Four symbols are missing, all of them **the kit's own
dependencies** on category libraries that do not exist in small model:

    retrycount()       MISC, wanted by CHKAPPEN
    findstartofname()  MISC, wanted by DATAFIL2
    _int23hnd          DOS,  wanted by HANDLERS
    _int24hnd          DOS,  wanted by HANDLERS

`toolkit\pwa153\bc31\lib` holds `MISC_L`, `DOS_L` and the rest in **large
model only**, plus `PCBKBCS/C/M/L`. The small-model category libraries are
the "4 models" leg of the SDK matrix that is still open — `TK.CFG` has
`-ml` baked in, so MODEL is currently cosmetic. Build those and this links.

## The 15.4 file — the one you run

    usage: PACKFIDO <areas.dat> <cnames> <numconf> [/R]

Reads `AREAS.DAT`: a 2-byte file version of 3, then `NAREA_STRUCT` records
to end of file. No count, no trailer.

**The record size is probed, not assumed.** Three sources, three answers,
and the file wins:

| Source | Says |
|---|---:|
| `STRUCTS.H`, `sizeof(NAREA_STRUCT)` | 81 |
| `FIDO.DOC`, offsets 2..82 | 81 |
| **Clark's own `AREAS.DAT`, measured** | **71** |

    48,566 − 2 = 48,564 ;  48,564 / 71 = 684.0000 exactly
    684 of 684 records sane.  At 81: 45 bytes over, 9 of 599 sane.

The 10 bytes are the trailing `reserved[10]` — in the header, in the
document, not in the data. `probe_v3()` tries 81 first and 71 second, takes
the first that divides evenly *and* scans clean, and says which it used.
The pack loop reads raw bytes rather than a struct, so bytes it does not
understand — including a `reserved[10]` that may or may not be there — are
copied through instead of destroyed.

Checked against Clark's own `AREAS.DAT` in `..\FIDOUTIL\`: 684 records at 71
bytes probed correctly, 683 kept, the one non-Fido conference dropped, `/R`
leaves the file untouched. No shipped binary ever packed version 3, so
there is no oracle for this one and that is said plainly rather than
papered over.

## The conference scan reads two files, not one

`pcbconftype` is 739 bytes and **there is no 739-byte file anywhere**. It
is a merge, built by `opencnames()` and `getconfrecord()` in
`toolkit/pwa153/SOURCE/TOOLKIT/CNAMES.C`:

| File | Header | Record | Seek |
|---|---|---:|---|
| `CNAMES.@@@` | `short RecSize` | `oldconftype`, 548 | `ConfNum*548 + 2` |
| `CNAMES.ADD` | none | `addconftype`, 256 | `ConfNum*256` |

Both indexed by conference number **directly** — conference 0, the Main
Board, is record 0. `CNAMES.DOC` says `(ConfNum-1)`; the code says
`ConfNum`, and the code wrote the files. `ConfType` lives in `CNAMES.ADD`,
which is optional — a board without it has no Fido conferences at all,
which is correct, because `ConfType` did not exist before it.

An earlier draft read one flat 739-byte-record file. It read a file that
has never existed.

## Duplicates die, and that is measured

Three bitmap routines in the shipped binary, one bit per conference:

    0x2EE3   or  [es:bx],dl      SET     bitmap[c>>3] |=  1<<(c&7)
    0x2DEB   shr al,cl ; and 1   TEST    bitmap[c>>3] >>  (c&7) & 1
    0x2F75   not dl ; and        CLEAR   bitmap[c>>3] &= ~(1<<(c&7))

`0x2F75` is a **clear**. So the bit means *this conference is still
available*, and the first area for a conference takes it — every later
duplicate is dropped. Nothing in the program's strings hints at this;
`removed %5d %s...` reads the same for all three drop reasons.

Confirmed by experiment, not just by reading: given twelve conferences
three and four times in shuffled order, Clark's binary dropped exactly the
later ones.

## PCBOARD.DAT lines 246 and 247 are not reserved

`PCBDAT.DOC` lists 245, 246 and 247 as "Reserved". They are not.
`DATAFILE.C` reads `EnableFido`, `FidoConfig` and `FidoQueue` there, right
after `AllFilesList` on 244 — and Clark's own `PCBOARD.DAT` in `..\FIDOUTIL\`
has `PCBFIDO.CFG` on 246 and `FIDOQUE.DAT` on 247. Line 324 is the 15.22
`FidoLoc` and the 1994 binary ignores it.

`PCBOARD.DAT` is a **line-oriented text file**. `pcbdattype` in
`NEWDATA.H` is the in-memory structure the reader fills, not the layout on
disk.

The docs are now wrong about three files in a row: `FIDO.DOC` by 10 bytes,
`CNAMES.DOC` by one in the seek, `PCBDAT.DOC` by three lines that are in
use.

## The test data is next door, in `..\FIDOUTIL\`

There was an `EXAMPLES\` here for a day. It was a third copy: all 16
files byte-for-byte identical to `pcb153\SOURCE\MISC\FIDOUTIL\` — Clark's
own placement, as received — and to
`reference/pcball/pcboard/pcb-misc/FIDOUTIL/`. It was removed rather than
carried, and everything that referred to it now points at `..\FIDOUTIL\`.

### It is a mixed-era fixture — check the version before you trust a file

The Fido data files are **version 3 (15.22 and later)**. `PCBOARD.DAT` is
**15.21**. That board had its Fido data converted and its `PCBOARD.DAT`
never grew.

| File | Version | Usable for |
|---|---|---|
| `AREAS.DAT` 48,566 | **3** | the 15.4 PACKFIDO — this is what it is checked against |
| `AKAS`, `FREQDENY`, `FREQPATH`, `MAGICNAM`, `NODEARC`, `NODELIST`, `ORIGINS`, `PHONEX` | **3** | 15.22+; all but ORIGINS and FREQDENY are 2-byte headers with no records |
| `PCBFIDO.CFG` 2,009 | **3** | 15.22+ — the directories/EMSI/FREQ/archiver block, **not** an area file any more |
| `PCBOARD.DAT` 1,501 | **15.21** — 323 lines, no line 324 | reading lines 31, 108, 246, 247; **not** a 15.3 board file |
| `FIDONET.NA`, `PASSTHRU.NA` | text | nodelist area lists, no version word |
| `ILPTARS.DAT`, `PTARS.DAT` | `00 00` | empty |

**What this fixture cannot do: test the byte-exact 15.3 PACKFIDO.** That
one reads a **version 2** `PCBFIDO.CFG` with the areas still inside it,
and the `PCBFIDO.CFG` here is version 3 with the areas long since moved
out to `AREAS.DAT`. There is no version 2 Fido data anywhere in the
archive. That is why the acceptance harness had to *write* one from
`CONVERT.CPP`'s field-by-field reader rather than find one.

`PCBOARD.DAT` being 15.21 is not a problem for what it was used for —
it is what settled that lines 246 and 247 are `FidoConfig` and
`FidoQueue`, not "Reserved" — but do not point a 15.3 build at it.

## Version 3 `PCBFIDO.CFG` — the source gives the order, not the widths

`CONVERT.CPP` reads a version 2 `PCBFIDO.CFG` field by field, which is
where the whole 15.21 layout came from. It also **writes** the version 3
one, in `TMPtoCFG()`, and that is the v3 order, from Clark's own code:

    word         file version = 3
    DIRECTORIES  dirs
    EMSI_DATA    emsidata
    FREQ_INFO    frq
    ARCHIVERS    rec

Note the swap only the source reveals: the intermediate `PCBFIDO.TMP` is
written `dirs, emsidata, ARCHIVERS, FREQ`, and `TMPtoCFG()` writes them
out to the `.CFG` as `dirs, emsidata, FREQ, ARCHIVERS`. Read the TMP
order into a v3 file and every byte after the EMSI block is wrong.

### `ARCHIVERS` is nailed down — three independent confirmations

| Check | Result |
|---|---|
| `sizeof(ARCHIVERS)` = 4×66 + 4×80 + 4×66 + 4×80 | **1168** |
| `PKZIP.EXE` — `archivers[0]` — found at | **+841** |
| `PKUNZIP.EXE` — `unarchivers[0]` — 841 + 264 + 320 = | **+1425**, exactly where it is |
| 841 + 1168 | **2009** = the file size, exactly |

So the block before it — `DIRECTORIES` + `EMSI_DATA` + `FREQ_INFO` — is
**839 bytes**, and `STRUCTS.H` agrees: 594 + 230 + 15 = 839.

### CORRECTED — the split does match. One of the two files is an older generation.

**Superseded 2026-09-23, the same day it was written.** Everything below
the rule was measured on a *single* file and concluded `STRUCTS.H` was
wrong. There is a **second** `PCBFIDO.CFG` in this repo, and it settles
it the other way: `STRUCTS.H` is right to the byte, and the file that
disagreed is an earlier generation wearing the same version number.

Two version-3 files, both 2,009 bytes:

| | Path | Directories |
|---|---|---:|
| **A** | `pcb1541\install\dist\target\FIDO\PCBFIDO.CFG` | **9** |
| **B** | `..\FIDOUTIL\PCBFIDO.CFG` and its three copies | **6** |

A is accounted for completely:

| Offset | Size | Field | Confirmation |
|---:|---:|---|---|
| 0 | 2 | version = 3 | |
| 2 | 594 | `DIRECTORIES`, 9 × `MAXFLEN` 66 | all nine paths present at 66 stride |
| 596 | 230 | `EMSI_DATA` 60/30/30/50/10/50 | region is clean zeros |
| 826 | 15 | `FREQ_INFO` 2+2+4+4+1+2 | |
| 841 | 1168 | `ARCHIVERS` | PKZIP +841, PKUNZIP +1425, ends at 2009 |

2 + 594 + 230 + 15 + 1168 = **2,009**, the file size exactly, with every
archiver string landing where the struct puts it. `DATA.CPP` reads the
file in precisely that order, `sizeof` each, no seeks. Nothing in
`STRUCTS.H` needed changing.

**Clark documented this himself** — `docs\PCBXDOT\DOCDEV\FIDO.TXT`, found
after the fact, prints all four structures in order with the nine
directories, `EMSI_DATA` 60/30/30/50/10/50, `FREQ_INFO`, and
`ARCHIVERS[4]`, and states outright that *"PCBoard 15.22 data files have
a version number of 3, where as 15.21 data files have a version number of
2."* It is the first of these format documents to be right. `FIDO.DOC`
was wrong by 10 bytes on `AREAS.DAT`, `CNAMES.DOC` by one in the seek,
`PCBDAT.DOC` by three lines — `FIDO.TXT` matches the bytes exactly.
What it does not say, because Clark did not know he needed to, is that
two different files both answer to version 3.

### The real defect: version 3 is not self-describing

B carries six directories — `IN OUT MSG BAD NODELIST WORK`, which is
exactly `OLDDIRECTORIES` in FIDOUTIL's `CONVERT.HPP`, 6 × 66 = 396 —
**and a version word of 3.** `passthrough`, `securemail` and `messages`
were added to `DIRECTORIES` afterwards and the version word was never
bumped. `ArchiverInfo()` still reads the short block and writes the long
one:

    fread (&dirs, sizeof(OLDDIRECTORIES), 1, oldfile);   /* 396 - read  */
    fwrite(&dirs, sizeof(dirs),           1, afile);     /* 594 - write */

So two incompatible directory blocks share one version number, and any
reader that trusts the word is 198 bytes out on everything after the
directories. Nothing errors — it returns the wrong fields, which is
exactly where the "the headers are wrong" reading below came from.

**Fixed by probing rather than trusting** — `cCONFIG::probeLayout()` in
`pcb153\SOURCE\FIDO\DATA.CPP`, the same discipline PACKFIDO uses on
`AREAS.DAT`'s 81-vs-71 record:

* `ARCHIVERS` is the last block in the file, so it is anchored at
  `size - sizeof(ARCHIVERS)` — 841 in both files, either generation.
* A directory slot is either blank or a path. Walk the nine; a six-entry
  file puts its EMSI data where slot 6 belongs and fails on sight.

Verified against both files: A classifies NEW(9) and reads through; B
classifies OLD(6), its six directories read, and its EMSI/FREQ are
**refused** rather than handed back as rubbish. The puts refuse on the
six-entry generation too — writing a 594-byte `DIRECTORIES` into a
396-byte hole destroys the sysop's configuration.

### The six-entry generation's block, decomposed

`FREQ_INFO` is 15 bytes and `ARCHIVERS` is anchored at 841, so the block
between B's directories and its FREQ is **398 → 826, 428 bytes**. It
decomposes almost completely:

| rel | offset | Field | Width | Value in B |
|---:|---:|---|---:|---|
| 0 | 398 | `BBS_Name` | 60 | `Stan's Test 2` |
| 60 | 458 | `City` | 30 | `Murray` |
| **90** | **488** | **unidentified** | **132** | one string at +108 (see below) |
| 222 | 620 | `Sysop` | 30 | `Stan Paulsen` |
| 252 | 650 | `Phone` | 50 | `29` |
| 302 | 700 | `Baud` | 10 | `9600` |
| 312 | 710 | `Flags` | 50 | *(empty)* |
| 362 | 760 | `bool` | 1 | `01` |
| 364 | 762 | `THIS_ADDRESS` | 43 | see below |
| 407 | 805 | `bool` | 1 | `01` |
| 408 | 806 | *(zero)* | 20 | |
| 428 | 826 | `FREQ_INFO` | 15 | all zero |

**The EMSI tail is the modern struct, unchanged.** Relative to `Sysop`,
`EMSI_DATA` spaces its remaining fields `Phone +30`, `Baud +80`,
`Flags +90`, end `+140`. B's are `+30`, `+80`, `+90`, `+140` — four
fields, three exact deltas, and the same `BBS_Name[60]`/`City[30]` head.
The field widths never changed. **132 bytes were inserted between `City`
and `Sysop` and later removed**, and that is the whole difference.

**`THIS_ADDRESS` at +762 is a hard identification**, not a guess:

    nodestr[25]  +762   "1:311/40"
    this_zone    +787   1
    this_net     +789   311
    this_node    +791   40
    this_point   +793   0
    reserved[10] +795

`1`, `311`, `40` in binary against `"1:311/40"` in text, in the struct's
own order, at the struct's own widths. The system address used to live
in `PCBFIDO.CFG`; by the nine-directory generation it had moved out to
`AKAS.DAT`.

**What is genuinely still open is 132 bytes**, down from 443. They are
zero except for `", Utah"` at +596, which is +108 into the gap. Two
readings fit and one file cannot choose between them:

* a 108-byte field followed by a 24-byte one, `", Utah"` starting the
  second — the sysop having typed the comma into a state field; or
* residue, the gap being 2 × 66 and `", Utah"` the tail of an earlier
  longer value, in which case there is no field boundary at +596 at all.

Settling it needs a **second** six-directory `PCBFIDO.CFG`. Searched for,
and there is not one — this is a closed negative result, not an
unfinished search:

* Every file in the repo of exactly 2,009 bytes, and every `PCBFIDO.*`,
  `*.BAK` and `*.OLD`: **two distinct files, seven copies.** Six are B
  (one of them inside `DOSBOXX.ZIP` at
  `pcbirc/BUILDROOT/PCB153/.../PCBFIDO.CFG`, sha256 `8bac4ccc…`,
  byte-identical), one is A.
* Every `.zip` in the tree, listed and searched by member name. `DOSBOXX.ZIP`
  is the only archive holding one, and it is that same copy of B.
* No `PCBFIDO.TMP` survives anywhere — the intermediate `TMPtoCFG()`
  reads would have shown the layout directly.

The other way in would be a struct that accounts for 132 bytes.
Enumerating every `typedef struct` in `STRUCTS.H` at pack(1): **nothing
is 132**, and only two pairs sum to it — `HELLO_T` + `AINDEX` and
`QUEUE_RECORD` + `AINDEX`. Both are out. `HELLO_T` puts `bbs_name` at
+10 behind five `sint`s, not at +0, and neither pair explains a string
starting at +108.

The placement itself is settled even though the contents are not. The
alternative — `EMSI_DATA` sitting at +530 with a 132-byte struct ahead of
it — would require `BBS_Name` and `City` to be *blank inside* `EMSI_DATA`
while those same two values sit 132 bytes earlier at exactly the
`BBS_Name[60]` / `City[30]` spacing. The reading above is the one where
all six EMSI fields hold coherent values in the struct's own order.

Nothing is blocked by what remains: `probeLayout()` reads that
generation's directories and archivers and refuses its EMSI, which is the
correct answer under either reading of the gap.

---

*Original text below. Its measurements are correct; only its conclusion
was wrong, and it is kept because the reasoning is what found the second
file.*

### But the split inside those 839 bytes does not match the headers

The total is right and the division is wrong, which is the worst
combination — nothing overflows, nothing crashes, and every field after
the first is off.

Reading it the way `STRUCTS.H` says, with 9 directories and `EMSI_DATA`
starting at +596:

    BBS_Name  ', Utah'
    City      ''
    Sysop     ''
    Phone     ''
    Baud      '1/40'
    Flags     ''

`Baud` of `'1/40'` is a **slice of the FidoNet address** `1:311/40`
sitting at +762. That is what a wrong field split looks like when it
still parses.

Reading it the way the data says — **six** directories, 6 × 66 = 396,
EMSI starting at +398:

    +398   BBS_Name    "Stan's Test 2"
    +458               'Murray'
    +596               ', Utah'
    +620               'Stan Paulsen'
    +650               '29'
    +700               '9600'
    +762               '1:311/40'

A BBS name, a city, a state, the sysop, a baud rate and a FidoNet
address — coherent, in that order, and `Stan Paulsen` is the author
named at the top of `CONVERT.CPP`.

The six directories present are `IN`, `OUT`, `MSG`, `BAD`, `NODELIST`,
`WORK` — exactly the **first six** of the nine `FIDO.DOC` lists. The
three absent are `passthrough`, `securemail` and `messages`, the last
three in the struct. So this file predates them, and its EMSI block is
443 bytes where `STRUCTS.H` has 245.

**The exact widths of the fields after `BBS_Name[60]` are not settled** —
only their start offsets are measured. Anyone writing a v3 `PCBFIDO.CFG`
back out should nail those down first, against more than one file.

This is the same failure as `AREAS.DAT` being 71 bytes where the header
says 81: the shipped data and the headers in this archive are different
generations, and the header is not the authority. Measure the file.

## There *is* a 15.3 PCBOARD.DAT — in `pcb153\TEST\`

Found 2026-09-23, and it is the file the FIDOUTIL fixture does not have.
`pcb153\TEST\` is Clark's own test board directory, as received:
`BOARD.BAT`, `PCBDOS.BAT`, `PCBOARD.DAT`, `PCBOARD.SYS`, `PCBQWK.BAT`,
`PCBRZ.BAT`, `PCBSZ.BAT`.

    PCBOARD.DAT   1,640 bytes   349 lines + a 0x1A EOF marker

**349 lines is 15.3 complete.** `PCBDAT.DOC`'s own table: 319–323 were
added in 15.21, 324–348 in 15.22, and 349 in 15.3. All of them are
present.

| Line | Value | What |
|---:|---|---|
| 31 | `D:\PCB\MAIN\LCLCNAME` | CNAMES base name |
| 108 | `2` | highest conference — a two-conference test board |
| 246 | `SAVE\PCBFIDO.CFG` | `FidoConfig` — the 15.0 line |
| 247 | `R:\FIDOQUE.DAT` | `FidoQueue` |
| 324 | `D:\PCB\FIDO\` | `FidoLoc` — the 15.22 line |
| 349 | *(empty)* | the 15.3 addition, unset |

### It settles 246 vs 324 a second time, independently

Lines 246 and 324 are **both populated, with different values** —
`SAVE\PCBFIDO.CFG` and `D:\PCB\FIDO\`. They are not the same setting
renamed; they are the 15.0 config *file* and the 15.22 config
*directory*, coexisting on one 15.3 board. The first proof came from
`DATAFILE.C` and the 15.21 fixture; this is a second witness from a
completely different file, and it is the one that shows both lines live
at once.

That also confirms the 1994 binary's behaviour from the other side: it
reads 246 and has never heard of 324, which is why pointing 324 at a
config file made it print `done.` and change nothing.

### What it is good for

Anything needing a **real 15.3 board file** — the eventual no-argv
PACKFIDO, which reads `PCBOARD.DAT` from the current directory the way
Clark's did. It is the only 15.3-era `PCBOARD.DAT` in the tree; the one
in `..\FIDOUTIL\` stops at 323.

It does **not** rescue the byte-exact 15.3 test. That still needs a
version 2 `PCBFIDO.CFG`, and `SAVE\PCBFIDO.CFG` is a path this board
pointed at, not a file that came with the archive.

Both this file and the `..\FIDOUTIL\` one use bare LF rather than CRLF —
they have been through a Unix tool somewhere in the archive's history.
Harmless: DOS text-mode reads split on LF either way.

## Where the source went

`PCBSRCV/000/MISC/IDX/MAKEIDX.DSK` and
`PCBSRCV/000/UTIL/PCBMONI/PCBMONI.DSK` both list `E:\TC\PACKFIDO\PACKFIDO.C`
with neighbours `E:\TC\SCANLOG\SCANLOG.C`, `E:\TC\PCBMONI\PCBMONI.C` and
others. `E:\TC\` is the developer's scratch drive; the product tree is
`D:\PROJ\...` on the other drive in the same list. The archive was made
from the product tree.

## FIDOUTIL does not need it, and never did

`packfido.obj` supplies one symbol, `do_pack()`. Every occurrence in the
archive: a declaration at `CONVERT.CPP:54` and a commented-out call at
`CONVERT.CPP:125`. On 2026-09-17 it was dropped from `FIDOUTIL.MAK`;
restoring it means uncommenting three places, and the file says which.

`CONVERT.CPP` is also where the 15.21 layout came from — its `Areas()`
reads a version 2 `PCBFIDO.CFG` field by field, which is the whole file
format in two lines of Clark's own source.
