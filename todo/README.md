# PCBoard SDK

For people writing doors and utilities that run under PCBoard.

## What is in here

| File | What it is |
|---|---|
| `PCBTOOLS.H` | The header. 198 function declarations plus the PCBOARD.DAT / PCBOARD.SYS structures. |
| `PCBKIT_*.LIB` | The prebuilt library. Screen output, keyboard/modem input, message base access, users file, DOS helpers. |
| `COMMDRV.OBJ`, `FOSSIL.OBJ` | Two serial implementations. Link one. |
| `NOxxx.OBJ` | Stub objects for features you do not use. See "Making your door smaller". |
| `DOCS` | Clark's 392 K toolkit manual. |
| `SAMPLES` | Example doors with source. |

## The serial API is six functions

```c
void pascal initport(void);              /* open and init the comm port  */
int  pascal commportinkey(void);         /* read one byte, non-blocking  */
int  pascal modemcommand(char *Cmd, modemverifytype Verify);
void pascal slowsendtomodem(char *Str);
int  pascal showmodem(char *Buf, int Size, modemverifytype Verify);
```

Plus globals: `LostCarrier`, `ConnectSpeed`, `ModemSpeed`, `ComPortNumber`,
`IrqNum`, `BaseAddress`.

`COMMDRV.OBJ` and `FOSSIL.OBJ` are two implementations of exactly these.
`PCBCOMM.OBJ` will be a third. Nothing above this layer changes.

## Vocabulary

- **`.OBJ`** — one source file compiled to machine code. Not runnable on its own.
- **`.LIB`** — a bag of `.OBJ` files. The linker pulls out only what you use.
- **`.PRJ`** — a Borland project file. A list of what to link together.
- **linker** — glues `.OBJ` files and `.LIB` files into a finished `.EXE`.

## Choosing your serial backend

A door's project file looks like this:

```
TESTDOOR
COMMDRV.OBJ
PCBKIT_S.LIB
```

To use a FOSSIL driver instead, change one line:

```
TESTDOOR
FOSSIL.OBJ
PCBKIT_S.LIB
```

To use pcbcomm, change it again:

```
TESTDOOR
PCBCOMM.OBJ
PCBKIT_S.LIB
```

Recompile. No source changes — the function names and signatures are the
same in all three. This is what "drop-in" means.

## Making your door smaller

A hello-world door linked against the whole library is about 49 K, because
the library is coarse-grained: asking for one function can drag in a whole
subsystem.

The fix is stub objects. `NOCHAT.OBJ` contains the chat functions with empty
bodies. List it before the `.LIB` and the linker takes the empty versions,
leaving the real chat code behind:

```
TESTDOOR
NOCHAT.OBJ
NOSCREEN.OBJ
NOSYS.OBJ
PCBCOMM.OBJ
PCBKIT_S.LIB
```

Available stubs:

| Stub | Removes |
|---|---|
| `NOCHAT.OBJ` | sysop chat |
| `NOHELP.OBJ` | help subsystem |
| `NOINPUT.OBJ` | `inputfield...()` functions |
| `NOLOG.OBJ` | writing to the callers log |
| `NOSCREEN.OBJ` | save/restore screen |
| `NOSHELL.OBJ` | shell to DOS |
| `NOSTATUS.OBJ` | status line |
| `NOSYS.OBJ` | USERS.SYS access |
| `NOTXT.OBJ` | PCBTEXT |
| `NOUPDSYS.OBJ` | writing a new PCBOARD.SYS |
| `NOXLATE.OBJ` | @-code translation |
| `NOANSI.OBJ` | ANSI output |
| `NODISP.OBJ` | display subsystem |
| `NOLANG.OBJ` | multi-language |
| `NOMEMORY.OBJ` | memory manager |
| `PCBDAT.OBJ` | reads fewer PCBOARD.DAT fields |
| `SMALLERR.OBJ` | smaller error handler |
| `SMALLTXT.OBJ` | removes PCBTEXT |

pcbcomm backends work the same way — link only the one you use, so the
multiport code costs nothing on a single-modem board.

## Two constraints if you build your own backend

**Pascal calling convention, not C.** Clark chose it for code size: the
called function cleans up the stack once, instead of the caller cleaning up
at every call site. A side effect is that names are folded to uppercase, so
`foo()` and `FOO()` are the same symbol. Include `PCBTOOLS.H` and the
compiler handles it. Get this wrong and nothing links.

**Memory models.** 16-bit DOS came in four flavours — Small, Medium,
Compact, Large — and the toolkit ships all four for three compilers
(`PCBKIT_*`, `PCBKBC_*`, `PCBKMS_*`), twelve libraries in total. A backend
`.OBJ` needs to exist in each variant it wants to serve. Our OpenWatcom
flat-model build is a thirteenth target, not a replacement for the twelve.

## Libraries beyond the core toolkit

The core toolkit is Clark's, rebuilt from source. These are additions,
following the same conventions — Pascal calling, memory-model matrix, and
link-out stubs so unused code costs nothing.

### OLMS — offline mail

Build and parse QWK, QWKE and Blue Wave packets. A door can offer offline
mail without reimplementing packet formats. Backend is `pcbolms` (OpenOLMS,
45 Pascal files). Same code serves the sysop-side door and pcbnav.

### PCBDraw — ANSI and RIP drawing

Canvas, drawing tools, block operations, ten file formats. Lets a door draw
rather than hand-rolling ANSI escape sequences. Source from sysop/0 and
Mystic.

### net — SMTP, POP3, NNTP

Ported from VSOUP (GPL, attribution kept in headers), not bundled. VSOUP was
added to the project to be ported and is untested; carrying it unmodified
would mean shipping code nobody owns. In `lib/net/` it gets a maintainer,
house style and tests.

### term and draw

`lib/term/` — ANSI, RIP, terminal emulation. kiddo's work.
`lib/draw/` — canvas, drawing tools, block ops. PCBDraw's.

Separate on purpose: `term/` receives and renders a remote session, `draw/`
creates graphics. PCBDraw uses `term/` to display; a terminal needs no
drawing tools.

### third-party

Reserved for code we genuinely do not touch — unmodified, own licence file,
own subdirectory. If we are editing it, it belongs in `lib/` with its origin
recorded in the headers.

## What is not in the SDK

- **TCP.** The toolkit is from 1994. We add it for 15.41.
- **Teleconference.** It lives inside PCBOARD.EXE. Doors do not call it.
- **PCBMODEM.** Not an SDK component — it is a standalone utility with its
  own source in `Pcb-util/PCBMODEM/`, built in Phase 1 alongside the other
  Clark binaries. It reads MODEMS.DAT; doors do not link against it.
