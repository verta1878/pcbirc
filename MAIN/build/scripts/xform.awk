# xform.awk - expand MASM 6.0 @@:/@F/@B anonymous labels.
# Handles BOTH regular code and macro bodies:
#   - Outside macros: @@ -> LBL_N (file-global numbered)
#   - Inside macros:  @@ -> LBL_N with LOCAL declaration auto-inserted
#     after the MACRO line, making them per-invocation unique (TASM's
#     LOCAL directive gives each macro expansion fresh labels).
# Case-insensitive: @F/@f/@B/@b.

BEGIN {
    total_lines = 0
}

{
    lines[++total_lines] = $0
}

function is_macro_start(s,   lo) {
    lo = tolower(s)
    return match(lo, /^[ \t]*[a-z_?@$][a-z_?@$0-9]*[ \t]+macro([ \t]|$)/)
}
function is_macro_end(s,   lo) {
    lo = tolower(s)
    return match(lo, /^[ \t]*endm([ \t;]|$)/)
}

END {
    # Pass 1: determine macro spans
    in_macro = 0
    for (ln = 1; ln <= total_lines; ln++) {
        macro_of[ln] = 0
        if (in_macro) macro_of[ln] = in_macro
        if (is_macro_start(lines[ln])) {
            in_macro = ln           # macro span keyed by macro-start line
            macro_of[ln] = 0        # the MACRO line itself is a header, not body
        }
        if (is_macro_end(lines[ln])) in_macro = 0
    }
    
    # Pass 2: number @@: labels per scope (global scope = 0, per-macro = macro-start line)
    for (ln = 1; ln <= total_lines; ln++) {
        if (match(lines[ln], /^[ \t]*@@:/)) {
            scope = macro_of[ln]
            n_in_scope[scope]++
            label_at_line[ln] = "LBL_" n_in_scope[scope]
            atline_by_scope[scope, n_in_scope[scope]] = ln
        }
    }
    
    # Pass 3: build LOCAL declaration for each macro that uses @@
    for (macro_ln = 1; macro_ln <= total_lines; macro_ln++) {
        if (is_macro_start(lines[macro_ln]) && n_in_scope[macro_ln] > 0) {
            decl = ""
            for (i = 1; i <= n_in_scope[macro_ln]; i++) {
                if (decl == "") decl = "\tLOCAL\tLBL_" i
                else decl = decl ",LBL_" i
            }
            local_after[macro_ln] = decl
        }
    }
    
    # Pass 4: emit rewritten lines
    for (ln = 1; ln <= total_lines; ln++) {
        line = lines[ln]
        scope = macro_of[ln]
        # rewrite @@:
        if (ln in label_at_line) sub(/@@:/, label_at_line[ln] ":", line)
        # find next/prev in same scope
        next_lbl = ""; prev_lbl = ""
        for (i = 1; i <= n_in_scope[scope]; i++) {
            if (atline_by_scope[scope,i] > ln) { next_lbl = "LBL_" i; break }
        }
        for (i = n_in_scope[scope]; i >= 1; i--) {
            if (atline_by_scope[scope,i] < ln) { prev_lbl = "LBL_" i; break }
        }
        if (next_lbl != "") {
            while (match(line, /@[Ff]([^A-Za-z0-9_]|$)/)) {
                head = substr(line, 1, RSTART - 1)
                tail = substr(line, RSTART + 2)
                line = head next_lbl tail
            }
        }
        if (prev_lbl != "") {
            while (match(line, /@[Bb]([^A-Za-z0-9_]|$)/)) {
                head = substr(line, 1, RSTART - 1)
                tail = substr(line, RSTART + 2)
                line = head prev_lbl tail
            }
        }
        print line
        # emit LOCAL declaration right after macro header line
        if (ln in local_after) print local_after[ln]
    }
}
