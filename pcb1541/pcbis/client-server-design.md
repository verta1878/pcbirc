# PCB 15.41 — Network Client/Server + Teleconference — Design Draft

**Home: pcbis (PCBoard Internet Services).** pcbis is already the
multi-protocol TCP/IP daemon in front of PCBoard (Telnet, BinkP, FTP,
HTTP, SMTP, NNTP), so the network client/server and teleconference layer
belongs here. Build and prove it inside pcbis first; **if it works well,
we can then consider folding parts of it into PCBOARD.EXE itself.**

This is how to run PCBoard across a network: a server process that owns
the board's core data, and client applications (local console, remote
terminals, monitors, doors, teleconference participants) that talk to it
over TCP or serial. A design draft, not committed code.

## Where this starts

The starting point is **sysop/0's 386 client/server + teleconference**
code (adapted from the PabloDraw networking model — the same
transport/session ideas PabloDraw uses for its collaborative ANSI
sessions). That code already demonstrates the hard parts we need: a
listener accepting multiple connections, a session per connection, and a
real-time multi-user channel (teleconference). That is the seed of the
PCBoard 15.41 client/server.

## The core idea

One **server** owns the core data (message bases, user file, file
directories, config). **No client touches those files directly.**
Clients connect to the server and ask it to do things on their behalf.
This gives us, for free, the things a shared-file design can never do
safely:

- **Consistency** — one writer to the core data; no two nodes corrupting
  the user file at once.
- **Security** — clients get exactly the access their login grants; the
  data files are never exposed on a share.
- **Reach** — a client can run on the same box or across the network; it
  only needs to find and connect to the server.
- **Scale** — add nodes/clients without adding copies of the data.

## Pieces

### Server
- Owns core data; the single source of truth.
- Listens on a TCP port (and/or a serial line for legacy links).
- Accepts connections, creates one **session** per connection, tracks
  them, and serializes access to the shared data.
- Enforces auth and per-session access level before honoring requests.

### Client
- Any program that connects: the local console, a remote terminal
  front end, a monitor/spy tool, a door, a teleconference participant.
- Speaks the same request/response protocol regardless of transport.
- Holds a session handle from the server; never opens core data itself.

### Session (per connection)
A session moves through clear states, in increasing order of access:

1. **Connected, not logged in** — only a handful of calls allowed
   (identify server, begin login).
2. **User session** — after login, full access subject to that user's
   security level. One logged-in user per session.
3. **System session** — trusted internal tools (mail tosser, utilities):
   full data access, no per-user restriction. Gated by a **system
   password** so a random client that reaches the port can't get in.
4. **Config session** — system access *plus* configuration; the most
   privileged, gated by its own password, ideally one-at-a-time.

Each session has a unique connection id, used for addressing between
sessions (see Teleconference).

### Node model
Keep PCBoard's node concept: a logged-in session is assigned a node
number, and node config says what call types it allows (local, dialup,
telnet, SSH, FTP, front-end). This preserves how sysops think about the
board and how existing config maps in.

## Transports

- **TCP** — the primary path. Server listens on a configurable port;
  clients connect by host/port. Local console just connects to
  127.0.0.1.
- **Serial** — retained for legacy/direct links; same protocol over a
  serial line via the comm layer.
- Transport is a **backend**: the session/protocol layer above does not
  care whether bytes arrive over TCP, serial, telnet, or SSH. This is
  the same "one session interface, many backends" rule pcbcomm is being
  remade around — SSH (dropbear/) and telnet are just backends here too.

## Finding the server (discovery)

Two ways for a client to locate the server, mirroring how networked BBS
servers handle this in practice:

- **Broadcast/auto-discover** on the local subnet — convenient when
  everything is on one LAN.
- **Explicit config** — the client is told the server's IP/name directly
  (e.g. an env var or config entry). This is the reliable path **across
  routers/subnets**, where broadcasts don't cross. When in doubt, prefer
  explicit config; it avoids the whole class of "client can't find the
  server through the router" problems.

Practical guidance to carry into the docs:
- Test reachability first (ping the server by name and IP; confirm the
  route). Most "it won't connect" cases are a networking/routing problem,
  not the board.
- Use **one well-known server port**. **Block it at the router by
  default**; only open it when clients genuinely run off-box, and then
  require the system/config password.



sysop/0's teleconference is the model for real-time multi-user chat:
a **broadcast channel** where sessions join, send, and receive messages
addressed by connection id. Generalize it into a small channel service:

- sessions subscribe to a channel,
- a message sent to the channel fans out to all subscribers,
- private/targeted messages address a specific connection id.

That single primitive covers teleconference, node-to-node messages, and
sysop broadcast — and it's already proven in the 386 code.

## Security notes

- **Block the server port at the router** by default; only open it when
  clients genuinely run off-box.
- **System / config passwords** stamped once per trusted workstation so
  trusted tools connect without prompting, but a stranger who reaches the
  port still can't escalate.
- Per-session access level is enforced **on the server**, never assumed
  from the client.

## Why not just share the files

A shared-drive, everyone-opens-the-.DAT design is simpler to start but
can't guarantee consistency (multiple writers), can't secure the core
data (it's on a share), and doesn't cross networks cleanly. The
client/server model costs more up front and pays it back in exactly the
places a multi-node board gets bitten.

## Language / implementation

Home is **pcbis**, which today is built with fpc264irc (Pascal) as the
TCP/IP daemon. sysop/0's existing **386 client/server + teleconference**
(from the PabloDraw networking model) is the base to build from. The
transport backends line up with the comm layer (pcbcomm) — TCP, serial,
telnet, SSH (dropbear) all look the same to the session layer above.

Prove it in pcbis first. If it works, the parts worth having in the core
(the session/node model, the broadcast channel) can be considered for
folding into PCBOARD.EXE — at which point the C/C++ toolkit side and the
SDK's session/IO abstraction (todo/SDK-1541-IMPROVEMENTS.md) become the
in-core client interface.

## Open questions

- Protocol framing: fixed request/response records vs a small tagged
  message format.
- Threading model on the server (thread-per-session vs pooled).
- How much of PCBoard's existing file access is refactored behind the
  server vs wrapped.
- Where the local console sits (thin client over loopback vs special-
  cased in-process path).
