#!/bin/bash
# rebuild.sh — regenerate the PCBoard 15.41 install target/ tree from INSTALL.zip
#
# Extracts all 8 archives inside pcb1541/install/INSTALL.zip (6 .RED archives
# plus PCBDISK.002 and PCBDISK.003), then places each source file into its
# target/ location per INSTALL.DAT's @File @Out directives. Byte-perfect
# against the original WCSC installer's output.
#
# Usage:  cd pcb1541/install/dist/target && ./rebuild.sh
# Or:     bash pcb1541/install/dist/target/rebuild.sh
#
# Requires: bash, unzip, python3, a C compiler (for redx).

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
INSTALL_ZIP="$REPO_ROOT/pcb1541/install/INSTALL.zip"
REDX_DIR="$REPO_ROOT/pcb1541/install/archivers/redx"
TARGET_DIR="$SCRIPT_DIR"
WORK_DIR="$(mktemp -d)"

echo "  Repo root: $REPO_ROOT"
echo "  Working:   $WORK_DIR"

[ -f "$INSTALL_ZIP" ] || { echo "ERROR: $INSTALL_ZIP not found"; exit 1; }
[ -d "$REDX_DIR" ]    || { echo "ERROR: $REDX_DIR not found"; exit 1; }

REDX="$WORK_DIR/redx"
echo "  Building redx..."
if ! cc -O2 -o "$REDX" "$REDX_DIR/redx.c" "$REDX_DIR/red_pack.c" "$REDX_DIR/red_decompress.c" 2>&1; then
    echo "ERROR: redx build failed"; exit 1
fi
[ -x "$REDX" ] || { echo "ERROR: redx build failed"; exit 1; }

echo "  Extracting archives from INSTALL.zip..."
mkdir -p "$WORK_DIR/ext"
# 6 .RED archives use the .RED extension; PCBDISK.002/003 don't
for entry in COMMDRV.RED:COMMDRV PCBCFGS.RED:PCBCFGS PCBMAIL.RED:PCBMAIL \
             PCBOARD.RED:PCBOARD PCBOARD2.RED:PCBOARD2 PPLC.RED:PPLC \
             PCBDISK.002:PCBDISK.002 PCBDISK.003:PCBDISK.003; do
    zipname="${entry%%:*}"
    archname="${entry##*:}"
    unzip -p "$INSTALL_ZIP" "$zipname" > "$WORK_DIR/$zipname" 2>/dev/null
    if [ -s "$WORK_DIR/$zipname" ]; then
        mkdir -p "$WORK_DIR/ext/$archname"
        (cd "$WORK_DIR/ext/$archname" && "$REDX" extract "$WORK_DIR/$zipname" > /dev/null)
    else
        echo "  WARN: $zipname not in INSTALL.zip (skipping)"
    fi
done

unzip -p "$INSTALL_ZIP" INSTALL.DAT > "$WORK_DIR/install.dat" 2>/dev/null

echo "  Placing files into target/..."
python3 "$SCRIPT_DIR/rebuild_place.py" "$WORK_DIR" "$TARGET_DIR"

rm -rf "$WORK_DIR"

echo ""
echo "  Done. target/ has been rebuilt from INSTALL.zip."
