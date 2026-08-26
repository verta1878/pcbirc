#!/bin/bash
# normalize_case.sh - Create lowercase copies of source/header files tree-wide
#                     so Linux / OpenWatcom / ow2irc builds find them regardless
#                     of the original DOS uppercase naming.
#
# Makes lowercase COPIES (does not delete the uppercase originals), so DOS
# compilers (Borland, Turbo C, MSC) that expect UPPERCASE still work, while
# case-sensitive hosts get the lowercase names they need. Idempotent - safe
# to run repeatedly.
#
# Run from the repo root.
#
# Scope: C/C++ source and headers (.c .h .hpp .cpp .ext .inc .asm) plus, with
# --dirs, lowercase directory copies via symlink. Default is files only.
#
# Usage:
#   ./normalize_case.sh            # lowercase file copies, tree-wide
#   ./normalize_case.sh --dirs     # also add lowercase dir symlinks
#   ./normalize_case.sh PATH ...   # limit to given paths

set -u

DO_DIRS=0
ROOTS=()
for arg in "$@"; do
    case "$arg" in
        --dirs) DO_DIRS=1 ;;
        *) ROOTS+=("$arg") ;;
    esac
done
[ ${#ROOTS[@]} -eq 0 ] && ROOTS=(.)

# Extensions to normalize (case-insensitive match)
EXTS="c h hpp cpp cxx cc ext inc asm def rc"

lower() { echo "$1" | tr 'A-Z' 'a-z'; }

files_done=0
dirs_done=0

for root in "${ROOTS[@]}"; do
    # 1. Lowercase file copies
    for ext in $EXTS; do
        # find files with an uppercase-containing name of this extension
        while IFS= read -r -d '' f; do
            dir=$(dirname "$f")
            base=$(basename "$f")
            low=$(lower "$base")
            if [ "$base" != "$low" ] && [ ! -e "$dir/$low" ]; then
                cp "$f" "$dir/$low" && files_done=$((files_done+1))
            fi
        done < <(find "$root" -type f -iname "*.$ext" -print0 2>/dev/null)
    done

    # 2. Optional: lowercase directory symlinks (for path-case-sensitive builds)
    if [ "$DO_DIRS" -eq 1 ]; then
        while IFS= read -r -d '' d; do
            parent=$(dirname "$d")
            base=$(basename "$d")
            low=$(lower "$base")
            if [ "$base" != "$low" ] && [ ! -e "$parent/$low" ]; then
                ( cd "$parent" && ln -s "$base" "$low" ) && dirs_done=$((dirs_done+1))
            fi
        done < <(find "$root" -depth -type d -print0 2>/dev/null)
    fi
done

echo "Case normalization complete: $files_done file copies, $dirs_done dir links."
