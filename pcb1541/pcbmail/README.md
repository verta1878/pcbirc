# pcbmail — PCBoard Mail

Scaffold for reproducing Clark's PCBMAIL.EXE (333 KB).

Windows GUI message reader/editor built with Borland C++ 4.50.
Reference design for pcbnav's message reader/editor.

- `src/pcbmail.c` — scaffold with correct message header struct
  from DOCDEV/MSGS.TXT (bsreal/MBF floats, 128-byte headers,
  11 status flags, MsgBaseHeader)

Features from PCBMAIL.HLP: configurable fonts (header: any Windows
font, body: fixed-pitch only, Terminal default), message editor,
address dialog with To/Subject/Cc, @LIST@ mailing, private/public
toggle, reads PCBoard message bases directly, CP437 support.
