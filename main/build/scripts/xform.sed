# xform.sed - patch MASM 5.10/6.0 idioms TASM 3.1 misparses

# Rule 1: comment out .xcref lines that have argument lists
s/^\(\.xcref[ 	][A-Za-z_?@$]\)/;\1/
s/^\([ 	][ 	]*\.xcref[ 	][A-Za-z_?@$]\)/;\1/

# Rule 2: rename @Version to ?VERSION
s/@Version/?VERSION/g

# Rule 3: drop absolute-address group definitions
/^RGROUP[ 	][ 	]*group/d
/^AGROUP[ 	][ 	]*group/d
/^PSPGRP[ 	][ 	]*group/d
/^CGROUP[ 	][ 	]*group/d

# Rule 4: rename groups to their sole segment
s/RGROUP/ROMSEG/g
s/AGROUP/ALLMEM/g
s/PSPGRP/PSPSEG/g
s/CGROUP/CPUID_SEG/g

# Rule 5: loop dword ptr -> loopd (LOOPD is TASM's mnemonic)
s/\bloop[ 	][ 	]*dword ptr[ 	][ 	]*/loopd /g

# Rule 6: loop LABEL.EDD -> loop LABEL (strip decorative type hint)
s/\bloop[ 	][ 	]*\([A-Za-z_][A-Za-z_0-9]*\)\.EDD/loop \1/g

# Rule 7: 0&&SYM&&h -> 0&SYM&h (MASM 6 double-subst to TASM single)
s/&&\(@[A-Z_][A-Z_0-9]*\)&&/\&\1\&/g

# Rule 8: bt-family byte ptr -> word ptr (bt-family needs min word)
s/\(bt\|bts\|btr\|btc\)[ 	]\{1,\}byte ptr[ 	]\{1,\}/\1 word ptr /g

# Rule 9a: bt-family SEGREG:[reg].field, EXX  -> ... dword ptr ...
# (32-bit register second operand requires dword operand)
s/\(bt\|bts\|btr\|btc\)[ 	]\{1,\}\([A-Z_][A-Z_0-9]*:\[[^]]*\][^,]*\),[ 	]*\(e[abcds][xip]\|e[bs]p\)/\1 dword ptr \2,\3/g

# Rule 9b: bt-family SEGREG:[reg] with immediate or 16-bit reg -> word ptr
# (matches remaining cases after 9a already fired)
s/\(bt\|bts\|btr\|btc\)[ 	]\{1,\}\([A-Z_][A-Z_0-9]*:\[\)/\1 word ptr \2/g

# Rule 10: `COMMENT<delim>` -> `COMMENT <delim>` (TASM needs whitespace)
s/^COMMENT\([^A-Za-z0-9 	]\)/COMMENT \1/

# Rule 11: `mov al, es:[bx].OPROG_PCT` in UTIL_OPD.ASM — force byte-only read
# from a dw struct field. Safe: OPROG_PCT is a 0-100 percentage that fits in
# a byte, and we only need the low byte for CMP AL,100.
s/mov[ 	]\+al,[ 	]*es:\[bx\]\.OPROG_PCT/mov al,byte ptr es:[bx].OPROG_PCT/g

# Rule 12: `push DTE_DPMILDT` — DTE_DPMILDT is `equ (DTE_TSS+8)` (numeric
# constant, not dq). In USE16 code we want a 16-bit push of the selector.
s/push[ 	]\+DTE_DPMILDT/push word ptr DTE_DPMILDT/g
