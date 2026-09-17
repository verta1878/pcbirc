# packfido — the source is missing

`packfido.c` is **not in this repo, not in the archive, and not in any
Clark material we have.** This directory exists to give it a defined
home, so that if a copy ever turns up it drops in and `FIDOUTIL.MAK`
builds without further edits.

## What references it

`pcb153/SOURCE/MISC/FIDOUTIL/FIDOUTIL.MAK`, in three places:

```
EXE_DEPENDENCIES = ... packfido.obj ...          (line 97)
$(OBJDIR)\packfido.obj+                          (line 124, link list)
packfido.obj: $(ROOT)\source\misc\packfido\packfido.c
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(ROOT)\source\misc\packfido\packfido.c
```

Clark's original path was `$(ROOT)\packfido\packfido.c` — a **top-level
directory under `\PROJ`**, outside the PCBoard source tree, exactly like
`$(ROOT)\md5\os2\md5.obj`. Those two were the only FIDOUTIL/PCBOARD2
dependencies that lived outside `$(ROOT)\source`, which is itself a
strong hint about what they were: external drops Clark kept beside the
project rather than modules he wrote.

`md5` turned out to be exactly that — a public-domain MD5 package from a
BBS file area (see `../MD5/README.md`). `packfido` is likely the same
kind of thing: a small third-party or in-house utility module for
packing outbound FidoNet mail.

The path here was repointed to match the repo layout on 2026-09-17,
alongside the MD5 recovery.

## What we know about it

Almost nothing, and that itself is the finding. The string `packfido`
appears in **exactly one place** across the entire source archive
(`reference/pcb153src0014.zip`, inner `PCBoard 15.3 source code
v0.014.zip`) and the whole repo: the makefile above. There is no header,
no prototype, no call site naming it, no `.OBJ`, and — unlike `md5` —
not even an empty directory stub in `PCBSRCV/000/`.

Checked and ruled out:

| Candidate | Why not |
|---|---|
| `pcb153/SOURCE/FIDO/PACKMSG.CPP` | Similar name, wrong thing. It is an include-fragment — a bare statement body with no function wrapper, meant to be `#include`d, not compiled to an object. |
| `PCBSRCV/000/MISC/` in the archive | Contains only `BCDOS.BAT`, `BCOS2.CMD`, `WATOS2.CMD`. No `PACKFIDO` directory at any level. |
| `devtools/Md5.zip` | The md5 drop. Nothing Fido-related in it. |

## Consequence

`FIDOUTIL.EXE` cannot link. `packfido.obj` is in both the dependency
list and the linker response list, so the target fails at compile, not
at link — which is the better failure, since it names the missing file.

This is not fixable by repointing a path. The file is gone. If it
surfaces, drop it here as `packfido.c` and the makefile is already
pointing at it.
