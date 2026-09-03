# PCBIC 1.2 — PCBoard InterCom

Clark Development's InterCom add-on — an internet/TCP-IP suite for
PCBoard (Telnet, FTP, Gopher, Finger, Ping, PPP/SLIP dialup, WHO).
Sold separately as a commercial add-on, never bundled with the main
PCBoard distribution.

## How this archive came to be recovered

The copy that reached us was ZipCrypto-encrypted by a third party — not
Clark. The archive (`Pcbic12.zip`) was locked with a password that was
never public and is not PCB153 (the password that works on Clark's other
zips). Whoever repackaged it added the encryption after the fact; it is
not how Clark originally distributed the IC.

Getting into it was not easy. The decryption and full extraction were
done by **byte** as a separate effort, afterwards — this is recovered material, not
something that arrived unlocked. The unlocked package now lives in
`decrypted/` (38 files). That recovery is the reason we have anything to
work from here at all.

## Files

```
Pcbic12.zip              Original encrypted archive (provenance)
Pcbic12-decrypted.zip    Unlocked archive (result of the recovery)
decrypted/               Extracted package (38 files)
```

## Package Contents

### Executables (rebuild targets)
- Pcbic.exe (313,310 bytes, 1997-04-17) — main InterCom
- Pcbic2.exe (217,111 bytes, 1997-04-30)
- PCBICCFG.EXE (185,398) — configuration
- PCBICEVT.EXE (89,612) — event handler
- TESTIC.EXE (40,104), TESTIC2.EXE (46,627) — test utilities

### Source we can rebuild
- RUNINET.PPS (4,016) — PPL source for the SLIP/PPP launcher PPE
- RUNINET.PPE (1,808) — compiled PPE (rebuild target from .PPS)

### Data / config
- DATA/ — menus, TCP text, service definitions (FTP, TELN, PING, etc.)
- SCRIPTS/ — SLIP/PPP dialer scripts (Win31, Win95, OS/2)

### Docs
- DOCS/PCBIC.DOC (111,614), PCBIC.PDF (339,182), README.1ST

## Rebuild Directive

Rebuild byte-for-byte with the same bugs. The .PPS -> .PPE rebuild via
our PPLC is the first verifiable target (compare output to the shipped
RUNINET.PPE). Fix bugs only AFTER full byte-for-byte restoration.

The main EXEs (Pcbic.exe etc.) have no recovered source yet — they're
preserved as reference binaries and the byte-exact targets. Source hunt
continues.

## Roadmap

See [`ROADMAP.md`](ROADMAP.md) for the v1.9+ byte-exact reconstruction
plan covering RUNINET.PPE (source in hand), the 6 EXEs (Ghidra Linux
workflow), and the adjacent INSTALL.EXE completion.
