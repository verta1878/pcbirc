# PCBMODEM — no binary analysis required

Full C++ source exists in Clark's archives:

    Pcb-util/PCBMODEM/PCBMODEM/PCBMODEM.MAK    PCBMODEM.EXE
    Pcb-util/PCBMODEM/MSETUP/MSETUP.MAK        MSETUP.EXE
    Pcb-util/PCBMODEM/SOURCE/                  shared sources
        PCBMODEM.CPP  MSETUP.CPP  MDMCMDS.CPP
        COMMON.CPP    QUESTION.CPP  ERRORS.CPP
        TOKEN.C       INPUTNUM.C
        MODEMS.H      MSETUP.H      COMMON.H

MODEMS.H gives the exact MODEMS.DAT layout: headertype (16 bytes),
manufdatatype (48 bytes), modemdatatype (448 bytes).

An earlier scaffold here guessed the record layout and got every field
wrong. It has been deleted. Build from Clark's source instead.

Source archive password: pcb153 (lowercase).
