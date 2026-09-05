# INSTALL.EXE Gap Analysis (install v1.11.4)

Structural comparison between Clark's reference INSTALL.EXE
(338,548 B) and our v1.11.3 build (56,574 B). This document
defines v1.11.5 through v1.11.9.

## 1. File Size Comparison

| | Clark's | Ours | Delta |
|---|---|---|---|
| File size | 338,548 B | 56,574 B | 281,974 B |
| MZ stub (before NE) | 117,248 B | 128 B | 117,120 B |
| NE portion | 221,300 B | 56,446 B | 164,854 B |

## 2. NE Header Comparison

| Field | Clark's | Ours | Match? |
|---|---|---|---|
| Linker | 5.10 | 5.10 | YES |
| Target OS | 0x01 (OS/2) | 0x01 (OS/2) | YES |
| Flags | 0x0002 | 0x000A | NO (0x0002 vs 0x000A) |
| Segments | 6 | 4 | NO |
| Stack size | 8000 | 0 | NO |
| Heap size | 0 | 4096 | NO |
| Module refs | 3 | 0 | NO |
| Auto data seg | 6 | 3 | NO |

## 3. Segment Table

### Clark's INSTALL.EXE

| Seg# | Type | File Offset | Length | Alloc | Flags |
|---|---|---|---|---|---|
| 1 | CODE | 0x01CC00 | 54,574 | 54,574 | 0x0D00 |
| 2 | CODE | 0x02A200 | 63,460 | 63,460 | 0x0D00 |
| 3 | CODE | 0x039E00 | 51,600 | 51,600 | 0x0D00 |
| 4 | DATA | 0x046C00 | 165 | 368 | 0x0C01 |
| 5 | DATA | 0x000000 | 65,536 | 2,166 | 0x0C01 |
| 6 | DATA | 0x046E00 | 48,210 | 55,040 | 0x0D01 |
| | **CODE total** | | **169,634** | | |
| | **DATA total** | | **113,911** | | |

### Our INSTALL.EXE (v1.11.3)

| Seg# | Type | File Offset | Length | Alloc | Flags |
|---|---|---|---|---|---|
| 1 | CODE | 0x000200 | 18,849 | 18,849 | 0x1D50 |
| 2 | CODE | 0x005200 | 19,413 | 19,413 | 0x1D50 |
| 3 | DATA | 0x00A800 | 10,724 | 10,724 | 0x0D51 |
| 4 | DATA | 0x000000 | 65,536 | 5,120 | 0x0C51 |
| | **CODE total** | | **38,262** | | |
| | **DATA total** | | **76,260** | | |

### Segment Size Deltas

| Category | Clark's | Ours | Delta |
|---|---|---|---|
| CODE segments | 169,634 B | 38,262 B | 131,372 B |
| DATA segments | 113,911 B | 76,260 B | 37,651 B |
| MZ stub | 117,248 B | 128 B | 117,120 B |
| Segment count | 6 | 4 | 2 |

## 4. Import Table

| | Clark's | Ours |
|---|---|---|
| Module ref count | 3 (DOSCALLS, KBDCALLS, VIOCALLS) | 0 |
| Import names | DOSCALLS, KBDCALLS, VIOCALLS | (none — /Toe without API.LIB) |

## 5. String Table Comparison

| | Clark's | Ours |
|---|---|---|
| Unique strings (>= 4 chars) | 2848 | 512 |
| In both | 36 | |
| Only in Clark's | 2812 | |
| Only in ours | | 476 |

### Notable strings in Clark's binary not in ours

    Cannot join or substitute drive having directory that is target of previous subs
                                                                                   
      If you are installing onto a hard disk, RAM disk, or other NON-REMOVABLE
      floppy diskette, 3.5" microfloppy diskette, or other REMOVABLE disk
      disk please select "Hard Disk".  If you are installing onto a 5.25"
    The INPUT disk drive is the disk drive (usually a floppy disk drive)
    floppy.  In order for INSTALL to modify the system files correctly,
    Corrupted library file - Unknown record type %d.  This compressed
    containing the first disk of the software you are now installing.
    to be overwritten.  Are you sure you want to overwrite this file?
          Authors():   eric jon heflin, larry hastings, darryl rust,
    Corrupted library file - The End Of File marker was encountered
    This system boots from drive %c:, which is a removable (floppy)
    you reboot your computer, you should restore the original file.
    drive.  However, that drive does not currently contain the boot
    String starting on line %lu exceeds maximum length of %d bytes
    before it should have (the file is probably severely damaged)
    Please place a disk with at least %lu bytes free in drive %c:
    Don't use an @Set... on the PATH variable; use @Path instead.
    Attempt to use ERestoreCWD() without calling ESaveCWD() first
    Press  N  to create sample system files on drive %c: instead
    Otherwise, you may instead have INSTALL create sample system
    ESaveCWD(): Error - unable to save Current Working Directory
    get_autoexec: Allocating new variable for an @Set... command
    Data pointed to by this block (may include hi-bit on chars):
    has been damaged.  You may either skip this file and attempt
    you must now insert the boot floppy which contains the file:
    INSTALL.EXE could not find the software you are installing.
    Internal memory error: Attempted to allocate zero-len block
    The input file is corrupted; decompression cannot continue.

## 6. Categorized Gap Inventory

### Implementation gap

Clark's binary supports 329 directives; our v1.11.2 implements 60
(the ones actually used by INSTALL.DAT). The remaining 269 are
stubbed in the dispatch table. Clark's binary has full handler code
for all 329, plus error messages, help text, screen rendering, and
input validation that our stubs skip.

Estimated implementation gap: ~131,372 bytes of code
(handlers, error paths, UI rendering, validation).

### RTL gap

Clark's binary links against BC 3.1's full C runtime (CL.LIB).
Our binary also links CL.LIB but exercises fewer runtime functions
(no floating-point, no overlay manager, limited string functions).
The RTL contribution scales with code: as we implement more
handlers, more RTL functions get pulled in by the linker.

### Resource gap

Clark's MZ stub is 117,248 bytes; ours is 128 bytes.
Delta: 117,120 bytes. The MZ stub in a Family API
binary contains the real-mode DOS loader that provides Family API
services when running under plain DOS (not OS/2). Clark's larger
stub contains the full DOSCALLS/KBDCALLS/VIOCALLS emulation layer;
ours contains only the minimal BCC-generated MZ header.

String count delta: 2336 more unique strings
in Clark's binary (error messages, help text, screen layouts,
directive names, format strings).

### Compiler-artifact gap

Both binaries use TLINK 5.10. Flag difference (0x0002 vs 0x000A)
is bit 3 (0x0008 = 'errors in image' flag, cosmetic). Segment
count difference may close as more code is added — the linker
creates segments as needed based on code/data size thresholds.

## 7. Phase Recommendations for v1.11.5-v1.11.9

Based on the gap categories above:

| Phase | Focus | Gap Category | Estimated Impact |
|---|---|---|---|
| v1.11.5 | Remaining 269 directive handlers (prioritized by Clark's INSTALL.DAT usage) | Implementation | Largest code growth |
| v1.11.6 | Error messages, help text, screen rendering | Implementation + Resource | String table parity |
| v1.11.7 | Family API MZ stub (DOSCALLS/KBDCALLS/VIOCALLS emulation) | Resource | MZ stub size parity |
| v1.11.8 | Linker flags, segment layout, NE alignment | Compiler-artifact | Header parity |
| v1.11.9 | RTL function coverage + compile-diff loop | RTL + final | Size convergence |

v1.11.10 = understanding-complete milestone. v1.12 = byte-exact arc.

## Source Data

- Clark's reference: `pcb1541/install/reference/INSTALL.EXE`
  (338,548 B, md5 5239767bfced0689a1da961799a0f79c)
- Our build: v1.11.3 output from BLDINS.BAT under DOSBox-X
  (56,574 B, built 2026-09-05)

--hexadecimal, install v1.11.4, 2026-09-05
