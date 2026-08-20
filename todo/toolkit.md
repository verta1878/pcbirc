# Toolkit — two versions, one tree

The toolkit follows the same split as everything else in the project: **15.4
restores what Clark shipped, 15.41 adds what we build.**

Two earlier drafts of this document got this wrong. The first merged Clark's
toolkit with our shared code and then spent pages solving problems the merge
created. The second separated them entirely — which protected 1995 doors but
denied new door authors everything we build. Neither was right.

## The two versions

| | Toolkit 15.4 | Toolkit 15.41 |
|---|---|---|
| **Purpose** | rebuild a 1995 door from source | write new doors |
| **Target** | 16-bit, four models, three compilers | **16-bit and flat** |
| **Convention** | Pascal, as Clark had it | C, with a Pascal-compatible variant |
| **Language** | pure C | C API, C++ internals allowed |
| **Contents** | Clark's 198 functions | Clark's 198 **plus crew code** |
| **Status** | frozen | grows |

**Crew code is not a separate product. It is what the 15.41 toolkit adds.** A
sysop writing a door in 2026 links the 15.41 toolkit and gets Zmodem, QWK,
RIP, SMTP, TCP and everything else the crew has built.

The two versions differ in **content**, not architecture. 15.4 is frozen at
Clark's 198 functions; 15.41 grows. Both build 16-bit.

## Why two versions at all, if both build 16-bit

Because 15.4 must stay **exactly** what Clark shipped — same 198 functions,
same behaviour, bug for bug. That is the preservation guarantee. The moment
we add a function it is no longer what Clark shipped.

15.41 is where things get added. Same architecture, growing content.

## Why 15.4 is frozen but not a burden

Existing door *binaries* do not use the toolkit. They are already compiled.
The toolkit only matters to someone **building** a door.

So Toolkit 15.4 is a preservation artifact — it exists so a period door can
be rebuilt from source with a period compiler. It is finished once and does
not change again.

Toolkit 15.41 is the living one. Everyone writing new code uses it.

That is why freezing 15.4 costs nothing: it is not in anyone's way.

## Two decisions this forces

**C++ across compilers.** C++ has no stable ABI — name mangling and vtable
layout differ between Borland, Watcom and Microsoft. Clark's own split shows
he knew this: 88 C++ files internally, zero in the toolkit, all 39 toolkit
files pure C.

The 15.41 answer is a C API with C++ internals allowed. `extern "C"` on the
surface, classes behind it. Any compiler can link it; we still get classes
where they help. Standard practice, costs nothing.

**Memory models — write 16-bit clean, build both.** The flat 32-bit model was
never a decision. It came in with code ported from Mystic, which is 32-bit. We
inherited it rather than choosing it.

16-bit-clean C compiles fine in flat model. The reverse does not. So writing
crew code 16-bit clean costs nothing and gets both targets from one source —
and a 16-bit door author is not shut out of Zmodem, QWK and RIP.

Clark's own library shows what this costs in practice: **16 of roughly 240
files use far or huge pointers.** About 7%, concentrated exactly where you
would expect — large-array sort and search (`ZSORT`, `ZSEARCH`, `ZSWAP*`),
block memory ops (`FMEMCPY`, `FMEMSET`), and screen buffers (`SAVEREST`,
`WINDOW`, `GETMODE`). For data exceeding even that, `MISC/VIRTUAL.C` is a
disk-backed virtual memory layer with a block cache, addressed through
`VirType huge *`.

Mapping crew code against the 64K limit:

| Module | Largest working set | 16-bit clean? |
|---|---|---|
| `comm/` | port buffers, a few KB | yes |
| `xfer/` | Zmodem 8K blocks | yes |
| `ftn/` | packets are streamed | yes |
| `mail/` | QWK 128-byte blocks | yes |
| `net/` | line-based protocols | yes |
| `crypto/` | block state, tiny | yes |
| `pcb/` | message indexes can be large | use the `VIRTUAL.C` pattern |
| `term/` | framebuffer 224K as written | yes, via row pointers |
| `draw/` | canvas, same | yes, via row pointers |

Seven of nine are naturally 16-bit clean — discipline, not rework, since they
already work in small buffers.

**`term/` and `draw/` come from Pascal and are being ported to C.**

kiddo's RIP engine and canvas (`mtrip.pas` 946 lines, `mtripgfx.pas` 859
lines) and PCBDraw's editor are Free Pascal today. They port to C and land in
the toolkit. `mterm` becomes `pcbterm` inside pcbnav. See `PORT-PCBDRAW.md`.

**And they can be 16-bit clean.** The Pascal canvas is byte-per-pixel —
`array[0..349, 0..639] of Byte` is 224,000 bytes, 3.4x over the limit, plus
another 224,000 for the flood-fill visited buffer. Two changes in the port fix
that, and both are improvements regardless of memory model:

- **Row-pointer array** — `unsigned char *rows[350]`, each row 640 bytes. No
  allocation over 64K, all pixel arithmetic stays inside a row.
- **Bitmask for the visited buffer** — 640 x 350 bits = 28,000 bytes, one
  segment. Down from 224,000, and faster: better cache behaviour, word-at-a-
  time clearing.

So all nine modules are 16-bit clean. The earlier "flat-only first" carve-out
is withdrawn.
They are the two where 16-bit is genuinely expensive: a framebuffer over 64K
needs `huge *` threaded through the whole module, not just at the edges. They
are also the two with the most existing flat-model work already done (kiddo's
RIP and terminal, PCBDraw's canvas). Forcing a 16-bit port up front risks
discarding real effort for a hypothetical user.

So: all nine modules 16-bit clean. Seven are so already; `term/` and `draw/`
get there through the row-pointer and bitmask changes during the C port. A 16-bit door author gets comm, xfer, ftn, mail, net,
crypto and pcb — most of the value — and gets RIP and drawing if and when
somebody actually needs them in 16-bit. Clark's `SAVEREST.C` and `WINDOW.C`
show how it would be done when that day comes.

The discipline, then: **16-bit clean by default.** Explicit sized types, no
assuming an allocation can exceed 64K, no pointer arithmetic that crosses a
segment. Where a module genuinely needs more, use `huge *` as Clark did, or
`VIRTUAL.C` for disk-backed data. That is house style already — 300+ files of
worked examples to follow.

---

# What goes in 15.41

The crew's programs have grown duplicate copies of the same code. FOSSIL
handling exists in `MODEMFOS.C` and again in QFront's `serial.c`. Zmodem
exists in QFront, in `Pcb-misc/ZMODEM/`, and in ZMRECV/ZMSEND. RIP is about
to exist in both PCBDraw and kiddo's terminal work. Every copy drifts; every
fix gets missed somewhere.

Consolidating them serves two purposes at once: our programs stop duplicating,
and door authors get the result.

Written 16-bit clean so it builds for both 16-bit models and flat.

```
toolkit-15.41/
  comm/     serial: UART, FOSSIL, multiport, TCP   (= pcbcomm)
  xfer/     Zmodem, Xmodem, Ymodem, HS/Link, SEAlink
  ftn/      FidoNet: BSO, EMSI, YooHoo, nodelist, .PKT, TIC
  mail/     QWK, QWKE, Blue Wave                    (from OLMS)
  net/      SMTP, POP3, NNTP                        (ported from VSOUP)
  term/     ANSI, RIP, terminal emulation           (kiddo)
  draw/     canvas, drawing tools, block ops        (PCBDraw)
  crypto/   MD5, DES, IDEA, MDC
  pcb/      PCBOARD.DAT/SYS, USERS, CNAMES, message base
  clark/    Clark's original 198 functions, flat-model build
```

`term/` and `draw/` are separate on purpose: `term/` receives and renders a
remote session, `draw/` creates graphics. PCBDraw uses `term/` to display; a
terminal needs no drawing tools.

---

# The backend ABI

Recovered from the object files, not the header. This is what any serial
backend must export.

**Symbol form:** `ASYNC_` prefix plus the uppercase function name — C linkage
with the Pascal convention's uppercase folding, applied to functions in the
ASYNC module. No C++ mangling anywhere.

**The 24 symbols both `COMMDRV.OBJ` and `FOSSIL.OBJ` export:**

| Group | Symbols |
|---|---|
| Lifecycle | `ASYNC_INIT` `ASYNC_OPENCOM` `ASYNC_CLOSECOM` `ASYNC_SETPORT` |
| I/O | `ASYNC_CSENDBYTE` `ASYNC_CSENDSTR` `ASYNC_CGETBUF` `ASYNC_CGETSTR` `ASYNC_COMMINKEY` |
| Carrier / status | `ASYNC_CDSTILLUP` `ASYNC_ONLINE` `ASYNC_CHECKCOMM` |
| Modem control | `ASYNC_TURNONDTR` `ASYNC_TURNOFFDTR` `ASYNC_TURNONRTS` `ASYNC_TURNOFFRTS` |
| Hardware | `ASYNC_TURNONFIFO` `ASYNC_TURNONXMIT` |
| Buffers | `ASYNC_CLEARINBUF` `ASYNC_CLEAROUTBUF` |
| Flow control | `ASYNC_COMMGO` `ASYNC_COMMPAUSE` `ASYNC_COMMSTOP` |
| Session | `ASYNC_DISCONNECTMODEM` |

**Where they differ:**

- COMM-DRV also exports `OVERRUNERRORS`, `PARITYERRORS`, `RINGDETECT`,
  `REOPENPORT`, `OPENMODEM` — error statistics, because it owns the hardware
  and can count them.
- FOSSIL also exports `BAUDDIVISOR`, because the FOSSIL specification exposes
  it directly.

**pcbcomm implements the union, not the intersection.** It owns the hardware,
so it can supply the error counters, and it can expose the baud divisor too.
Anything that linked either old backend then links pcbcomm unchanged.

---

# Vocabulary

Three terms that caused confusion.

## "Pascal calling convention" does not mean written in Pascal

It is C code. `pascal` is a keyword in Borland and Watcom C:

```c
void pascal initport(void);      /* still C */
```

After a call, somebody must remove the arguments from the stack. Two rules:

- **C rule** — the caller cleans up. Cleanup code at every call site.
- **Pascal rule** — the function cleans up itself, once, inside itself.

Clark chose the Pascal rule to save space: with hundreds of call sites those
few bytes each add up. Side effect: names fold to uppercase, so `foo()` and
`FOO()` become the same symbol.

15.4 keeps the Pascal rule. 15.41 uses the C rule with a Pascal-compatible
variant available — a compiler flag per build, not a rewrite.

## Memory models

16-bit x86 could only address 64 KB at a time. Four compromises:

| Model | Code | Data |
|---|---|---|
| Small | under 64K | under 64K |
| Medium | can exceed 64K | under 64K |
| Compact | under 64K | can exceed 64K |
| Large | can exceed 64K | can exceed 64K |

A pointer is 16 bits in Small, 32 in Large — so the same source is built four
times. `TOOLKIT/OTHER/MODELS.BAT` shows Clark's build doing exactly this, one
source to four outputs via `__s__` / `__c__` / `__m__` / `__l__` defines.

Flat 32-bit has none of this — one model, everything addressable. But crew
code is written 16-bit clean anyway, so it builds for both. See the memory
model section above for why: flat model was inherited from Mystic, not chosen,
and 16-bit-clean code compiles in flat model while the reverse does not.

## Classes

Clark's split:

| | C files | C++ files |
|---|---:|---:|
| Internal code | 309 | 88 |
| Toolkit | 39 | 0 |

C++ freely where he controlled both sides of the compile; pure C at the
boundary where he did not. 15.41 keeps the same discipline — C++ inside, C
API outside.

---

# The refactor

Only 15.41 involves moving existing code. 15.4 is a build exercise over source
already in hand.

## What moves

QFront splits roughly in half.

| Moves to toolkit-15.41 | Stays in QFront |
|---|---|
| `serial.c`, `modem.c` → `comm/` | `qfront.c` — the front-end loop |
| `zmodem.c`, `xmodem.c` → `xfer/` | `qfconfig.c`, `qscan.c`, `qnlist.c`, `qfutil.c` |
| `emsi.c`, `wazoo.c` → `ftn/` | `session.c` — session policy |
| `nodelist.c`, `bso.c`, `tic.c`, `route.c` → `ftn/` | `events.c`, `semaphore.c`, `frequest.c` |

PCBoard keeps `MODEM.C` as its own policy layer; the backends `MODEMASY.C`,
`MODEMFOS.C`, `MODEMDRV.C` become `comm/` backends.

ZMRECV and ZMSEND become thin wrappers over `xfer/`.

## Risk order

Doors do **not** link against PCBoard. `DOORS.C` writes `DOOR.SYS`, then
`spawndos()` launches the door as a separate process. No shared memory, no
linkage — the contract is file formats only: DOOR.SYS, PCBOARD.SYS,
USERS.SYS, USERS.INF.

So PCBoard's internals can change freely. Only what it *writes* must stay
byte-exact.

| Program | Risk | Why |
|---|---|---|
| PCBoard | low | process boundary, file contract, easy to verify |
| QFront | higher | split in half; the policy/primitive seam must be right |

Do PCBoard first. Biggest program, safest move, right place to prove the
method.

## Order of work

Move one subsystem. Verify the program builds and behaves identically. Next
subsystem. Not all at once.

## Sequencing note

Phase 15 (QFront Clark-style conversion) is 17 of 21 files done and on hold.
Ten of those files are headed for the 15.41 toolkit. Style carries over
unchanged — same code — but do the move before finishing the last four, or
they get styled in a tree they are about to leave.

---

# VSOUP — port, do not bundle

VSOUP was added to be ported, not carried as third-party. It is GPL, so
folding it into `net/` is permitted, and it is untested — which is the
argument for porting rather than bundling. Carried unmodified it is code
nobody owns: it ships, it breaks, and the answer to "whose is this" is
"nobody's". Ported, it gets a maintainer, house style and tests.

Work: bring SMTP, POP3 and NNTP into project style, build under OpenWatcom,
test against a real server before anything depends on it. Keep GPL
attribution in the file headers — porting does not erase provenance.

`third-party/` is reserved for code we genuinely do not touch: unmodified,
own licence file, own subdirectory. If we are editing it, it belongs in the
toolkit with its origin recorded.
