# The version 2 `PCBFIDO.CFG`, recovered from Clark's shipped binary

No version 2 `PCBFIDO.CFG` survives anywhere in this repo. The layout below
was **read out of the code** of the shipped `FIDOUTIL.EXE` —
`pcb1541\install\dist\target\FIDOUTIL.EXE`, 214,586 bytes, sha256
`7b5eed4041ba…`, whose own strings date it to **15.22 / 1995**:

    Convert 15.21 data to 15.22 format? (Y/N)
    Copyright Clark Development 1995

(The source in this tree is the later one — it says *15.23* and *1996*.)

## Method

`convert()` walks the version 2 file **sequentially**, one `fread` per
record, and writes each block out to its own `.DAT`. Every `fread` in that
binary goes through one routine at `0x483D`, and Borland loads the record
size as `mov ax,imm16` a few instructions before the call. Disassembling
`0xC000` onward and pairing each call with its preceding immediate
recovers the whole read order — 49 `fread` sites, of which the first
eleven are `convert()`.

## The layout

| code | size | struct | what it is | where it went in 15.22 |
|---|---:|---|---|---|
| `0xC74F` | **243** | `AREA_STRUCT` | Fido area records | `AREAS.DAT`, rewritten at 81 (**71 on disk**) |
| `0xC976` | **1168** | `ARCHIVERS` | 4 archivers + switches | stays in `PCBFIDO.CFG` |
| `0xC9A0` | **396** | `OLDDIRECTORIES` | **6** × `MAXFLEN` 66 | rewritten to 594 (9 entries) |
| `0xC9E6` | **230** | `EMSI_DATA` | the EMSI `{IDENT}` addon | stays in `PCBFIDO.CFG` |
| `0xCB21` | **47** | `NODE_T` | per-node archiver choice | `NODEARC.DAT`, as `NNODE_T` (22) |
| `0xCC62` | **43** | `THIS_ADDRESS` | EMSI system address list | `AKAS.DAT`, as `NADDRESS` (92) |
| `0xCD9A` | **70** | `TRANSLATE` | phone number translation | `PHONEX.DAT` |
| `0xCE9E` | **101** | `NODELIST` | nodelist compile info | `NODELIST.DAT` |
| `0xCFB8` | **80** | `FREQ_PATH` | file-request paths | `FREQPATH.DAT`, as `NFREQ_PATH` (86) |
| `0xD088` | **15** | `FREQ_INFO` | freq restrictions | stays in `PCBFIDO.CFG` |
| `0xD19B` | **80** | `FREQ_MAGIC` | magic filenames | `MAGICNAM.DAT`, as `NFREQ_MAGIC` (106) |

Two things worth noting about the order:

* In the **version 2** file `ARCHIVERS` comes *before* `DIRECTORIES`. In the
  version 3 file `TMPtoCFG()` writes it *last*. The block order is not
  preserved by the conversion, which is why reading a v3 file in the v2
  order — or the `PCBFIDO.TMP` order — puts every field wrong.
* The four blocks that **stay** in `PCBFIDO.CFG` are exactly the four that
  Clark's own `docs\PCBXDOT\DOCDEV\FIDO.TXT` documents. Everything else
  became a `.DAT`.

## EMSI here is the FTSC packet, not a Clark invention

`docs\fido\FSC-0056.txt` defines `EMSI_DAT` and its optional addon fields.
The `{IDENT}` addon is:

    {IDENT}{[system_name][location][sysop][phone][baud][flags]}

which is `EMSI_DATA` field for field — `BBS_Name[60]`, `City[30]`,
`Sysop[30]`, `Phone[50]`, `Baud[10]`, `Flags[50]`. Clark's larger `Emsi`
struct in `STRUCTS.H` is the **whole** `EMSI_DAT` packet: `addr[2000]`,
`pw[25]`, `line[20]`, `codes[50]`, `prodcode[50]`, `prodname[50]`,
`prodver[10]`, `prodserial[50]`, then the six IDENT fields — matching the
spec's system address list, password, link codes, compatibility codes,
mailer product code, name, version and serial.

So the two structs are the standard, split: `Emsi` is the wire packet,
`EMSI_DATA` is the part a sysop configures, and `THIS_ADDRESS` is the
system address list that later moved out to `AKAS.DAT`.

## What this does and does not settle about file B

`reference\pcball\pcboard\pcb-misc\FIDOUTIL\PCBFIDO.CFG` (**B**) is 2,009
bytes, and 2 + 594 + 230 + 15 + 1168 = 2,009 — so B fits the version 3
shape **arithmetically**, exactly as file A does. What does not fit is the
content: B's directory slot 6 holds `"Stan's Test 2"`, a BBS name, where A
holds a path.

Reading B's EMSI-shaped content from +398 gives `BBS_Name` and `City` in
the right places and at the right widths, then **132 bytes**, then `Sysop`,
`Phone`, `Baud` and `Flags` at exactly the struct's own spacing. None of
the eleven structures above is 132 bytes, and none of them lands a field
boundary at +108 into the gap, where B's only non-zero content sits.
Candidates that merely *sum* to 132 — `FREQ_LIMIT` × 4, `NNODE_T` × 6,
`MAXFLEN` × 2 — all fail that boundary test.

**So the binary answers the question it can answer.** It gives the complete
v2 layout, it confirms which blocks became which `.DAT`, and it confirms
`EMSI_DATA` against the FTSC spec. It does not decompose B's 132 bytes,
because that binary never read a file shaped like B — its `DIRECTORIES`
read is 594, not 396, so it predates nothing and postdates B's generation.

Nothing is blocked by the gap: `cCONFIG::probeLayout()` uses size
arithmetic — `(filesize - 1415) / 66` — to determine the directory count,
so it handles any valid v3 file regardless of what the directory slots
contain.

## The 132 bytes: EMSI is ruled out, by Clark's own manual

`docs\sysop\ADDENDUM.DOC` is the 15.2 sysop documentation and it describes
both PCBSETUP screens that write this file. Two things fall out of it, and
both are independent of any binary or header.

**1. 15.2 had exactly six directories.** The *File & Directory
Configuration* screen documents `Dir of Incoming Packets`,
`Dir of Outgoing Packets`, `Dir to store Bad Packets`,
`Dir of Nodelist Database`, `Work Directory` and `Dir to store *.MSG`.
Six — the same six file B carries, and the same six as `OLDDIRECTORIES`
in `CONVERT.HPP`. `passthrough`, `securemail` and `messages` are not
there. So B's 396-byte directory block is confirmed as the 15.2 layout
from Clark's manual, not just inferred from the bytes.

**2. The EMSI profile did not change.** The 15.2 *EMSI Profile* screen
documents `BBS Name`, `SysOp Name`, `City/State`, `Phone`, `Baud` and
`Flags` — the same six fields, in the same roles, as 15.3's `EMSI_DATA`
and as the FSC-0056 `{IDENT}` addon. **No EMSI field was ever removed.**

That is what closes the EMSI theory. The 132 bytes cannot be dropped EMSI
fields, because there are none to drop. They are a separate block that sat
between `DIRECTORIES` and `EMSI_DATA` in the 15.2 file and is not present
in 15.3.

### What is known about the block, and what is not

Known: it is **132 bytes**, it begins at +398 immediately after the six
directories, and `EMSI_DATA` begins at +530 immediately after it — that
placement is forced, because `Sysop`, `Phone`, `Baud` and `Flags` land at
+620, +650, +700 and +710, which is the struct's own spacing (+0, +30,
+80, +90 from `Sysop`) to the byte. Within the block, a 60-byte field at
+0 holds `"Stan's Test 2"` and a field at +60 holds `"Murray"` — a name
and a city, at `BBS_Name[60]` / `City[30]` widths.

Not known: the remaining 42 bytes, and why the board's name and city
appear here at all when `EMSI_DATA` at +530 is empty apart from `", Utah"`
residue at +596. One file, with everything else in it zeroed, does not
carry enough signal to assign those widths, and no second file of this
generation exists in the repo or in any archive in it.

**This does not block anything.** `cCONFIG::probeLayout()` reads that
generation's directories and archivers and refuses its EMSI, which is
correct whatever the 132 bytes turn out to be.

## File B is the author's own beta board, and that is why it fits nothing

`docs\sysop\WHATSNEW.FID` is the **15.22 beta documentation**, written by
**Stan Paulsen** — *"you can address all your Fido related questions to me,
Stan Paulsen"* — and it uses `1:311/40` as his own example address.

File B's `EMSI_DATA.Sysop` is `"Stan Paulsen"` and its `THIS_ADDRESS` is
`1:311/40`. **B is Stan Paulsen's own machine**, captured during the 15.22
beta cycle, and it shipped in the FIDOUTIL directory as that program's test
fixture.

The same document settles the directories. It lists as **new 15.22
features**:

* *Secure Netmail* — "netmail packets will be moved to a secure directory"
* *Autoadd areas as passthru areas*
* *Predefined response messages* — "the directory setting for these in
  PCBSETUP | FIDO CONFIGURATION | FILE AND DIRECTORY INFO"

Those are `securemail`, `passthrough` and `messages` — precisely the three
entries that take `DIRECTORIES` from six to nine. They were **added during
the 15.22 cycle**, not in 15.3.

So B is a version-3 file written **mid-beta**, after the format went binary
and the version word became 3, but before the three directories were added.
It is not a released layout. That is why it matches neither the 15.21
structure the 1995 binary reads nor the 15.22 structure it writes, and why
no second copy of it exists anywhere — there was never a population of
these files, only the author's test machine.

### The 42 bytes, closed

*"15.21 data was in text, maybe it is in binary now."* That is the
resolution. `PCBOARD.DAT` is line-oriented text to this day, and
`WHATSNEW.FID` opens with *"Virtually EVERY fido related file has changed
format"* — 15.22 is where the Fido configuration became a binary struct.
The 132-byte block is `BBS_Name[60]`, `City[30]`, and **42 bytes of binary
fields** — counts, flags, bools — for settings that were text lines in
15.21 and had just been given struct members.

They are unreadable here for the most ordinary reason available: on a test
board with those settings unconfigured, binary fields are **zero**, and a
scan for strings will never show them. There is nothing hiding in the 42
bytes. There is nothing *in* them.

That also settles the `", Utah"` at +596. With the block ending at +530 and
`EMSI_DATA` running +530 to +760, `City[30]` occupies +590 to +620, and
`", Utah"` sits six bytes inside it — **residue**, not a field boundary.
The two readings this document previously could not choose between are now
one.

### Status, stated exactly

**Answered, not recovered.** Those are different things and this project's
standard is the difference.

Answered: what the file is, why it matches no released layout, why no
second copy exists, why `", Utah"` is residue, and why the 42 bytes read as
nothing.

**Not recovered: a single field name, width or type inside those 42
bytes.** The account above — binary fields, unconfigured, therefore zero —
is an inference from the 15.21-to-15.22 text-to-binary transition plus the
fact that every one of those bytes is zero. It is not a measured layout,
and it should not be cited as one. Recovering it would take a beta-era
`PCBFIDO.CFG` from a board that actually had those settings turned on. No
such file exists here.

**One open lead, recorded so it is not lost.** The shipped `FIDOUTIL.EXE`
contains a **90-byte** record size at three call sites — `0xE472`, `0xEF36`
and `0xFA04`. 90 is exactly `BBS_Name[60] + City[30]`, the identified head
of this block. Those sites sit well past `convert()` and their argument
order has not been resolved, so the match is arithmetic only and is **not**
evidence yet. It is the next thing to check if this is ever reopened.

`probeLayout()` reads that generation's directories and archivers and
refuses its EMSI, which is right for a beta-era file that no released
PCBoard ever wrote.

---

## CORRECTION — the table above was built from the wrong call sites

**2026-09-23, same day.** The first pass at this file paired each record
size with calls to `0x483D`, on the assumption that it was `fread`. **It is
`memset`.** Its argument pattern is `(size, 0, ptr)` and it has 136 call
sites; `fread` is `0x3FD6` (28 sites) and `fwrite` is `0x4246` (27).

Because Borland emits `memset(&x,0,sizeof(x))` immediately before reading
into `x`, most of the sizes came out right anyway — but **not the one that
mattered**. The directory entry read `594` from the memset of the
nine-entry destination struct. The actual `fread` is at `0xC9A0` and it
reads **396**.

So the claim published here earlier — *"that binary never read a file
shaped like B — its `DIRECTORIES` read is 594, not 396"* — **was wrong.**
The shipped 1995 `FIDOUTIL.EXE` reads `sizeof(OLDDIRECTORIES)` = 396,
exactly as the 1996 source does. `OLDDIRECTORIES` is not a later addition.

### `ArchiverInfo()` as the binary actually has it

    0xC900  memset(rec,  0, 1168)
    0xC915  memset(dirs, 0,  594)     <- destination is the NINE-entry struct
    0xC92A  memset(emsi, 0,  230)
    0xC976  fread (rec,   1168)        ARCHIVERS
    0xC9A0  fread (dirs,   396)        OLDDIRECTORIES - SIX directories
    0xC9BC  fwrite(dirs,   594)        DIRECTORIES    - NINE, tail zeroed
    0xC9E6  fread (emsi,   230)        EMSI_DATA
    0xCA02  fwrite(emsi,   230)
    0xCA2A  fwrite(rec,   1168)

Line for line the 1996 source. And note `0xC915`: **`dirs` is zeroed before
the 396-byte read**, so the 198-byte tail the converter writes out is
zeros.

### What that does to file B

B has `"Stan's Test 2"` at +398, inside that 198-byte window. A converter
that zeroes the tail cannot produce it. **So B is not converter output at
all** — it was written directly by a PCBSETUP whose `DIRECTORIES` was six
entries, through `cCONFIG::putDirs` / `putEMSI`, which place `EMSI_DATA` at
`2 + 396 = 398`.

That is exactly where B's `BBS_Name` and `City` are. It makes the beta
`EMSI_DATA`:

    BBS_Name[60]  +398
    City[30]      +458
    ??? 132 bytes +488
    Sysop[30]     +620
    Phone[50]     +650
    Baud[10]      +700
    Flags[50]     +710      ends +760

**362 bytes**, against 230 released. 132 bytes were cut from between `City`
and `Sysop` before 15.22 shipped.

`362`, `132` and `198` appear **nowhere** in the shipped binary as
immediates — checked across all 82,204 disassembled lines. The released
code has no memory of the beta struct, which is the expected outcome and
also the end of what this binary can say.

---

## SECOND CORRECTION — the "362-byte beta EMSI" above is also withdrawn

Two readings of file B fit different evidence, they contradict each other,
and nothing available decides between them. Both are recorded here so the
next person starts from the contradiction instead of rediscovering it.

**Reading 1 — nine directories. Fits the size exactly.**

    2 + (9 x 66) + 230 + 15 + 1168 = 2009 = the file size

and only 9 works: 6 gives 1811, 7 gives 1877, 8 gives 1943. `ARCHIVERS`
lands at 841, which is independently confirmed by `PKZIP.EXE` at +841 and
`PKUNZIP.EXE` at +1425. Under this reading every string in B is junk in an
unconfigured field: slot 6 `passthrough` holds `"Stan's Test 2"`,
`EMSI_DATA.BBS_Name` holds `", Utah"`, `Baud` holds `"1/40"`.

**Reading 2 — six directories. Fits the content exactly.**

`EMSI_DATA`'s last four fields are spaced `Sysop +0`, `Phone +30`,
`Baud +80`, `Flags +90`. In B, `"Stan Paulsen"` is at +620, `"29"` at
+650, `"9600"` at +700 — `+0`, `+30`, `+80` to the byte, with `Flags`
ending at +760 and `THIS_ADDRESS` holding `"1:311/40"` immediately after.
Four fields, three exact intervals, all semantically right. That is not a
coincidence and it cannot be junk.

**The two cannot both be true.** Reading 1 puts `EMSI_DATA` at +596;
reading 2 requires its tail at +620, which is 132 bytes later. The size
arithmetic and the field spacing point in different directions, and the
file is one sample with nearly everything zeroed.

### What is safe to rely on

* `ARCHIVERS` at +841, 1168 bytes, running to EOF. Confirmed three ways.
* `FREQ_INFO`, 15 bytes, immediately before it at +826.
* The first six directories at +2, +68, +134, +200, +266, +332 — real
  paths under either reading.

### What is not safe

Anything about `EMSI_DATA` in this file, and any claim about a 132-byte
block, a 362-byte struct, or a beta-era layout. **Two such claims were
published in this document and both were wrong.** They are struck.

`cCONFIG::probeLayout()` refuses this file's EMSI, which is the correct
behaviour under either reading, and is the only reason none of this
blocks anything.
