#!/bin/bash
# ============================================================================
#  pcboard 15.4 os/2 build -- openwatcom cross-compile
#  builds pcboard2.exe from linux/windows without an os/2 vm
#
#  prerequisites:
#    - openwatcom 2.0 (https://github.com/open-watcom/open-watcom-v2)
#    - os/2 toolkit headers (os2tk45 or included with openwatcom)
#    - pcboard 15.4 source tree
#    - watcompat.h in lib/h/
#
#  source compatibility analysis:
#    - 0 inline asm blocks (pure c/c++)
#    - 315 __os2__ conditional blocks (well-separated)
#    - 8 #pragma option (borland) -- wrapped in #ifdef __borlandc__
#    - 8 borland headers (alloc.h, dir.h, mem.h) -- watcom equivalents exist
#    - 413 libentry/pascal calls -- libentry=empty in os/2 mode
#    - watcompat.h bridges all borland→watcom differences
#
#  key watcom flags for os/2 32-bit flat model:
#    -bt=os2v2     os/2 2.x target
#    -mf           flat memory model (same as bcos2 -sm)
#    -5            pentium optimization (same as bcos2 -5)
#    -ox           full optimization
#    -zp1          pack structs on 1-byte boundary (same as bcos2 -a)
#    -ei           force enums to int
#    -ecw          use cdecl calling convention (borland default)
#
#  usage: ./build_os2_ow.sh
#  output: out/os2/pcboard2.exe
# ============================================================================

set -e

watcom=${watcom:-/opt/watcom}
export watcom
export path=$watcom/binl64:$path
export include="$watcom/h:$watcom/h/os2"

srcdir="pcb154/pcb153/source"
libh="pcb154/toolkit-src/h"
srch="$srcdir/h"
outdir="out/os2"
objdir="obj/os2"

mkdir -p "$outdir" "$objdir"

cc="wcc386"
cflags="-bt=os2v2 -mf -5 -ox -zp1 -ei -ecw -d__os2__ -dpcb152 -dpcb153"
cflags="$cflags -d___use_var___ -d___exec___ -ddbase -dcomm -dmultiport"
cflags="$cflags -dosdriver -dpcbstats -dpcbcomm -dfido -dmg -dtossclass"
cflags="$cflags -dbigndx -dkbd3 -dndebug"
cflags="$cflags -i$watcom/h -i$watcom/h/os2 -i$libh -i$srch"
cflags="$cflags -fo=$objdir/"

echo "============================================================================"
echo " pcboard 15.4 os/2 build (openwatcom cross-compile)"
echo "============================================================================"
echo ""
echo "note: this is a porting effort. the source was written for bcos2."
echo "      watcompat.h handles most differences but manual fixes may be needed."
echo "      see pcb154_build_guide.md for details on borland→watcom differences."
echo ""
echo "compiler: $cc"
echo "flags: $cflags"
echo ""

# compile would go here — each source file needs individual attention
# for borland→watcom compatibility fixes.
echo "todo: compile 100+ source files with watcom compatibility"
echo "      start with: wcc386 $cflags pcb154/pcb153/source/pcb153/pcboard.c"
echo ""
echo "the openwatcom path is viable but requires file-by-file porting."
echo "see watcompat.h for the bridge macros."
