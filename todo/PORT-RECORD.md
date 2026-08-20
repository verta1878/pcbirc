# Port Record

C code ported from Pascal in `verta1878/mysticbbsirc`.
Pascal source copied to `1541/wip/` for porting. Pin commit hash when done.

Source commit: (fill when porting)

## Ported files

| pcbirc destination | mysticbbsirc source | author | status |
|---|---|---|---|
| `1541/wip/term/rip.c` | `mystic_rip/v4/rip4api.pas` | kiddo | planned |
| `1541/wip/term/ripdraw.c` | `ripviewer/source/ripdraw.pas` + `mystic_rip/rip_surface.pas` | kiddo | planned |
| `1541/wip/term/ripsound.c` | `mterm/mtsound.pas` | kiddo | planned |
| `1541/wip/fmt/ansi.c` | `ansiedit/m_pdansi.pas` | kiddo | planned |
| `1541/wip/fmt/ansiw.c` | `ansiedit/m_pdansiw.pas` | kiddo | planned |
| `1541/wip/fmt/ascii.c` | `ansiedit/m_pdascii.pas` | kiddo | planned |
| `1541/wip/fmt/avatar.c` | `ansiedit/m_pdavatar.pas` | kiddo | planned |
| `1541/wip/fmt/binary.c` | `ansiedit/m_pdbinary.pas` | kiddo | planned |
| `1541/wip/fmt/bitfont.c` | `ansiedit/m_pdbitfont.pas` | kiddo | planned |
| `1541/wip/fmt/idf.c` | `ansiedit/m_pdidf.pas` | kiddo | planned |
| `1541/wip/fmt/rip.c` | `ansiedit/m_pdrip.pas` | kiddo | planned |
| `1541/wip/fmt/sauce.c` | `ansiedit/m_pdsauce.pas` | kiddo | planned |
| `1541/wip/fmt/tundra.c` | `ansiedit/m_pdtundra.pas` | kiddo | planned |
| `1541/wip/fmt/xbin.c` | `ansiedit/m_pdxbin.pas` | kiddo | planned |
| `1541/wip/mail/qwk.c` | `mterm/OL_QWK.pas` | kiddo | planned |
| `1541/wip/mail/bluewave.c` | `mterm/OL_BlueWave.pas` | kiddo | planned |
| `1541/wip/mail/hudson.c` | `mterm/OL_Hudson.pas` | kiddo | planned |
| `1541/wip/mail/jam.c` | `mterm/OL_JAM.pas` | kiddo | planned |
| `1541/wip/mail/packer.c` | `mterm/OL_Packer.pas` | kiddo | planned |
| `1541/wip/mail/transfer.c` | `mterm/OL_Transfer.pas` | kiddo | planned |
| `1541/wip/mail/editor.c` | `mterm/OL_Editor.pas` | kiddo | planned |
| `1541/wip/mail/filter.c` | `mterm/OL_Filter.pas` | kiddo | planned |
| `1541/wip/comm/serial.c` | `mdl/m_serial.pas` + `mdl/serial.pas` | kiddo | planned |
| `1541/wip/comm/fossil.c` | `mdl/m_fossil.pas` + `mdl/m_fossil_io.pas` | kiddo | planned |
| `1541/wip/xfer/zmodem.c` | `mdl/m_prot_zmodem.pas` | kiddo | planned |
| `1541/wip/xfer/xmodem.c` | `mdl/m_protocol_xmodem.pas` | kiddo | planned |
| `1541/wip/xfer/ymodem.c` | `mdl/m_protocol_ymodem.pas` | kiddo | planned |
| `1541/wip/xfer/kermit.c` | `mdl/m_protocol_kermit.pas` | kiddo | planned |
| `1541/wip/net/tcp.c` | `mdl/m_tcp_client.pas` + `mdl/m_io_sockets.pas` | kiddo | planned |
| `1541/wip/net/ftp.c` | `mdl/m_tcp_client_ftp.pas` | kiddo | planned |
| `1541/wip/net/smtp.c` | `mdl/m_tcp_client_smtp.pas` | kiddo | planned |
| `1541/wip/crypto/crc.c` | `mdl/m_crc.pas` | kiddo | planned |
| `1541/wip/crypto/crypt.c` | `mdl/m_crypt.pas` | kiddo | planned |
| `1541/wip/pcb/dropfile.c` | `ansiedit/pcbdrop.pas` | kiddo | planned |
| `programs/pcbnav/pcbterm/term.c` | `mterm/mterm.pas` | kiddo | planned |
| `programs/pcbnav/pcbterm/trip.c` | `mterm/mtrip.pas` + `mterm/mtripgfx.pas` | kiddo | planned |
| `programs/pcbdraw/pcbdraw.c` | `ansiedit/ansiedit.pas` | kiddo | planned |

## Not ported -- Clark already has it

| mysticbbsirc | Clark equivalent | Why skip |
|---|---|---|
| `mdl/m_pdpcboard.pas` | `Pcb-main/SOURCE/DISPLAY/XLATE.C` (810 lines) | @X colour translator already exists, plus NOXLATE.OBJ stub |
| `ansiedit/m_pdpcboard.pas` | same XLATE.C | duplicate |
| MDL UI units (21 units, 9,312 lines) | `Pcb-libs/SOURCE/SCREEN/` (41 files) + `MISC/` (90 files) | strings, dates, input, output, menus already in Clark's code |

## Not ported -- stays in Mystic

| mysticbbsirc | Reason |
|---|---|
| `ripviewer/` | Mystic's full-stack viewer, no PCBoard equivalent |
| `mystic_rip/v4/img/` (32 units) | image codecs -- port on demand via plugin registration |
| `mystic_rip/v4/wav/` (44 units) | audio -- feeds Phase 12 (pcbwave), sysop/0 owns |
| `mystic_rip/v4/prt/` (6 units) | print drivers -- port on demand |
| `mystic_rip/v4/pasjpeg/` (58 units) | IJG port -- link libjpeg in C instead |

## Provenance audit -- upstream work

~27 files in mysticbbsirc's live tree contain inaccurate derivation claims
(references to RIPaint, RIPtel, RIPterm). Per verta1878, those products were
not used; the work came from published white papers. The audit is re-citation,
not deletion, and belongs upstream in mysticbbsirc before the port so
inaccurate citations are not copied into C.
