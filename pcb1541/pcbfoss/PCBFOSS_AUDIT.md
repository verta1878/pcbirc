# pcbfoss Audit — Bugs Fixed in This Release

| ID | File | Severity | Issue | Fix |
|----|------|----------|-------|-----|
| BUG-1 | pcbfoss.pas | Medium | Fn $02 returned 0 immediately without blocking | FOSSILReadCharTimeout: spin-wait with DCD check + configurable timeout |
| BUG-2 | pcbfoss.pas | Medium | Fn $13 wrote single char, spec says CX-length string at ES:DI | FOSSILANSIWrite: copies string to DOS transfer buffer, passes CX+ES:DI |
| BUG-3 | pcbfoss.pas | Low | Double-init cleared pending data (no FActive check) | FOSSILInit: exits immediately if FActive already True |
| BUG-4 | pcbfoss_rings.pas | Low | Block I/O was per-byte (4096 calls for 4KB) | RingBlockWrite/Read: two-phase memcpy (head-to-end + wrap) |
| IMP-1 | pcbfoss.pas | Low | Overrun bit never set | FOSSILStatus: tracks FSTAT_OVERRUN in FOverrun flag |
| IMP-2 | pcbfoss.pas | Low | BaudRate 115 should be 1152 | FOSSILSetBaud: stores baud in units of 100 bps per FSC-0015 |

Reference: cyfossil.asm (Cyclades CD1400 FOSSIL driver, FTS-0001 Rev 5)
