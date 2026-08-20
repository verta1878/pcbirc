# binkd — BinkP Reference Source

Reference BinkP/1.1 implementation by Dima Maloff (2:5047/13).
GPL v2+. Source: https://github.com/pgul/binkd

## Files

- protocol.c/h (3,473 lines) — full BinkP/1.1 protocol
- client.c (719 lines) — outbound session handler
- server.c (334 lines) — inbound session handler
- crypt.c/h — CRAM-MD5 authentication
- ftnaddr.c/h — FidoNet address parsing
- ftnnode.c/h — node configuration
- ftnq.c/h — outbound queue (BSO directory)
- bsy.c/h — busy flag management

## Usage in pcbrevival

Reference for implementing PCBoard 15.41's integrated BinkP
mailer (section 20 of PCB1541_DRAFT.md). The PCBoard
implementation will use binkd's protocol logic adapted for
PCBoard's session management and PCBTEXT string display.

## Credits

Dima Maloff (2:5047/13), Pavel Gulchouck, and binkd contributors.
