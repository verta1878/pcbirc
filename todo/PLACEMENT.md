# Where the PCBDraw / mterm work lands

Package: `hexadecimal-pcbdraw-mterm-20260819.zip`, 2,664 files, 15 MB.

## pcbirc does not carry the Pascal as-is

The Pascal lives in **`verta1878/mysticbbsirc`**, maintained by kiddo. pcbirc
holds a **work-in-progress copy** of the Pascal source needed for porting,
under `1541/wip/`, alongside the C ports as they are written.

The toolkit (`toolkit-15.4/`, `toolkit-15.41/`) is Clark's code. Kiddo's code
is not part of the toolkit — it is what we are porting into 15.41.

## Layout

```
pcbirc/
  1541/
    wip/                      kiddo's Pascal, copied for porting
      mail/                   OpenOLMS — OL_*.pas (15 units)
      fmt/                    format handlers — m_pd*.pas (14 units)
      term/                   RIP engine — rip4api.pas, ripdraw.pas,
                              rip_surface.pas, mtsound.pas
      comm/                   serial — m_serial.pas, serial.pas,
                              m_fossil.pas, m_fossil_io.pas
      xfer/                   protocols — m_prot_zmodem.pas,
                              m_protocol_xmodem.pas, _ymodem, _kermit
      net/                    TCP — m_tcp_client.pas, m_io_sockets.pas,
                              m_tcp_client_ftp.pas, _smtp.pas
      crypto/                 CRC, crypt — m_crc.pas, m_crypt.pas
      pcb/                    drop files — pcbdrop.pas
      pcbterm/                terminal — mterm.pas, mtrip.pas, mtripgfx.pas
      pcbdraw/                editor — ansiedit.pas and related units

  programs/
    pcbdraw/                  C port of ansiedit — the editor
    pcbnav/
      pcbterm/                C port of mterm + mtrip — client terminal

  assets/
    rip-fonts/                10 CHR stroked fonts + RIPTERM.FNT
    rip-icons/                184 ICN icons

  tests/
    rip-corpus/               acceptance criteria for the C port
```

Each `wip/` subdirectory holds the Pascal source being ported and the C port
growing beside it. When a port is complete and tested, the C file moves to its
final home in the 15.41 tree and the Pascal stays as reference.

## PCBoard-specific pieces

Four units in the package are ours rather than Mystic's:

| Unit | Lines | Destination |
|---|---:|---|
| `ansiedit/pcbdrop.pas` | 309 | `1541/wip/pcb/` — merges with the C drop-file code |
| `mterm/OL_DropFile.pas` | 275 | `1541/wip/mail/` |

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

## OpenOLMS

`mterm/OL_*.pas`, 15 units, 4,098 lines. **GPLv3**, permission from Peter
Rocca, settled.

Goes to `1541/wip/mail/`. This is Phase O1's pcbolms and Phase 24's
offline-mail UI, already written.

**Untested — but the easiest thing here to test, so it goes first.** QWK is a
data format with objective pass/fail:

| Test | Oracle |
|---|---|
| Round-trip | pack, unpack, compare — byte comparison |
| Generate with PCBoard, read with OpenOLMS | Clark's QWK code is the reference |
| Write a `.REP`, import into PCBoard | if PCBoard accepts it, it is correct |
| Interop | OLX, SLMR, Blue Wave under DOSBox |

## Port order

1. **`mail/` — OpenOLMS.** Test first, then port. Smallest, objective
   oracles, feeds two phases, no RIP dependency.
2. `pcb/` — merge `pcbdrop.pas` with the C drop-file code.
3. `fmt/` — format handlers, no engine dependency.
4. **Test the Pascal RIP engines upstream** — run the corpus, review, fix in
   Pascal, then baseline.
5. `term/` — the RIP engine, verified against the reviewed baseline.
6. `pcbterm/` inside pcbnav.
7. `pcbdraw/`.

Steps 1-3 have no dependency on RIP and can proceed while the Mystic-side
RIP-MIG work is still running.

## Credit

- **kiddo** — v1-v4 RIP engines, `mtrip.pas`, mterm, ansiedit, MDL units
- **sysop/0** — codecs, HTML renderer, print drivers, audio, scene utilities
- Upstream: `verta1878/mysticbbsirc`, GPLv3

## Provenance audit -- upstream work

~27 files in mysticbbsirc's live tree contain inaccurate derivation claims
(references to RIPaint, RIPtel, RIPterm). Per verta1878, those products were
not used; the work came from published white papers. The audit is re-citation,
not deletion, and belongs upstream in mysticbbsirc before the port so
inaccurate citations are not copied into C.
