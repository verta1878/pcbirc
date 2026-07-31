# pcbrevival

**PCBoard 15.4 source code recovery and production build.**

12 of 12 binaries built from Clark Development's 15.3 source + reverse-engineered 15.4b features. Complete DOS and OS/2 platform. First time PCBoard source has been compilable outside Clark Development Company.

## What Is This

PCBoard was the dominant BBS software of the dial-up era, written by Clark Development Company. When Clark was closed by the bank in the late 1990s, the source code nearly disappeared. Corey Blake purchased what may be the only source license ever sold. PWA (Pirates with Attitude) preserved that 15.3 source archive — without them, this project would not exist.

This repo takes that 15.3 source, applies every feature and change from Clark's unreleased 15.4b beta, and produces all 12 shipping binaries from source.

## Build Status

### DOS: 11 of 11 ✅

| Binary | Size | Description |
|---|---|---|
| PCBOARDM.EXE | 974,144 | Main BBS engine (386+COMM) |
| LOCAL.EXE | 698,672 | Local login mode |
| PCBOARD.EXE | 1,011,232 | Non-386 overlay version |
| PPLC.EXE | 195,870 | PPL 3.40 compiler |
| PCBSM.EXE | 200,688 | System Manager |
| PCBSETUP.EXE | 380,752 | Setup utility |
| MKPCBTXT.EXE | 74,352 | Text file generator |
| UUIN.EXE | 281,432 | UUCP import |
| UUOUT.EXE | 203,994 | UUCP export |
| UUUTIL.EXE | 175,292 | UUCP utilities |
| UUXFER.EXE | 196,632 | UUCP transfer |

### OS/2: 1 of 1 ✅

| Binary | Size | Description |
|---|---|---|
| PCBOARD2.EXE | 1,354,240 | OS/2 32-bit native BBS engine |

Clean link, zero unresolved symbols. Only tested with OpenWatcom 2.0. Borland C++ 3.1 does not support OS/2 32-bit flat model — Clark used Watcom for the OS/2 target.

## 15.4 Features Implemented

- MD5 challenge-response login handshake
- PPL 3.40 compiler (14 new tokens including GETMSGHDR/SETMSGHDR/MOVEMSG)
- Personal PSA + Time/Byte Bank PSA
- CHAT @X color codes + COLOR command
- UUCP REJECTS sender-blacklist filter
- EMAIL:/WEB: login display fields
- U_SHORTDESC, U_GENDER, U_BIRTHDATE, U_EMAIL, U_WEB user fields
- MKPCBTXT 15.4 prompts (747-750)

## Build Requirements

**DOS:** Borland C++ 3.1, TASM 3.1, DOSBox 0.74
**OS/2:** OpenWatcom 2.0 (wpp386/wcc386/wasm cross-compile from Linux)

See [PCB154_BUILD_GUIDE.md](PCB154_BUILD_GUIDE.md) for full build instructions.

## Repo Structure

```
pcbrevival/
├── README.md                      This file
├── LICENSE.md                     GPL v3.0 + Clark's proprietary + LGPL
├── PCB154_BUILD_GUIDE.md          Complete build documentation
├── CHECKSUMS.md                   SHA-256 checksums for all binaries
├── 153_to_154.patch               Full provenance diff (~1350 files)
├── pcb153src0014.zip              Original 15.3 source (unmodified)
├── PCBSRC/                        Complete 15.4 source tree with patches
├── LIBS/
│   ├── CODEBASE/SOURCE/           CodeBase 4.x (LGPL v3.0)
│   └── PREBUILT/BC31/             10 .LIB + 10 .386
├── OUT/
│   ├── DOS/                       11 DOS binaries
│   └── OS2/                       PCBOARD2.EXE native OS/2
├── docs/                          Sysop guides, caller guide, modem configs
├── devtools/                      Developer kit, PPL toolkit, RIPscrip toolkit
├── historical/                    coreyblake.txt, source provenance
└── ppe-examples/                  PPE tutorials and samples
```

## Credits

### pcbrevival Crew
- **hexadecimal** — source maintainer, 15.3→15.4 port
- **verta1878** — crew

### Preservation
- **PWA (Pirates with Attitude)** — preserved the original pcb153src0014.zip source archive
- **Corey Blake** — purchased the 15.3 source license from Clark Development Company

### Original Software
- **Clark Development Company** — PCBoard BBS software, Copyright (C) 1996
- **Sequiter Software** — CodeBase 4.x database library (LGPL v3.0)

### The BBS Crew
- **hexadecimal** — pcbrevival, PCBoard 15.4 source maintainer
- **evga/kiddo** — Mystic BBS maintainer
- **sysop/0** — fpc264, Free Pascal cross-compiler maintainer
- **wrench** — netmodem2irc maintainer

## License

Our additions are **GPL v3.0**. Clark's PCBoard source is proprietary. CodeBase is **LGPL v3.0**. See [LICENSE.md](LICENSE.md) for details.
