#!/usr/bin/env python3
"""
stack_check.py — per-procedure push/pop stack-balance checker for
FOSSIL.ASM (or any BeginProc/EndProc-style VMM assembly source).

Usage:
    python3 stack_check.py path/to/FOSSIL.ASM

Scans each BeginProc/EndProc block and reports any push/pop-family
mnemonic-count mismatch, grouped by real opcode family (push/pushd/
pushw all balance against plain pop; pushfd/pushf/pusha/pushad each
balance against their own exact opcode — this codebase never uses
'popd'/'popw', confirmed by inspection, so those aren't modeled as
separate families).

IMPORTANT — read before trusting the output: a flagged procedure is
NOT automatically a bug. This tool counts textual occurrences, not
control flow. The dominant false-positive pattern in FOSSIL.ASM is
one push/pushad written once in source (at a loop top or procedure
entry), matched by a pop/popad written once per separate exit branch
or loop iteration — completely correct code that this tool cannot
distinguish from a real leak without manual tracing. Every procedure
this tool flags should be read by hand before being treated as a
defect; see docs/RECOVERY.md section 8 for a full 2026-08 trace that
did this for all 19 procedures FOSSIL.ASM's current source flags,
and found no genuine bug among them.
"""
import re, sys

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} path/to/FOSSIL.ASM")
    sys.exit(1)

path = sys.argv[1]
lines = open(path, encoding='latin-1').read().splitlines()

procs = []
cur_name = None
cur_start = None
for i, line in enumerate(lines):
    m = re.match(r'^\s*BeginProc\s+([A-Za-z_][A-Za-z0-9_]*)', line)
    if m:
        cur_name = m.group(1)
        cur_start = i
        continue
    m2 = re.match(r'^\s*EndProc\b', line)
    if m2 and cur_name:
        procs.append((cur_name, cur_start, i))
        cur_name = None

# Correct grouping: push/pushd/pushw all push a value popped by generic
# pop (this codebase never uses 'popd'/'popw' at all — verified by grep).
# pushfd/popfd, pushf/popf, pusha/popa, pushad/popad are distinct real
# opcodes and must each balance with their own exact counterpart.
GROUPS = [
    (['push', 'pushd', 'pushw'], ['pop']),
    (['pushfd'], ['popfd']),
    (['pushf'], ['popf']),
    (['pusha'], ['popa']),
    (['pushad'], ['popad']),
]

def strip_comment(line):
    in_str = False
    for i, ch in enumerate(line):
        if ch == '"':
            in_str = not in_str
        elif ch == ';' and not in_str:
            return line[:i]
    return line

all_mnems = set()
for pu, po in GROUPS:
    all_mnems.update(pu); all_mnems.update(po)

results = []
for name, start, end in procs:
    counts = {k: 0 for k in all_mnems}
    for ln in lines[start:end+1]:
        code = strip_comment(ln).strip()
        if not code:
            continue
        m = re.match(r'^([A-Za-z][A-Za-z0-9]*)\b', code)
        if not m:
            continue
        mnem = m.group(1).lower()
        if mnem in counts:
            counts[mnem] += 1

    imbalances = []
    for pu_list, po_list in GROUPS:
        pu_total = sum(counts[k] for k in pu_list)
        po_total = sum(counts[k] for k in po_list)
        if pu_total != po_total:
            imbalances.append(f"{'+'.join(pu_list)}={pu_total} vs {'+'.join(po_list)}={po_total}")

    if imbalances:
        results.append((name, start+1, end+1, imbalances))

print(f"Checked {len(procs)} procedures (BeginProc/EndProc pairs).\n")
if not results:
    print("No push/pop-family imbalance found in any procedure.")
else:
    for name, s, e, imb in results:
        print(f"PROC {name} (lines {s}-{e}):")
        for i in imb:
            print(f"    {i}")
    print(f"\n{len(results)} procedure(s) flagged for manual review.")
