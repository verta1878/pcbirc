# packfido.c — Clark's source is still missing; the program is not

**Superseded 2026-09-23.** This note used to say packfido was gone and
record where a recovered file would go. Clark's `packfido.c` is still
gone and is not coming back — but the program has been reconstructed from
the shipped binary, and there are now three of them. Read
`../PACKFIDO/README.md` first; this file is kept only for the search
history.

## Where Clark's source was

`E:\TC\PACKFIDO\PACKFIDO.C`

Two Borland IDE desktop files record it: `PCBSRCV/000/MISC/IDX/MAKEIDX.DSK`
and `PCBSRCV/000/UTIL/PCBMONI/PCBMONI.DSK`, with neighbours
`E:\TC\SCANLOG\SCANLOG.C` and `E:\TC\PCBMONI\PCBMONI.C`. `E:\TC\` was the
developer's Turbo C scratch drive; the product tree was `D:\PROJ\...` on
the other drive in the same list, and the archive was made from the
product tree.

That also settles two things this note previously got wrong: `\PROJ\packfido\`
was never its home, and it was **not** an external drop like `md5` — it
was Clark's own code, beside two other programs that are unambiguously
his.

## What replaced it

| Path | What |
|---|---|
| `pcb153\SOURCE\MISC\PACKFIDO\` | aimed at a byte-exact rebuild of Clark's shipped `PACKFIDO.EXE` — 15.21 layout, kit-linked, no argv |
| `pcb153\upd154\SOURCE\MISC\PACKFIDO\` | the 15.4 upgrade Clark never shipped — `AREAS.DAT` v3, Borland C++ 3.1 |
| `pcb154\MAIN\SOURCE\MISC\PACKFIDO\` | the same source, OpenWatcom, with a native OS/2 file layer |

## FIDOUTIL still does not need it

`packfido.obj` supplied exactly one symbol, `do_pack()` — declared at
`CONVERT.CPP:54`, and the only call to it, at `CONVERT.CPP:125`, is
commented out in Clark's own source.

The three `packfido` lines in `FIDOUTIL.MAK` had been re-enabled and
repointed at `SOURCE\MISC\PACKFIDO\PACKFIDO.C`. They are commented out
again, because that file **cannot** be linked into FIDOUTIL: it is a
program with `main()`, not a `do_pack()` module, and it is small-model
C++ against the kit while FIDOUTIL is `$(MDL)`. The makefile carries the
reasoning at its foot.

Re-enabling it properly means writing a `do_pack()` module. That is real
work and nobody needs it: FIDOUTIL.EXE builds without it, and always did.
