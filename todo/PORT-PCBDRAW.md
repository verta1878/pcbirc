# PCBDraw + mterm — port to C

Package: `hexadecimal-pcbdraw-mterm-20260819.zip`, from the crew, 2026-08-19.

**Decision: port the Pascal to C.** `mterm` becomes `pcbterm` and ships as
part of pcbnav. That removes the parallel-tree problem an earlier draft of
this document proposed, and lands everything in the toolkit where door authors
can reach it.

## What this simplifies

| Earlier concern | Now |
|---|---|
| Pascal and C objects do not link | one language |
| Parallel `pcbpas/` tree | not needed |
| RIP unreachable from C doors | it *is* C |
| Two drop-file implementations that could drift | one |
| `rip/` and `draw/` flat-only | 16-bit achievable, see below |

MDL largely dissolves rather than porting. It is Mystic's UI library — 21
units, 9,312 lines of strings, datetime, input, output, menu. **The Clark
toolkit already has all of that**: `SCREEN/` (41 files), `MISC/` (90 files
covering strings, dates, sorting, wildcards), and the `inputfield...()`
family. Porting MDL would mean carrying a second implementation of code we
already own.

## 16-bit is achievable after all

The Pascal canvas is byte-per-pixel:

```pascal
FPixels: array[0..349, 0..639] of Byte      { 224,000 bytes }
TVisArr = array[0..639, 0..349] of Boolean  { another 224,000 }
```

3.4x over the 64K limit. Two changes in the C port fix that, and both are
improvements regardless of memory model:

**Row-pointer array.** `unsigned char *rows[350]`, each row 640 bytes. No
single allocation exceeds 64K and all pixel arithmetic stays within a row.
Mechanical change from the 2D array.

**Bitmask for the flood-fill visited buffer.** 640 x 350 bits = 28,000 bytes,
comfortably one segment. Down from 224,000. Faster too — better cache
behaviour and word-at-a-time clearing.

So `rip/` and `draw/` join the other seven modules as 16-bit clean. The
earlier "flat-only first" carve-out is withdrawn.

## Where the code lands

```
1541/wip/
  term/     surface/canvas + terminal bridge  from rip_surface, rip_term,
                                              rip_window, mtripgfx
    rip1/   RIPscrip 1.54 — 53 commands       16-bit clean
    rip2/   +256-colour, printers             16-bit clean
    rip3/   +RGB24/32                         flat only
    rip4/   +native printers                  flat only
    ripimg/ image decoders, one stub each     flat only, optional
  draw/     editor canvas, tools, blocks      from ansiedit
  fmt/      ANSI, ASCII, Avatar, Binary,      from m_pd*
            IDF, PCBoard @X, RIP, SAUCE,
            Tundra, XBIN, bitfont
  pcb/      drop file read/write              pcbdrop merges with USERSYS.C

programs/
  pcbnav/
    pcbterm/    the terminal — was mterm
  pcbdraw/      the editor — was ansiedit

assets/
  rip-fonts/    10 CHR stroked fonts + RIPTERM.FNT
  rip-icons/    184 ICN icons

docs/
  ripscrip/     23 spec files, v1.54 to v3.2, incl. riplib source
  testing/      RIP corpus, 100-125 files from 16colo.rs
```

`pcbterm` inside pcbnav is the right home: pcbnav is already the client —
connection layer, address book, message reader, offline mail. A terminal is
what it was missing.

`pcbdraw` stays a separate program. It is an editor, not a client, and a
sysop wants it standalone.

## Dependency analysis — what can move first

kiddo built these to be liftable:

| Unit | Lines | Depends on | Port order |
|---|---:|---|---|
| `mtripgfx.pas` — RIP canvas | 859 | `SysUtils, Classes, Math` only | 1st |
| `mtrip.pas` — RIP engine, 53 commands, no stubs | 946 | `mtripgfx`, `mtsound` | 2nd |
| `mtsound.pas` — SDL_mixer audio | 108 | stdlib | with 2nd |
| `pcbdrop.pas` — drop file reader | 309 | `SysUtils` only | anytime |
| `m_pd*.pas` — format handlers | 4,283 | mixed | 3rd |
| `mterm.pas` + `mtphone.pas` — UI | ~1,170 | MDL | 4th, → pcbterm |
| `ansiedit` — editor | ~10,000 | MDL heavily | last |

No MDL coupling in the RIP path at all. That is what makes the first three
steps cheap.

**Rough total for the terminal path: ~7,700 lines, plus ~10,000 for the
editor.** MDL's 9,312 lines mostly do not port — they map onto existing
toolkit functions.

That covers mterm's compact engine only. The full `mystic_rip/` v1-v4 suite is
a further ~34,000 lines and is tiered rather than ported wholesale — see
below.

---

# The RIP suite — four reconstructed specs, not four tiers

**Correction.** An earlier version of this section proposed collapsing v1-v4
into feature tiers and described v4 as "v3 plus native printers". Both were
wrong, and the second was read off a one-line summary table without opening
the directory.

## Why there are four engines

The RIPscrip API was lost. These are reconstructions.

| Engine | Lines | Commands | Reconstructed from |
|---|---:|---:|---|
| v1 `ripscr.pas` | 4,186 | 51/51 v1.54 | published v1.54 specification |
| v2 `rip2api.pas` | 5,394 | 67 | published white papers |
| v3 `rip3api.pas` | 8,371 | — | published white papers |
| v4 `rip4api.pas` | 8,646 | — | v3 base plus v4 extensions |

**Provenance corrected.** Earlier drafts of this document repeated claims from
the engines' own READMEs that v2 was "decoded from RIPaint 2.1 scene files"
and v3 "confirmed via RIPtel Visual Telnet 3.1". Per verta1878 none of those
products were used. The work was done from published white papers plus a small
number of files, and the sources consulted are already recorded. Those README
claims are inaccurate and are being audited out — see "Provenance audit" below.

That is the same category of work as Phase 27's binary analysis, applied to a
graphics protocol instead of an executable. Each engine records what was
recovered and from which source. **Collapsing them into tiers would destroy
that provenance** — the answer to "which spec level does this behaviour belong
to, and how do we know" lives in the separation.

Keep four engines. They stay four engines.

## What v4 actually contains

Not "native printers". From `RIPSCRIPT_V4_ROADMAP.md`, all marked wired:

- Unicode CP437 to UTF-8 (`cp437u8`, `u8render`, `rip4uni`)
- TTF font loading (`ttfglyph`)
- Full-motion video — MPEG demux, video decode, buffering, playback, streaming
- FLI/FLC animation (320x200 FLI plus arbitrary FLC)
- MIDI synthesis — FM, 32 voices, plus player and stream
- HTML 1.0 — parser (44 tags), DOM tree, box-model layout, direct renderer,
  and an HTML-to-RIP command translator
- Print API with six drivers: BMP, ESC/P, PCL, PostScript, raw
- 83 codec units renamed for DOS 8.3 compliance, zero violations

## Real scale

| Area | Lines | Units |
|---|---:|---:|
| v1-v4 engines | ~27,500 | 4 |
| `img/` — codecs, HTML engine, gfx primitives, Unicode, TTF | 8,625 | 32 |
| `wav/` — audio | 13,597 | 44 |
| `pasjpeg/` | 33,843 | 58 |
| `prg/` — scene utilities | 1,948 | 6 |
| `prt/` — print drivers | 882 | 6 |
| Support units, standalone tools | ~3,250 | |
| **Total** | **~89,600** | |

An earlier figure here said 34,000. That counted engines only.

## Organise by what things are, not where they landed

Several subsystems sit under `mystic_rip/v4/` but are not RIP.

### `wav/` is a general audio library — and it answers Phase 12

13,597 lines across 44 units: MP3 with full huffman/IMDCT/requantise/synthesis,
MIDI with 32-voice FM synthesis plus player and streaming, MOD/S3M/XM trackers,
FLAC, ADPCM, AIFF, AU, VOC, WAV, a Sound Blaster IRQ-driven DMA driver with
double-buffered streaming, a 16-stream mixer with saturation clipping, ring
buffer, and network audio.

**Phase 12 (pcbwave) asks for**: WAV playback, MOD/S3M optional, a `PLAYWAVE()`
PPL function. Owner listed as TBD.

That phase is largely already built. It should point at this work and its
owner should be whoever wrote it. What remains is the PCBoard-side wiring —
streaming to the caller, event triggers, the PPL function, multi-node
behaviour — not the codecs.

Destination: `pcbwave` / toolkit `audio/`, not under RIP.

### `pasjpeg/` is third-party derived

33,843 lines, 58 units — a Pascal port of the Independent JPEG Group library.
That is over a third of the whole RIP package by line count and it is not our
code. It needs its own provenance handling, its own licence file, and a
decision about whether the C port uses it at all or links libjpeg.

### The HTML engine is not RIP either

`htmlpars`, `htmltree`, `htmllayo`, `htmlrend`, `htmlrip` — parser, DOM,
layout, renderer, and RIP translator. RIPscrip 4 specified HTML support, so it
belongs to v4, but as a component it is an HTML engine and should be
identifiable as one.

### Graphics primitives are shared

`grbezier`, `grclip`, `grfill`, `grfx`, `grtexmap` are canvas-level and used
across engines. Same role as `rip_surface.pas` (775 lines), which is already
the canvas split cleanly out of the v4 engine, independent of the command
parser. That split is the right shape and survives the port: one surface,
four parsers over it.

## What this means for the port

The port is not one job. It is at least five, with different owners and
different urgency:

| Subsystem | Lines | Note |
|---|---:|---|
| RIP engines v1-v4 | ~27,500 | four reconstructed specs, kept separate |
| Surface and gfx primitives | ~1,800 | shared foundation, port first |
| Audio | 13,597 | separate module, answers Phase 12 |
| Image codecs | ~8,600 | link-out per format |
| PasJPEG | 33,843 | third-party, decide before porting |
| HTML engine | within `img/` | v4 component, identify separately |
| Print drivers | 882 | v4 component |

**Port order stands for the graphics path** — surface first, then v1, verified
against the corpus. But v2/v3/v4 are not "later tiers to do on demand". They
are completed reconstruction work and porting them is a decision about
sequence, not about whether.

The 16-bit question resolves per subsystem rather than globally: v1 and the
surface can be 16-bit clean with row pointers and a bitmask; v3's 24/32-bit
buffers, the image codecs and the audio streaming cannot.

## Attribution

**kiddo built the v1-v4 engines** and `mtrip.pas`, plus the terminal and
editor side — `mterm/`, `ansiedit/`, the MDL units. **sysop/0 built the rest**
of `mystic_rip/`: codecs, HTML, print drivers, audio, utilities.

(Corrected twice. An earlier draft credited the engines to sysop/0; kiddo's
own first reply did the same before verta1878 corrected it.)

## Current state: the two are unconnected

As shipped in this package, dated 2026-08-19:

- **Nothing outside `mystic_rip/` references the v-engines.** No file in
  `mterm/`, `ansiedit/` or `mdl/` uses `rip4api`, `rip3api`, `rip2api` or
  `ripscr`.
- **`mtrip.pas` is standalone.** Header: *"mterm RIP Graphics — RIPscrip
  v1.54 command dispatcher"*, uses only `mtripgfx`, copyright FPC264IRC
  Contributors. A separate v1.54 implementation.
- **`mdl/` contains no RIP.** 22 units: datetime, fileio, input and output
  with five platform variants each, menus, strings, and `m_pdpcboard` for @X.
  A UI and platform library.

If kiddo has since moved to v4 in MDL, that is newer than this package or
lives in the Mystic tree. Worth confirming which before planning around it.

## Can pcbterm use v4? No — and for a better reason than buffers

Two earlier answers here were wrong. The first said v4 was ruled out by its
24/32-bit buffers. The second said it was fine if the buffer dimensions became
a build-time constant. kiddo corrected both, and the code confirms it.

### The dependency chain, not the buffer

`rip4api.pas` line 1124:

```pascal
Uses
  jpgdecr, gifdecr, pngdecr, rffdecr, BMPDec, PCXDec, TGADec, PBMDec,
  ICODec, PNGCodec, pngintl, GIFAnim, gifintl, JPEGProg, spranim,
  GRFill, GRFx, GRBezier, GRTexMap, GRClip, cp437u8, u8render, TTFGlyph,
  ripdecr, ripbind, RIPTile, riplayr, ripchnge, riprndr, midsynth,
  MPGPlay, HTMLPars, HTMLTree, HTMLLayo, HTMLRip, HTMLRend,
  PrnAPI, PrnBMP, PrnEscP, PrnPCL, PrnPS, PrnRaw;
```

A hard dependency on 42 units. This is not a question of whether the linker
strips unreferenced code — it is a question of whether the thing compiles at
all. On i8086, TTF, MPEG, HTML and PasJPEG will not build. Calling only the
v1.54 subset does not help when the unit cannot be compiled for the target.

The buffer constant is still worth fixing, but it was never the binding
constraint.

### The deeper reason: server-side vs client-side

From `rip4api.pas`'s own header:

> *The engine operates server-side: instead of sending raw RIP codes to the
> terminal, Mystic renders them internally and sends the resulting screen
> image as ANSI or bitmap data.*

**v1-v4 are server-side renderers.** They render locally and send the picture
out. **`mtrip.pas` is a client-side parser** — it receives RIP from a remote
system and draws locally. Opposite data flows.

pcbterm is a client. `mtrip.pas` is therefore the architecturally correct
choice, not merely the smaller one. kiddo's "two different implementations for
two different jobs" was precise; an earlier reading of that as a size
trade-off was wrong.

### The DOS ceiling is v2

kiddo's assessment, and the `Uses` clause supports it:

| Engine | Lines | DOS viable? |
|---|---:|---|
| v1 | 4,186 | yes, fits easily |
| v2 | 5,394 | yes, still manageable — adds 256-colour and WAV |
| v3 | 8,371 | heavy — tables, forms, conditional logic |
| v4 | 8,646 + ~58,000 in dependencies | no |

### Agreed: two engines, one server, one client

Confirmed by verta1878 — DOS i8086 is a live target, and it is **real-mode**.

`m_rip_dos.pas` was killed. Plugin codec registration means the DOS engine is
the *same* `m_rip.pas` with no codecs linked — a link-time decision, not a
source fork.

| Engine | Built from | Role | Architecture |
|---|---|---|---|
| `m_rip.pas` | v4 | server-side: render RIP, produce pixel buffer | plugin codecs via registration |
| `mtrip.pas` | exists, 946 lines | client-side: receive RIP, draw locally | lightweight |

| Build | Engine | Codecs linked |
|---|---|---|
| DOS i8086 mterm | `mtrip.pas` | none |
| DOS i8086 server | `m_rip.pas` | none, v2 command set |
| Modern mterm | `mtrip.pas` | none |
| ansiedit | `m_rip.pas` | PCX, BMP |
| ripviewer / MIS | `m_rip.pas` | all |

v1 and v3 retire to `attic/` — v1 is a subset of v2, v3 of v4.

### Two gaps registration does not close

Codecs plug out cleanly because they are separate units. Two things are not.

**1. v3/v4 command handlers live inside `m_rip.pas` itself.** rip4api is 8,646
lines against rip2api's 5,394 — roughly 3,250 lines of v3/v4 handler code that
a DOS build would compile in and never call. Handlers that merely call a codec
degrade gracefully when the slot is nil, but the code is still bytes in a
640K real-mode space where the pixel buffer alone takes 224K.

`{$IFDEF}` would work. Better: **make the command dispatch table a registration
point too**, exactly like the codecs. Handlers register into the table, so a
DOS build links only the v1.54/v2 handler set. That makes it one mechanism
rather than two, and it means the DOS build is described entirely as a link
set — which was the point of killing `m_rip_dos.pas`.

**2. `SavedScreens` is not mentioned in the migration plan.**

```pascal
SavedScreens : Array[0..9] of PRIPPixelBuffer;   { rip4api line 593 }
```

Ten slots at 640x350 indexed is 2,240,000 bytes. They are pointers so they
allocate on demand, but a scene using several save slots will exhaust
conventional memory on DOS. Row pointers solve the per-allocation 64K limit,
not the total.

Options in order:

1. **Disk-backed slots** — Clark's `MISC/VIRTUAL.C` is precisely this: a block
   cache over a disk file addressed through `huge *`. Working precedent exists.
2. **EMS/XMS for slot storage** — era-standard, keeps conventional memory free.
3. **Fewer slots on DOS** — if real scenes use two or three, ten is
   speculative.

Worth settling inside RIP-MIG-1, because it changes the buffer type.

### Carry the provenance markers into the merged engines

Retiring v1 and v3 is right functionally — nothing is lost, and `attic/`
preserves the originals. But it does put a preservation obligation on the
merge.

The sources are published white papers, all present in the package:

| Document | Covers |
|---|---|
| `docs/ripscrip/historical/RIPSCRIP_v154.DOC` | v1.54 |
| `docs/ripscrip/historical/RIPScrip-2.0-alpha-4.txt` and `RIPSCRIP_v2A4.PRN` | v2.0 |
| `docs/ripscrip/historical/RIPScrip-3.x-technical-whitepaper.txt` | v3.x |
| `mystic_rip/ripscrip-irc-whitepaper.htm` | general |
| `mystic_rip/v3/ripscrip-v3-implementation-whitepaper.htm` | v3 implementation |

Once v2 and v4 merge into one engine, someone reading the result cannot tell
which behaviours came from which specification level. Cheap now, impossible
later: cite the document and section on each handler as the merge happens.

```pascal
// v2.0 — RIPScrip 2.0 alpha 4 whitepaper, section 3.2
```

A document citation is stronger than an inference record: it is verifiable
against a file in the repo.

## Provenance audit — upstream work, in mysticbbsirc

The engines' own READMEs contain derivation claims that are inaccurate. Per
verta1878, RIPaint, RIPtel and RIPterm were not used; the work came from the
white papers above.

**This audit belongs upstream in `verta1878/mysticbbsirc`, not in pcbirc** —
pcbirc does not carry the Pascal. It is recorded here because pcbirc's C port
will inherit whatever provenance comments the Pascal carries at port time, so
it is worth the audit landing before the port rather than after.

242 files in the upstream live tree mention those product names, in three
groups, and only one is the target.

**A. Source documents — do not touch (17 files).** The white papers and specs
themselves. TeleGrafix wrote them; their name belongs in them. Stripping it
would falsify the source material, which is the opposite of good provenance.

**B. Third-party art — do not touch (13 files).** `FRACTMTN.RIP`,
`STEREO1.RIP`, `DON.RIP`, font `.inc` files. Not ours to edit.

**C. Our derivation claims — the audit (~27 files).**

| File | Claim |
|---|---|
| `mystic_rip/v2/README.md` | "decoded from RIPaint 2.1 scene files" |
| `mystic_rip/v3/README.md` | "confirmed via RIPtel Visual Telnet v3.1" |
| `RIPAINT_FINDINGS.md` (v2, v3, v4) | filename is itself a claim |
| `RIPTEL_PROTOCOL_ANALYSIS.md` (v3, v4) | filename is itself a claim |
| `docs/ripscrip/v300-research/riptel-*.md` (4 files) | filenames |
| README / features.txt / AUDIT.md across v2-v4, ripviewer, docs/ripscrip | scattered mentions |

Five have a product name in the filename, so renaming is part of the work —
`RIPAINT_FINDINGS.md` becomes a format-notes file citing the 2.0 alpha 4
white paper by section, the `riptel-*` research notes cite the 3.x technical
white paper.

The audit is **re-citation, not deletion**. The same findings stay; the source
attributed changes from a product to a document.

## Real-mode DOS: the memory budget

`m_rip.pas` on DOS is **server-side** — it renders inside the BBS process, so
it shares 640K conventional memory with DOS itself, the BBS, the comm driver
and any TSRs. The 640x350 indexed buffer is 224,000 bytes on its own: a third
of the space.

The migration plan's row-pointer layout is right and handles the per-allocation
64K limit:

```pascal
TPixelRow = Array[0..RIP_MAX_X] of Byte;   { 640 bytes }
TVisitRow = Array[0..79] of Byte;          { 640 bits = 80 bytes }
```

28K for the visited buffer, down from 224K. That part is settled.

## Command counts reconciled

Resolved, and it is not a spec disagreement:

| | Level 0 | Level 1 | Total |
|---|---:|---:|---:|
| `mtrip.pas` | 37 | 16 | 53 |
| `v1/ripscr.pas` | 36 | 15 | 51 |

kiddo counted `>` (EraseEOL) as a command distinct from `K`, and counted `1W`
(WriteIcon) in Level 1 dispatch where `ripscr.pas` had it as an uncounted
no-op. Same specification, different treatment of aliases and no-ops.

An earlier note here suggested the difference was "information about the spec".
It is information about *counting*. Still worth settling before the port so
both implementations report the same figure, but no behaviour is in question.

## The two v1.54 implementations, per kiddo

Neither is a subset of the other.

`ripscr.pas` has, and `mtrip.pas` does not: ten screen save/restore slots, 43
text variables (against 32), ICN/MSK/HIC icon formats (against ICN write
only), PCX/BMP loading, a full button renderer with 14 parameters.

`mtrip.pas` has, and `ripscr.pas` did not until patched: SDL_mixer audio via
`mtsound.pas`, `ProcessFile` for recursive scene load, file query with the
`FILEERR` variable, EGA64-to-RGB palette conversion at set time.

## pcbterm needs a GUI — that is a backend, not an engine

mterm today renders RIP to *text*. `RenderToText` samples the 640x350 buffer
into 80x22 cells using half-block characters (`0xDF`), two colours per cell.
Clever, and it works in a terminal, but lossy.

A GUI means real pixel output. That is a new output backend, not a new engine:
the engine already produces a correct pixel buffer — `SaveBMP` proves it —
and `RenderToText` is merely one consumer of it.

| Target | Backend |
|---|---|
| DOS | VGA mode 12h (640x480x16) or EGA mode 10h (640x350x16), planar write to A000:0000 |
| Windows | framebuffer blit — GDI or SDL |
| Linux | SDL or X11 |

SDL is already a dependency: `mtsound.pas` uses SDL_mixer. Using it for video
as well is consistent rather than additional weight.

Keep `RenderToText` regardless. A text-mode fallback is genuinely useful over
a plain telnet session where no graphics surface exists.

## Testing comes before porting — the engines are untested

**Correction.** An earlier version of this section said: render the corpus
with the Pascal build, render it with the C port, compare. That assumed the
Pascal output is correct. Per verta1878, **nothing in v1-v4 is tested yet**.

Diffing against untested output enshrines its bugs as the specification, and
afterwards a port bug and an original bug are indistinguishable — both present
as "C differs" or, worse, as neither differing.

Current state in the package: 441 RIP files, 101 in the main corpus, and
exactly two BMPs (`ROCK100.BMP`, duplicated). The harnesses exist —
`mtrip_test`, `test_rip_files.pas` — they have not been run as an acceptance
suite. **No baseline exists.**

### For reconstructed code, the corpus is not verification — it is the spec

v2 was reconstructed by observing RIPaint 2.1 scene files. Those files are the
evidence the specification was derived from. Running them is not QA after the
fact; it is the final step of the reconstruction. Until they render correctly,
the reconstruction is a hypothesis.

Same for v3, confirmed against RIPtel Visual Telnet 3.1.

### Sequence

1. Run the corpus through the Pascal engines now, while the Pascal builds
2. Human review — RIP faults are visually loud: misplaced fills, wrong
   palette, missing text, bad clipping
3. Fix in Pascal, re-run, until output is judged correct
4. That becomes the baseline
5. C port target: byte-identical to the reviewed baseline, so any difference
   is unambiguously a port bug

**Do not port a known bug intending to fix it in C.** That splits the fix
across two codebases and leaves the Pascal wrong permanently.

### Priority, since 441 files is a lot of eyeballing

| Order | What | Why |
|---|---|---|
| 1 | v1.54 corpus | what real BBS files are, and the pcbterm path |
| 2 | v2 against RIPaint 2.1 scene files | validates the reconstruction guesses specifically |
| 3 | v3 against RIPtel 3.1 material | same, one level up |
| 4 | v4 | modern-platform only, lowest risk to defer |

The 23 RIPscrip specs (v1.54 through v3.2, including riplib source) are the
written reference behind the review.

### Scale note

~86,000 lines untested is a large surface. The right response is not alarm —
it is that testing is now the highest-value next step, ahead of porting.
Porting untested code doubles the quantity of untested code and makes the
eventual debugging harder, not easier.

## Drop files — now one implementation

`pcbdrop.pas` (309 lines, `SysUtils` only) reads PCBOARD.SYS, DOOR.SYS and
DORINFO1.DEF. In C it merges with the toolkit's existing drop-file code rather
than sitting beside it as a second implementation. One reader, checked against
`PCBXDOT/DOCDEV/` — PCBSYS.TXT, USERSYS.TXT, USERS.TXT.

`bbs_doors.pas` (374 lines) is Mystic's drop file *writer* — the counterpart,
and the reference for PCBD-2.

## Sequencing

Port before finishing the nine open PCBD phases. Work done in Pascal now gets
done twice.

kiddo's README already anticipates consolidation: *"Drop file code in
bbs_doors.pas for now. Future: mdl/m_door.pas."*

Full order is under "Revised port order" above. PCBD-1 through PCBD-9 run
against the C tree once steps 1-7 land.

## What the crew already finished

Worth recording so it is not re-litigated during the port:

- All 53 RIPscrip v1.54 commands, **zero stubs**
- 640x350 EGA with live palette and EGA64 conversion, line patterns
- Text variables, recursive scene load, ICN writer, sound
- ANSI engine: CSI, SGR, scroll region, device status
- Phonebook, status bar, virtual pages, RIP auto-sense
- Editor: canvas, line draw, blocks, undo, ICE, SAUCE
- Teleconference: virtual pages, chat, PabloDraw protocol
- All 13 security findings fixed
