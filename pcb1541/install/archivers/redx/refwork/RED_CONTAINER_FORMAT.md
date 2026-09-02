# WCSC .RED container format

Reverse-engineered by cross-referencing INSTALL.zip archives against redx's
now-fixed parser output.

## Record layout

Each record is a **fixed 41-byte header** followed by a variable-length payload:

```
Offset  Size  Field
------  ----  ---------------------------------------------------
0-1     2     'RR' magic
2       1     format version (observed: 0x01)
3-7     5     DOS timestamp (5 bytes)
8-11    4     cmp_size (LE32) — compressed payload size in bytes
12-15   4     unc_size (LE32) — uncompressed size in bytes
16-17   2     0xffff (reserved)
18-19   2     crc16 (LE16) — CRC of uncompressed data
20-23   4     0x00000100 (flags, observed constant)
24-25   2     method (LE16): 0x0001 = STORED, 0x000B = LHA-lh5
26-40   15    filename slot (null-terminated, zero-padded)

41...41+cmp_size   payload bytes
```

**Payload starts immediately after the filename null terminator**, not at offset
41. The bytes between (fn_end+1) and (pos+41) are the FIRST bytes of the payload.
For a 12-char filename, payload begins at pos+39 and includes pos+39, pos+40 as
its first two bytes.

Next record begins at `pos + 41 + cmp_size` (no trailer).

## Historical parser bug

The old redx parser walked the filename until it found a null, then set
payload_offset = fn_end + 1 (correct) but advanced pos to
payload_offset + cmp_size + 2 (incorrect — treated the header padding as a
CRC trailer). This caused it to land in the middle of the next record's data
and stop with "1 record" or similar undercounts.

Fix (2026-09-03): parse filename from the fixed 15-byte slot, advance pos to
`pos + 41 + cmp_size` via a new `next_offset` field on `red_record_t`.

## What this unlocked

Applying the fix to the 6 .RED files in INSTALL.zip:

| Archive       | Was |   Now  | Notes |
|---------------|----:|-------:|-------|
| COMMDRV.RED   |  10 |     22 | +12 hidden: COMMTSR/DRVSETUP/TEST/XABIOS/BOCA1610/XACOOK/XACOMX/MONITOR.BAT/ARNETSP4/8.DAT/DIGI4/8E.DAT |
| PCBCFGS.RED   |   1 |    171 | WCSC config files (single-char filenames 0..N) |
| PCBMAIL.RED   |   1 |      4 | +BWCC.DLL, BC450RTL.DLL, PCBMAIL.HLP |
| PCBOARD.RED   |   1 |      4 | +PCBOARDM.EXE, PPLC100.EXE, PPLC330.EXE |
| PCBOARD2.RED  |   1 |     10 | Full OS/2 support (PCBCP/PCBMONI2/PCBOARD2/PCBPACK2/...) |
| PPLC.RED      |   3 |   ~24+ | HELLO examples + DBASE + DOORS + KAL + HOWTODBF.TXT + ... |

**Total: 205+ files unlocked** (was ~15).

## Files that still fail decompression (rc=-3)

These files have payloads starting with a double-zero prefix (`00 00 XX XX`),
which our current WCSC prefix logic doesn't handle (it assumes 2- or 3-byte
prefixes based on whether cmp[0] is zero).

Known failures in COMMDRV.RED:
- XABIOS.BIN   (cmp=1399, unc=2048)
- XACOOK.BIN   (cmp=3722, unc=6144)
- XACOMX.BIN   (cmp=3520, unc=6144)
- TEST.EXE     (cmp=10417, unc=16482)
- DIGI4E.DAT   (cmp=388,   unc=2053)
- DIGI8E.DAT   (cmp=447,   unc=3397)

Similar failures across PCBMAIL/PCBOARD2/PPLC — see extraction logs.

### Getting oracles

Working uncompressed copies of some of these files exist in the PWA-Delta and
Delta-OW trees. When available, drop them into
`archivers/redx/refwork/pairs/<NAME>.oracle` and pair with the .payload
extracted from the raw .RED archive to serve as decoder test vectors for
whatever mode uses the `00 00` prefix.

Path forward: either RE the double-zero prefix mode from INSTALL.EXE (a
follow-up to the CRC-mode session), or just use PWA-Delta / Delta-OW copies
as ground truth and skip the decoder work for these specific files.
