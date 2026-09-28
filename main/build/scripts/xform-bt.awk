# xform-bt.awk - size-annotate bt-family + push/pop of dq descriptor selectors
# NOTE: bt/bts/btr/btc do NOT support byte operands (x86 rule).
#       For byte-sized flags, widen to WORD PTR — same as MASM 6.
# NOTE: `push DTE_XXX; pop segreg` (loading a selector from an 8-byte
#       descriptor into a segment register) needs `word ptr` on the push.
#       TASM 3.1 can't infer the size; MASM 6 could.

BEGIN {
    if (mode == "rewrite") {
        while ((getline line < symtab) > 0) {
            gsub(/\r$/, "", line)
            n = split(line, parts, "\t")
            if (n >= 2) sym_size[parts[1]] = parts[2]
        }
        close(symtab)
    }
}

mode == "scan" {
    # Case 1: SYMBOL db/dw/dd/dq VALUE
    if (match($0, /^[ \t]*([A-Za-z_?@$][A-Za-z_?@$0-9]*)[ \t]+(db|dw|dd|dq)[ \t]/, m)) {
        sym = m[1]; dir = m[2]
        if      (dir == "db") sz = "byte"
        else if (dir == "dw") sz = "word"
        else if (dir == "dd") sz = "dword"
        else if (dir == "dq") sz = "qword"
        else sz = ""
        if (sz != "" && !(sym in sym_size)) sym_size[sym] = sz
    }
    # Case 2: RECNAME record field:width,field:width,...
    else if (match($0, /^[ \t]*([A-Za-z_?@$][A-Za-z_?@$0-9]*)[ \t]+record[ \t]+(.+)/, m)) {
        recname = m[1]
        fields = m[2]
        gsub(/[ \t\r]+$/, "", fields)
        total = 0
        n = split(fields, flist, ",")
        for (i = 1; i <= n; i++) {
            if (match(flist[i], /:([0-9]+)/, w)) total += w[1] + 0
        }
        if      (total <= 8)  sz = "byte"
        else if (total <= 16) sz = "word"
        else                  sz = "dword"
        if (!(recname in rec_size)) rec_size[recname] = sz
        if (!(recname in sym_size)) sym_size[recname] = sz
    }
    # Case 3: INSTANCE RECNAME <values>   (record instance)
    else if (match($0, /^[ \t]*([A-Za-z_?@$][A-Za-z_?@$0-9]*)[ \t]+([A-Za-z_?@$][A-Za-z_?@$0-9]*)[ \t]+</, m)) {
        instname = m[1]; typname = m[2]
        inst_type[instname] = typname
    }
}

mode == "rewrite" {
    line = $0
    # Rule A: bt-family with bare symbol (not struct.field)
    if (match(line, /^([ \t]+)(bt|bts|btr|btc)([ \t]+)([A-Za-z_?@$][A-Za-z_?@$0-9]*)([, \t])/, m)) {
        indent = m[1]; instr = m[2]; ws = m[3]; sym = m[4]; after = m[5]
        if (sym in sym_size) {
            eff = sym_size[sym]
            if (eff == "byte") eff = "word"       # bt-family: min word
            if (eff == "qword") eff = "word"      # shouldn't happen but be safe
            new_head = indent instr ws eff " ptr " sym after
            rest = substr(line, RSTART + RLENGTH)
            line = new_head rest
        }
        print line; next
    }
    # Rule B: `push SYMBOL` where SYMBOL is dq (descriptor selector load)
    if (match(line, /^([ \t]+)(push|pop)([ \t]+)([A-Za-z_?@$][A-Za-z_?@$0-9]*)([, \t]|$)/, m)) {
        indent = m[1]; instr = m[2]; ws = m[3]; sym = m[4]; after = m[5]
        if ((sym in sym_size) && sym_size[sym] == "qword") {
            new_head = indent instr ws "word ptr " sym after
            rest = substr(line, RSTART + RLENGTH)
            line = new_head rest
        }
    }
    print line
    next
}

END {
    if (mode == "scan") {
        for (inst in inst_type) {
            typ = inst_type[inst]
            if (typ in rec_size && !(inst in sym_size)) sym_size[inst] = rec_size[typ]
        }
        for (s in sym_size) print s "\t" sym_size[s]
    }
}
