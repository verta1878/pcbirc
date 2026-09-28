#!/bin/bash
# ============================================================================
# mkimage.sh - create a fresh PCBBLDBT.IMG from source + devtools
#
# Right-sized (96 MB by default): current content is ~61 MB, this gives
# ~35 MB headroom for growth (PCBKMS OBJ/LIB outputs, delta/ow2irc work).
# FAT16 with FreeDOS 1.3 boot sector.
#
# Usage:  mkimage.sh [output_path] [size_mb]
#   output_path  destination for the .IMG (default: /tmp/goldimg/PCBBLDBT.IMG)
#   size_mb      total image size in MB (default: 96)
#
# Prereqs on host: mtools, unzip, 7z
# Requires in repo: devtools/*.zip, MAIN/build/PCBBLDBT.CONF,
#                   toolkit/, pcb1541/, MSC70BT.ZIP (or devtools/MSC70.zip)
# ============================================================================
set -euo pipefail

OUT="${1:-/tmp/goldimg/PCBBLDBT.IMG}"
SIZE_MB="${2:-96}"

# Compute geometry: 63 sectors/track, 16 heads, 512 bytes/sector
# cyls = (size_mb * 1024 * 1024) / (63 * 16 * 512) = size_mb * 2.032
CYLS=$(( (SIZE_MB * 1024 * 1024) / (63 * 16 * 512) ))
TOTAL_SECTORS=$(( CYLS * 16 * 63 ))
IMG_BYTES=$(( TOTAL_SECTORS * 512 ))

REPO="$(cd "$(dirname "$0")/../../.." && pwd)"

echo "==> mkimage.sh"
echo "    output:  $OUT"
echo "    size:    ${SIZE_MB} MB (${IMG_BYTES} bytes, ${CYLS} cyls x 16 heads x 63 spt)"
echo "    repo:    $REPO"

# 1) Create sparse image, format FAT16 with FreeDOS-compatible params
mkdir -p "$(dirname "$OUT")"
rm -f "$OUT"
truncate -s "$IMG_BYTES" "$OUT"

mformat -T "$TOTAL_SECTORS" -h 16 -s 63 -H 0 -i "$OUT" -F ::

# 2) Standard dirs
for d in FDOS BC31 TC201 MSC70 CWSDPMI HX D32A 386MAX_S TOOLKIT PCB153 \
         BUILD OUT SCRIPTS TMP; do
    mmd -i "$OUT" "::/$d" 2>/dev/null || true
done

# 3) FreeDOS kernel + shell + boot loader
#    Assumes devtools/freedos/ has KERNEL.SYS, COMMAND.COM, SYS.COM, FDCONFIG.SYS
FDDIR="$REPO/devtools/freedos"
if [ -d "$FDDIR" ]; then
    mcopy -i "$OUT" "$FDDIR/KERNEL.SYS"  ::/KERNEL.SYS
    mcopy -i "$OUT" "$FDDIR/COMMAND.COM" ::/COMMAND.COM
    mcopy -i "$OUT" "$FDDIR/FDCONFIG.SYS" ::/FDCONFIG.SYS 2>/dev/null || true
    # Install FreeDOS boot sector (requires syslinux-utils or freedos SYS)
    # Deferred: currently boot flow uses fdboot floppy launcher, so raw FAT VBR is enough
else
    echo "    WARNING: $FDDIR not found - image will not boot standalone"
    echo "             (fdboot floppy launcher can still chain-boot it)"
fi

# 4) Extract dev tools from pinned archives
extract_zip() {
    local src="$1" dst="$2"
    if [ -f "$src" ]; then
        local tmp
        tmp="$(mktemp -d)"
        unzip -q "$src" -d "$tmp"
        mcopy -i "$OUT" -o -s -Q "$tmp"/* "::/$dst/" 2>/dev/null || true
        rm -rf "$tmp"
    fi
}
extract_zip "$REPO/devtools/BC31.zip"        BC31
extract_zip "$REPO/devtools/TURBOC201.zip"   TC201
extract_zip "$REPO/MSC70BT.ZIP"              MSC70
extract_zip "$REPO/devtools/cwsdpmi.zip"     CWSDPMI
extract_zip "$REPO/devtools/hxrt216.zip"     HX

# TODO (later): Watcom for Delta 15.4 + irc 1541 (modernized free build system)
# extract_zip "$REPO/devtools/OW2.zip"        WATCOM
# 5) Source trees
mcopy -i "$OUT" -o -s -Q "$REPO/toolkit"/*  ::/TOOLKIT/ 2>/dev/null || true
mcopy -i "$OUT" -o -s -Q "$REPO/pcb1541"/*  ::/PCB153/  2>/dev/null || true

# 6) Build scripts + config
mcopy -i "$OUT" -o -s -Q "$REPO/MAIN/build/scripts"/*.BAT ::/SCRIPTS/
mcopy -i "$OUT" -o -s -Q "$REPO/MAIN/build/scripts"/*.RSP ::/SCRIPTS/ 2>/dev/null || true
mcopy -i "$OUT" -o -s -Q "$REPO/MAIN/build/scripts"/xform.sed ::/SCRIPTS/
mcopy -i "$OUT" -o -s -Q "$REPO/MAIN/build/scripts"/xform*.awk ::/SCRIPTS/

# 7) Baseline AUTOEXEC + CONFIG.SYS on C:
cat > /tmp/_cfg.sys << 'CFG'
FILES=40
BUFFERS=20
STACKS=9,256
SHELL=C:\COMMAND.COM /P /E:1024
CFG
sed -i 's/$/\r/' /tmp/_cfg.sys
mcopy -i "$OUT" -o /tmp/_cfg.sys ::/CONFIG.SYS
rm /tmp/_cfg.sys

echo "==> done"
echo "    used:    $(mdir -i "$OUT" :: | tail -2 | head -1)"
echo "    boot with: dosbox-x -conf $REPO/MAIN/build/PCBBLDBT.CONF"
