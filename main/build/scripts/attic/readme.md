# attic — superseded build scripts

Older scripts kept here for reference. Nothing in this directory is
called by the current build system.

| File | Superseded by | Why |
|------|---------------|-----|
| `BLDMENU.BAT` | `../BUILD.BAT` | Interactive CHOICE-based menu replaced by a non-interactive, target-based dispatcher (needed for headless / scripted runs, and to add per-version + meta targets like `all` / `clean` / `mrproper` / `status`). |

If you're looking at these because a current script broke, the attic
version is preserved verbatim and may help reconstruct intent — but
don't put it back in play without moving it out of `attic/` and
updating `../README.md` to match.
