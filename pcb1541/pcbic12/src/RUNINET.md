# IC Rebuilt Components

Our rebuilds from source, for comparison against the shipped originals
in ../bin/.

## RUNINET.3.40.PPE

RUNINET.PPS compiled with our PPLC 3.40 (from pcb153/upd154).
- Our build: 2,286 bytes (PPL 3.40 bytecode)
- Clark's original: 1,808 bytes (PPL 3.20 bytecode)

NOT byte-exact — Clark compiled with PPL 3.20 (PCBoard 15.22 era), we
have 3.30/3.40. The programs are functionally equivalent; the byte
difference is the compiler version. Byte-exact rebuild needs PPLC 3.20.

This proves: our PPLC correctly compiles the recovered PPS source. The
remaining gap is purely the compiler version.
