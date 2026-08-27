# Cross-Compilation Guide — Linux → Windows/DOS with OpenWatcom

## Overview

This project cross-compiles a Windows WDM kernel driver and DOS
utilities on Linux using OpenWatcom v2. This document captures
everything we learned — pitfalls, workarounds, and a checklist
for anyone doing similar work.

## Quick Reference: Build Commands

```bash
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl64:$PATH

# DOS 16-bit (MZ executable)
wcl -ox -bt=dos -ml -fe=CYTEST.EXE cytest.c

# DOS 32-bit (DOS/4GW LE executable)
wcl386 -ox -bt=dos -l=dos4g -fe=CYTEST32.EXE cytest.c

# Windows NT user-mode (PE32 console)
wcl386 -ox -bt=nt -l=nt -fe=tool.exe tool.c

# Windows NT kernel driver (PE32 DLL)
wcc386 -ox -w3 -bt=nt -3s -i=inc -i=$WATCOM/h -i=$WATCOM/h/nt \
       -i=$WATCOM/h/nt/ddk src/file.c -fo=file.obj
wlink system nt_dll name cyport.sys file file.obj \
      library ntoskrnl library hal library clib3s \
      libpath $WATCOM/lib386/nt libpath $WATCOM/lib386/nt/ddk
```

## Checklist: Before Cross-Compiling

Run through this checklist for every source file:

### 1. Calling Conventions

- [ ] Every callback function has NTAPI (__stdcall) in its signature
- [ ] KeSynchronizeExecution callbacks: `BOOLEAN NTAPI fn(PVOID)`
- [ ] DPC callbacks: `VOID NTAPI fn(PKDPC, PVOID, PVOID, PVOID)`
- [ ] IoCompletion callbacks: `NTSTATUS NTAPI fn(PDEVICE_OBJECT, PIRP, PVOID)`
- [ ] IoCsq callbacks: all 6 types with NTAPI
- [ ] Forward declarations match definitions exactly
- [ ] OW enforces strict type matching — MSVC DDK does not

**Why:** OW generates __stdcall code for NTAPI functions (parameters
cleaned by callee) vs __cdecl (cleaned by caller). Mismatch corrupts
the stack. MSVC DDK implicitly adds __stdcall to callbacks via
macros in ntddk.h, so you never notice the missing NTAPI. OW's
headers don't do this — you must add NTAPI yourself.

### 2. Variadic Macros

- [ ] All `__VA_ARGS__` have the `##` prefix: `##__VA_ARGS__`
- [ ] Check: debug macros, trace macros, logging macros

**Why:** `CyInfo("text\n")` has zero variadic args. MSVC accepts
`__VA_ARGS__` with zero args. GCC and OW require `##__VA_ARGS__`
which removes the trailing comma when the variadic part is empty.
Without `##`, the macro expands to `DbgPrint("text\n",)` — note
the trailing comma — which is a syntax error.

### 3. Library Calling Convention (-3s vs -3r)

- [ ] ALL .obj files compiled with the SAME convention
- [ ] Use `-3s` (stack) to match `clib3s.lib`
- [ ] Or use `-3r` (register) to match `clib3r.lib`
- [ ] Never mix — linker will report undefined symbols with `_` suffix

**Why:** OW's register convention appends `_` to C function names
(e.g., `memset_`). The stack convention doesn't. If you compile
one file with -3r and link with clib3s.lib, `memset_` is undefined
because clib3s.lib exports `memset` (no suffix).

### 4. OW Header Bugs (Known Typos)

- [ ] `SERIAL_BAUD_RATE.BuadRate` → define `BaudRate` as alias
- [ ] `IOCTL_SERIAL_SET_BUAD_RATE` → define `SET_BAUD_RATE`
- [ ] `IOCTL_SERIAL_SET_DIR` → define `SET_DTR`
- [ ] Check ntddser.h for other typos when upgrading OW version

**Why:** OpenWatcom's DDK headers are community-maintained and have
typos. These are real bugs in OW, not in our code. We work around
them with `#define` aliases in `owcompat.h`.

### 5. Missing Headers/Functions

- [ ] `csq.h` doesn't exist in OW — use `wdm.h` for IO_CSQ types
- [ ] IoCsqInitialize prototype is in `wdm.h`, not a separate header
- [ ] `PAGED_CODE()` may not be defined — provide fallback `#define`
- [ ] `ERROR_LOG_MAXIMUM_SIZE` may not be defined
- [ ] Serial event flags (EV_RXCHAR etc.) may not be defined
- [ ] Serial MSR bits (SERIAL_MSR_CTS etc.) may not be defined

**Why:** OW's DDK headers cover ~80% of the MS WDK. The remaining
20% must be provided via a compatibility shim (`owcompat.h`).

### 6. IoCsq Callback Type Matching

- [ ] `IO_CSQ_RELEASE_LOCK` takes `PKIRQL` in OW (pointer), not `KIRQL` (value)
- [ ] Forward-declare all IoCsq callbacks before `IoCsqInitialize` call
- [ ] OW checks function types at both call site AND definition

**Why:** OW's type checker is stricter than MSVC's. If it sees a
function pointer cast at the call site, it records the function's
type from the cast. When it later sees the actual definition, it
compares — and if modifiers (like `static`) differ, it errors.
Forward declarations before the first use avoid this.

### 7. Platform Variable Sizes

- [ ] `ULONG_PTR` for pointer-derived values (not `ULONG`)
- [ ] `IoStatus.Information` is `ULONG_PTR`
- [ ] Pointer arithmetic: cast through `(ULONG_PTR)` before `(ULONG)`
- [ ] Use `_snprintf` / `_snwprintf` (not `sprintf` / `swprintf`)
- [ ] See `PLATFORM_SIZES.md` for the full type table

### 8. Macro Definition Order

- [ ] Macros must be defined BEFORE first use (OW requirement)
- [ ] MSVC DDK allows forward references to macros (OW does not)
- [ ] Move all `#define` blocks to the top of the file or to headers

**Why:** We had `CY_ISR_DBG` defined at line 574 but used at line
274. MSVC processed it; OW treated it as an undefined function call.

## Platform Differences: Linux vs Windows vs DOS

### Register Access
| Concept | Linux | Windows | DOS |
|---------|-------|---------|-----|
| Memory-mapped I/O | `readb()`/`writeb()` | `READ_REGISTER_UCHAR()` | direct pointer deref |
| I/O port access | `inb()`/`outb()` | `READ_PORT_UCHAR()` | `in`/`out` ASM |
| Memory barrier | `mb()`, `rmb()`, `wmb()` | built into READ/WRITE_REGISTER | none (x86 strong order) |
| MMIO mapping | `ioremap()` | `MmMapIoSpace()` | far pointer via MK_FP |

### Interrupt Handling
| Concept | Linux | Windows | DOS |
|---------|-------|---------|-----|
| ISR registration | `request_irq()` | `IoConnectInterrupt()` | hook INT vector |
| ISR context | `irqreturn_t` | `BOOLEAN` (TRUE=handled) | `iret` |
| Deferred work | tasklet/workqueue | DPC (`KeInsertQueueDpc`) | not applicable |
| Sync with ISR | `spin_lock_irqsave()` | `KeSynchronizeExecution()` | `cli`/`sti` |

### Memory Allocation
| Concept | Linux | Windows | DOS |
|---------|-------|---------|-----|
| Non-paged | `kmalloc(GFP_ATOMIC)` | `ExAllocatePoolWithTag(NonPagedPool)` | static/BSS |
| Paged | `kmalloc(GFP_KERNEL)` | `ExAllocatePoolWithTag(PagedPool)` | not applicable |
| Free | `kfree()` | `ExFreePoolWithTag()` | not applicable |

### Calling Convention
| Platform | Default | Kernel Callbacks | Notes |
|----------|---------|-----------------|-------|
| Linux x86 | cdecl | cdecl (asmlinkage) | Consistent |
| Windows x86 | cdecl | __stdcall (NTAPI) | MUST mark callbacks |
| DOS 16-bit | cdecl | cdecl | Consistent |
| OW -3r | register | must cast | OW-specific, avoid for drivers |
| OW -3s | stack/cdecl | __stdcall via NTAPI | Matches clib3s.lib |

### String Functions
| Function | Linux | Windows Kernel | OW |
|----------|-------|---------------|-----|
| `memset` | libc | ntoskrnl or intrinsic | clib3s.lib |
| `memcpy` | libc | ntoskrnl `RtlCopyMemory` | clib3s.lib |
| `sprintf` | libc | `_snprintf` (bounded) | clib3s.lib |
| `wcslen` | libc | ntoskrnl | clib3s.lib |
| `strlen` | libc | ntoskrnl or intrinsic | clib3s.lib |

## What We Fixed During Cross-Compilation

| Issue | Files | Root Cause | Fix |
|-------|-------|-----------|-----|
| 24 callbacks missing NTAPI | all src/*.c | MSVC DDK adds implicitly | Added NTAPI to every callback |
| `__VA_ARGS__` without `##` | cydebug.h, cytrace.h, cyinstall.c | MSVC allows, OW/GCC don't | `##__VA_ARGS__` everywhere |
| `memset_` undefined | link | -3r vs -3s mismatch | Compile everything with -3s |
| `BuadRate` typo | ntddser.h | OW header bug | `#define BaudRate BuadRate` |
| `csq.h` missing | cyserial.c, cyread.c, cywrite.c | OW doesn't ship csq.h | `#ifndef __WATCOMC__` guard |
| `CY_ISR_DBG` undefined | cyisr.c | Used before defined | Moved `#define` to top |
| `PAGED_CODE()` missing | cydebug.h | OW headers don't define it | `#ifndef PAGED_CODE` fallback |
| `IO_CSQ_RELEASE_LOCK` signature | cyserial.c | OW uses `PKIRQL`, WDK uses `KIRQL` | Changed to `PKIRQL` |
| IoCsq callback type conflict | cyserial.c | OW strict type checking | Forward declarations |
| `MmMapIoSpace` broken call | cyenum.c | Editing artifact (line split) | Reconstructed call |
| `SERIAL_MSR_*` not defined | owcompat.h | Missing from OW ntddser.h | Defined in owcompat.h |
| `SERIAL_EV_*` not defined | owcompat.h | Missing from OW ntddser.h | Defined in owcompat.h |

## Verification: Confirming the Build

After cross-compilation, verify each binary:

```bash
# Check file format
file cyport.sys    # Should say: PE32 executable (DLL) (GUI) Intel 80386
file cyinstall.exe # Should say: PE32 executable (console) Intel 80386
file cytest.exe    # Should say: MS-DOS executable, MZ for MS-DOS

# Check for undefined symbols (should be empty)
wdis -l cyport.sys | grep "undefined"

# Check imports (should reference ntoskrnl.exe, HAL.dll)
wdis -e cyport.sys | head -20

# Check size (kernel driver should be 40-60 KB)
ls -la cyport.sys
```

## Files Modified for OW Compatibility

| File | Changes | Impact |
|------|---------|--------|
| `inc/owcompat.h` | New file — 77 lines of OW workarounds | OW-only, guarded by `__WATCOMC__` |
| `inc/cycommon.h` | Added `#include "owcompat.h"` + `wdm.h` | Both compilers |
| `inc/cydebug.h` | `##__VA_ARGS__`, `PAGED_CODE` fallback | Both compilers |
| `inc/cytrace.h` | `##__VA_ARGS__` | Both compilers |
| `src/*.c` (all 9) | Added NTAPI to callbacks | Both compilers (NTAPI is correct regardless) |
| `src/cyserial.c` | Forward declarations, PKIRQL, csq.h guard | Both compilers |
| `tools/cyinstall.c` | `##__VA_ARGS__` in debug macros | Both compilers |

## Additional Porting Topics

### DMA

The CD1400 does NOT use DMA — it's a PIO-only UART with a 12-byte
hardware FIFO. The ISR reads bytes one at a time from RDSR into a
software ring buffer. No DMA mapping, scatter-gather, or buffer
descriptors are needed.

If porting a driver that DOES use DMA:
- Linux: `dma_map_single()`, `dma_alloc_coherent()`
- Windows: `IoAllocateMdl()`, `MmBuildMdlForNonPagedPool()`,
  `MapTransfer()` or WDF DMA framework

### Device Model Differences

Linux uses `/dev/ttyC0` device files created by udev rules.
Windows uses `\Device\CycladesCOM3` device objects with
`\DosDevices\COM3` symbolic links created in START_DEVICE.

- Linux: `alloc_chrdev_region()`, `cdev_add()`, `device_create()`
- Windows: `IoCreateDevice()`, `IoCreateSymbolicLink()`
- Linux: sysfs attributes for configuration
- Windows: Registry keys under `Services\cyport\Parameters`

### PnP Resource Parsing

Linux PCI drivers use `pci_resource_start()` / `pci_resource_len()`
to get BAR addresses. Windows PnP delivers resources via the
`IRP_MN_START_DEVICE` IRP in `AllocatedResourcesTranslated`.

Our driver parses `CmResourceTypeMemory` for the PCI BAR and
`CmResourceTypeInterrupt` for the IRQ vector, level, and affinity.
ISA cards fall back to a default memory address.

### Structure Alignment

Windows and Linux x86 both use the same default alignment rules
(natural alignment, no structure reordering). Our driver structures
(`CY_FDO_EXT`, `CY_PDO_EXT`) use standard WDK types and don't
need `#pragma pack` because they're never shared across processes
or written to disk.

Cross-process or disk-serialized structures WOULD need explicit
packing via `#include <pshpack1.h>` / `#include <poppack.h>` on
Windows or `__attribute__((packed))` on Linux.
