# Release Build & Packaging Guide

## Release Package Contents

A complete release zip contains:

```
cyclades-1.0.0/
  README.txt              — Quick start guide
  LICENSE.txt             — GPLv3
  
  driver/
    cyport.sys            — Driver binary (FREE build, signed)
    cyclom-y.inf          — Bus enumerator INF
    cyyport.inf           — Port driver INF
    cyport.cat            — Catalog file (signed)
  
  driver_debug/
    cyport.sys            — Driver binary (CHECKED build, signed)
    cyclom-y.inf          — Same INFs
    cyyport.inf
    cyport.cat
  
  tools/
    cyinstall.exe         — Installer/uninstaller
    cytest.exe            — Hardware detection utility
    cyloopback.exe        — Loopback test (requires cable)
    cystress.exe          — Stress test (no cable needed)
  
  signing/
    cytest.cer            — Test certificate (if test-signed)
    MakeCert.exe          — Certificate creation tool
    signtool.exe          — Signing tool
    pvk2pfx.exe           — Key converter
    SIGNING_WORKFLOW.md   — How to sign for production
  
  doc/
    INSTALL_GUIDE.md      — Installation instructions
    TROUBLESHOOTING.md    — Debug and recovery guide
    BUILD.md              — How to build from source
    CHANGELOG.md          — Version history
  
  source/                 — Complete source code
    inc/                  — Headers
    src/                  — Driver source
    test/                 — Test utilities source
    tools/                — Installer source
    inf/                  — INF files
    build/                — WDK build configs
```

## How to Build the Release

### Prerequisites

1. **Windows DDK/WDK** — one of:
   - DDK 3790 (Server 2003 DDK) — targets Win2K/XP
   - WDK 7600.16385.1 — targets Win2K through Win7
   - WDK 10 — targets Win7 through Win11

2. **Windows SDK** (for user-mode tools):
   - Visual Studio 2005+ or Windows SDK with cl.exe

3. **Signing tools** (included in signing/ directory):
   - MakeCert.exe, signtool.exe, pvk2pfx.exe

### Step 1: Build the Driver (Kernel Mode)

```bat
REM === CHECKED (DEBUG) BUILD ===
REM Open "Windows XP Checked Build Environment" from WDK Start menu
cd cyclades\branch_a_kernel\build\checked
build -cef
REM Output: obj\i386\cyport.sys

REM === FREE (RELEASE) BUILD ===
REM Open "Windows XP Free Build Environment" from WDK Start menu
cd cyclades\branch_a_kernel\build\free
build -cef
REM Output: obj\i386\cyport.sys
```

### Step 2: Build the Message Compiler Output

```bat
REM Run the message compiler on cylog.mc
cd cyclades\branch_a_kernel\src
mc -v -w cylog.mc
REM Produces: cylog.h, cylog.rc, MSG00409.bin
REM Then rebuild the driver (cylog.h is now available)
```

### Step 3: Build User-Mode Tools

```bat
REM Using Visual Studio command prompt or Windows SDK
cd cyclades\branch_a_kernel\tools
cl /O2 /W4 cyinstall.c /Fe:cyinstall.exe ^
   setupapi.lib newdev.lib advapi32.lib cfgmgr32.lib

cd ..\test
cl /O2 /W4 cytest.c /Fe:cytest.exe
cl /O2 /W4 cyloopback.c /Fe:cyloopback.exe
cl /O2 /W4 cystress.c /Fe:cystress.exe
```

### Step 4: Create Test Certificate and Sign

```bat
REM Create a self-signed test certificate
MakeCert.exe -r -pe -ss PrivateCertStore ^
    -n "CN=Cyclades Test" -a sha256 ^
    -eku 1.3.6.1.5.5.7.3.3 ^
    -sv cytest.pvk cytest.cer

REM Convert to PFX
pvk2pfx.exe -pvk cytest.pvk -spc cytest.cer -pfx cytest.pfx

REM Sign the FREE build driver
signtool.exe sign /v /f cytest.pfx ^
    /t http://timestamp.digicert.com ^
    driver\cyport.sys

REM Sign the CHECKED build driver
signtool.exe sign /v /f cytest.pfx ^
    /t http://timestamp.digicert.com ^
    driver_debug\cyport.sys

REM Create and sign the catalog
REM (inf2cat from WDK creates .cat from .inf)
inf2cat /driver:driver /os:2000,XP_X86,7_X86,7_X64
signtool.exe sign /v /f cytest.pfx ^
    /t http://timestamp.digicert.com ^
    driver\cyport.cat
```

### Step 5: Package the Release

```bat
REM Run the release packaging script
cd cyclades
package_release.bat 1.0.0
REM Produces: cyclades-1.0.0.zip
```
