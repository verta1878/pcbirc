# IC Rebuilt Components

Our rebuilds from source, for comparison against the shipped originals
in ../bin/.

## RUNINET.3.30.PPE

RUNINET.PPS compiled with PPLC 3.30 (from pcb1541/install/dist/target/).
- Our build: 2,261 bytes (PPL 3.30 bytecode)
- Clark's original: 1,808 bytes (PPL 3.20 bytecode)

NOT byte-exact — Clark compiled with PPL 3.20 (PCBoard 15.22 era).
Header identical through byte 42; divergence is bytecode instruction
encoding, not source. The programs are functionally equivalent.

## RUNINET.3.40.PPE

RUNINET.PPS compiled with our PPLC 3.40 (from pcb153/upd154).
- Our build: 2,286 bytes (PPL 3.40 bytecode)
- Clark's original: 1,808 bytes (PPL 3.20 bytecode)

Also NOT byte-exact — same compiler-version bytecode difference.

## Next: PPLC 3.20

PPLC320.EXE (222,176 B, md5 2a23e7686f79ea07bbb3c4d04e064a75) at
toolkit/pplc/3.20/PPLC320.EXE is the correct compiler. .gitignore
updated to allow toolkit/pplc/**/*.EXE. Once committed, one compile
should produce the byte-exact 1,808 B target.
