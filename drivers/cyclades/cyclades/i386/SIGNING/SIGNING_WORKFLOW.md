# Driver Signing Tools & Workflow

## Where to Get These Tools

**Signing tools (MakeCert, signtool, pvk2pfx):**
  https://github.com/ricaun/MakeCert
  Release zip: https://github.com/ricaun/MakeCert/releases/download/1.0.0/MakeCert.zip
  Extracted from Microsoft Windows SDK 10.0.22621.0 (x64).
  These are portable — copy to any machine and run, no install needed.

**Certificate tools (certutil, certreq):**
  Ship with Windows Server 2003+ and XP+.
  Also available from the Windows SDK or Server install media.

**Windows Driver Kit (WDK) — needed for building, not just signing:**
  WDK 7600 (targets Win2K/XP/Vista/7): https://www.microsoft.com/en-us/download/details.aspx?id=11800
  WDK 10 (targets Win7+): https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk

**FindWDK (CMake module for building drivers without build.exe):**
  https://github.com/SergiusTheBest/FindWDK

---

## Platform Compatibility

| Tool | Runs on Win2K/XP? | Runs on Vista+? | Source |
|------|--------------------|-----------------|--------|
| certutil.exe (Srv2003) | YES | YES | Your zip — Server 2003 SP1 |
| certreq.exe (Srv2003) | YES | YES | Your zip — Server 2003 SP1 |
| MakeCert.exe (SDK 22621) | NO (needs Vista+) | YES | ricaun/MakeCert on GitHub |
| signtool.exe (SDK 22621) | NO (needs Vista+) | YES | ricaun/MakeCert on GitHub |
| pvk2pfx.exe (SDK 22621) | NO (needs Vista+) | YES | ricaun/MakeCert on GitHub |
| makecert.exe (PSDK 2003) | YES | YES | xp_compatible/ — Platform SDK Srv2003 SP1 |
| signtool.exe (PSDK 2003) | YES | YES | xp_compatible/ — Platform SDK Srv2003 SP1 |
| pvk2pfx.exe (PSDK 2003) | YES | YES | xp_compatible/ — Platform SDK Srv2003 SP1 |
| Cert2Spc.exe (PSDK 2003) | YES | YES | xp_compatible/ — cert to SPC converter |
| CertMgr.exe (PSDK 2003) | YES | YES | xp_compatible/ — certificate manager |

### XP-compatible versions INCLUDED in xp_compatible/ subfolder
Extracted from Platform SDK for Windows Server 2003 SP1 (April 2005).
Source: https://archive.org/details/platform-sdk-from-microsoft-windows-server-2003-with-service-pack-1-april-2005-edition-english
All x86 PE32, run on Win2K/XP/Server 2003 and later.

### However: Win2K/XP don't ENFORCE driver signing
On Win2K, unsigned drivers load without any prompt.
On XP, unsigned drivers show a warning dialog — click "Continue Anyway."
So you may not need signing tools on XP at all. They're only strictly
required for Win7 x64 and later.

### Practical recommendation
- **Building on Win2K/XP:** Use certutil/certreq from your zip, or
  don't bother signing (not enforced)
- **Building on Win7+:** Use MakeCert/signtool from this zip (SDK 22621)
- **Targeting Win7 x64:** Must sign. Use this zip's tools on a Win7+ build machine
- **Targeting Win10/11:** Must get MS attestation signing (EV cert + Dev Portal)

---

## Tools Included

| Tool | Version | From | Purpose |
|------|---------|------|---------|
| signtool.exe | 4.00 (SDK 22621) | Windows 11 SDK | Sign .sys, .dll, .cat files |
| MakeCert.exe | 10.0.22621.3233 | Windows 11 SDK | Create test certificates |
| pvk2pfx.exe | 10.0.22621.3233 | Windows 11 SDK | Convert .pvk+.cer to .pfx |
| mssign32.dll | SDK 22621 | Windows 11 SDK | Required by signtool |
| certutil.exe | 5.2.3790.1830 | Server 2003 SP1 | Manage certificate stores |
| certreq.exe | 5.2.3790.1830 | Server 2003 SP1 | Submit CSRs to a CA |
| certadm.dll | 5.2.3790.0 | Server 2003 RTM | Certificate admin DLL |
| certcli.dll | 5.1.2600.5512 | XP SP3 | Certificate client DLL |

## Test Signing Workflow (Windows 7 x64)

### Step 1: Create a test certificate
```bat
MakeCert.exe -r -pe -ss PrivateCertStore -n "CN=Cyclades Test" ^
    -a sha256 -sky signature ^
    -eku 1.3.6.1.5.5.7.3.3 ^
    -sv cytest.pvk cytest.cer
```
- `-r` = self-signed root
- `-pe` = private key exportable
- `-ss PrivateCertStore` = store in named cert store
- `-a sha256` = SHA-256 hash
- `-sky signature` = key type is signature
- `-eku 1.3.6.1.5.5.7.3.3` = code signing OID
- `-sv cytest.pvk` = private key file
- `-n "CN=Cyclades Test"` = certificate subject

### Step 2: Convert to PFX (combines cert + private key)
```bat
pvk2pfx.exe -pvk cytest.pvk -spc cytest.cer -pfx cytest.pfx
```

### Step 3: Sign the driver
```bat
signtool.exe sign /v /f cytest.pfx /t http://timestamp.digicert.com ^
    cyport.sys
```
- `/v` = verbose
- `/f cytest.pfx` = certificate file
- `/t` = timestamp server (proves when it was signed)

### Step 4: Create and sign the catalog file
```bat
REM Create .cat from .inf using makecat (from WDK, not included here)
signtool.exe sign /v /f cytest.pfx /t http://timestamp.digicert.com ^
    cyport.cat
```

### Step 5: Enable test signing on target machine
```bat
bcdedit.exe -set TESTSIGNING ON
REM Reboot required
```

### Step 6: Install the certificate on target machine
```bat
certutil.exe -addstore TrustedPublisher cytest.cer
certutil.exe -addstore Root cytest.cer
```

### Step 7: Install the driver
```bat
REM Via Device Manager "Have Disk" or:
pnputil.exe -a cyport.inf
```

## Production Signing Workflow (Win10/Win11 with Secure Boot)

### Step 1: Purchase an EV code-signing certificate
- DigiCert, Sectigo, GlobalSign (~$300-500/year)
- Must be EV (Extended Validation) for kernel drivers
- Delivered on a hardware token (USB dongle)

### Step 2: Register at Microsoft Partner Center
- https://partner.microsoft.com/dashboard
- Associate your EV cert with your Partner Center account

### Step 3: Sign the driver package
```bat
signtool.exe sign /v /a /fd sha256 /tr http://timestamp.digicert.com ^
    /td sha256 cyport.sys
```

### Step 4: Submit to Microsoft Dev Portal
- Upload: cyport.sys + cyport.inf + cyport.cat
- Microsoft runs automated tests
- Microsoft signs with their root certificate
- Download the Microsoft-signed package

### Step 5: Distribute
- The Microsoft-signed driver loads on all Windows versions
- Secure Boot enabled: works
- No TESTSIGNING needed
