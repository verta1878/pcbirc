# wip — Work In Progress (Pascal to C Ports)

Pascal source copied from `verta1878/mysticbbsirc` for porting to C.
Each subdirectory holds the Pascal being ported and the C growing
beside it.

| Directory | What | Source |
|---|---|---|
| `mail/` | OpenOLMS — QWK, Blue Wave, Hudson, JAM | OL_*.pas (15 units) |
| `fmt/` | Format handlers — ANSI, ASCII, Avatar, etc | m_pd*.pas (12 units) |
| `term/` | RIP engine + canvas | rip4api.pas, rip_surface.pas |
| `comm/` | Serial, FOSSIL | m_serial.pas, m_fossil.pas |
| `xfer/` | Zmodem, Xmodem, Ymodem, Kermit | m_prot_*.pas |
| `net/` | TCP, FTP, SMTP | m_tcp_*.pas |
| `crypto/` | CRC, crypt | m_crc.pas, m_crypt.pas |
| `pcb/` | Drop files | pcbdrop.pas |
| `pcbterm/` | Terminal client | mterm.pas, mtrip.pas |
| `pcbdraw/` | ANSI editor | ansiedit.pas |

Port order: mail first (objective test oracles), then pcb, fmt,
test RIP engines, term, pcbterm, pcbdraw.

Credit: kiddo (engines, terminal, editor), sysop/0 (codecs, audio).
Upstream: verta1878/mysticbbsirc, GPLv3.
