#!/bin/bash
# BUILD_OW.SH — Build PCBCP under OpenWatcom 2.0 for OS/2
# Part of pcbrevival (GPLv3)

WPP="/opt/watcom/binl64/wpp386"
WRC="/opt/watcom/binl64/wrc"
WLINK="/opt/watcom/binl64/wlink"

SRCDIR="$(dirname "$0")/SOURCE"
OBJDIR="$(dirname "$0")/OBJ"
OUTDIR="$(dirname "$0")"

PCBLIBH="$(dirname "$0")/../../PCBSRC/LIB/H"
PCBMAINH="$(dirname "$0")/../../PCBSRC/MAIN/SOURCE/H"
OS2TK="$(dirname "$0")/../../tools/fpc264irc/sdk/os2tk45/lib"

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
  "$(dirname "$0")/../../PCBSRC/LIB/SOURCE/SYSTEM/SEMAFORE.CPP" \
  -fo="$OBJDIR/SEMAFORE.obj"

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
  file "$OBJDIR/pcbcp_stubs.obj"

ls -lh "$OUTDIR/PCBCP_W.EXE" 2>/dev/null && echo "BUILD OK" || echo "BUILD FAILED"
