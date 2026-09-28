# QFront — FidoNet Mailer for PCBoard

Phase 15. Full FidoNet mailer: EMSI handshake, Zmodem file transfer,
event scheduling, file request handling.

22 source files in `src/`, style-audited (Phase 15). Compiles with
OpenWatcom (DOS/32A). 5 binaries: QF.EXE, QCONFIG.EXE, QLINK.EXE,
QSCAN.EXE, QUTIL.EXE.

Key files: emsi.c (EMSI protocol), modem.c (serial I/O),
session.c (mail session), events.c (event scheduler),
frequest.c (file request), bso.c (Binkley-Style Outbound).
