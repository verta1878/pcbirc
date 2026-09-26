# comm.h — two reconstructions, and which one the tree uses

There are now two independent reconstructions of WCSC's `comm.h`, both
written 2026-09-22, neither of them WCSC's file:

| | Author | Size | Method |
|---|---|---|---|
| A | sysop/0, shipped in `pcbcomm-fossil5c.zip` | 6,594 B | written alongside the pcbcomm backends |
| B | this session | 15,592 B | inferred from `pcb153\SOURCE\MODEM\MODEMDRV.C` alone |

**A is the one the tree uses.** B is retired to
`reference/commdrv/comm-h-recon-b.h` and is not on any include path.

## They already agree on the part that is actually recovered

Both declare the same **13 `ser_rs232_*` prototypes**, in the same order,
with `LIBENTRY`, `int` port, `int` count, `unsigned char *` buffer. That
set is closed: it is every COMM-DRV function `MODEMDRV.C` calls, and
`MODEMDRV.C` is the whole of PCBoard's use of COMM-DRV. Both also agree
on `LENGTH_5..8`, `PARITY_NONE/ODD/EVEN`, `RS232ERR_NONE/BUSY/PARAM/NOPORT`
and the `opcb`/`auxpcb`/`port_param` field *semantics*.

## Where they disagree, and why A wins

`MODEMDRV.C` names exactly two constants — `PROT_RTSRTS` (line 442) and
`CARD_DIGCXI` (lines 210, 304, 330, 366) — and **never compares either to
a number**. So the only consumer we have proves the names must exist and
proves nothing at all about their values. Both files satisfy it. The
tiebreaker has to come from somewhere else.

| Symbol | A | B |
|---|---|---|
| `PROT_XONXOFF` | 1 | 2 |
| `PROT_RTSRTS` | 2 | 1 |
| `CARD_*` | 1-based, 16 cards + `CARD_NONE` | 0-based, 8 cards |
| `BAUD*` | absent | divisor-latch values (`BAUD9600` = 12) |
| `XMTOFF_STATE` | `0x01` | `0x0001` |

A's card list maps **one-to-one onto backend source that exists**:

    CARD_8250      uart_backend.c            CARD_ROCKET    rocket_backend.c
    CARD_BOCA      boca_backend.c            CARD_EASYIO    easyio_backend.c
    CARD_CYCLOM    cyclom_backend.c          CARD_ARNET     arnet_backend.c
    CARD_DIGPCXE   digi_pcxe_backend.c       CARD_HUB6      hub6_backend.c
    CARD_DIGPCXI   digi_fep.c                CARD_GTEK      gtek_backend.c
    CARD_DIGACCEL  digi_accel_backend.c      CARD_STALLION  stallion_brumby_backend.c
    CARD_DIGCXI    digi_comxi_backend.c      CARD_CHASE     chase_iolan_backend.c
                                             CARD_EQUINOX   equinox_sst_backend.c

Fourteen backends in `pcb1541/pcbcomm/src/`, fourteen card constants,
plus `CARD_NONE` for a disabled port and `CARD_BOCA16` for the 16-port
Boca. B's eight were a guess made before those backends were read.

That is the whole argument: A corresponds to code, B corresponds to an
inference. B's baud table is the one thing worth carrying over, and it
is carried in the retired copy for whoever writes `bauddivisor()`.

## What neither of them recovers

**WCSC's byte layout.** Field widths and offsets in both files are a
choice, not a finding. A program built against either header talks to a
driver built against the same header and will **not** interoperate with a
genuine WCSC COMM-DRV TSR. If interop with the real product is ever
wanted, the layout has to come out of the v15.0b runtime binaries in
`pcb1541/install/dist/target/COMMDRV/` — nine `.DRV` files, `COMMDRV.EXE`
and `COMMTSR.EXE` — and not out of either reconstruction.

## The old shim's third layout — already dealt with

`ser_rs232_shim.c` declared its **own** `struct port_param` at lines
49-57 — different field names, different order, no `opcb`/`auxpcb`
pointers at all, matching neither A nor B. That would have been a silent
corruption, not a link error: PCBoard writes `pcb.protocol` where the
shim reads `data_bits`.

It is retired, in
`attic/superseded-pcbcomm/pcb1541/pcbcomm/src/ser_rs232_shim.c`, and
`commdrbl.c` replaces it. `commdrbl.c` does the right thing already —
`#include "comm.h"` at line 24, no local struct, and `LIBENTRY` on all
thirteen definitions. Nothing to fix; recorded because the shim is still
in the tree and someone will read it.

The two traps that made this worth checking, both hit once in this
project:

1. `LIBENTRY` must be on every **definition**, not just the declaration.
   Without it BCC emits `_ser_rs232_getpacket` where `MODEMDRV.C` wants
   `@SER_RS232_GETPACKET$QIINUC`.
2. `#include "comm.h"` must sit **outside** any `extern "C"` block.
   Inside one, C linkage wins and the mangling disappears again.

`libsbl.c` adds six helpers beyond the thirteen — `ser_rs232_strerror`,
`_detect`, `_baud_to_divisor`, `_divisor_to_baud`, `_cardname`,
`_defaults`. None is called by `MODEMDRV.C`, so whether Clark's
`LIBSBL.LIB` held these or something else is still unknown.

## Settled, for the record

sysop/0's `INT14-FOSSIL5C-STATUS.md` still carries this as open item 4:

> Reconcile sysop/0's comm.h against hexadecimal's
> pcbcbase/COMMDRV/H/COMM.H (15,592 bytes) — pick one.

It is picked. His is live at `pcbcbase/COMMDRV/H/COMM.H`; the 15,592-byte
one is retired to `reference/commdrv/comm-h-recon-b.h`. The doc is landed
as he sent it rather than edited, so the item reads open there.

The same doc gives the pcbcomm paths as `pcb154/pcbcomm/`. In this tree
they are `pcb1541/pcbcomm/`.
