# thdproscan — PCBoard File Scanner

Clean room file integrity scanner for PCBoard 15.x uploads.
Design by pcbirc crew (evga/kiddo/sysop0).

## What It Does

Tests uploaded files for integrity and safety:

1. **File exists** and has non-zero size
2. **ZIP integrity** — verifies central directory, detects truncation
3. **Zip bomb detection** — limits file count per archive (10,000 max)
4. **Path traversal** — rejects `../`, drive letters, absolute paths
5. **Archive type detection** — magic byte identification for ZIP/ARJ/ARC/LZH/RAR/7Z/GZ
6. **External virus scanner** — optional hook via PCBPROSCAN_AV environment variable

## How PCBoard Calls It

PCBoard has built-in upload testing via `PcbData.TestUploads` flag
(configured in PCBSETUP). When enabled, PCBoard runs `PCBTEST.BAT`
after each upload via `verifyfile()` in SHELL.C:

```
PCBoard upload
  → verifyfile(fullpath, filename, descfile)
    → shell to PCBTEST.BAT %1 %2 %3 %4
      → pcbpscan tests the file
        → exit code 0: PASS (PCBPASS.TXT created)
        → exit code 1: FAIL (PCBFAIL.TXT created by .BAT)
    → PCBoard reads PCBFAIL.TXT / PCBPASS.TXT
    → displays result to caller
    → rejects file if failed
```

## Installation

1. Copy `pcbpscan.exe` and `PCBTEST.BAT` to your PCBoard directory
2. In PCBSETUP, enable "Test Uploads" (set to YES)
3. Optional: set `PCBPROSCAN_AV=clamdscan` (or your AV command)
4. Optional: set `PCBPROSCAN_VERBOSE=1` for detailed output

## PCBTEST.BAT

```batch
@echo off
pcbpscan %1 %2 %3 %4
if errorlevel 1 echo File failed testing > PCBFAIL.TXT
```

## Arguments

| Arg | Description | Example |
|-----|-------------|---------|
| %1 | Full path to file | C:\PCB\UL\MYFILE.ZIP |
| %2 | Upload type | UPLOAD, ATTACH, or TEST |
| %3 | Description file | C:\PCB\UPDESC.TMP |
| %4 | Original filename | MYFILE.ZIP |

## Exit Codes

| Code | Meaning | PCBoard Action |
|------|---------|----------------|
| 0 | PASS | File accepted, PCBPASS.TXT shown |
| 1 | FAIL | File rejected, PCBFAIL.TXT shown |
| 2 | ERROR | Scanner couldn't run |

## Virus Scanner Hook

Set the `PCBPROSCAN_AV` environment variable to chain an
external virus scanner:

```batch
SET PCBPROSCAN_AV=clamdscan --no-summary
```

pcbpscan runs the AV command with the file path as argument.
Non-zero exit from the AV = file fails.

## Compiling

```bash
gcc -o pcbpscan pcbpscan.c -Wall -O2
wcc386 pcbpscan.c -bt=dos -mf -5 -ox
```

## Files

```
thdproscan/
├── pcbpscan.c     Source code (C, portable)
├── pcbpscan       Compiled binary
├── PCBTEST.BAT      PCBoard integration script
└── README.md        This file
```

## Design Philosophy

thdproscan is a standalone tool. It knows nothing about PCBoard
internals — no headers, no libraries, no dependencies. PCBoard
calls it via PCBTEST.BAT and reads the result files. This loose
coupling means:

- Works with any PCBoard version (15.0+)
- Can be replaced with any other scanner
- Can be tested independently
- No risk of breaking PCBoard if scanner has bugs
- Same scanner works for FTP uploads, TIC imports, etc.

## Credits

Clean room design: evga, kiddo, sysop/0 (pcbirc crew)
Part of pcbrevival (GPL v3.0)
