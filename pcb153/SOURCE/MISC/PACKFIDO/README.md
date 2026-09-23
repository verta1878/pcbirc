# pcb153/SOURCE/MISC/PACKFIDO — stub, awaiting reconstruction

`PACKFIDO.C` here is **a stub written by pcbirc**, not Clark's code.

## What the real PACKFIDO does

Clark's `PACK.DOC`: written for PCBoard 15.21 to pack the FIDO
configuration file `PCBFIDO.CFG`; as records are packed out of the file it
is noted on the screen. `WHATSNEW.FID` says to run it before FIDOUTIL's
conversion, to pack out duplicates first.

## Why the stub exists

`FIDOUTIL.MAK` builds `packfido.obj` and links it. The only symbol it
supplies is `do_pack()`, which `CONVERT.CPP` declares at line 53; its one
call, at line 124, is commented out. So FIDOUTIL needs the module to link
but never calls into it. The stub prints one line and changes nothing —
packing a live `PCBFIDO.CFG` wrongly would lose records, and that is worse
than not packing.

## What is known for the reconstruction

- The shipped binary is in `PCBinstalled.zip` on the share: `PCB\PACKFIDO.EXE`,
  with `PCB\DOC\PACK.DOC` and `WHATSNEW.FID` beside it.
- Borland C++ build; the runtime string says Copyright 1991, so BC++ 2.0/3.0
  era rather than 3.1.
- Strings in the EXE: `PCBOARD.DAT`, `CNAMES.@@@`, `CNAMES.ADD`,
  `FIDOQUE.DAT`, `tmp.cfg`, "scanning conference configuration...",
  "packing the fido configuration file...", "removed %5d %s...".
- The v2 (15.21) `PCBFIDO.CFG` record layout it walks is the `oldver == 2`
  path in `FIDOUTIL/SOURCE/CONVERT.CPP` / `CONVERT.HPP`.
- Test fixtures: the `PCBFIDO.CFG` data files already in this repo.

Replacing the stub means decompiling `PACKFIDO.EXE` and rebuilding it to
match, the same method used for PCBIC. Until then FIDOUTIL builds and the
missing behaviour is documented rather than invented.
