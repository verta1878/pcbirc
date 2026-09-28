# MISC/MD5 — MD5 for the Shared Secret logon

Recovered 2026-09-17 from `devtools/Md5.zip`. `PCBOARD2.MAK` has always
referenced `md5.obj` and the object was nowhere in the source tree —
`PCBSRCV/000/MISC/MD5/` carried the RFCs and two **empty** `DOS/` and
`OS2/` directories.

## Contents

| File | What |
|---|---|
| `MD5.ASM` | the implementation, MASM 6.0, public domain |
| `OS2/MD5.OBJ` | prebuilt OMF object (THEADR `md5.asm`, 1,492 B) |
| `MD5.TXT` | VMODEM "Shared Secret Logon Technique" |
| `RFC1321.TXT`, `RFC1725.TXT` | the MD5 and POP3 RFCs |
| `FILE_ID.DIZ` | the package's own BBS description — provenance |

## Why OS2/

`MD5.ASM` exports `md5String`, `md5String32`, **`DoThunk32to16`** and
**`FlatToSel`**. The 32-bit-to-16-bit thunking entry points are OS/2
specific — that is what places this object on the OS/2 side rather than
DOS. The algorithm itself is OS-neutral (no INT 21h, no Dos* calls), so
the same source assembles for DOS if a DOS object is ever wanted.

`FILE_ID.DIZ` says what the package is, in its author's words: source
and text describing the **"Shared Secret" method of password exchange
for use on BBSes**, based on MD5 as described in the included RFCs. That
is exactly the PCBoard 15.4 MD5LOGIN feature, which is why Clark had it.

## Not copied: ARSENAL.LIB

`ARSENAL.LIB` (1,577 B) was the seventh file in the zip and is **not**
here. Despite the extension it contains no object code at all — it is
plain ASCII, a distributor's stamp:

    Arsenal Computer Services - 3721 SW Plaza Drive, Topeka KS 66609-2002
    ...
    This file has been triple inspected for Virus infection, CRC
    corruption, and illegal material prior to it's being offered for
    distribution.

Arsenal Computer Services was a BBS CD-ROM distributor; they added this
notice to files they redistributed. `.LIB` here is the BBS file-area
convention for a label/notice text file, not a linker library. Nothing
in PCBoard references it, and adding it would put a 1990s advert in the
source tree. Left in `devtools/Md5.zip`, where the original is intact if
anyone wants to see it.

## Related

`pcb153/SOURCE/MAIN/MD5IMPL.CPP` is a different thing: the crew's
MD5LOGIN handshake work (PCBSRCV revision 025), not Clark's assembly.
