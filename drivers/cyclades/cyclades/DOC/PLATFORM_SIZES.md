# Platform Variable Size Guide — x86 vs x64

## Why This Matters

Windows kernel drivers can run on both 32-bit (x86) and 64-bit
(x64) platforms. Many data types change size between the two:

| Type | x86 Size | x64 Size | Notes |
|------|----------|----------|-------|
| `ULONG` | 4 bytes | 4 bytes | Always 32-bit on Windows |
| `ULONG_PTR` | 4 bytes | **8 bytes** | Pointer-sized unsigned |
| `LONG_PTR` | 4 bytes | **8 bytes** | Pointer-sized signed |
| `SIZE_T` | 4 bytes | **8 bytes** | Same as ULONG_PTR |
| `PVOID` | 4 bytes | **8 bytes** | Pointer |
| `HANDLE` | 4 bytes | **8 bytes** | Pointer-sized |
| `PUCHAR` | 4 bytes | **8 bytes** | Pointer |
| `KIRQL` | 1 byte | 1 byte | Always UCHAR |
| `NTSTATUS` | 4 bytes | 4 bytes | Always LONG |
| `BOOLEAN` | 1 byte | 1 byte | Always UCHAR |
| `KAFFINITY` | 4 bytes | **8 bytes** | Processor affinity bitmask |
| `KINTERRUPT_MODE` | 4 bytes | 4 bytes | Enum (Latched/LevelSensitive) |
| `PHYSICAL_ADDRESS` | 8 bytes | 8 bytes | Always LARGE_INTEGER (64-bit) |
| `IO_CSQ` | ~48 bytes | ~96 bytes | Contains function pointers (grow on x64) |
| `KSPIN_LOCK` | 4 bytes | **8 bytes** | ULONG_PTR internally |
| `LIST_ENTRY` | 8 bytes | **16 bytes** | Two pointers (Flink/Blink) |
| `KDPC` | ~32 bytes | ~64 bytes | Contains pointers (grow on x64) |
| `KTIMER` | ~40 bytes | ~72 bytes | Contains pointers (grow on x64) |
| `IO_REMOVE_LOCK` | ~32 bytes | ~56 bytes | Contains pointers + events |

**The dangerous type is ULONG_PTR.** It's 4 bytes on x86 and 8 on
x64. Any code that stores a pointer in a ULONG will truncate the
high 32 bits on x64 and crash.

## Rules for This Driver

### Rule 1: Never cast a pointer to ULONG
```c
/* WRONG — truncates on x64 */
ULONG addr = (ULONG)somePointer;

/* RIGHT — preserves full pointer width */
ULONG_PTR addr = (ULONG_PTR)somePointer;
```

### Rule 2: Use ULONG_PTR for IoStatus.Information
```c
/* IoStatus.Information is ULONG_PTR, not ULONG.
 * Using ULONG works (widening conversion) but is sloppy.
 * Use the correct type to avoid /W4 warnings. */
Irp->IoStatus.Information = (ULONG_PTR)bytesReturned;
```

### Rule 3: Pointer arithmetic produces ptrdiff_t, not ULONG
```c
/* WRONG — ptrdiff_t is 64-bit on x64 */
ULONG offset = (ULONG)(ptrA - ptrB);

/* RIGHT — cast through ULONG_PTR */
ULONG offset = (ULONG)((ULONG_PTR)(ptrA - ptrB));

/* BEST — use ULONG_PTR if the value could exceed 32 bits */
ULONG_PTR offset = (ULONG_PTR)(ptrA - ptrB);
```

### Rule 4: Use _snwprintf / _snprintf, never swprintf / sprintf
```c
/* WRONG — no buffer size, potential overflow */
swprintf(buf, L"COM%lu", portNum);
sprintf(buf, "\\\\.\%s", name);

/* RIGHT — buffer-size-limited */
_snwprintf(buf, sizeof(buf)/sizeof(WCHAR), L"COM%lu", portNum);
_snprintf(buf, sizeof(buf), "\\\\.\%s", name);
```

### Rule 5: Format strings for pointer-sized values
```c
/* ULONG (always 32-bit): use %lu or %lx */
CyInfo("count=%lu\n", someUlong);

/* ULONG_PTR (32 on x86, 64 on x64): use %Iu or %Ix */
CyInfo("info=%Iu\n", someUlongPtr);

/* Pointers: use %p */
CyInfo("ptr=%p\n", somePointer);

/* NTSTATUS: use %lX (always 32-bit) */
CyInfo("status=0x%08lX\n", ntStatus);
```

### Rule 6: Structure alignment
```c
/* Kernel structures are naturally aligned. On x64:
 * - PVOID fields are 8-byte aligned
 * - ULONG fields are 4-byte aligned
 * - Mixing creates padding
 *
 * Put pointer-sized fields together, then smaller fields,
 * to minimize padding waste. */

typedef struct {
    PVOID       Ptr1;       /* 8 bytes on x64           */
    PVOID       Ptr2;       /* 8 bytes                  */
    ULONG       Count;      /* 4 bytes + 4 padding      */
    BOOLEAN     Flag;       /* 1 byte + 7 padding       */
} BAD_LAYOUT;               /* 32 bytes total, 11 wasted */

typedef struct {
    PVOID       Ptr1;       /* 8 bytes                  */
    PVOID       Ptr2;       /* 8 bytes                  */
    ULONG       Count;      /* 4 bytes                  */
    BOOLEAN     Flag;       /* 1 byte + 3 padding       */
} BETTER_LAYOUT;             /* 24 bytes total, 3 wasted */
```

## What We Fixed in This Driver

| Location | Bug | Fix |
|----------|-----|-----|
| cyisr.c ×3 | `(ULONG)(chipBase - CardBase)` — ptrdiff_t truncated | Cast through `(ULONG_PTR)` first |
| cyioctl.c | `ULONG info` assigned to `IoStatus.Information` (ULONG_PTR) | Changed `info` to `ULONG_PTR` |
| cypdo.c ×4 | `swprintf` without buffer size | Changed to `_snwprintf` with size |
| cyinstall.c ×2 | `sprintf` without buffer size | Changed to `_snprintf` with size |
| test/*.c ×10 | `sprintf` without buffer size | Changed to `_snprintf` with size |

## Types Used in This Driver

| Our Field | Type | Why |
|-----------|------|-----|
| PortIndex | ULONG | Max 32 ports — fits in 32 bits |
| BaudRate | ULONG | Max 230400 — fits in 32 bits |
| RxCount/TxCount | ULONG | Max 4096 — fits in 32 bits |
| RxHead/RxTail | ULONG | Array index — fits in 32 bits |
| ChipBase | PUCHAR | Pointer — MUST be pointer-sized |
| CardBase | PUCHAR | Pointer — MUST be pointer-sized |
| PhysicalBase | PHYSICAL_ADDRESS | 64-bit on all platforms |
| MappedLength | ULONG | Max 16KB — fits in 32 bits |
| IoStatus.Information | ULONG_PTR | System-defined — pointer-sized |
| DumpData[] | ULONG | IoWriteErrorLogEntry spec — 32-bit |


## Structure Size Impact on x64

Our device extensions (`CY_FDO_EXT`, `CY_PDO_EXT`) grow significantly
on x64 because they contain many pointers, spinlocks, and DPCs:

| Structure | x86 (est) | x64 (est) | Why |
|-----------|-----------|-----------|-----|
| `CY_COMMON_EXT` | ~40 bytes | ~72 bytes | IO_REMOVE_LOCK has pointers |
| `CY_FDO_EXT` | ~260 bytes | ~400 bytes | Chip array, PDO array, interrupt |
| `CY_PDO_EXT` | ~8.5 KB | ~8.7 KB | Dominated by 8KB ring buffers |
| `CY_SAVED_STATE` | 12 bytes | 12 bytes | All UCHAR/BOOLEAN fields |

The ring buffers (RxBuf + TxBuf = 8192 bytes) dominate `CY_PDO_EXT`
and don't change between x86 and x64. The ~200-byte difference from
pointer fields is negligible.

`CY_FDO_EXT` contains the child PDO array (`ChildPDOs[32]`) which
is 128 bytes on x86 and 256 bytes on x64. With 8 chips × 4 channels
= 32 pointers, this is the largest pointer-dependent field.

## OpenWatcom Cross-Compilation Notes

The user-mode tools are cross-compiled with OpenWatcom v2 on Linux:
- `wcl386 -bt=nt -l=nt` targets 32-bit Windows NT (PE32 i386)
- OW produces smaller binaries than MinGW (35-53 KB vs 248-262 KB)
- OW's `h/nt/` has full Win32 API headers including `setupapi.h`
- `cfgmgr32.h` is in `h/nt/ddk/` (add to include path)
- Libraries: `setupapi.lib`, `advapi32.lib`, `newdev.lib`, `cfgmgr32.lib`
  in `lib386/nt/` and `lib386/nt/ddk/`
- `##__VA_ARGS__` works in OW for zero-argument variadic macros

The kernel driver (`cyport.sys`) cannot be built with OW — it requires
the Windows DDK/WDK headers (`ntddk.h`, `ntddser.h`) and libraries
(`csq.lib`, `ntoskrnl.lib`) that OW doesn't provide.
