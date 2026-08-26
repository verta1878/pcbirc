# dropbear — SSH Reference Source (vendored)

Small, portable SSH server and client by Matt Johnston.
MIT-style license (see LICENSE). Source: https://github.com/mkj/dropbear
Upstream's own readme is preserved here as `README.dropbear.md`.

## Why Dropbear for PCBoard 15.41

PCBoard historically took callers over modem, then telnet. Telnet is
plaintext. Dropbear gives the 15.41 IRC branch a **secure remote-access
front end** — SSH instead of (or alongside) telnet — with a footprint
small enough to suit the retro/DOS spirit of the project. It sits
naturally beside the other network front-ends here (syncterm, binkd,
pcbcomm): the caller connects over SSH and the session is handed to
PCBoard like any telnet/FOSSIL session.

## Layout (upstream)

- `src/` — server (dropbear), client (dbclient), key tools
  (dropbearkey), format converter (dropbearconvert)
- `libtomcrypt/`, `libtommath/` — bundled crypto + bignum (self-
  contained, no external OpenSSL — ideal for porting to constrained
  targets)
- `options.h` / `default_options.h` — compile-time feature switches
- `SMALL.md`, `MULTI.md` — trimming size / single multi-call binary

## Usage in pcbrevival

Reference for a secure remote-access layer for PCBoard 15.41:
- SSH server terminating caller connections; session handed to PCBoard
  through the unified serial/comm layer (see pcbcomm/).
- The bundled libtomcrypt/libtommath make Dropbear self-contained —
  far easier to port than an OpenSSL-linked SSH.
- SMALL.md / MULTI.md upstream describe a small single multi-call
  binary — the right model for a BBS front end.

## Credits

Matt Johnston <matt@ucc.asn.au> and Dropbear contributors.
libtomcrypt / libtommath by Tom St Denis and contributors.
