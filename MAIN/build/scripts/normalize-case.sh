#!/bin/bash
# normalize-case.sh — lowercase header copies for case-sensitive builds
# the crew 4free — GPLv3
#
# WHY
#   DOS is case-insensitive; Linux is not. The source says
#       #include <vmdata.h>
#   while the file on disk is VMDATA.H. Borland under DOSBox-X does not
#   care. A Linux-native build — OpenWatcom's wcc/wlink, which is how
#   delta154 and irc1541 are meant to be built — fails to find it.
#
#   The header dirs are already mixed case and always have been:
#   toolkit/pwa153/H has misc.h, pcb.h, borland.h alongside SCREEN.H,
#   TYPES.HPP, NEWDATA.H. Renaming them would change Clark's filenames,
#   so this makes lowercase COPIES instead.
#
# SCOPE
#   Supersedes pcb154/normalize_case.sh, which covered only
#   LIB/H, MAIN/SOURCE/H and MAIN/SOURCE/H/H — no toolkit tree at all,
#   so toolkit/<branch>/H was never normalised by anything.
#
# THE COPIES ARE BUILD OUTPUT. DO NOT COMMIT THEM.
#   Run this when staging a build root, not against the repo you commit
#   from. Committing the copies doubles every header in git and gives you
#   two files to keep in sync — which is how a stale header outlives the
#   one it was copied from. `--check` reports without writing anything.
#
# USAGE
#   ./normalize-case.sh [--check] [repo-root]
#
set -u

CHECK=0
[ "${1:-}" = "--check" ] && { CHECK=1; shift; }
ROOT="${1:-.}"
cd "$ROOT" || { echo "no such directory: $ROOT" >&2; exit 1; }

# Every header directory across all four branches, plus the program trees.
DIRS="
toolkit/pwa153/H
toolkit/pwa153/SOURCE
toolkit/pwa154/H
toolkit/pwa154/SOURCE
toolkit/delta154/H
toolkit/delta154/SOURCE
toolkit/irc1541/H
toolkit/irc1541/SOURCE
pcb153/SOURCE/H
pcb153/upd154/SOURCE/H
pcb154/LIB/H
pcb154/MAIN/SOURCE/H
"

made=0; skipped=0; absent=0; clash=0

for dir in $DIRS; do
    if [ ! -d "$dir" ]; then
        absent=$((absent+1))
        printf '  %-34s (absent)\n' "$dir"
        continue
    fi
    n=0
    # -maxdepth 1: headers only, not the whole source tree below SOURCE/
    while IFS= read -r f; do
        base=$(basename "$f")
        lower=$(printf '%s' "$base" | tr 'A-Z' 'a-z')
        [ "$base" = "$lower" ] && continue
        target="$dir/$lower"
        if [ -e "$target" ]; then
            # A real file of that name already exists. Only a problem if
            # the contents differ — then one of them is stale.
            if ! cmp -s "$f" "$target"; then
                printf '  !! %s/%s and %s DIFFER — not overwriting\n' "$dir" "$base" "$lower"
                clash=$((clash+1))
            else
                skipped=$((skipped+1))
            fi
            continue
        fi
        if [ "$CHECK" = "1" ]; then
            n=$((n+1)); made=$((made+1))
        else
            cp -p "$f" "$target" && { n=$((n+1)); made=$((made+1)); }
        fi
    done <<EOF
$(find "$dir" -maxdepth 1 -type f \( -name '*.H' -o -name '*.HPP' -o -name '*.EXT' -o -name '*.ASI' \) 2>/dev/null)
EOF
    printf '  %-34s %s\n' "$dir" "$n"
done

echo
if [ "$CHECK" = "1" ]; then
    echo "would create: $made   already present: $skipped   dirs absent: $absent   CLASHES: $clash"
else
    echo "created: $made   already present: $skipped   dirs absent: $absent   CLASHES: $clash"
fi
[ "$clash" -gt 0 ] && {
    echo
    echo "A clash means two files differ only in filename case and have"
    echo "different contents. Resolve those by hand — one of them is stale,"
    echo "and on a case-insensitive checkout only one of them exists at all."
    exit 2
}
exit 0
