# Phase 9: Hardware Drivers & Serial Stack (evga)

## Overview

evga handles all hardware-level driver work: serial I/O, UART,
multi-port cards, modem control, and FOSSIL interfaces. This phase
covers everything that talks directly to hardware or OS serial APIs.

Work starts after the Cyclades CD1400 driver is complete.

## Current: Cyclades Cyclom-8YO Driver

**Status: IN PROGRESS**

8-port ISA serial card using Cirrus Logic CD1400 UART chips.
Friend's card, needs drivers for DOS/OS2/Win2K/XP.

- [ ] Study CD1400 register set (documented in Linux kernel `drivers/char/cyclades.c`)
- [ ] DOS TSR driver (memory-mapped I/O at D4000h-D5FFFh)
- [ ] OS/2 device driver (.SYS)
- [ ] Windows 2000/XP driver (.SYS, WDM)
- [ ] FOSSIL INT 14h interface for all platforms
- [ ] Test with PCBoard multi-node (8 simultaneous nodes)

Reference: Linux `cy` driver has full CD1400 register documentation.

---

## Queue: After Cyclades Driver

### 1. DigiBoard SDK Integration

Multi-port serial card support for PCBoard multi-node setups.

- [ ] Obtain DigiBoard SDK (AccelePort, PC/Xi, PC/Xe series)
- [ ] Study DigiBoard memory-mapped I/O vs standard UART
- [ ] Write FOSSIL INT 14h wrapper for DigiBoard ports
- [ ] Test with pcbdraw teleconference (sub-phase 5c)
- [ ] Document supported card models and jumper settings

### 2. pcbdraw Serial Transport (pdserial.c)

Feeds into pcbdraw Phase 5c. evga writes the low-level serial I/O,
pcbdraw team wires it into the editor.

- [ ] Abstract transport layer API (function pointers for send/recv)
- [ ] FOSSIL INT 14h backend (DOS)
- [ ] SIO driver backend (OS/2) — evga already knows this cold
- [ ] Direct UART backend (16550A/16650/16750, fallback when no FOSSIL)
- [ ] Modem control: ATZ init, ATDT dial out, ATA answer
- [ ] Carrier detect (DCD line monitoring)
- [ ] Ring detect (RI line monitoring for auto-answer)
- [ ] DTR drop for hangup
- [ ] Baud rate negotiation / CONNECT string parsing
- [ ] Hardware flow control (RTS/CTS)
- [ ] Software flow control (XON/XOFF) — needed for some terminal emulators

### 3. SIO v2 / SIO2K Enhancements

Follow-up to the 31-bug audit. Now that the driver is clean,
potential enhancements:

- [ ] 16950 UART support (128-byte FIFO, auto RTS/CTS)
- [ ] USB-to-serial adapter pass-through (OS/2 USB stack)
- [ ] Performance profiling on real OS/2 hardware
- [ ] SIO API documentation for third-party developers
- [ ] Test with pcbbinkp answer mode (serial BinkP over modem)

### 4. FOSSIL Driver Enhancements (with wrench)

wrench owns the FOSSIL socket layer, evga provides hardware expertise.

- [ ] Review wrench's serial_ext.pas against real hardware behavior
- [ ] Test DosIdle yield on OS/2 VDM under load
- [ ] FOSSIL compliance test suite (INT 14h function 00h-1Bh)
- [ ] High-speed FOSSIL (115200+ baud) — 16550A FIFO tuning
- [ ] FOSSIL over USB-serial adapters

### 5. Internet Email Tools — C Ports

Recreate the PCBoard internet email PPEs in C for 15.4.
evga handles the low-level SMTP/POP3 protocol, pcbirc team
handles PCBoard integration.

- [ ] Study ADRS101 PPE source — email address display
- [ ] Study ENAME101 PPE source — username→email conversion (RFC 1137)
- [ ] Study E-BLT12 PPE source (FREEWARE w/ source) — FidoNet↔Internet address
- [ ] pcbmail.c: standalone SMTP send (outbound email from PCBoard)
  - [ ] SMTP client (connect, EHLO, AUTH, MAIL FROM, RCPT TO, DATA, QUIT)
  - [ ] MIME message formatting (headers, body, attachments)
  - [ ] TLS/STARTTLS support (if feasible on DOS/OS2)
  - [ ] Queue spooler (write to outbound/, send on schedule)
- [ ] pcbpop3.c: standalone POP3 receive (inbound email to PCBoard)
  - [ ] POP3 client (USER, PASS, STAT, LIST, RETR, DELE, QUIT)
  - [ ] Import into PCBoard message conference
  - [ ] MIME parsing (decode base64/quoted-printable)
- [ ] pcbsmtp.c: SMTP listener (receive email directly)
  - [ ] SMTP server (listen, accept, receive DATA)
  - [ ] Route to PCBoard conference based on recipient
  - [ ] Relay protection (only accept for local domains)

### 6. VMODEM Enhancements

evga's VMODEM (virtual modem) is already in the SIO package.
Potential enhancements for pcbdraw and pcbbinkp:

- [ ] VMODEM telnet negotiation (IAC DO/DONT/WILL/WONT)
- [ ] VMODEM SSH tunnel support
- [ ] VMODEM as pcbdraw transport (--vmodem flag)
- [ ] VMODEM integration with pcbbinkp answer mode

---

## Dependencies

| Task | Depends On | Blocks |
|------|-----------|--------|
| Cyclades driver | CD1400 register docs (Linux kernel) | Nothing (friend's request) |
| DigiBoard SDK | SDK acquisition | pcbdraw multi-port |
| pdserial.c | Cyclades/DigiBoard experience | pcbdraw Phase 5c |
| SIO v2 enhancements | SIO audit complete ✅ | Nothing |
| FOSSIL enhancements | wrench's serial_ext.pas | Nothing |
| Email C ports | PPE source study | Phase 5f, future email phase |
| VMODEM enhancements | Current VMODEM code | pcbdraw, pcbbinkp |

## Priority Order

1. **Cyclades CD1400 driver** (in progress, friend waiting)
2. **pdserial.c** (pcbdraw Phase 5c needs this)
3. **DigiBoard integration** (multi-port for BBS operators)
4. **Email tools study** (extract PPE sources, document patterns)
5. **SIO enhancements** (nice to have)
6. **FOSSIL work with wrench** (coordination)
7. **VMODEM enhancements** (future)
8. **pcbmail/pcbpop3/pcbsmtp** (future email phase)

## Delivered So Far

| Deliverable | Status | Lines |
|------------|--------|-------|
| SIO v1 rebuild (31 bugs fixed) | ✅ DONE | 3,800+ asm |
| SIO2K rebuild | ✅ DONE | 4,200+ asm |
| VMODEM | ✅ DONE | ~1,200 |
| md5.c/h (used by pcbbinkp) | ✅ DONE | 163 |
| BUGFIXES.md audit trail | ✅ DONE | 200+ |

## Credits

evga — SIO OS/2 driver, VMODEM, hardware expertise
wrench — FOSSIL socket layer, serial_ext.pas
hexadecimal — SIO audit (4 ISR/ring buffer issues found)
