# pcbtic — TIC File Echo Processor

Processes FidoNet TIC (Tick) file echo distributions.

- `pcbtic.c` — TIC processor (594 lines)

Handles incoming .TIC files: validates CRC, moves associated files
to correct PCBoard file areas, updates file directories, and
forwards TIC packets to downlinks.
