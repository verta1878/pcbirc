# pcbmailer / QFront — FidoNet Mailer Orchestrator

## Status: DELIVERED (evga)

evga delivered a clean-room QFront replacement: 6,552 lines, 15 C files,
GPLv3. Written from FTS/FSC specs + QFront.DOC + string extraction only.
No reverse engineering.

Location: tools/qfront/

## What Exists

| Component | Author | Lines | Status |
|-----------|--------|-------|--------|
| QFront orchestrator | evga | 6,552 | Delivered |
| pcbbinkp (BinkP transport) | sysop/0 | 2,008 | Built |
| pcbis BinkP handler | sysop/0 | 257 | Built |
| PCBTOSS (tosser) | Clark | 6,217 | In PCBOARDM source |
| PCBNLC (nodelist compiler) | Clark | 1,037 | Linked (77K) |

## QFront Source Files

| File | Lines | What |
|------|-------|------|
| qfront.c | 708 | Main loop, config parser, logging |
| zmodem.c | 777 | Zmodem send/receive with CRC-16/32 |
| serial.c | 533 | Serial + TCP I/O (Win32 + POSIX) |
| session.c | 523 | Session manager, BSO file dispatch |
| emsi.c | 467 | EMSI/IEMSI handshake (FTS-0056) |
| xmodem.c | 449 | Xmodem-1K with CRC |
| events.c | 414 | Event scheduler (time-based + crash) |
| bso.c | 367 | BSO outbound scanner (FTS-5005) |
| modem.c | 367 | AT command modem control |
| nodelist.c | 343 | Nodelist parser (FTS-5000) |
| wazoo.c | 328 | YooHoo/WaZOO handshake |
| frequest.c | 312 | File request handler (magic names) |
| semaphore.c | 304 | Semaphore files (QQUEUE/QPOLLED) |
| route.c | 299 | Netmail routing (QTRANS.DAT equiv) |
| tic.c | 158 | TIC file processor (FTS-5006) |
| qfront.h | 203 | Shared types and defines |

## Architecture

QFront (orchestrator)
  - config parser (qfront.cfg)
  - event scheduler (time-based, crash detect)
  - BSO outbound scanner (FTS-5005)
  - nodelist lookup (FTS-5000 parser)
  - session dispatcher (EMSI/WaZOO + Zmodem/Xmodem built-in, BinkP via pcbbinkp)
  - modem control (AT commands)
  - file request handler
  - routing (QTRANS.DAT format)
  - semaphores (QQUEUE/QPOLLED/QUNDIAL)
  - post-session: drop .PKT to inbound, trigger PCBTOSS
  - TIC processor (FTS-5006)

## Specs Used (clean-room sources)

| Spec | What |
|------|------|
| FTS-0001 | .PKT file format |
| FTS-5000 | Nodelist format |
| FTS-5001 | BinkP protocol |
| FTS-5005 | BSO directory layout |
| FTS-5006 | TIC file format |
| FSC-0053 | BSO specification |
| FTS-0056 | EMSI session negotiation |

## Bug Audit (hexadecimal, August 2026)

Fixed:
- TIC filename path traversal (reject /, \, .., :)
- Zmodem received filename sanitization (strip paths, reject ..)
- SIGPIPE handling (signal(SIGPIPE, SIG_IGN))

Deferred to evga:
- EMSI payload buffer (2048 vs EMSI_DAT_MAXLEN 4096)
- Semaphore file locking (no flock/lockf)

## What We Do NOT Build

- Tosser: PCBTOSS (Clark, 6,217 lines) already does this
- Nodelist compiler: PCBNLC (Clark) already does this
- BinkP transport: pcbbinkp (sysop/0, 2,008 lines) already does this
