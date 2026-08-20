# Phase 21 — pcbis.exe (PCBoard Internet Services) — Full MIS-Style Plan

Updated from daemon-only to full MIS-style multi-protocol server with ANSI WFC console.
See conversation for complete plan. Key additions from original scope:

## Protocol Servers (6)
1. **Telnet** (2323) — bridges to PCBoard nodes, writes PCBOARD.SYS/CALLERS
2. **BinkP** (24554) — FidoNet mailer, QFront integration
3. **FTP** (21) — lightweight, reads DLPATH.LST for file areas
4. **HTTP** (8080) — static index.html + /status, /callers, /online endpoints
5. **SMTP** (outbound) — validation emails via relay
6. **Events** — BinkP poll schedule, log rotation

## WFC Console UI
- Adapted from MIS (mis_ansiwfc.pas) + Mystic ANSI framework (3,510 lines)
- TAB cycles: Telnet → BinkP → FTP → HTTP → SMTP → Events
- Shows: port, max, active, blocked, refused, total per protocol
- Scrollable server status log

## PCBoard Integration
- Writes PCBOARD.SYS, USERS.SYS, CALLERS, NODE*.DAT
- Who's online + last callers visible from inside PCBoard
- Reads PCBOARD.DAT for paths and node configuration

## Architecture: ~5,350 lines across 12 units (MIS pattern)
- TServerManager (threaded, per-protocol)
- TServerClient per connection
- Import ANSI UI from mystic-bbs-irc (no LCL)

## Implementation: 4 sub-phases
- 21a: Core + Telnet + WFC
- 21b: BinkP + Events
- 21c: FTP + HTTP
- 21d: SMTP + Polish

o7
