# pcbpscan — PCBoard File Scanner

**Clean-room replacement for THD ProScan.**

Part of pcbrevival (GPL v3.0). Tests uploaded files for integrity,
extracts FILE_ID.DIZ, runs virus scans, and updates PCBoard DIR listings.

## Why Not THD ProScan?

THD ProScan (thdproscan) has 4 bugs that make it unusable for
production PCBoard systems:

1. **Config never loads** — `TODO: read binary TCfg record from THDINSTL`.
   THDINSTL writes a binary config, but the reader was never implemented.
   Only the text KEY=VALUE fallback works.

2. **ClamAV exit code 2 ignored** — ClamAV returns 2 for scanner errors
   (corrupt file, engine failure). ProScan only checks for 1 (virus found)
   and ignores 2, silently passing files the scanner couldn't read.
   Same issue with McAfee RC=2.

3. **Archive listing incomplete** — `m_archive.pas:363`: "Currently returns
   empty list for external tools (parse TODO)". Only ZIP gets internal
   listing. RAR/ARJ/LHA can test/extract but return no file list, so
   path traversal and zip bomb checks can't run on those formats.

4. **BBS file writer not implemented** — `thdplus.pas:108`: "TODO: Write
   to FILES.BBS / RA / Renegade / PCBoard / Mystic". The database updater
   reads TESTINFO.DAT but can't write results back to any BBS format.
   FILE_ID.DIZ extraction is dead code.

pcbpscan fixes all 4.

## Features

- ZIP integrity check (central directory verification)
- ARJ/RAR/LHA/7Z integrity via external tools (arj/unrar/lha/7z)
- FILE_ID.DIZ/ANS extraction from ZIP (internal) and other formats (external)
- Writes extracted DIZ to PCBoard upload description file
- Appends to PCBoard DIR listing file
- External virus scanner hook with proper exit code handling:
  RC=0 clean, RC=1 virus (FAIL), RC=2 scanner error (ERROR)
- Banned file extension checking
- Path traversal detection in archived filenames
- Zip bomb detection (max file count limit)
- Max file size limit
- Config file (pcbpscan.cfg) with KEY=VALUE format
- Environment variable overrides

## Usage

Called by PCBTEST.BAT (PCBoard's upload verification hook):

```bat
@echo off
pcbpscan %1 %2 %3 %4
if errorlevel 2 echo ERROR > PCBERROR.TXT
if errorlevel 1 echo FAILED > PCBFAIL.TXT
```

Arguments (from PCBoard `verifyfile()`):
- `%1` = full path to uploaded file
- `%2` = "UPLOAD", "ATTACH", or "TEST"
- `%3` = upload description file path
- `%4` = original filename

Exit codes:
- 0 = PASS (file is OK)
- 1 = FAIL (corrupt, dangerous, or virus found)
- 2 = ERROR (scanner malfunction, needs manual review)

## Configuration

Create `pcbpscan.cfg` alongside the executable:

```
# Virus scanner command (file path appended automatically)
AV_CMD=clamscan --no-summary

# Verbosity (0=quiet, 1=verbose)
VERBOSE=0

# Maximum file size in bytes (default 100MB)
MAX_FILE_SIZE=104857600

# Maximum files in one archive (zip bomb protection)
MAX_FILES_IN_ARC=10000

# Banned file extensions (space-separated)
BANNED_EXTS=.exe .com .bat .cmd .scr .pif .vbs .js

# Write FILE_ID.DIZ to upload description file (1=yes)
DIZ_TO_DESC=1

# PCBoard DIR listing file to append entries to (blank=disabled)
DIR_FILE=C:\PCB\GEN\DIR0
```

Environment variables override config:
- `PCBPROSCAN_AV` = virus scanner command
- `PCBPROSCAN_VERBOSE` = enable verbose output

## Build

OpenWatcom (NT):
```
wcc386 -5r -oxs -bt=nt pcbpscan.c
wlink system nt name PCBPSCAN_W.EXE file pcbpscan.obj
```

## Source

770 lines of C, single file, no dependencies. Compiles under
OpenWatcom 2.0 (DOS/OS2/NT).
