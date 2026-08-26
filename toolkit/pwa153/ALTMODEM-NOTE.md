# ALTMODEM — what it is, and what it helps compile

ALTMODEM ("alternative modem") = toolkit/pwa153/SOURCE/TOOLKIT/ALTMODEM.C

## What it is

A standalone modem test/demo utility with its own `main()`. It
demonstrates the toolkit's modem layer (initmodem, modemcommand,
openmodem/closemodem, slowsendtomodem) in isolation.

To link standalone, it deliberately STUBS OUT several toolkit functions
(the source says so in a comment): loguseroff, writelog, kbdinkey,
insertbuffer, watchsystemfunctions, and errorexittodos. These local
stubs are why it clashes with the real toolkit modules (e.g. EXITDOS.C's
errorexittodos) — so ALTMODEM must NOT go in the main library.

## What it helps compile — TO INVESTIGATE

Clark's TOOLKIT/MAKEFILE combines altmodem + nodisp + pcbdat into an
internal `toolkitl.lib`. But the SHIPPED SDK only includes NODISP.OBJ
and PCBDAT.OBJ as loose objects — ALTMODEM was source-only.

Open question: what downstream program links ALTMODEM's object or uses
it as a template? Candidates to check:
- The modem/serial utilities (PCBoard's own modem init path)
- Door examples that need a minimal modem harness
- pcb1541/pcbcomm (our serial work)

Action: trace who (if anyone) links altmodem.obj, and whether it's a
build helper for a specific tool. Until then: keep as source, do NOT
put in the library, do NOT ship as a loose override OBJ.
