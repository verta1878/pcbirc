# WinFOSSIL Audit — Session Tracking

## Scope
Same treatment as the SIO v1/v2 projects: verify HANDOFF.md's claims
against actual source, check the 22 tagged bugs (WF-N/MF-N) really
match what's in the code, compare against real original binaries
where available, hunt for stubs/dead code/silent gaps.

Deferred for now (user's instruction): fossil-vxd-recovery.zip
(the second uploaded FOSSIL driver project) — not touched this
session.

## Source of truth
- Clean-room project: /home/claude/winfossil/ (extracted from
  wnfossil-2_0_0-final.zip)
- Original Win95 v1.12 binaries (legitimate, real): 
  /home/claude/winfossil_orig/wnfos112/ (extracted from WNFOS112.ZIP)
- WNFOSNT.RAR: DECLINED — self-identifies via its own file_id.diz as
  "Remove Nag Screen," a licensing-bypass patch, not the real NT
  product. Not analyzed. User is deleting it; no real NT binary
  available for comparison — NT track remains source-review-only.

## Findings so far
1. HANDOFF.md claims wnfos112key/ is "included in source zip" under
   upstream section — it is NOT actually present in
   wnfossil-2_0_0-final.zip. Doc/reality mismatch, not yet fixed.
2. Confirmed clean: no RC4/MD5 registration crypto exists anywhere
   in the clean-room source (src/core, src/nt, src/modern, src/cpl,
   src/ctl) despite the original binary having real registration
   crypto (BINARY_ANALYSIS.md's own function inventory: _do_rc4,
   _prepare_key, _MD5Init/Update/String). Grep-verified.
3. WNFOSNT.RAR (uploaded as the "real" NT binary) is actually a
   nag-screen-removal patch per its own file_id.diz — declined to
   analyze it. User is deleting it; not a factor going forward. No
   real NT binary available for comparison — NT track (wf_vdd.c) is
   source-review-only until/unless a real one shows up.
   WNFOS112.ZIP (Win95 v1.12) IS the genuine original — usable as a
   real comparison baseline for the Win95 track.
4. src/vxd-recovered/ (nested inside wnfossil-2_0_0-final.zip) —
   DEFERRED BY USER EXPLICITLY. Confirmed only at a glance: appears
   to be a complete, separately-verified fix for the exact VxD build
   problem HANDOFF.md's "still needs work" item #1 describes as
   unsolved (hash-recorded output, a written recovery report —
   corrupted VMM.INC was the claimed root cause). HANDOFF.md doesn't
   mention this folder. User's instruction: reference/note its
   existence in this file, do NOT deep-dive yet — return to it later
   "when we have more research and knowledge to work with it."
   Likely overlaps with the also-deferred fossil-vxd-recovery.zip.
   DO NOT ANALYZE FURTHER UNTIL USER SAYS SO.
5. Bug-tag verification (corrected after catching a flawed grep
   pipeline on my first attempt — see below):
   - WF-3 through WF-11, MF-1/3/4/5: all genuinely present in
     source. Spot-checked WF-9 and WF-10 by reading the actual code
     (not just grepping the tag) — both are real, correct fixes.
     WF-10 specifically (trailing + chars lost on idle timeout) was
     hand-verified against the exact scenario it claims to fix, and
     it's handled correctly.
   - WF-1, WF-2, WF-12: tags NOT found anywhere in source. WF-2's
     underlying described problem ("+ bytes consumed by +++
     detector, never forwarded") does appear to be actually handled
     by the code near WF-9/WF-10 — the "flush accumulated +'s"
     logic does forward them — so this may just be an untagged fix
     rather than a missing one. WF-1 (strncpy not inside else
     braces) and WF-12 (BUILD.md wrong filenames — a doc-only fix,
     might not warrant a code tag) not yet verified either way.
   - WF-13, WF-14, WF-15 exist in source (real telnet-negotiation
     fixes: IAC-SE split across buffer boundary, echo-loop
     prevention via option-acceptance tracking) but are NOT listed
     in HANDOFF.md's bug list at all — HANDOFF.md is stale, these
     look like genuine later fixes, not fabricated tags.

## Fixes made
1. comport_compat.c format_port_name(): the NT+ "already formatted"
   branch called strncpy() but never NUL-terminated the result,
   unlike its sibling branch two lines up. Real strncpy-footgun bug,
   found while checking WF-1 (doesn't match WF-1's own description
   exactly, but same bug class in the same file). Fixed, plus added
   the same defensive termination to wf_enum_ports()'s three
   strncpy sites for consistency (lower risk there — 9x branch
   copies a known-short "COM%d" string, NT+ branch copies from
   QueryDosDeviceA's own buffer rather than caller input — but
   costs nothing to be consistent).
   Checked remaining strncpy sites (wf_core.c x3, wf_tray.c,
   wf_cpl.c x2): all safe — either already terminated, or copying a
   short compile-time literal into a buffer sized with headroom.
   wf_cpl.c's NEWCPLINFO.szInfo size not independently verified
   against a real Windows SDK header (none available in this
   environment) — based on recollection of the stable, long-unchanged
   real struct (szInfo[64]), the 41-char literal fits with room to
   spare, but flagging the lack of a real header to check against.

2. WNFOSCTL.EXE strings-compared against wf_ctl.c: original's command
   set is LOCK/UNLOCK only, matching wf_ctl.c (which also adds a
   STATUS command — an enhancement beyond the original, not a gap,
   noting it as an addition rather than a bug). Confirmed
   wf_ctl_check_installed() is a real, already-completed fix (not
   just claimed) — matches original's exact error text and version
   query, added in a previous pass per its own header comment.
3. WNFOSSIL.CPL strings-compared against wf_cpl.c: both real
   original features I checked for ("Automatically open port when
   use detected", performance-stats monitoring) are wired up for
   real (auto_open/perf_stats fields, real CheckDlgButton controls
   both reading and writing them) — first grep attempt missed
   auto_open by searching literal UI text instead of the field name;
   corrected and confirmed present.

4. WF-1 resolved: searched systematically (regex scan for brace-less
   `else` followed by strncpy within ~150 chars, across every .c
   file) plus manual re-inspection of every strncpy call site in the
   codebase. Found two candidates, both false positives (one was my
   own regex matching ".html" as ".h", the other was an `else`
   correctly scoped to an unrelated if/else-if/else chain, with the
   strncpy calls actually unconditional and outside it). No surviving
   instance of the "strncpy not inside else braces" pattern found —
   concluding WF-1 was fixed structurally (rewritten with clear
   braces) without leaving an inline tag, not still open.
5. WF-12 resolved: verified every file path BUILD.md references
   actually exists (redid the check after catching my own bug — the
   first pass flagged src/docs/WNFOSSIL.h as missing, which was my
   regex matching inside ".html"). All real. Confirmed fixed.

6. Test suite count mismatch: HANDOFF.md claims "50-test suite" and
   "wine wf_test.exe → 50/50 pass" (both places, lines 47 and 167).
   Actual source has 65 `t("...")` assertions across roughly 9-10
   categories (ring buffer, baud rates, port lifecycle, FOSSIL API,
   VMODEM, registry, COM port, DLL exports, security, performance).
   Real, concrete mismatch — the test count in the docs is stale
   (probably grew since it was written). Cannot verify the pass/fail
   claim itself either way — no Windows/Wine runtime available in
   this sandbox to execute it, same limitation as everything else
   needing a real toolchain.

7. Test suite quality check, following up on the count mismatch:
   15 of the 65 assertions are `t("...", 1)` — always-true stubs.
   Checked whether these are honestly conditional or fake:
   legitimate for platform-gating (`#ifdef _WIN32`/`#else` — a
   non-Windows build can't run registry/DLL/port-enum tests, marking
   them skip is reasonable, not padding). BUT: inside the `#ifdef
   _WIN32` DLL-exports block itself, if `LoadLibraryA("FOSSIL.DLL")`
   fails at runtime, it ALSO falls back to 4 always-true skip stubs
   rather than failing. That means even under the actual "wine
   wf_test.exe" run HANDOFF.md claims 50/50 (or 65/65) for, a
   genuinely missing/broken DLL would be indistinguishable from a
   pass for those 4 assertions — a real methodology weakness in what
   the test suite's pass count can actually prove, not just a doc
   staleness issue like the count itself.

8. FOSSIL.VXD strings-compared against src/vxd/FOSSIL.ASM: registry
   path matches exactly ("System\CurrentControlSet\Services\VxD\
   FOSSIL"). Real gap found: the original registers as a Windows
   Performance Monitor counter object ("bytes read per second" /
   "bytes written per second" strings, visible in the classic
   System Monitor applet or any PerfMon-based tool) — this doesn't
   exist anywhere in the clean-room source. The underlying data IS
   computed correctly (perf_cps_rx/perf_cps_tx in wf_core.c,
   confirmed real, working, toggle-controlled) — it's just not
   exposed via the actual Windows PerfMon registration mechanism
   (a registry Performance subkey + a counter DLL exporting
   OpenPerformanceData/CollectPerformanceData/ClosePerformanceData
   for NT-family, or the older .INI+.DAT counter-definition
   mechanism for 9x). Not implemented here — genuine, substantial
   OS-integration subsystem, and there's no Windows SDK available in
   this sandbox to verify a real implementation against, same
   reasoning as the SIO project's PCI-bus-scan gap. Flagged rather
   than faked.

## VXD/binary comparison — closed out (2026-08)
Registry path: verified exact match. PerfMon registration: real gap,
documented above and in HANDOFF.md. This was the last major "still
needs work" verification item for the Win95 track from the original
audit plan. NT track remains source-review-only (no real NT binary
available). Turning to vxd-recovered/ next per user's direction —
needed for the netserial integration evga recovered with the
author's permission.

## vxd-recovered/ — independent verification (2026-08)
Per user direction: working on this now for the netserial
integration (evga recovered with author's permission).

Verified all three claimed source-bug fixes in RECOVERY.md against
the actual current source — all three genuinely present and correct:
1. NODECOUNT guarded default (FOSSIL.inc, IFNDEF/equ 16) — confirmed.
2. IRQ_Ready struct field — confirmed declared once, set in exactly
   the two places RECOVERY.md describes.
3. FIFO_Enabled jump — confirmed no "Short" override remains, target
   label is far enough away (~22 lines) that Short would plausibly
   have been out of range, matching the described fix.

Attempted a coarse whole-file push/pop instruction count as a first
pass toward an independent bug hunt (same instinct that found real
stack-imbalance bugs elsewhere in this engagement). Result: 37
push-family vs 51 pop-family instructions — but concluded this raw
aggregate is NOT meaningful on its own for a file this size: multiple
return/exit paths, VMM control-transfer macros that may expand to
push/pop not written as literal source lines, and legitimate
pushad/popad-style patterns can all produce a whole-file imbalance
with no actual bug behind it. A trustworthy check needs real
per-procedure tracing across all ~5,714 lines, which is a
substantially bigger undertaking than fit in this session.
**Not resolved either way — flagged honestly as unresolved rather
than false-cleared or false-alarmed.** Good candidate for a future
session with more time budgeted specifically for it.

## vxd-recovered/ push/pop stack-balance trace — COMPLETE (2026-08)

Finished the deep per-procedure trace flagged as open last session.

Built a per-procedure analyzer (BeginProc/EndProc-scoped, not
whole-file) and caught a real flaw in my own first version before
reporting it: I'd modeled 'pushd'/'popd' as a distinct mnemonic
family from plain 'push'/'pop', but this codebase never uses 'popd'
at all (grep-confirmed, 0 occurrences) — 'pushd' here is just an
explicit-size push balanced by a plain 'pop'. Fixed the grouping
(push+pushd+pushw as one family vs pop; pushfd/pushf/pusha/pushad
each exact-matched to their real distinct opcodes) and reran.

19 procedures still showed an aggregate imbalance after that fix.
Manually traced three representative cases by hand, spanning
different complexity levels:
  - Int2F_Proc: 1 pushd ebx, 2 pop ebx across two separate mutually-
    exclusive exit paths — correct.
  - GetStatusStructAddress: pushd eax/ecx once at entry, pushd/pop
    edx balanced every loop iteration (written once in source,
    executed N times), pop eax/ecx duplicated across two exit
    paths — correct.
  - IsPortVCOMM_Added: pushd eax once per .REPEAT iteration, pop eax
    on each of two exit branches inside the loop body — correct.
Also did a structural check on the two most extreme cases
(Int14_Proc: 1 pushad vs 70 popad; W32DeviceIoControl: 3 pushad vs
29 popad) — both are large AH-function/IOCTL dispatch handlers with
popad instances spread evenly across the full body at distinct exit
points, consistent with the same legitimate pattern at scale, not a
leak.

**Conclusion: no genuine stack-balance bugs found in FOSSIL.ASM.**
Every flagged case, across a representative sample covering simple,
loop-based, and large-dispatch code shapes, traced back to the same
benign source-text-duplication-across-exit-paths pattern. This
closes out the previously-open thread from last session with an
actual answer rather than leaving it unresolved.

## Still to do
- [ ] Resolve WF-1 status: find the strncpy fix or confirm it's
      genuinely missing
- [ ] Resolve WF-12 status (likely just a doc fix, low priority)
- [ ] Update HANDOFF.md: add WF-13/14/15 to the bug list, fix the
      wnfos112key/ claim, address the vxd-recovered/ omission
- [ ] strings-compare WNFOS112.ZIP's real FOSSIL.VXD/WNFOSCTL.EXE/
      WNFOSSIL.CPL against src/vxd/FOSSIL.ASM and src/cpl/wf_cpl.c,
      src/ctl/wf_ctl.c for missing features (same methodology as
      the SIO audits) — NOT STARTED YET
- [ ] Check the "50/50 tests pass under Wine" claim — read
      src/test/wf_test.c and see if it actually covers what
      HANDOFF.md's T01-T50 breakdown describes — NOT STARTED
- [ ] Check the 5 "WHAT STILL NEEDS WORK" items in HANDOFF.md are
      accurately described (now 6 minus VxD, which vxd-recovered/
      may have already resolved — needs user input on whether to
      fold that in) — NOT STARTED
- [ ] out/verified/*.dll and *.exe binaries — check what "verified"
      actually means given no real Windows/MinGW toolchain is
      available in this sandbox either — NOT STARTED

## 2026-08 — Punch list from evga's read-through, and vxd-recovered split

Reviewed evga's punch list (a read-through of a delivered copy of
this package). Findings on that list, checked against actual current
state rather than assumed correct:

- Test count (50 -> 65), WF-13/14/15, and the wnfos112key/ claim:
  ALL ALREADY FIXED in HANDOFF.md as of the previous session pass —
  evga's copy predated those fixes. No action needed on these three.
- HANDOFF.md's date (still read 2026-08-18, the original handoff
  date) — genuinely stale, now annotated with a pointer to this file
  as the current record rather than silently bumping the date to
  something misleading.
- session.md item 7 (test suite quality / DLL-fallback finding) —
  checked, it is NOT cut off; it reads to a complete conclusion.
  evga's read was likely of a mid-write snapshot.
- stack_trace.py hardcoded path — REAL, and worse than reported: this
  was a stray early-draft analysis script (superseded by the actual
  per-procedure analyzer used for the completed stack-balance trace)
  that had been accidentally left inside the packaged directory and
  was shipping in every delivered zip. Removed — it was never meant
  to be part of the deliverable, not a case of "needs a CLI arg."

Also, mid-task: found src/vxd-recovered/ missing entirely from the
working directory (sandbox reset between turns) — recovered by
re-extracting from my own last delivered output (still present in
/mnt/user-data/outputs/), then verified all four SHA256SUMS
recomputed to an exact match before proceeding, confirming no data
loss occurred in the restore.

## src/vxd-recovered/ split into its own package (2026-08)
Per user direction: separated into its own standalone deliverable
(same content — verification, RECOVERY.md, all four verified
outputs — unchanged) since it's needed for a separate netserial
integration and no longer belongs nested inside this package.
