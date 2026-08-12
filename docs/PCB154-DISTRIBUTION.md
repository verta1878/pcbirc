# PCBoard 15.4 Distribution Plan

## Original 15.3 Distribution (September 1996)

Three ZIP files (electronic distribution via BBS):
- `pcb153-1.zip` (1.2MB) — Part 1: Installation + Setup
- `pcb153-2.zip` (1.4MB) — Part 2: Programs + Executables  
- `pcb153-3.zip` (1.2MB) — Part 3: Documentation

Three floppy disks (retail boxed distribution):
- Disk 1 of 3 — Installation Disk
- Disk 2 of 3 — Program Disk
- Disk 3 of 3 — Documentation Disk

Reference: Internet Archive has PCBoard v15.2 disk images with label scans:
https://archive.org/details/pcboard-v-15.2-clark-dev-co.-1994

## PCBoard 15.4 Distribution (pcbrevival)

### Repo Structure Update

```
pcbrevival/
├── dist/
│   ├── disk1/              ← Installation disk
│   │   ├── INSTALL.EXE     ← PCBoard installer
│   │   ├── INSTALL.TXT     ← Installation guide
│   │   ├── README.1ST      ← Read me first
│   │   └── PKUNZIP.EXE     ← Archive extractor
│   │
│   ├── disk2/              ← Program disk
│   │   ├── PCB154.ZIP      ← Main PCBoard executables
│   │   ├── PCBSETUP.ZIP    ← Setup utility
│   │   ├── PCBSM.ZIP       ← System manager
│   │   ├── PCBTEXT.ZIP     ← Text editor
│   │   └── PPLC.ZIP        ← PPL compiler
│   │
│   ├── disk3/              ← Documentation disk
│   │   ├── FEATURES.TXT    ← New features in 15.4 Revival
│   │   ├── UPGRADE.TXT     ← Upgrade guide from 15.3
│   │   ├── SYSGUIDE.ZIP    ← Sysop's guide
│   │   ├── CALGUIDE.ZIP    ← Caller's guide
│   │   └── WHATSNEW.TXT    ← Change log
│   │
│   ├── disk4/              ← Internet Server disk
│   │   ├── PCBIS.EXE       ← PCBoard Internet Services daemon
│   │   ├── PCBIS.CFG       ← Sample configuration
│   │   ├── PCBIS.TXT       ← Internet Services guide
│   │   ├── INDEX.HTM       ← Default BBS web page
│   │   ├── QFRONT.TXT      ← QFront BinkP integration notes
│   │   ├── PCBUUCP.EXE     ← UUCP over Internet (TCP transport, same protocol)
│   │   ├── PCBNNTP.EXE     ← NNTP newsgroup client (native internet protocol)
│   │   ├── PCBNNTPO.EXE    ← NNTP outbound poster
│   │   ├── UUCP.TXT        ← UUCP-over-Internet setup guide
│   │   └── NNTP.TXT        ← NNTP newsgroup setup guide
│   │
│   └── full/               ← Single-archive distribution
│       └── PCB154R.ZIP     ← Everything in one file
│
├── images/
│   ├── disk1_label.png     ← Disk 1 label (15.4 style)
│   ├── disk2_label.png     ← Disk 2 label
│   ├── disk3_label.png     ← Disk 3 label
│   ├── pcb154_logo.png     ← PCBoard 15.4 logo
│   ├── pcb154_splash.ans   ← ANSI splash screen
│   └── box_art.png         ← Retail box mockup
│
├── docs/
│   ├── FEATURES.TXT        ← 15.4 feature list
│   ├── INSTALL.TXT         ← Installation guide
│   ├── UPGRADE.TXT         ← Upgrade from 15.3
│   ├── WHATSNEW.TXT        ← Detailed changelog
│   ├── SYSGUIDE.TXT        ← Sysop's guide
│   └── PCBIS.TXT           ← pcbis Internet Services guide
│
└── ... (existing repo structure)
```

### Disk Label Design (15.4)

Based on the 15.2 retail disk labels (white labels, Clark Development Co. logo):

**Original 15.2/15.3 label format:**
```
┌──────────────────────────────────────────┐
│  Clark Development Company, Inc.          │
│                                           │
│         ╔═══════════════════╗             │
│         ║    PCBoard BBS    ║             │
│         ║   Version 15.3    ║             │
│         ║                   ║             │
│         ║   Disk X of 3     ║             │
│         ║  [disk function]  ║             │
│         ╚═══════════════════╝             │
│                                           │
│  Copyright (C) 1983-1996                  │
│  Clark Development Company, Inc.          │
│  Murray, Utah                             │
└──────────────────────────────────────────┘
```

**Updated 15.4 Revival labels (PCB Revival Project):**
```
┌──────────────────────────────────────────┐
│  PCBoard BBS Revival Project              │
│                                           │
│         ╔═══════════════════╗             │
│         ║    PCBoard BBS    ║             │
│         ║  15.4 Revival     ║             │
│         ║                   ║             │
│         ║   Disk X of 4     ║             │
│         ║  [disk function]  ║             │
│         ╚═══════════════════╝             │
│                                           │
│  Based on PCBoard by Clark Dev. Co.       │
│  Source preserved by PWA                  │
│  github.com/verta1878/pcbrevival          │
└──────────────────────────────────────────┘
```

Disk functions:
- Disk 1 of 4: Installation Disk
- Disk 2 of 4: Program Disk
- Disk 3 of 4: Documentation Disk
- Disk 4 of 4: Internet Server

### PCBoard Color Scheme (for pcbis UI)

PCBoard's signature colors:
- **Header/title bar:** White on Blue (Attr $1F)
- **Menu highlight:** Yellow on Blue (Attr $1E)
- **Normal text:** Light Gray on Black (Attr $07)
- **Status line:** Black on Cyan (Attr $30)
- **Borders:** Cyan on Black (Attr $03)
- **Error/warning:** White on Red (Attr $4F)
- **Input field:** White on Black (Attr $0F)
- **Prompt:** Yellow on Black (Attr $0E)

PCBoard uses single-line box drawing (─│┌┐└┘├┤┬┴┼) for most borders,
double-line (═║╔╗╚╝╠╣╦╩╬) for headers.

The pcbis WFC screen should match this palette exactly.

### Documentation Files

**FEATURES.TXT** — New in 15.4:
- OS/2 native binary (OpenWatcom cross-compiled)
- pcbis.exe Internet Services daemon (telnet, BinkP, FTP, HTTP)
- QFront BinkP integration for FidoNet
- OpenWatcom C compiler toolchain (no Borland dependency)
- All 11 DOS binaries rebuilt from source
- Source code available (GitHub)

**INSTALL.TXT** — Installation guide:
- System requirements
- New install vs upgrade
- Disk-by-disk installation procedure
- pcbis setup
- Network configuration

**UPGRADE.TXT** — Upgrade from 15.3:
- Backup procedures
- File compatibility
- New configuration options
- pcbis migration from modem-based setup
- FOSSIL driver no longer needed (pcbis handles TCP directly)

**WHATSNEW.TXT** — Detailed changes:
- Per-binary changelog
- Source port notes
- Toolchain changes
- Known issues

### TODO for verta1878
- [ ] Create `dist/` directory structure in pcbrevival repo
- [ ] Generate ANSI splash screen (`images/pcb154_splash.ans`)
- [ ] Write FEATURES.TXT, INSTALL.TXT, UPGRADE.TXT, WHATSNEW.TXT
- [ ] Create disk label artwork (based on 15.2 Internet Archive scans)
- [ ] Package dist/disk1-3 with actual built binaries when hexadecimal's port is done
- [ ] Add pcbis to the distribution

o7
