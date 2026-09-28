#!/bin/bash
# BUILD_OW.SH — Build PCBCP under OpenWatcom 2.0 for OS/2
# Part of pcbrevival (GPLv3)

WPP="/opt/watcom/binl64/wpp386"
WRC="/opt/watcom/binl64/wrc"
WLINK="/opt/watcom/binl64/wlink"

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$HERE"
while [ "$REPO" != "/" ] && [ ! -f "$REPO/APPLY.txt" ]; do REPO="$(dirname "$REPO")"; done
if [ ! -f "$REPO/APPLY.txt" ]; then
  echo "ERROR: APPLY.txt not found above $HERE - is this inside the repo?" >&2
  exit 1
fi

SRCDIR="$(dirname "$0")/SOURCE"
OBJDIR="$(dirname "$0")/OBJ"
OUTDIR="$(dirname "$0")"

PCBLIBH="$REPO/toolkit/pwa153/H"
PCBMAINH="$REPO/pcb153/SOURCE/H"
OS2TK="${OS2TK45LIB:-$REPO/devtools/os2tk45/lib}"

mkdir -p "$OBJDIR"

echo "Compiling PCBCP for OS/2 PM..."
PASS=0; FAIL=0
for src in MAIN DLG FILE HELP INIT PNT THRD USER; do
    $WPP -bt=os2 -mf -5 -ox -zp1 -w0 \
      -i="$SRCDIR" -i="$PCBLIBH" -i="$PCBMAINH" \
      -i=/opt/watcom/h -i=/opt/watcom/h/os2 \
      -dLIBENTRY=__pascal -d_FARDATA_= -dfar= -dnear= -dhuge= \
      "$SRCDIR/$src.C" -fo="$OBJDIR/$src.obj" 2>&1 | tail -1
done

# Compile SEMAFORE.CPP (from PCBoard library)
$WPP -bt=os2 -mf -5 -ox -w0 \
  -i="$PCBLIBH" -i="$PCBMAINH" \
  -i=/opt/watcom/h -i=/opt/watcom/h/os2 \
  -dLIBENTRY=__pascal -d_FARDATA_= -dfar= -dnear= -dhuge= \
  "$REPO/toolkit/pwa153/SOURCE/SYSTEM/SEMAFORE.CPP" \
  -fo="$OBJDIR/SEMAFORE.obj"

# PCBCP_STUBS.C is referenced by the original port notes ("PCBoard library
# functions stubbed for initial link") but was never committed.  Link it if
# it turns up; otherwise carry on and let wlink name what is unresolved,
# which is more useful than a missing-file error.
STUBOBJ=""
if [ -f "$SRCDIR/PCBCP_STUBS.C" ]; then
  $WPP -bt=os2 -mf -5 -ox -w0 -i="$SRCDIR" -i=/opt/watcom/h -i=/opt/watcom/h/os2 \
    "$SRCDIR/PCBCP_STUBS.C" -fo="$OBJDIR/PCBCP_STUBS.obj" 2>&1 | tail -1
  STUBOBJ="file $OBJDIR/PCBCP_STUBS.obj"
else
  echo "NOTE: SOURCE/PCBCP_STUBS.C not present - linking without it."
fi

echo "Linking PCBCP_W.EXE..."
WATCOM=/opt/watcom $WLINK \
  format os2 le \
  name "$OUTDIR/PCBCP_W.EXE" \
  option quiet option stack=32768 \
  libpath /opt/watcom/lib386/os2 \
  libpath /opt/watcom/lib386 \
  libpath "$OS2TK" \
  library os2386 library clib3r library plib3r \
  file "$OBJDIR/MAIN.obj" \
  file "$OBJDIR/DLG.obj" \
  file "$OBJDIR/FILE.obj" \
  file "$OBJDIR/HELP.obj" \
  file "$OBJDIR/INIT.obj" \
  file "$OBJDIR/PNT.obj" \
  file "$OBJDIR/THRD.obj" \
  file "$OBJDIR/USER.obj" \
  file "$OBJDIR/SEMAFORE.obj" \
  ${STUBOBJ}

ls -lh "$OUTDIR/PCBCP_W.EXE" 2>/dev/null && echo "BUILD OK" || echo "BUILD FAILED"
