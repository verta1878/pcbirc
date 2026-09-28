# WCSC .RED archive contents — what pcbirc covers

## COMMDRV.RED (22 files) — CommDrv driver package

| File | Size | pcbirc parity | Notes |
|---|---:|---|---|
| COMMDV00.DRV | 1130 | ✅ pcbdcom uart_backend | 8250/16550 UART |
| COMMDV01.DRV | 1115 | ✅ pcbdcom hub6_backend | HUB-6 8-port |
| COMMDV02.DRV | 2276 | ✅ pcbdcom digi_comxi_backend | Digi COM/Xi |
| COMMDV03.DRV | 2686 | ✅ pcbdcom arnet_backend | Arnet SmartPort |
| COMMDV04.DRV | 2797 | ✅ pcbdcom boca_backend | Boca BB series |
| COMMDV05.DRV | 4883 | ✅ pcbdcom digi_pcxe_backend | Digi PC/Xe |
| COMMDV06.DRV | 1662 | ✅ pcbdcom gtek_backend | GTek 8/16-port |
| COMMDV07.DRV | 1212 | ✅ pcbdcom int14_backend | INT 14h passthrough |
| COMMDV08.DRV | 2284 | ✅ pcbdcom cyclom_backend | Cyclom Y/M (est.) |
| COMMDRV.EXE | 90827 | ✅ PCBDCOM.EXE | Main driver library |
| COMMTSR.EXE | 64101 | ✅ PCBDTSR.EXE | TSR component |
| DRVSETUP.EXE | 29752 | ✅ PCBDSET.EXE | Driver setup utility |
| TEST.EXE | 16482 | ✅ PCBDTEST.EXE | Test tool |
| XABIOS.BIN | 2048 | ❌ | WCSC copyright BIOS blob (firmware?) |
| XACOOK.BIN | 6144 | ❌ | WCSC binary blob |
| XACOMX.BIN | 6144 | ❌ | WCSC binary blob |
| BOCA1610.BIN | 3228 | ❌ | Boca 16/10 board firmware |
| ARNETSP4.DAT | 2053 | ❌ | Arnet SmartPort 4-port config |
| ARNETSP8.DAT | 3397 | ❌ | Arnet SmartPort 8-port config |
| DIGI4E.DAT | 2053 | ❌ | Digi 4-port EPROM config |
| DIGI8E.DAT | 3397 | ❌ | Digi 8-port EPROM config |
| MONITOR.BAT | 24 | n/a | Trivial batch file |

**Coverage: 13/22 (all executables + all 9 drivers).**

Missing: BIN/DAT files are hardware-specific firmware/config blobs.
pcbdcom drivers likely reference these at runtime — worth checking
if drivers embed the binary blobs directly or expect them on disk.

## PCBOARD.RED (4 files) — Main PCBoard binaries

| File | Size | pcbirc parity | Notes |
|---|---:|---|---|
| PCBOARD.EXE | 1034096 | ⏳ (upstream) | Main BBS binary — see pcb1541/, pcb154/, pcb153/ trees |
| PCBOARDM.EXE | 990944 | ⏳ | Multi-node variant |
| PPLC100.EXE | 67786 | ⏳ toolkit/pplc/ | PPL compiler v1.00 |
| PPLC330.EXE | 191918 | ⏳ toolkit/pplc/ | PPL compiler v3.30 — we have 3.00, 3.10, 3.20 |

## PCBOARD2.RED (10 files) — OS/2 support + call processor

| File | Size | pcbirc parity | Notes |
|---|---:|---|---|
| PCBCP.EXE | 94729 | ❌ | Call processor |
| PCBOARD2.EXE | 877627 | ❌ | OS/2 PCBoard core |
| PCBMONI2.EXE | 70676 | ❌ | OS/2 monitor |
| PCBPACK2.EXE | 96794 | ❌ | OS/2 packer |
| PCBTITLE.COM | 1962 | ❌ | Title displayer |
| USERNET2.EXE | 39440 | ❌ | Multi-node user tracker |
| BOARD.CMD | 292 | n/a | OS/2 startup script |
| STARTOS2.CMD | 535 | n/a | OS/2 launcher |
| SAMPLE.OS2 | 1374 | n/a | Sample config |
| PCBCP.HLP | 89095 | n/a | Help text |

## PCBMAIL.RED (4 files) — QWK mail packet handler

| File | Size | pcbirc parity | Notes |
|---|---:|---|---|
| PCBMAIL.EXE | 333312 | ❌ | Main mailer |
| BWCC.DLL | 164928 | 3rd party | Borland Windows Custom Controls |
| BC450RTL.DLL | 219648 | 3rd party | Borland C++ 4.50 runtime |
| PCBMAIL.HLP | 759288 | n/a | Help file |

## PPLC.RED (46 files) — PPL sample programs

Mix of .PPE (compiled) and .PPS (source). Examples:
HELLO1-7, ACCNTDBF, DBASE, DOORS, HAMURABI, KAL, LANGUAGE,
MORE, NODEFILE, OPPAGE, ORDER, PWRDWARN, START, WELFIRST.
Plus HOWTODBF.TXT, RUN1/2.BAT, WHATSNEW.200/300/310.

**These are TEACHING EXAMPLES for the toolkit/pplc/ compiler we already have.**
Worth landing under `toolkit/pplc/samples/` for reference.

## PCBCFGS.RED (171 files) — WCSC config data

Cryptically named 0, 1, ..., Z, 00, 10, ..., Z0, 01, 11, ..., Z3.
Small binary files, likely lookup tables or default configs for WCSC's
CommDrv product line. Not directly relevant to pcbdcom but worth
archiving as reference.

---

## Recommendations for v1.7+

1. **XABIOS.BIN / XACOOK.BIN / XACOMX.BIN / BOCA1610.BIN**
   Reverse-engineer to determine if they're runtime dependencies of
   pcbdcom drivers. If yes, either embed as `.h` blob or ship in
   drivers/ tree.

2. **ARNETSP4/8.DAT and DIGI4/8E.DAT**
   Likely EEPROM contents / init sequences. Cross-reference against
   arnet_backend.c and digi_pcxe_backend.c to see what they read.

3. **PPLC330.EXE parity**
   Toolkit currently ships PPLC 3.00/3.10/3.20. Landing 3.30 completes
   the PPL compiler lineage.

4. **PCBoard 2 OS/2 subtree**
   Not in pcbirc scope (pcbdcom is DOS-focused), but the OS/2 binaries
   are historically interesting. Could ship as reference/ tree.
