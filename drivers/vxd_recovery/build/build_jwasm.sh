#!/bin/sh
# ============================================================
#  FOSSIL.VXD build — JWasm + Open Watcom wlink (Linux)
#  Reproduces the verified VxD from the ported source in ../src
# ============================================================
#  Requires:
#    - jwasm            (MASM-compatible assembler)
#    - wlink            (Open Watcom linker, VxD/LE support)
#  Both build cleanly on Linux; see docs/RECOVERY.md for sources.
# ============================================================
set -e
JWASM="${JWASM:-jwasm}"
WLINK="${WLINK:-wlink}"
SRC=../src
OUT=../verified

echo "[1/2] Assembling FOSSIL.ASM (COFF, MASM6 dialect)..."
"$JWASM" -c -coff -DBLD_COFF -DIS_32 -Sg -DMASM6 -W2 -Zp1 -DNODECOUNT=16 \
    -I"$SRC" -Fo FOSSIL.obj "$SRC/FOSSIL.ASM"

echo "[2/2] Linking FOSSIL.VXD (LE format)..."
cat > fossil.lnk << LNK
format windows vxd dynamic
file FOSSIL.obj
name FOSSIL.vxd
export FOSSIL_DDB.1
LNK
"$WLINK" @fossil.lnk

echo "Done. FOSSIL.vxd:"
file FOSSIL.vxd
