# pcbis Completion Plan — Remaining Work

## Status: 13 units coded (3,167 lines), 6 protocols wired

### What's Done
- Core daemon, select() loop, connection manager
- Telnet with pcbfoss FOSSIL bridge + PCBoard node file I/O
- BinkP/1.1 session handler
- FTP server (command parser, security TODOs documented)
- HTTP server (static + /status, /callers, /online)
- SMTP outbound relay + queue scanner
- Events engine (BinkP poll, SMTP queue, log rotate)
- Config parser, logging, sample config

### What's Remaining

---

## 1. QWK/QWKE Mail Networking

PCBoard already has QWK support for offline mail reading. What's needed:
QWK *networking* — board-to-board mail exchange via QWK packets.

### Architecture (from Wildcat! GATEWAY reference):
```
Export:  PCBoard → pcbgate e → .QWK packet → transport → hub
Import:  hub → transport → .REP packet → pcbgate i → PCBoard
```

### Files needed:
- `pcbis_qwk.pas` — QWK packet builder/reader
  - Read PCBoard message bases → build MESSAGES.DAT + CONTROL.DAT
  - Read incoming .REP packets → inject into message bases
  - Conference mapping (QWK conference numbers ↔ PCBoard conferences)
  
- `pcbis_qwke.pas` — QWKE extensions
  - Extended headers (To/From internet email, kludge lines)
  - UTF-8 support
  - Longer subject lines

### Transport options (Disk 4):
- Via FTP: push/pull QWK packets to/from hub's FTP server
- Via HTTP: POST .REP / GET .QWK from hub
- Via BinkP: piggyback on FidoNet sessions (QWK-over-BinkP)

---

## 2. Events System (Batch File Execution)

PCBoard supports 6 timed events that run batch files.
pcbis needs the same — plus internet-specific events.

### Event slots:
```
[events]
event1_time = 02:00
event1_batch = C:\PCB\EVENTS\MAINT.BAT
event1_days = MTWTFSS
event1_desc = Nightly maintenance

event2_time = 06:00
event2_batch = C:\PCB\EVENTS\BINKPOLL.BAT
event2_days = MTWTFSS
event2_desc = Morning BinkP poll

event3_time = 00:00
event3_batch = C:\PCB\EVENTS\LOGROT.BAT
event3_days = MTWTFSS
event3_desc = Log rotation

event4_time = 04:00
event4_batch = C:\PCB\EVENTS\QWKXFER.BAT
event4_days = MTWTF--
event4_desc = QWK network exchange

event5_time = 
event5_batch = 
event5_days = 
event5_desc = (unused)

event6_time = 
event6_batch = 
event6_days = 
event6_desc = (unused)
```

### Event execution flow (from Wildcat! B4GATE.BAT):
1. Bring nodes to "event pending" state
2. Wait for active callers to finish
3. Run batch file
4. Resume accepting callers

### Changes to pcbis_events.pas:
- Add TEventSlot record with time, batch path, day mask, description
- Add 6 configurable event slots from pcbis.cfg
- Shell execution: `fpSystem(BatchPath)` on Linux, `Exec` on DOS
- Event lock: pause accepting new connections during event
- Event log: timestamp + event name + batch exit code

---

## 3. Logging Plan

### Log files:
| Log file | What gets logged |
|----------|-----------------|
| `pcbis.log` | Main server log — startup, shutdown, config reload, errors |
| `telnet.log` | Telnet: connect, disconnect, node assignment, login, idle timeout |
| `binkp.log` | BinkP: session start, auth, file transfer, EOB, errors |
| `ftp.log` | FTP: login, CWD, RETR (download), STOR (upload), file sizes |
| `http.log` | HTTP: Apache common log format — IP, date, method, path, status, size |
| `smtp.log` | SMTP: queue, send attempt, relay response, delivery/failure |
| `event.log` | Events: trigger time, batch file, exit code, duration |
| `security.log` | Failed logins, blocked IPs, rate limit hits, auth failures |

### Log format (all logs):
```
YYYY-MM-DD HH:MM:SS [LEVEL] [PROTOCOL] message
```

### HTTP access log (Apache common format for compatibility):
```
127.0.0.1 - frank [10/Oct/2000:13:55:36 -0700] "GET /index.htm HTTP/1.0" 200 2326
```

### Per-protocol log detail:

**Telnet:**
```
2026-08-04 10:15:23 [INFO] [TELNET] 192.168.1.50 connected → node 1
2026-08-04 10:15:24 [INFO] [TELNET] 192.168.1.50 node 1 FOSSIL init (sig=$1954)
2026-08-04 10:15:30 [INFO] [TELNET] 192.168.1.50 node 1 login: SYSOP
2026-08-04 10:45:30 [INFO] [TELNET] 192.168.1.50 node 1 idle timeout (1800s)
2026-08-04 10:45:30 [INFO] [TELNET] 192.168.1.50 node 1 disconnect (duration: 30m00s)
```

**BinkP:**
```
2026-08-04 06:00:01 [INFO] [BINKP] Polling uplink 1:123/0 (hub.fido.net:24554)
2026-08-04 06:00:02 [INFO] [BINKP] 1:123/0 session authenticated
2026-08-04 06:00:03 [INFO] [BINKP] 1:123/0 receiving 00010023.PKT (12,450 bytes)
2026-08-04 06:00:05 [INFO] [BINKP] 1:123/0 received OK → /pcb/inbound/
2026-08-04 06:00:06 [INFO] [BINKP] 1:123/0 sending 00010024.PKT (3,200 bytes)
2026-08-04 06:00:07 [INFO] [BINKP] 1:123/0 EOB — session complete
```

**FTP:**
```
2026-08-04 14:20:00 [INFO] [FTP] 10.0.0.5 connected
2026-08-04 14:20:01 [INFO] [FTP] 10.0.0.5 login: JOHNDOE (security level 30)
2026-08-04 14:20:10 [INFO] [FTP] 10.0.0.5 RETR GAME.ZIP (1,234,567 bytes, 45.2 KB/s)
2026-08-04 14:20:15 [WARN] [FTP] 10.0.0.5 download limit reached (5MB daily)
2026-08-04 14:20:20 [INFO] [FTP] 10.0.0.5 logout (downloaded: 1 file, 1,234,567 bytes)
```

**HTTP:**
```
2026-08-04 12:00:00 [INFO] [HTTP] 192.168.1.1 GET / 200 2326
2026-08-04 12:00:01 [INFO] [HTTP] 192.168.1.1 GET /online 200 1450
2026-08-04 12:00:05 [INFO] [HTTP] 192.168.1.1 GET /missing 404 312
```

**SMTP:**
```
2026-08-04 03:00:00 [INFO] [SMTP] Queue scan: 2 messages pending
2026-08-04 03:00:01 [INFO] [SMTP] Sending to user@example.com via smtp.relay.com:587
2026-08-04 03:00:03 [INFO] [SMTP] Delivered: user@example.com (250 OK)
2026-08-04 03:00:04 [WARN] [SMTP] Failed: bad@invalid.com (550 No such user)
2026-08-04 03:00:04 [INFO] [SMTP] Queue: 1 delivered, 1 failed (retry in 300s)
```

**Security:**
```
2026-08-04 10:00:00 [WARN] [SECURITY] 10.0.0.99 FTP login failed: BADUSER (3rd attempt)
2026-08-04 10:00:01 [WARN] [SECURITY] 10.0.0.99 BLOCKED — 3 failed logins in 60s
2026-08-04 10:05:00 [INFO] [SECURITY] 10.0.0.99 unblocked after 300s cooldown
```

**Events:**
```
2026-08-04 02:00:00 [INFO] [EVENT] Event 1 triggered: Nightly maintenance
2026-08-04 02:00:00 [INFO] [EVENT] Pausing new connections...
2026-08-04 02:00:05 [INFO] [EVENT] All nodes idle, running: C:\PCB\EVENTS\MAINT.BAT
2026-08-04 02:03:15 [INFO] [EVENT] MAINT.BAT completed (exit code 0, duration: 3m10s)
2026-08-04 02:03:15 [INFO] [EVENT] Resuming connections
```

### Log rotation:
- Daily rotation at midnight (or configurable)
- Keep N days of logs (configurable, default 30)
- Compressed archives: pcbis-2026-08-03.log.gz

### Changes to pcbis_log.pas:
- Add per-protocol log file support
- Add log level filtering (DEBUG, INFO, WARN, ERROR)
- Add Apache common format for HTTP
- Add log rotation with date suffix
- Add configurable log directory

---

## 4. UI (WFC Screen from MIS)

### Source: mystic-bbs-irc/mystic/mis_ansiwfc.pas
- Import the ANSI console framework (3,510 lines)
- Adapt WFC layout for PCBoard colors (White/Blue headers, Cyan borders)
- TAB cycles: Telnet → BinkP → FTP → HTTP → SMTP → Events

### New unit: pcbis_wfc.pas
- DrawStatusScreen — the main WFC layout
- UpdateProtocolStats — refresh per-protocol counters
- UpdateServerLog — scroll the status log
- HandleKeyboard — TAB/SPACE/ESC/ALT-K

---

## 5. Remaining Code Changes

### pcbis_events.pas — add batch file execution:
- 6 configurable event slots from [events] section
- Time-of-day triggering with day-of-week mask
- Shell execution (fpSystem)
- Event lock (pause connections)

### pcbis_log.pas — expand to per-protocol logging:
- Multiple log file handles
- Apache common format for HTTP
- Security log for failed auth

### pcbis_config.pas — add new config sections:
- [events] event1-6 with time, batch, days, desc
- [ftp] port, enabled, anonymous access
- [http] port, enabled, docroot
- [logging] directory, level, rotation days
- [security] max_failed_logins, block_duration
- [qwk] hub_address, conferences, schedule

### pcbis_telnet.pas — NAWS support:
- Parse NAWS subnegotiation for window size
- Pass to PCBoard via FOSSIL info block (ScreenW/ScreenH)

---

## Implementation Order

1. **Logging expansion** — per-protocol logs, security log, rotation
2. **Events with batch execution** — 6 slots, shell exec, event lock
3. **Config expansion** — events, FTP, HTTP, logging, security sections
4. **QWK networking** — packet builder/reader, conference mapping
5. **WFC screen** — import MIS ANSI UI, PCBoard color scheme
6. **NAWS + polish** — window size, final integration

o7
