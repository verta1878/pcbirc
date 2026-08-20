# SDK source recovery

Goal: rebuild `PCBKIT_*.LIB`, `COMMDRV.OBJ`, `FOSSIL.OBJ` and the `NOxxx.OBJ`
stubs from source, so the SDK can be maintained rather than shipped as 1994
binaries.

**Most of it already exists.** A source check before starting found the
toolkit source under `Pcb-libs/SOURCE/` and the serial backends under
`Pcb-main/SOURCE/MODEM/`.

---

## What we have

### Toolkit stubs and glue — `Pcb-libs/SOURCE/TOOLKIT/` (41 files)

Every `NOxxx.OBJ` has its `.C`:

```
NOANSI.C  NOCHAT.C  NODISP.C   NOHELP.C   NOINPUT.C  NOLANG.C
NOLOG.C   NOMEMORY.C NOPCBSYS.C NOPRINT.C NOSCREEN.C NOSHELL.C
NOSTATUS.C NOSYS.C  NOTXT.C    NOUPDSYS.C NOXLATE.C
SMALLDLY.C SMALLERR.C SMALLSUB.C SMALLTXT.C PCBDAT.C
```

Plus the door-side glue: `INIT.C`, `PCBINIT.C`, `DOSINIT.C`, `INITPORT.C`,
`ALTMODEM.C`, `SLOWMODM.C`, `GOODBYE.C`, `RECYCLE.C`, `USERSYS.C`,
`CNAMES.C`, `COPY2MSG.C`, `HELP.C`, `CUSTHELP.C`, `INPUTREQ.C`, `REPTXT.C`,
`PRINTER.C`, `ATCLOSE.C`.

Note `NOPCBSYS.C` and `NOPRINT.C` have source but no matching `.OBJ` in
Toolkit3 — either dropped before release or built into a variant we do not
have.

### Library body — rest of `Pcb-libs/SOURCE/` (~240 files)

| Directory | Files | Contents |
|---|---:|---|
| `MISC/` | 90 | strings, dates, math, `CRYPT.C`, `WILDCARD.C`, `EVALUATE.C`, `SOUNDEX.C`, `VIRTUAL.C`, `XMODEM.ASM`, `SWAP.ASM` |
| `DOS/` | 47 | the `dosXXX()` file layer, error handlers, share/lock |
| `SCREEN/` | 41 | ANSI, windows, boxes, cursor, save/restore, `GIVEUP.C` |
| `PCB/` | 22 | PCBOARD.DAT read/write, CNAMES, conference funcs, path parsing |
| `SCRNIO/` | 20 | low-level screen I/O |
| `COUNTRY/` | 13 | locale-aware compare, date, comma formatting |
| `SYSTEM/` | 7 | system services |

That is the substance of `PCBKIT_*.LIB`.

### Serial backends — `Pcb-main/SOURCE/MODEM/` (3,502 lines)

| File | Lines | What |
|---|---:|---|
| `MODEM.C` | 551 | common modem layer above the backends |
| `MODEMASY.C` | 256 | direct UART (async) |
| `MODEMFOS.C` | 829 | FOSSIL — this is `FOSSIL.OBJ`'s source |
| `MODEMDRV.C` | 521 | COMM-DRV client — this is `COMMDRV.OBJ`'s source |
| `MODEMOS2.C` | 1081 | OS/2 device driver |
| `DEVIOCTL.C` | 231 | device ioctl |
| `TICDELAY.C` | 33 | timing |

Plus `Pcb-main/SOURCE/ASM/ASYNC.ASM` for the low-level UART work.

**`MODEMDRV.C` is the COMM-DRV *client*, not COMM-DRV itself** — the code that
talks to the driver. That is exactly what we need and it is not encumbered:
knowing how to call an interface is not the same as owning its implementation.

---

## The COMM-DRV ABI, recovered

`MODEMDRV.C` documents the interface pcbcomm must present to be drop-in.

Entry is INT 14h with extended function numbers in AX:

```c
asm mov Ax,1000h
asm mov Dx,Port
asm int 14h        /* returns port control block pointer */

asm mov Ax,1002h
asm mov Dx,Port
asm int 14h
```

The driver returns a pointer to a port control block which the client then
reads directly:

| Field | Purpose |
|---|---|
| `pcb.opcb->inbuf_count` | bytes waiting in the receive buffer |
| `pcb.opcb->outbuf_count` | bytes pending in the transmit buffer |
| `pcb.opcb->msr_reg` | modem status register (carrier, CTS, DSR) |
| `pcb.opcb->cardtype` | which multiport card |
| `pcb.opcb->flag` | port flags |
| `pcb.auxpcb->aux_frmint` | framing error count |
| `pcb.auxpcb->aux_ovrint` | overrun error count |
| `pcb.auxpcb->aux_parint` | parity error count |
| `pcb.baud`, `pcb.parity`, `pcb.block`, `pcb.lngth` | line settings |
| `pcb.protocol`, `pcb.cardtype`, `pcb.error`, `pcb.outbuf_len` | state |

**A Clark workaround worth keeping.** From the comment block in `MODEMDRV.C`:
the DigiBoard COM/Xi does not interrupt COMM-DRV often enough to keep
`inbuf_count` and `outbuf_count` current, so rather than trusting a zero
count, the code checks for a DIGCXI card and makes an extra call to fetch
incoming bytes, which refreshes the counter as a side effect. Guarded by an
`OLDARNET` conditional. pcbcomm's intelligent-board backend will hit the same
problem — the boards buffer more than 1 KB per port and report lazily, which
is also why FreeBSD's `digi` driver polls instead of using interrupts.

---

## What is genuinely missing

Nothing structural. The gaps are:

1. **Build files.** No `.MAK` producing `PCBKIT_*.LIB`. The source is
   scattered across seven directories and must be reassembled into a library
   build for four memory models across three compilers.
2. **`MODELS.BAT`** — checked, and it is the memory-model driver:

   ```bat
   tasm /w2 /mx /t /D__s__ %1,small\%1
   tasm /w2 /mx /t /D__c__ %1,compact\%1
   tasm /w2 /mx /t /D__m__ %1,medium\%1
   tasm /w2 /mx /t /D__l__ %1,large\%1
   ```

   One source, four models, selected by `__s__` / `__c__` / `__m__` / `__l__`
   defines, output into per-model subdirectories. That is the `.ASM` half; the
   C half follows the same shape with the compiler's model flag. So the
   twelve-variant matrix is one loop, not twelve ports.
3. **COMM-DRV itself.** Not needed. `MODEMDRV.C` is the client; pcbcomm
   replaces the driver behind it.

---

## Plan

1. Reassemble a library build from `MISC/ DOS/ SCREEN/ SCRNIO/ PCB/ COUNTRY/
   SYSTEM/ TOOLKIT/`. Verify the exported symbol set matches `PCBTOOLS.H`'s
   198 declarations.
2. Build the stubs individually — one `.OBJ` per `NOxxx.C`.
3. Build `FOSSIL.OBJ` from `MODEMFOS.C`, `COMMDRV.OBJ` from `MODEMDRV.C`.
4. Write `PCBCOMM.C` against the same seam, producing `PCBCOMM.OBJ`.
5. Verify: relink a sample door from `TOOLKIT/SAMPLES/` against the rebuilt
   library and confirm it runs.

Memory-model matrix stays a build-configuration problem, not a porting one —
`MODELS.BAT` shows the same sources compile to all variants from one loop.

`FEATURES` gives the design intent to preserve: 52 high-level functions, and
"a single function call does it all" for door init — read config, set up the
comm port, update USERS.INF TPA. Whatever we rebuild has to keep that
one-call simplicity, because that is what door authors actually used.
