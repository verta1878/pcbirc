# packfido.c — missing

`packfido.c` is **not in this repo, not in the archive, and not in any
Clark material we have.** This note records where it goes, what
references it, and what has already been ruled out, so nobody re-runs the
search from scratch.

## Where it goes

    pcb153/SOURCE/packfido.c        ->  $(ROOT)\source\packfido.c

`FIDOUTIL.MAK` already points there. A recovered file drops straight in
with no further edits.

## What references it

Only this makefile, in three places:

```
EXE_DEPENDENCIES = ... packfido.obj ...          (line 97)
$(OBJDIR)\packfido.obj+                          (line 124, link list)
packfido.obj: $(ROOT)\source\packfido.c          (line 161)
```

Clark's original was `$(ROOT)\packfido\packfido.c` — a **top-level
directory under `\PROJ`**, outside the source tree, exactly like
`$(ROOT)\md5\os2\md5.obj`. Across `FIDOUTIL.MAK` and `PCBOARD2.MAK`
those two were the *only* dependencies living outside `$(ROOT)\source`;
everything else resolves inside it. That is what made them the two
missing directories, and it says something about what they were:
external drops kept beside the project rather than modules Clark wrote.
`md5` proved to be exactly that — a public-domain package from a BBS
file area (see `../MD5/README.md`). `packfido` is likely the same kind of
thing: a small module for packing outbound FidoNet mail.

## What we know about it

Almost nothing, and that is itself the finding. The string `packfido`
appears in **exactly one place** across the entire source archive
(`reference/pcb153src0014.zip`, inner `PCBoard 15.3 source code
v0.014.zip`) and the whole repo: the makefile above. No header, no
prototype, no call site naming it, no `.OBJ`, and — unlike `md5` — not
even an empty directory stub in `PCBSRCV/000/`.

Checked and ruled out:

| Candidate | Why not |
|---|---|
| `pcb153/SOURCE/FIDO/PACKMSG.CPP` | Similar name, wrong thing. It is an include-fragment — a bare statement body with no function wrapper, meant to be `#include`d, not compiled to an object. |
| `PCBSRCV/000/MISC/` in the archive | Holds only `BCDOS.BAT`, `BCOS2.CMD`, `WATOS2.CMD`. No `PACKFIDO` at any level. |
| `devtools/Md5.zip` | The md5 drop. Nothing Fido-related in it. |

## Where it actually lived — found in two Borland IDE desktop files

Two `.DSK` files in the archive — Turbo C / Borland IDE **desktop
files**, which store the editor's recent-file history — recorded the
path. `PCBSRCV/000/MISC/IDX/MAKEIDX.DSK` and
`PCBSRCV/000/UTIL/PCBMONI/PCBMONI.DSK` both contain:

    E:\TC\PACKFIDO\PACKFIDO.C

and its neighbours in the same list:

    E:\TC\LIST\MAKEIDX.C      E:\TC\SCANLOG\SCANLOG.C
    E:\TC\PCBMONI\PCBMONI.C   E:\TC\TESTDOOR\TEST1.C
    E:\TC\TEST\TXT2DIR.C      E:\TC\TEST\NEWFILES.C

`E:\TC\` is the developer's **Turbo C scratch area**, not the product
tree — the product tree is on the other drive in the same list
(`D:\PROJ\IDX\MAKEIDX.C`, `D:\PROJ\LIB\BCDOS\*.LIB`,
`D:\PROJ\VMDATA\*.*`). So `packfido` was a **one-file standalone
utility kept in a personal working directory**, copied into
`\PROJ\packfido\` when FIDOUTIL needed it.

That explains the absence completely. The source archive was made from
the product tree. Anything that lived only under `E:\TC\` was never in
scope to be archived, which is why `packfido.c`, `scanlog.c`,
`txt2dir.c` and the rest of that directory are all missing while their
`\PROJ\` counterparts survive.

It also means `PACKFIDO.EXE` — listed among the 37 root EXEs the 15.3
installer shipped — has **no makefile anywhere**, the same as twelve
other shipped programs. See `OUT/README.md`.

Incidental find in the same `.DSK` list: `D:\PROJ\VMDATA\*.*` and
`D:\VMDATA\*.*`, which confirms where `\LIBS\VMDATA` sat on the
development machine.

## It is not needed — FIDOUTIL builds without it

`packfido.obj` supplies exactly **one** symbol, `do_pack()`, and FIDOUTIL
never calls it. Every occurrence of `do_pack` in the entire archive:

```
MISC/FIDOUTIL/SOURCE/CONVERT.CPP:53    void do_pack(void);
MISC/FIDOUTIL/SOURCE/CONVERT.CPP:124     //do_pack();
```

A declaration, and a call site that is **commented out**. Checked against
all eleven of FIDOUTIL's inputs — the seven modules in
`MISC/FIDOUTIL/SOURCE/`, `FIDO/DATA.CPP`, `FIDO/PASSTHRU.CPP` and
`PCBSETUP/SOURCE/CI_BUILD.C` — and `CONVERT.CPP` is the only file that
mentions it at all.

So the object contributed nothing to the linked image. On 2026-09-17
`packfido.obj` was **dropped** from `FIDOUTIL.MAK`'s `EXE_DEPENDENCIES`
and from its linker response list, and the build rule was commented out
rather than deleted. FIDOUTIL.EXE is buildable today, with all ten of its
real modules present.

Restoring it, if `packfido.c` ever surfaces, means uncommenting three
places in `FIDOUTIL.MAK` — the file says which.

## What is still blocked

`PACKFIDO.EXE` — the standalone program in the 15.3 ship list. That is a
separate binary with no makefile and no source, and dropping the object
from FIDOUTIL does nothing for it. Rebuilding it means writing
`packfido.c` from scratch against the FidoNet packet format; there is no
surviving reference to work from, not even a prototype beyond
`void do_pack(void)`.
