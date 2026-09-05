# devtools — pristine source archives

This directory holds the **original, pristine upstream archives** the
project was built from: compiler distributions, toolkit source, PPL
development kits, and assorted developer utilities. Nothing here is
edited — these are the untouched originals kept for provenance.

Many of these archives have an **extracted or curated form** elsewhere
in the repo (a working source tree, or a slimmed build-tools ZIP at the
root). That overlap is intentional: devtools/ is the archive of record;
the extracted/curated copies are what you actually build against. The
map below says where each one went.

For compiler-specific detail (the MSC 7.0 DPMI situation, how each
family maps to PCBKBC/PCBKIT/PCBKMS, decompression recipe), see
**COMPILERS.md** in this directory — this README is just the index.

## Archive map — original here → extracted/curated form

### Compilers
| Archive (here) | Extracted / curated form | Notes |
|---|---|---|
| `TURBOC201.zip` | root `TC201BT.ZIP` (build subset) | Turbo C 2.01 → PCBKIT |
| `MSC51.zip` | not yet extracted — install v1.11 use | **MS C 5.1** — 14 floppy images (5.25"). Ships **LINK 5.01.21** (Microsoft Segmented-Executable Linker), MS OS/2 Libraries (API.LIB with DOSCALLS/KBDCALLS/VIOCALLS), CodeView for OS/2. Candidate toolchain for install v1.11 (byte-exact rebuild of Clark's INSTALL.EXE which has linker version bytes 5.10 in its NE header). |
| `MSC60A.zip` | not yet extracted — install v1.11 use | **MS C 6.0a** — 6 floppy images (5.25" HD). Contains newer LINK.EX$ (SZ-compressed, decompresses during Setup). Also targets DOS + OS/2. Backup candidate if MSC 5.1's LINK version doesn't match Clark's 5.10 exactly. |
| `OS2SDK103.zip` | not yet extracted — install v1.11 use | **MS OS/2 SDK 1.03** — 11 floppy images (3.5"). Contains same LINK.EXE as MSC 5.1 (identical md5), plus PM SDK, Petzold sample code, toolkit binaries. Provides OS/2 host environment for running the MSC 5.1 or MSC 6.0a linker. |
| `MSC70-retail.7z` | root `MSC70BT.ZIP` (packaged) | MS C 7.0 retail disks → PCBKMS |
| `C7OS2.zip` | folded into `MSC70BT.ZIP` | C7 OS/2 hosted add-on (the DPMI unlock) |
| `MSC70-patches.7z` | applied during build | LINK/LIB/PWB/CV fixes |

(Borland C++ 3.1 lives at root `PCB153BT.ZIP` / inside `DOSBOXX.ZIP`;
its raw distro is not in devtools.)

### DPMI host (for the MSC 7.0 DOS route)
| Archive (here) | What it is |
|---|---|
| `386MAX-803.7z` | Qualitas 386MAX 8.03 (2 floppy images). The 32-bit DPMI host the MSC 7.0 DOS compiler ("3216" passes) requires. Build-time tool for PCBKMS Route A only. Proprietary (abandonware). NOT a 1541 dependency - see todo/SDK-1541-OPENSOURCE-MIGRATION.md. |
| `386max.7z` | 386MAX source code from https://github.com/sudleyplace/386MAX (GPLv3). Open-source path for PCBKMS Route A. |
| `d32a.7z` | DOS/32A Advanced DOS Extender v9.1.2 source + prebuilt binaries (github.com/amindlost/dos32a, Adapted Apache 1.1 license). The DPMI host the crew uses for forward Watcom-based work (delta154, irc1541). Drop-in replacement for DOS/4GW. Runtime binaries staged into `PCBBLDBT.IMG` at `C:\D32A\` for immediate use; compile-from-source is a roadmap task (needs TASM 5.0 + Watcom C 11.0). |

### Toolkit source
| Archive (here) | Extracted / curated form | Notes |
|---|---|---|
| `Toolkit3.zip`, `toolkit3a.ZIP`, `toolkit3b.ZIP` | `toolkit/pwa153` (+ pwa154/delta154/irc1541) | PWA toolkit source, already extracted into the working trees |
| `TOOLKIT2.ZIP` | provenance only | earlier toolkit revision |

The `toolkit/` working trees are what the build uses; these ZIPs are the
pristine originals they came from.

### PPL (PowerBoard Programming Language) kits
| Archive (here) | What it is |
|---|---|
| `ppldevkit.zip` | PPL development kit installer (INSTALL.EXE + PPLDISK.EXE) — the software (distinct from `docs/ppldevelopmentkit.pdf`, which is the manual) |
| `pplx20.zip` | PPLX 2.0 |
| `ppld32.zip` | PPLD (32-bit decompiler tooling) |

### Developer docs / utilities
| Archive (here) | What it is |
|---|---|
| `Develop.zip` | PCBoard developer docs (STRUCTS/PCBDAT/USERS/USERSYS.DOC) |
| `Md5.zip` | MD5 reference (RFC1321 + ASM/OBJ) |
| `Ripkt120.zip` | RIPterm 1.20 kit (install/history docs) |
| `BUILD.BAT` | a stock country/dos/misc build driver (reference) |

## Rule of thumb

- Need to **build**? Use the curated form (root `*BT.ZIP`, `DOSBOXX.ZIP`,
  or the extracted `toolkit/` trees).
- Need the **pristine original** (provenance, re-extraction, verifying
  what shipped)? It's here.

Nothing in devtools/ should be edited. If a curated form needs changing,
change the copy, not the archive of record.
