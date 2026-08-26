# pcbis — PCBoard Internet Services

Multi-protocol internet daemon bridging PCBoard to TCP/IP.
Built with fpc264irc (FPC 2.6.4 fork). 18 Pascal units, 5,710 lines.

Services:
- Telnet (2323) — FOSSIL bridge, writes PCBOARD.SYS/CALLERS
- BinkP (24554) — FidoNet mailer, QFront integration
- FTP (21) — PCBoard file areas with security levels
- HTTP (8080) — status page, callers, who's online
- SMTP — outbound validation emails
- NNTP — news to PCBoard conference gateway
- QWK — offline mail networking
- Events — timed batch execution

Key files: pcbis.pas (main), pcbis_binkp.pas, pcbis_ftp.pas,
pcbis_http.pas, pcbis_net.pas, pcbfoss.pas (FOSSIL bridge).

## Client/Server + Teleconference (design)

A network client/server and teleconference layer for PCBoard is drafted
in `CLIENT-SERVER-DESIGN.md`, starting from sysop/0's 386 client/server +
teleconference (PabloDraw model). pcbis is its home; if it proves out,
parts may later fold into PCBOARD.EXE.
