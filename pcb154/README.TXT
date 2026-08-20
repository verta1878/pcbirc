PCBoard 15.3 source tree with 15.4b patches applied
====================================================

This tree is the 15.3 v0.014 anonymous reconstruction of Clark Development
Company's PCBoard source, plus the 15.4 fixes and additions applied as
patch revisions v0.015 through v0.023 (see PCBSRCV/CHANGES.TXT for the
full log).

FIRST-TIME SYSOP?
─────────────────
Read DOCS/SYSOP_154.TXT.  It covers everything you need to know:

    - Order of upgrade operations
    - How to rebuild each binary
    - How to install the new Personal PSA
    - How the new PPL functions work
    - How to disable individual new prompts
    - How to back out to 15.3 if needed
    - Known gaps in this port vs. the shipped 15.4b

CODE LAYOUT
───────────
    MAIN/SOURCE/    Main PCBoard source
        H/          Shared headers (USERS.H, PCBTEXT.H, NEWSCR.HPP, ...)
        NODE/       Per-caller logic (LOGIN.C - the registration flow)
        USERS/      USERS file / USERS.INF I/O
        PPL/        PPL runtime (NEWSCR.CPP token table, EVALP.CPP dispatcher)
        UTIL/       Utility source (PCBSM, PCBSETUP, MKPCBTXT, ...)
        MISC/       UUCP, Fido, help, index utilities

    LIB/            Shared library source and headers

    PCBSRCV/        Revision patches
        000/        Original CDC source (baseline)
        001-014/    Anonymous reconstruction to buildable state
        015/        PPL crash fix
        016-018/    Reverse-engineering (superseded — see corrections in CHANGES)
        019/        UUIN 15.4 multipart/alternative fix
        020/        PCBSM 15.4 gender + PSA gate fixes
        021/        PPL 15.4 tokens (U_PERSONAL, SHORTDESC, GETBANKVAL, GET/SETMSGHDR)
        022/        PCBSM Personal PSA installer entry
        023/        New-user registration prompts for Personal PSA
        024/        PPL 15.4 statements (MOVEMSG, SETBANKVAL, SHORTDESC)
        025/        MD5LOGIN handshake activation (external-door auth)
        CHANGES.TXT All revision descriptions

    DOCS/           Sysop-facing documentation
        SYSOP_154.TXT   THE PRIMARY SYSOP GUIDE

BUILD ORDER
───────────
The 15.4 features touch four binaries.  Rebuild in this order:

    1. PPLC.EXE     (main tree, pplc.bat)
    2. PCBOARDM.EXE (main tree, compile.bat)
    3. PCBSM.EXE    (util/pcbsm, compile.bat)
    4. MKPCBTXT.EXE (util/pcbtext, compile.bat)

Test PCBOARDM first — if it starts without complaint, PPLC is happy with
the token table.  Then test PCBSM by loading the sysop menu.

NEED TO ROLL BACK?
──────────────────
See DOCS/SYSOP_154.TXT section 7.  Short answer: keep 15.3 backups of
PCBOARDM.EXE, PPLC.EXE, PCBSM.EXE, MKPCBTXT.EXE, and USERS.INF.  The
USERS file itself doesn't need a backup — its byte layout is unchanged.

SOURCE LICENSE
──────────────
The code is Clark Development Company proprietary.  Read Clark's license
terms before redistributing anything from this tree.  Patches to fix
bugs and extend functionality (i.e., what's here) are not the same as
selling or redistributing the codebase.

Maintainer: hexadecimal
