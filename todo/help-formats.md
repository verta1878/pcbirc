# PCBMAIL.HLP — getting the spec out

`PCBMAIL.HLP` is 759 K and is the most complete surviving description of
PCBMAIL's behaviour. It is a compiled WinHelp file, not text.

## Format

WinHelp `.HLP`, magic `3F 5F 03 00`. RTF-based, compiled with HC30/HC31.
Abandoned after Windows XP; Vista onward needs a separate download to view,
and modern Windows will not open it at all without `winhlp32.exe` from
Microsoft's legacy package.

## Extraction — helpdeco

`helpdeco` decompiles `.HLP` back to its sources: `.RTF` (all topic text and
footnotes), `.HPJ` (project file), `.PH` (phrases), plus `.BMP`/`.WMF`/`.SHG`
for every embedded image. Written in ANSI C, freeware, source included.

- GitHub: `pmachapman/helpdeco` — maintained fork
- SourceForge: `helpdeco` — original
- The distribution includes `helpfile.txt`, a description of the HLP/MVB/ANN/
  MRB/SHG formats derived from analysis. That document is itself worth
  keeping.

Useful invocations:

| Command | Result |
|---|---|
| `helpdeco PCBMAIL.HLP` | full decompile: HPJ + RTF + images |
| `helpdeco PCBMAIL.HLP -r` | RTF that renders in Word as WinHelp showed it |
| `helpdeco PCBMAIL.HLP -n` | as above, no page breaks between topics |
| `helpdeco PCBMAIL.HLP -d` | list the internal directory |
| `helpdeco PCBMAIL.HLP -c` | generate a `.CNT` contents file |

The embedded bitmaps matter as much as the text: they are screenshots of
PCBMAIL's actual dialogs — font selection, address entry, the mailing-list
picker. Those are the UI spec for pcbnav, more precise than any prose
description we could reconstruct.

## Both formats

We hold the original `.HLP` and a newer conversion. Use both:

- **`.HLP` via helpdeco** — authoritative. Gives topic structure, hypertext
  links, and every original bitmap at original resolution.
- **Newer format** — convenient for reading and searching, but a conversion,
  so treat any disagreement as the `.HLP` being right.

Cross-check them. Conversions routinely drop images, flatten hotspots, and
lose the topic graph; if the newer file is missing something the `.HLP` has,
that is a conversion artifact rather than a source difference.

## Also applies to

`PCBSM.HLP` (278 K) and `PCBGUIDE.HLP` (88 K) are the same format and yield
to the same treatment. `QFCONFIG.HLP` is *not* WinHelp — it is QFront's own
TUI help format and needs separate handling.
