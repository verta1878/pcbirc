# Patches

Three patch layers. Apply in order to the 15.3 source archive.

## 1. 153_to_154.patch — Clark's 15.3→15.4 Changes

**What**: All changes Clark Development made between PCBoard 15.3
and the unreleased 15.4b beta.

**Size**: ~9MB, ~1,360 files

**How to apply**:
```
cd PCBSRCV/000
patch -p2 < patches/153_to_154.patch
```

Produces the 15.4 source tree that compiles under Borland C++ 3.1
in DOSBox. All 11 original DOS binaries build from this.

## 2. 154_watcom_phase0_complete.patch — Watcom Port + Phase 0

**What**: Complete port from Borland C++ to OpenWatcom 2.0 (DOS4G),
PLUS Phase 0 (all 12 Clark utilities compiled and linked).

**Size**: ~12MB, ~1,581 files

**Includes**:
- WATCOMPAT.H (Borland→Watcom compatibility layer)
- VMAVL library (sysop/0, 324 lines — AVL tree for PCBFILER)
- VMData library (hexadecimal v0.036 — virtual memory dataset)
- d4all.h CodeBase shim (sysop/0 — for PCBNLC)
- conio_compat.c (sysop/0 — Borland conio via BIOS INT 10h)
- Stub files for linking (stubs.cpp, modem_stubs.cpp, async_wrap.c,
  pcbdata_stub.cpp, pcbfiler_stubs.cpp, pcbnlc_stubs.cpp)
- PCBTITLE_WAT.C (inline ASM replaced with Watcom int386x)
- Source fixes: enum casts, Ctrl-Z removal, #if 0 guards,
  ftime compat, include guards, case-sensitivity symlinks

**How to apply** (from 15.4 Borland baseline):
```
diff base: PCBSRCV/000 (original 15.3)
diff target: PCBSRC/ (current working tree)
```

**Result**: 28 Clark binaries link (16 main + 12 Phase 0 utilities),
0 unresolved symbols each.

## 3. pcbcp_watcom_port.patch — OS/2 PM Control Panel

**What**: PCBCP (OS/2 Presentation Manager control panel) port.
Separate from main patch due to OS/2 PM API dependencies.

**Size**: ~8KB

## Legacy

- `154_borland_to_154_watcom.patch` — superseded by
  `154_watcom_phase0_complete.patch` (which includes all Watcom
  porting work plus Phase 0 utilities)
- `1541_additions.txt` — notes for planned 15.41 additions
