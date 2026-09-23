# attic/retired-build-files — build side-files nothing uses

Retired 2026-09-22. Nineteen hand-made compiler/linker side-files that
no makefile, batch file or script in the repo reads. They are kept here,
byte-for-byte as they were in the tree before the ROOT=\OUT pass (Clark's
`\PROJ` paths intact), so they can be brought back later.

Each file sits under the same path it had in the tree, e.g.
`pcb153/153/PCB_BC50.RES` here came from `pcb153/153/PCB_BC50.RES`.

## What they are

A **.CFG** file is a compiler settings file (include/library folders,
switches). A **.RES / .RSP** file is a "response file": a list of object
files or options handed to TLINK/TLIB/BCC with `@file`, because a DOS
command line is capped at 127 characters. The 41 CFG files that stayed in
the tree are written by their makefile at build time; these 19 are not
written or read by anything.

| Files | Copies | What it is | Why retired |
|---|---|---|---|
| `PCB_BC50.RES` | pcb153/153, pcb154/MAIN/153 | TLINK link list for PCBOARD.EXE built with **Borland C++ 5.0** (`obj\bc50`, `lib\bcdos\bc50`) | 15.3 builds with BC 3.1; no makefile references it |
| `PCB_TC30.RES` | pcb153/153, pcb154/MAIN/153 | Same link list for **Turbo C++ 3.0** (`obj\tc30`) | Compiler not used by this project |
| `INITBUILD.RSP` | pcb153, pcb153/upd154, pcb154 (`SOURCE/UTIL/PCBSM`) | One-off BCC options to compile PCBSM's `SOURCE\INIT.C` alone | PCBSM.MAK builds INIT.C itself |
| `UUIN154.CFG`, `UUOUT154.CFG`, `UTIL154.CFG`, `XFER154.CFG` | pcb153, pcb153/upd154, pcb154 (`SOURCE/UUCP/*`) | BCC settings for a 15.4 build of the UUCP tools (`-DUNIX -DLIB`, `\B\C31` compiler path) | The UU*.MAK files write and use `UUIN.CFG` etc.; the `…154` copies are read by nothing |

## Bringing them back

The BC 5.0 / TC 3.0 link lists are the useful ones if the project ever
adds a newer Borland compiler leg (Borland C++ 5.0x). To unretire:

1. Copy the file back to its original path.
2. Replace `\proj\pcb` with the branch source folder and `\proj\lib` with
   the toolkit folder (see APPLY.txt, "PATHS MIGHT NEED REPOINTED").
3. Better: move the list into the makefile as an inline response file
   (`copy &&| ... | file`) so it is regenerated on every build and
   removed by `make CLEAN`, like the other 41.

## Also here: pcb153/T.BAT and T.CMD  (added 2026-09-22)

Two one-line developer helpers that delete a single object file so the next
`make` rebuilds that module — `T.BAT` for DOS (`del obj\%bccompiler%\%1.obj`,
after `call \proj\bcdos.bat`) and `T.CMD` for OS/2 (`del obj\bcos2\%1.obj`).

Deleted by commit 37bf4e4 with the VIRTUAL1 files and never brought back;
recovered from `37bf4e4^`. They are kept here rather than restored because
both still point at Clark's `\proj` layout, which this tree no longer uses,
and `make CLEAN` now does the same job properly.
