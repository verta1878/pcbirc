# pwa153 SDK — Toolkit Libraries (Borland C++ 3.1 = PCBKBC)

Built from the 15.3 toolkit source. This matches Clark's SDK structure:
one library per memory model, plus loose override .OBJ files.

## Libraries (4 memory models)

| File | Model | Notes |
|---|---|---|
| PCBKBC_S.LIB | Small | |
| PCBKBC_M.LIB | Medium | **PCBoard itself uses medium** |
| PCBKBC_C.LIB | Compact | |
| PCBKBC_L.LIB | Large | |

PCBKBC = the Borland C++ 3.1 family (confirmed from Clark's own .LIB
compiler records). Each lib holds the 113 main toolkit modules.

## Loose override OBJs (obj/)

Shipped alongside the libraries, linked selectively — a door links the
library plus whichever overrides it needs (Clark's design):

- Feature stubs: NOANSI, NOCHAT, NODISP, NOHELP, NOINPUT, NOLANG,
  NOLOG, NOMEMORY, NOSCREEN, NOSHELL, NOSTATUS, NOSYS, NOXLATE,
  SMALLERR — link to disable/shrink a feature
- PCBDAT — PCBOARD.DAT access override

Note on ALTMODEM: it is NOT a library module or an override OBJ. Its
source (toolkit/pwa153/SOURCE/TOOLKIT/ALTMODEM.C) is a standalone modem
test utility with its own main() that deliberately stubs out toolkit
functions (loguseroff, writelog, kbdinkey, errorexittodos) so it links
by itself. It ships as SOURCE only (Clark never put it in the library),
alongside the samples.

Serial drivers (COMMDRV, FOSSIL) are a known gap — Clark shipped them
as loose OBJs but we don't have their source; our serial work is in
pcb1541/pcbcomm and drivers/netfosdl.

## Other compiler families (pending)

- PCBKIT (Turbo C 2.01) — compiler in devtools/TURBOC201.zip
- PCBKMS (Microsoft C 7.0) — compiler in devtools/MSC70.zip

See devtools/COMPILERS.md.

## Linking a door

```
tlink <startup> yourdoor obj\NODISP, yourdoor,, PCBKBC_L
```

Link order: your objects, then the loose overrides you want, then the
library.
