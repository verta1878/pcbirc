#!/bin/bash
# Verify pcb154/pcbdcom/ and pcb1541/pcbdcom/ are byte-identical.
# Run from repo root: bash pcb154/pcbdcom/CHECK-SYNC.sh
# Exit code 0 = in sync, 1 = drift detected.
set -e
if [ ! -d pcb154/pcbdcom ] || [ ! -d pcb1541/pcbdcom ]; then
    echo "ERROR: run from repo root."
    exit 2
fi
DIFF=$(diff -qr pcb154/pcbdcom pcb1541/pcbdcom | grep -v CHECK-SYNC || true)
if [ -z "$DIFF" ]; then
    echo "OK: pcb154/pcbdcom and pcb1541/pcbdcom are in sync."
    exit 0
fi
echo "DRIFT DETECTED between pcb154/pcbdcom and pcb1541/pcbdcom:"
echo "$DIFF"
echo ""
echo "To sync one direction (choose):"
echo "  cp -r pcb1541/pcbdcom/. pcb154/pcbdcom/"
echo "  cp -r pcb154/pcbdcom/. pcb1541/pcbdcom/"
exit 1
