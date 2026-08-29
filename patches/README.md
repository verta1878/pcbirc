# Patches

## 15.4-pwa.patch (canonical)

Transforms clean 15.3 PWA into 15.4 PWA (Clark's) — **source AND
toolchain together**.

The patch carries both:
- The 15.4 source changes (PSA fields, PPL 3.40 functions, file
  flagging, UUIN reject-by-name, FTP MGET, etc.)
- The 15.4 toolchain change (the single SPACERIGHTAT enum value in the
  toolkit's PCBTOOLS.H, needed for the @x color-code feature)

This means: apply the patch to a clean 15.3 tree (source + toolkit) and
you get a complete, buildable 15.4 PWA. The toolchain rides along
because you can't build 15.4 without it.

- 424 files (source + toolkit)
- Applies 100% cleanly
- Both endpoints build with Borland C++ 3.1

## 386max-tasm.patch (planned, populated on Gate 1)

An in-source equivalent of pcbirc's runtime transforms for building
sudleyplace/386MAX 8.03 under TASM 3.1 + TLINK 5.1. See
`todo/386max-build-downgrade.md` for context.

**Default pcbirc path**: source stays byte-identical on disk;
`MAIN/build/scripts/xform.awk` and `xform.sed` transform at build
time. Anyone who clones sudleyplace/386MAX still gets Bob's
original source verbatim.

**Alternative for sysops who prefer in-source patching**: this
patch, when applied to sudleyplace source, produces the same
result — TASM+TLINK-buildable source — but as a static in-tree
change. Useful when building outside DOSBox-X, when reviewing
transforms as a code artifact, or when gawk/sed aren't available.

Covers:
- `.xcref` directive commented out (TASM misparses its argument list)
- `@Version` renamed to `?VERSION` (TASM's native reserved-symbol prefix)
- `@@:`/`@F`/`@B` anonymous labels expanded to named `LBL_N:` labels
- Four `at`-address group definitions removed (RGROUP, AGROUP,
  PSPGRP, CGROUP) and references renamed to their sole underlying
  segment

Apply with:

    cd 386max-src/
    patch -p1 < ../pcbirc/patches/386max-tasm.patch

Populated once Gate 1 is reached (a byte-verified `386MAX.SYS`
from the pcbirc pipeline).

### What Clark's 15.4 changed

7 feature areas. Only ONE (@x color codes) touched the toolkit — it
added SPACERIGHTAT to the padtype enum. The other 6 are pure source
changes. So the toolkit barely moved; the patch is mostly source plus
that one enum line.

### Apply

The patch expects a combined tree layout (SOURCE/ + toolkit/):

```
# from a clean 15.3 tree containing SOURCE/ and toolkit/
patch -p1 < 15.4-pwa.patch
```

Labeled "15.4 PWA" — Clark's authentic 15.4, NOT the crew's 15.4 Delta
work (pcb154/, ongoing).

## attic/

Superseded patches from earlier phases. See attic/README.md.
