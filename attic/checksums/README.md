# Retired per-directory checksum files

Retired 2026-09-22 (v0.3.4). The repo now has **one** manifest:
`CHECKSUMS.sha256` and `CHECKSUMS.md5` at the root, repo-relative,
verifiable with `sha256sum -c CHECKSUMS.sha256` from the root. These six
are the scattered copies it replaces, kept verbatim as the record.

| Kept here | Was at | Lines |
|---|---|---:|
| `OUT-pwa153.sha256` | `OUT/pwa153/CHECKSUMS.sha256` | 10 |
| `OUT-pwa153-SDK-BC31.sha256` | `OUT/pwa153/SDK/BC31/CHECKSUMS.sha256` | 12 |
| `OUT-delta154.sha256` | `OUT/delta154/CHECKSUMS.sha256` | 15 |
| `OUT-clark-original.sha256` | `OUT/clark-original/CHECKSUMS.sha256` | 20 |
| `toolkit-pwa153-bc31-lib.sha256` | `toolkit/pwa153/bc31/lib/CHECKSUMS.sha256` | 13 |
| `toolkit-pwa153-bc31-lib.md5` | `toolkit/pwa153/bc31/lib/CHECKSUMS.md5` | 13 |

## Why they went

Not tidiness. Scattering the manifest let two of them rot unnoticed:

* **`OUT/pwa153/CHECKSUMS.sha256`** listed `bin/PCBOARDM.EXE` and
  `lib/*.LIB`. There is no `bin/` or `lib/` under `OUT/pwa153` — the
  EXEs are at the top level and the libraries are in `SDK/BC31/LIB/`. So
  `sha256sum -c` could never have passed there, and nobody noticed,
  because nobody ran it. It also covered 1 of the 7 EXEs in that folder.
* **`toolkit/pwa153/bc31/lib/CHECKSUMS.sha256`** had 13 entries for a
  folder that now holds 4 libraries. The nine category `.LIB` it also
  listed were moved to `attic/lib/pwa153/bc31/` when the rebuilds
  superseded them; the list was never updated.

The hashes in both were correct. It was the **paths** that were wrong,
and a per-directory file is exactly where that goes unnoticed — there is
no single command that checks them all, so a broken one is silent.

`OUT/clark-original/CHECKSUMS.sha256` and `OUT/delta154/CHECKSUMS.sha256`
were accurate; they are retired for the same reason, not as a fault.

## The one rule

**Do not create another per-directory checksum file.** A new artifact
gets a line in the root manifest. One file means one command, and a
command someone can actually run is the only kind that catches drift.
