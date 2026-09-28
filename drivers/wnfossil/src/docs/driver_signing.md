# Driver Signing Guide — SIO, Cyclades, WinFOSSIL VxD

Kernel-mode drivers (SIO2K.SYS, the Cyclades WDM driver, and the Win9x
FOSSIL.VXD) must be signed to load on 64-bit Windows. Since these are
GPLv3 hobbyist drivers without a commercial code-signing certificate,
we use **DSEO (Driver Signature Enforcement Overrider) 1.3b** to
self-sign and enable Test Signing Mode.

## What DSEO does
1. **Enable Test Mode** — puts Windows in test-signing mode so
   self-signed drivers load. A "Test Mode" watermark appears on the
   desktop (removable).
2. **Sign a System File** — applies a self-signed certificate to a
   .SYS / .VXD / .DLL so the loader accepts it.

## Signing our drivers (per driver)

```
dseo13b.exe
  → "Enable Test Mode"       (once per machine, then reboot)
  → "Sign a System File"
      C:\path\to\SIO2K.SYS
  → "Sign a System File"
      C:\path\to\CYCLADES.SYS
  → "Sign a System File"
      C:\path\to\FOSSIL.VXD   (Win9x only — no signing needed there)
```

Reboot after enabling Test Mode. The drivers then load with `sc start`
or via their INF install.

## Which drivers need signing

| Driver | Ring | Signing needed | Notes |
|--------|------|----------------|-------|
| WinFOSSIL FOSSIL.DLL (ring-3) | 3 | No | User-mode DLL, loads freely |
| WinFOSSIL FOSSIL.VXD (Win9x) | 0 | No | Win9x doesn't verify signatures |
| WinFOSSIL wf_vdd.c (NT VDD) | user | No | NTVDM loads it as a DLL |
| Cyclades WDM (Win2K-11) | 0 | **Yes** | Sign with DSEO on x64 |
| SIO2K.SYS (OS/2) | 0 | No | OS/2 has no signature enforcement |

Only the Cyclades WDM kernel driver strictly needs DSEO on modern x64
Windows. The FOSSIL drivers are either ring-3 (DLL) or target OSes that
don't enforce signatures (Win9x, OS/2).

## Production alternative
For a signed release without Test Mode, an EV code-signing certificate
(~$300/yr) would let the drivers load on stock x64 Windows. DSEO is the
free path for hobbyist / homelab use.

GPLv3 — FPC264IRC Contributors, 2026.
