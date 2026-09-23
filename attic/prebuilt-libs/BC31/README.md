# attic/prebuilt-libs/BC31 — 9 prebuilt PCBoard category libraries

Moved here 2026-09-22 from `pcbcbase/PREBUILT/BC31/`.

## Why they were moved

They were sitting in the CodeBase folder, but **they are not CodeBase**.
They are PCBoard's own category libraries -- the output of
`toolkit/pwa153/SOURCE/<DIR>/MAKEFILE` -- and a source tree is the wrong
place for compiled output.

Who built them is unknown: they carry no build log, and nothing in the
repo records where they came from. They are the only copies we have in
the `.386` (386-optimised, large model) form that `PCBOARD.MAK` links
when `-D386` is set, so they are kept, not deleted.

**We cannot currently rebuild them.** The toolkit MAKEFILEs are hard-coded
to `bc31` / `large.386` and point at `..\..\bcdos\bc31`, a folder that does
not exist. Fixing that is the toolkit phase of the build work.

## How to rebuild them when the time comes

Every OMF library records the name of each module inside it, so the
recipe is in the files themselves, not a guess. One module = one source
file, from the matching `toolkit/pwa153/SOURCE/<DIR>` folder, compiled
with `CMOD = large.386`, then `tlib`'d in.

To verify a rebuild: dump the module list from the rebuilt library and
compare it with the list below. Same names, same count, and the library
is the same shape as the original. The sha256 of each original is
recorded so a byte-comparison is possible if a rebuild ever reproduces
one exactly.

The module lists were read straight out of the files (OMF THEADR
records).

### COUNTRYL.386 — 6,144 bytes, 3 modules
sha256 e37e89c76f29b3a8afb836ea85be4e1259813390e7b2cd633f3e928e61d83d63

    COUNTRY, DCOMMA, DATE

### DOSCLS_L.386 — 11,776 bytes, 1 modules
sha256 54570253cb927e28994657aaaa12b80aae0a7cefe8dee4d31d6c6762fd20a9ce

    DOSCLASS

### DOS_L.386 — 44,544 bytes, 44 modules
sha256 373d8887dc8549da7e9ec1056fbca88b840a4f79d4d7a1e8c0e2743d7bc3853e

    CHKAPPEN, CHKCREAT, CHKDOSFO, CHKFOPEN, CHKFPRNT, CHKLOCK, CHKOPEN, CHKREAD
    CHKUNLNK, CHKWRITE, DOSAPPEN, DOSCLOSE, DOSCOMIT, DOSCREAT, DOSDUP, DOSERROR
    DOSFCLOS, DOSFGETS, DOSFIND, DOSFLUSH, DOSFNGTS, DOSFOPEN, DOSFPUTS, DOSFREAD
    DOSFSEEK, DOSFTRUN, DOSFUGTS, DOSFWRIT, DOSLSEEK, DOSOPEN, DOSREAD, DOSREWIN
    DOSSTBUF, DOSTRUNC, DOSWRITE, EXTENDED, GETDRIVE, GETPATH, HANDLERS, ISOPEN
    SAY, SETDRIVE, STRNCHR, SHOWERR

### MISC_L.386 — 79,872 bytes, 85 modules
sha256 26aea77621667f953dee24b0aa35f81857c65ca0d363bf674ee0309bbba17e09

    ABORT, ADDCHAR, ALLDIGIT, APPEND, ASCII, BD_DBLE, BD_LONG, BINARY
    BMSEARCH, BS_DBLE, BS_LONG, BUILDSTR, CHANGE, CHKMOVE, COMMA, COPYFILE
    COPYFP, CRYPT, CTOD, DAYOWEEK, DBLE_BD, DBLE_BS, DBLE_PR, DBL_LONG
    DCOMMA, DELFILES, DIRECTRY, DISKFREE, DRIVEOK, DTOC, EDITOR, ENDSTR
    EVALUATE, EXIST, EXITFUNC, FINDFOUR, FINDNAME, FMEMCPY, FMEMSET, FULLNAME
    HEXTOI, INDEX, ISSET, JULIAN, LASTCHAR, LEFTSTR, LONG_BD, LONG_BS
    LONG_DBL, LONG_PR, MIDSTR, MKUNIQUE, MOVEFILE, MSTRCPY, PADSTR, PRNREADY
    PROPER, PR_DBLE, PR_LONG, PSEARCH, RIGHTSTR, RLE, SETBIT, SHARE
    SOUNDEX, STRIPA, STRIPB, STRIPL, STRIPR, SUBST, SWAPENV, TIME
    TIMESTEN, TTOC, UNSETBIT, VALIDATE, VALIDSEM, VIRTUAL, WILDCARD, ZSEARCH
    ZSORT, ZSWAPINT, ZSWAPLNG, ZSWAPSTR, ZSWAPVIR

### PCB_L.386 — 99,840 bytes, 22 modules
sha256 7090ba8e3b5a03402f1be52c4c96b0e3ad1e0e8fc1ffdb66baec452329fb6788

    ABORT, ACCOUNT, ADDBACKS, ALLOW, CHKEXIST, CI_OTHER, CNAMES, CONFFUNC
    CONFIG, DATA120, DATADFLT, DATAFIL2, DATAFILE, DATAREAD, DATAWRIT, EXITDOS
    GETDAYS, PARSE, PARSEPTH, SAVETEXT, SRCHPATH, TEXT

### SCREEN_L.386 — 30,208 bytes, 39 modules
sha256 3c316993b72a69d21a99a990be1f934b12c5986959fb696c0b4df7d22909e18d

    ANSI, BOX, BOXCLS, CLS, CLSBOX, CLSCOLOR, CURSOR, DATESTR
    DELAY, DELETE, FASTPUTC, GETMODE, GETSEC, GIVEUP, GOTOXY, GROWBOX
    INSERT, PRINT, PRINTV, PRNTCNTR, PRNTMOVE, READSCR2, READSCRN, SAVEREST
    SAVERST2, SCROLLDN, SCROLLUP, SETATT, SETFONT, SETROWS, SOUND, TIME1
    TIME2, TIMECHNG, TWODIG, TWODIG0, WHEREX, WHEREY, WINDOW

### SCRNIO_L.386 — 48,128 bytes, 20 modules
sha256 0f59e052e25189526adcd81c5e73e1673afff2cedcc7adb02eed7f423312faa7

    TIMEDKEY, INPUTALL, INPUTEXT, INPUTLN, INPUTNUM, INPUTSTR, BGETKEY, CURSKEYS
    ERROR, FORMDATE, GENSCRN, GETKEY, GETKEYFL, HELP, INITSCRN, MALLOCHK
    MESSAGE, SCALE, SCRNINPT, MENU

### SYSTEM_L.386 — 2,560 bytes, 3 modules
sha256 66bbf3d7a3445b1c89cf80ae926fb71795dc4a53433aeaee4a21ff47dc4c7ab4

    BGETKEY, SYSDATE, SYSTIME

### TOOLKITL.386 — 6,656 bytes, 3 modules
sha256 784e65f39e7a0af424e170229b1efb8889ff0dcb41b7a3f25e4fca0e7198c2e9

    ALTMODEM, NODISP, PCBDAT

## Note for the VIRTUAL.C question

`MISC_L.386` contains one module named `VIRTUAL` and no `VIRTUAL1`.
Whatever Clark shipped in this library, it was built from one of the two
virtual-memory implementations, not both.
