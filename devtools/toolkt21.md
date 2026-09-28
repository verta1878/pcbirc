# TOOLKT21 — IBM Developer's Toolkit for OS/2 2.1

Extracted 2026-09-23 from `/DEVTOOLS/OS2TK21` on **The Developer
Connection for OS/2, Volume 1** (August 1993).

    archive.org   ibm-devcon-01
    devcon-01.iso 414,193,664 bytes
    sha256        7b9dc96bb07a078a99c6636a05c17f023e83bed5083c11cd176b2b120b0766c7
    origin.txt    "Original CD-ROM imaged and scanned by the OS/2 Museum."

1,052 files, 40,031,649 bytes, extracted verbatim. The `;1` ISO-9660
version suffixes are stripped; nothing else is changed.

## Three archives, because of a transfer limit

    TOOLKT21.ZIP    11,220,217   the toolkit itself - 1,094 files
    TK21D35.ZIP     11,001,108   DISKIMGS\35   - the 3.5" diskette set
    TK21D525.ZIP    10,999,728   DISKIMGS\525  - the 5.25" diskette set

One 33 MB archive was over the 20 MB per-file limit on the link that
carried it here, so it is split along the natural seam: the toolkit, and
the two shrink-wrap diskette sets that are the same toolkit in
installable-floppy form. Unzip `TOOLKT21.ZIP` first; the other two drop
their `35\` and `525\` directories into `TOOLKT21\DISKIMGS\`.

Nothing was left out. 1,094 + 12 + 12 = the 1,052 CD files plus this
note and the directory entries.

## Why this is here

PCBCP's source carries

    #include <\toolkt21\c\os2h\valapi.h>

and `\TOOLKT21\` is the directory **this** toolkit installs into. It is
**not** Clark's Doors Developer's TOOLKIT v2.0 (`devtools/TOOLKIT2.ZIP`),
which this project had assumed for a while. Two unrelated products whose
short names look alike.

## What it holds

| Path | What |
|---|---|
| `C\OS2H\` | the 85 OS/2 C headers — `OS2.H`, `BSEDOS.H`, `PMWIN.H`, `PMGPI.H`, the WPS class headers |
| `OS2LIB\` | **`OS2386.LIB`** (192,512 packed) and `OS2286.LIB`, plus `REXX`, `SOM`, `VDH` |
| `OS2BIN\` | `LINK386`, `RC`, `IMPLIB`, `NMAKE`, `IPFC`, `EXEHDR`, `MAPSYM`, `MARKEXE`, `DLGEDIT`, `ICONEDIT`, `FONTEDIT`, `PACK` |
| `C\SAMPLES\` | 28 sample programs |
| `ASM\`, `CPLUS\`, `REXX\` | assembler, C++ and REXX headers and samples |
| `BOOK\` | 17 INF reference books — PMWIN, PMGPI, CPGREF, TOOLINFO |
| `DISKIMGS\35`, `DISKIMGS\525` | the shrink-wrap diskette sets |
| `SC\` | 41 SOM class definition files |

`OS2386.LIB` is one of the two things the PCBCP OpenWatcom port was
missing. The other was `PCBCP_STUBS.C`, which is still missing and is
ours, not IBM's.

## The files are PACK'd

Extensions ending in `_` — `.H__`, `.LI_`, `.EX_`, `.IN_` — are
compressed with IBM's PACK utility. `OS2BIN\PACK.EX_` is the packer
itself; `UNPACK` ships with OS/2. They are left packed here because that
is how the CD holds them; unpacking is a build step, not an archive
decision.

## VALAPI.H is not in it

Searched: `VALAPI.H` does not exist anywhere on the CD, in this toolkit
or outside it. The only `VAL*` files on the whole disc are
`CCL2\PM\VALUEPM.EXE` and `CCL2\WINDOWS\VALUEWIN.EXE`, which are SAA CUA
Controls Library samples and unrelated.

So the header PCBCP included was never part of the OS/2 2.1 Toolkit. The
port's decision to remove that include stands, and now there is a reason
on the record rather than a judgement call: the file it named is not
there to find.
