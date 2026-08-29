#!/bin/bash
# ============================================================================
# pcbbldbt-smoke.sh - Robust DOSBox-X smoke-test runner for PCBBLDBT.IMG
#
# Runs one AUTOEXEC scenario in the golden build image cleanly, kills DOSBox-X
# via SIGKILL on its process group, captures log, extracts test outputs.
# A brief <defunct> DOSBox-X entry appears after each run (kernel process-table
# stub, ~0 resources) and is reaped by init within a few seconds. Not a leak.
#
# Usage:  pcbbldbt-smoke.sh <conf> <timeout_secs> [test_output_pattern]
#   conf                  DOSBox-X config file (e.g., /tmp/goldimg/PCBBLDBT.CONF)
#   timeout_secs          Hard kill timer (default 30)
#   test_output_pattern   glob of files to pull from ::/TMP/ (default *.TXT)
#
# Prereqs: mtools, Xvfb, dosbox-x, the target IMG referenced by <conf>.
# ============================================================================

set -u

CONF="${1:-/tmp/goldimg/PCBBLDBT.CONF}"
TMOUT="${2:-30}"
PATTERN="${3:-*.TXT}"

# Extract IMG path from the conf (best-effort - assumes first HDD imgmount)
IMG=$(grep -oE 'imgmount 2 [^ ]+' "$CONF" 2>/dev/null | awk '{print $3}' | head -1)
if [ -z "$IMG" ] || [ ! -f "$IMG" ]; then
    echo "ERROR: could not resolve HDD image from $CONF (found: '$IMG')" >&2
    exit 2
fi

LOG="/tmp/pcbbldbt-smoke.$$.log"
OUTDIR="/tmp/pcbbldbt-smoke-out.$$"
DISPLAY_NUM=99

# --- 1. Belt-and-suspenders: kill any prior dosbox-x/xvfb before starting
pkill -9 -f dosbox-x 2>/dev/null || true
pkill -9 -f Xvfb 2>/dev/null || true
sleep 2

# --- 2. Clear old test outputs on the image (silently)
mdel -i "$IMG" "::/TMP/$PATTERN" >/dev/null 2>&1 || true

# --- 3. Start a persistent Xvfb (we control it)
Xvfb :$DISPLAY_NUM -screen 0 800x600x24 >/dev/null 2>&1 &
XVFB_PID=$!
sleep 1

# --- 4. Run DOSBox-X in its own process group via setsid, capped
#    setsid  => new pgid, so we can kill the entire tree
#    exec    => replace the shell so timeout signals reach dosbox-x directly
setsid bash -c "exec timeout --kill-after=2 --signal=KILL $TMOUT \
    dosbox-x -silent -exit -conf '$CONF' 2>&1 | head -c 200000 > '$LOG'" &
GROUP_PID=$!

# --- 5. Wait for the group; if it doesn't finish in TMOUT+5, force-kill it
( sleep $((TMOUT + 5)); kill -9 -$GROUP_PID 2>/dev/null; ) &
WATCHDOG_PID=$!
wait $GROUP_PID 2>/dev/null
EXIT=$?
kill $WATCHDOG_PID 2>/dev/null

# --- 6. Really force-kill anything left in the group + xvfb
kill -9 -$GROUP_PID 2>/dev/null || true
{ kill -9 $XVFB_PID; wait $XVFB_PID; } 2>/dev/null || true
pkill -9 -f dosbox-x 2>/dev/null || true

# --- 6a. Give init a moment to reap the defunct dosbox-x entry
sleep 1

# --- 7. Pull test outputs
mkdir -p "$OUTDIR"
mcopy -i "$IMG" -o "::/TMP/$PATTERN" "$OUTDIR/" >/dev/null 2>&1 || true

# --- 8. Report
echo "=== pcbbldbt-smoke: exit=$EXIT log=$LOG outdir=$OUTDIR ==="
echo "--- log errors (DOSBox-X CPU / E_Exit / IRET / descriptor / GRP / 8087) ---"
grep -E 'ERROR|E_Exit|Illegal|Invalid|IRET|descriptor|GRP|8087' "$LOG" 2>/dev/null \
    | grep -v 'Keyboard layout' \
    | head -12
echo "--- outputs pulled ---"
ls -la "$OUTDIR" 2>/dev/null | tail -n +2
echo "=== done ==="
