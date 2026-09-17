# Patches

## 15.4-pwa.patch (current)

Transforms cleaned 15.3 MAIN source into 15.4 MAIN source.
Diffs pcb153/SOURCE against pcb153/upd154/SOURCE (600 files).

- NEWSCR.CPP CUR_PPE_VER 320→340 (intentional 15.4 bump)
- PSA fields, PPL 3.40 functions, file flagging
- UUIN reject-by-name, FTP MGET
- All 15.4 beta changes Clark was working on

Applies to cleaned 15.3 tree (v0.2.5+ build fixes applied).
Previous version moved to attic — generated against uncleaned source.

## 15.4-toolkit.patch (current)

Transforms cleaned 15.3 toolkit into 15.4 toolkit.
Diffs toolkit/pwa153 against toolkit/pwa154 + toolkit/delta154.

- pcbtools.h: SPACERIGHTAT added to padtype enum
- pcbdcom: drop-in COMMDRV replacement
- delta154 Watcom port changes (6 TOOLKIT files)
- Stub fixes applied to both sides

## attic/ — superseded patches

See attic/README.md. Kept for provenance, not for current use.
