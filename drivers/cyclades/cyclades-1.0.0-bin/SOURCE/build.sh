#!/bin/bash
# build.sh — Cross-compile with OpenWatcom v2 on Linux
# Builds DOS (16-bit + DOS/4GW) and i386 (Win32) binaries.
# x64 requires Windows WDK (OW has no x86-64 compiler).

set -e
WATCOM="${WATCOM:-/opt/watcom}"
export PATH="${WATCOM}/binl64:$PATH"
OWINC="-i=inc -i=${WATCOM}/h -i=${WATCOM}/h/nt -i=${WATCOM}/h/nt/ddk"

case "${1:-all}" in
dos)
    wcl -q -ox -w4 -bt=dos -ml -fe=out/DOS/CYTEST.EXE test/cytest.c
    wcl -q -ox -w4 -bt=dos -ml -fe=out/DOS/CYFTST.EXE test/cyftst.c
    wcl386 -q -ox -w4 -bt=dos -l=dos4g -fe=out/DOS/CYTEST32.EXE test/cytest.c
    ;;
i386)
    for f in cylog cyisr cypower cyenum cypdo cyserial cyread cywrite cyioctl; do
        E=""; [ "$f" = "cyisr" ] && E="-dCY_DEBUG_REGS=0"
        wcc386 -q -ox -oi -w4 -bt=nt -3s $E $OWINC src/$f.c -fo=out/i386/$f.obj
    done
    O=""; for f in cylog cyisr cypower cyenum cypdo cyserial cyread cywrite cyioctl; do
        O="$O file out/i386/$f.obj"; done
    wlink system nt_dll name out/i386/cyport.sys $O \
        library clib3s library ntoskrnl library hal \
        libpath ${WATCOM}/lib386/nt libpath ${WATCOM}/lib386/nt/ddk
    ;;
all)
    mkdir -p out/DOS out/i386
    $0 dos; $0 i386
    ;;
clean)
    rm -rf out ;;
esac
