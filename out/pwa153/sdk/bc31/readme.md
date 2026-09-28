# OUT\pwa153\SDK\BC31 — SDK example binaries

Clark's own sample doors from
`toolkit/pwa153/SOURCE/TOOLKIT/SAMPLES/`, compiled and linked against
`toolkit/pwa153/bc31/lib/PCBKBC{M,L}.LIB`. These exist to prove the SDK
builds end to end — that is the whole point of this directory.

Built 2026-09-17. SHA256 in `CHECKSUMS.sha256`.

    medium/   6 binaries, PCBKBCM.LIB   <- PCBoard itself uses medium
    large/    6 binaries, PCBKBCL.LIB

| Sample | What it demonstrates |
|---|---|
| HELLO | minimal door: `initdoor` / `println` / `closedoor` |
| CALLBACK | callback-driven door |
| COPYTEXT | text file into the message base |
| COPYBIN1 | binary file copy, simple |
| COPYBIN2 | binary file copy, buffered |
| UPGRADE | user upgrade / verification door |

These are **SDK examples, not shipping binaries.** Program EXEs sit at
the version top level (`OUT/pwa153/PCBOARDM.EXE`), never here.

## No small or compact

Every sample fails those two with `Segment _TEXT exceeds 64K`. Small and
compact have a single 64K code segment and the closure of `initdoor` is
larger than that by itself. `PCBKBCS.LIB` and `PCBKBCC.LIB` are correct
and complete — they are for doors that use a small slice of the toolkit
(string, date, DOS helpers), which is why Clark shipped four models.
Nothing to fix here.

## One sample not shipped

`SAMPLES/INPUTREQ.C` fails at link with `Undefined symbol _MAIN in
module C0.ASM`. It is a *replacement-module* example — it reimplements
the toolkit's own `inputfieldreqstr` — so it is meant to be compiled
into a door rather than linked standalone. The `.EXE` TLINK emits is
invalid, so it is deliberately absent. Unresolved.

## Two source fixes were needed

Both build-enabling, neither changes behaviour:

- `COPYBIN2.C:78` — `malloc` result assigned to `char *` without a cast.
  Legal C, rejected by C++; the samples compile as C++ under `-P` to
  match the library's name decoration.
- `UPGRADE.C:17` — `#include "\tc\tools\pcbtools.h"`, a hardcoded path
  from Clark's own machine, changed to `<pcbtools.h>`.

Full build account: `toolkit/pwa153/bc31/README.md`.
