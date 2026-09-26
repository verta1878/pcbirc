#!/bin/bash
# normalize_case.sh — Create lowercase copies of all headers for Linux builds
# sysop/0: "she needs a for f in *.H; do cp "$f" "$(echo $f | tr A-Z a-z)"; done"
#
# Run from PCBSRC root. Idempotent — safe to run multiple times.

for dir in LIB/H MAIN/SOURCE/H MAIN/SOURCE/H/H; do
    if [ -d "$dir" ]; then
        cd "$dir"
        for f in *.H *.HPP *.EXT; do
            [ -f "$f" ] || continue
            lower=$(echo "$f" | tr 'A-Z' 'a-z')
            if [ "$f" != "$lower" ] && [ ! -f "$lower" ]; then
                cp "$f" "$lower"
            fi
        done
        cd - > /dev/null
    fi
done

echo "Case normalization complete."
