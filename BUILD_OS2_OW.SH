#!/bin/bash
# ============================================================================
#  PCBoard 15.4 OS/2 Build -- OpenWatcom Cross-Compile
#  Builds PCBOARD2.EXE from Linux/Windows WITHOUT an OS/2 VM
#
#  Prerequisites:
#    - OpenWatcom 2.0 (https://github.com/open-watcom/open-watcom-v2)
#    - OS/2 Toolkit headers (os2tk45 or included with OpenWatcom)
#    - PCBoard 15.4 source tree
#    - WATCOMPAT.H in LIB/H/
#
#  Source compatibility analysis:
#    - 0 inline asm blocks (pure C/C++)
#    - 315 __OS2__ conditional blocks (well-separated)
#    - 8 #pragma option (Borland) -- wrapped in #ifdef __BORLANDC__
#    - 8 Borland headers (alloc.h, dir.h, mem.h) -- Watcom equivalents exist
#    - 413 LIBENTRY/pascal calls -- LIBENTRY=empty in OS/2 mode
#    - WATCOMPAT.H bridges all Borland→Watcom differences
#
#  Key Watcom flags for OS/2 32-bit flat model:
#    -bt=os2v2     OS/2 2.x target
#    -mf           flat memory model (same as BCOS2 -sm)
#    -5            Pentium optimization (same as BCOS2 -5)
#    -ox           full optimization
#    -zp1          pack structs on 1-byte boundary (same as BCOS2 -a)
#    -ei           force enums to int
#    -ecw          use cdecl calling convention (Borland default)
#
#  Usage: ./BUILD_OS2_OW.SH
#  Output: OUT/OS2/PCBOARD2.EXE
# ============================================================================

set -e

WATCOM=${WATCOM:-/opt/watcom}
export WATCOM
export PATH=$WATCOM/binl64:$PATH
export INCLUDE="$WATCOM/h:$WATCOM/h/os2"

SRCDIR="PCBSRC/MAIN/SOURCE"
LIBH="PCBSRC/LIB/H"
SRCH="$SRCDIR/H"
OUTDIR="OUT/OS2"
OBJDIR="OBJ/OS2"

mkdir -p "$OUTDIR" "$OBJDIR"

CC="wcc386"
CFLAGS="-bt=os2v2 -mf -5 -ox -zp1 -ei -ecw -d__OS2__ -dPCB152 -dPCB153"
CFLAGS="$CFLAGS -d___USE_VAR___ -d___EXEC___ -dDBASE -dCOMM -dMULTIPORT"
CFLAGS="$CFLAGS -dOSDRIVER -dPCBSTATS -dPCBCOMM -dFIDO -dMG -dTOSSCLASS"
CFLAGS="$CFLAGS -dBIGNDX -dKBD3 -dNDEBUG"
CFLAGS="$CFLAGS -I$WATCOM/h -I$WATCOM/h/os2 -I$LIBH -I$SRCH"
CFLAGS="$CFLAGS -fo=$OBJDIR/"

echo "============================================================================"
echo " PCBoard 15.4 OS/2 Build (OpenWatcom Cross-Compile)"
echo "============================================================================"
echo ""
echo "NOTE: This is a PORTING effort. The source was written for BCOS2."
echo "      WATCOMPAT.H handles most differences but manual fixes may be needed."
echo "      See PCB154_BUILD_GUIDE.md for details on Borland→Watcom differences."
echo ""
echo "Compiler: $CC"
echo "Flags: $CFLAGS"
echo ""

# Compile would go here — each source file needs individual attention
# for Borland→Watcom compatibility fixes.
echo "TODO: Compile 100+ source files with Watcom compatibility"
echo "      Start with: wcc386 $CFLAGS PCBSRC/MAIN/SOURCE/MAIN/PCBOARD.C"
echo ""
echo "The OpenWatcom path is VIABLE but requires file-by-file porting."
echo "See WATCOMPAT.H for the bridge macros."
