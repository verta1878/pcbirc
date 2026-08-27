/* ====================================================================
 * cydebug.h — Cyclades Driver Debug Infrastructure
 * ====================================================================
 * Provides debug print levels, assert macros, IRQL validation,
 * and register access tracing for the CYPORT driver.
 *
 * Debug output goes to kernel debugger (DbgPrint) in checked builds
 * and optionally in free builds when the debug level registry key
 * is set.
 *
 * WPP tracing (cytrace.h) is the primary tracing mechanism for
 * production use. This file provides the development-time helpers.
 * ====================================================================
 */

#ifndef CYDEBUG_H
#define CYDEBUG_H

/* ====================================================================
 * DEBUG LEVELS
 * ====================================================================
 * These control the verbosity of debug output. Set via registry key:
 *   HKLM\SYSTEM\CurrentControlSet\Services\cyport\Parameters
 *     DebugLevel : REG_DWORD : 0-5
 *
 * Level 0 = silent, Level 5 = maximum verbosity.
 * In checked (debug) builds, default is 3 (INFO).
 * In free (release) builds, default is 1 (ERROR only).
 * ==================================================================== */

#define CYPORT_NONE     0       /* No debug output whatsoever           */
#define CYPORT_ERROR    1       /* Errors: things that should not happen
                                 * and indicate a driver bug or HW fault.
                                 * Examples: failed resource allocation,
                                 * impossible register values, IRP with
                                 * invalid parameters.                  */
#define CYPORT_WARNING  2       /* Warnings: unexpected but recoverable.
                                 * Examples: baud rate clamped, FIFO
                                 * overrun detected, timeout expired.   */
#define CYPORT_INFO     3       /* Informational: normal operations that
                                 * are worth knowing about.
                                 * Examples: port opened/closed, baud
                                 * rate changed, chip detected, config
                                 * applied.                             */
#define CYPORT_TRACE    4       /* Detailed trace: every IRP, every
                                 * significant register access, every
                                 * state transition.                    */
#define CYPORT_VERBOSE  5       /* Maximum: ISR-level detail, individual
                                 * bytes received/transmitted, interrupt
                                 * vector values, spin lock acquire/
                                 * release.                             */

/* ====================================================================
 * GLOBAL DEBUG LEVEL
 * ====================================================================
 * This is the runtime debug level, loaded from registry at init.
 * Declared in cyenum.c (g_CyDebugLevel), read everywhere.
 * ==================================================================== */

extern ULONG g_CyDebugLevel;

/* ====================================================================
 * DEBUG PRINT MACROS
 * ====================================================================
 * CyDbgPrint(level, format, ...) — prints if level <= g_CyDebugLevel.
 * Always compiled in (debug and release). The if-test costs nothing
 * when the level is above threshold because DbgPrint is never called.
 *
 * Usage:
 *   CyDbgPrint(CYPORT_INFO, "Port %d opened, baud=%lu\n", port, baud);
 *   CyDbgPrint(CYPORT_ERROR, "FATAL: chip %d returned 0xFF\n", chip);
 * ==================================================================== */

#define CyDbgPrint(level, fmt, ...)                                    \
    do {                                                               \
        if ((level) <= g_CyDebugLevel) {                               \
            DbgPrint("CYPORT[%d]: " fmt, (level), ##__VA_ARGS__);     \
        }                                                              \
    } while (0)

/* Shorthand for common levels.
 * ##__VA_ARGS__: the ## removes the trailing comma when called
 * with zero variadic args (e.g. CyInfo("text\n")). Required by
 * GCC and OpenWatcom. MSVC DDK accepts either form. */
#define CyError(fmt, ...)    CyDbgPrint(CYPORT_ERROR,   fmt, ##__VA_ARGS__)
#define CyWarn(fmt, ...)     CyDbgPrint(CYPORT_WARNING, fmt, ##__VA_ARGS__)
#define CyInfo(fmt, ...)     CyDbgPrint(CYPORT_INFO,    fmt, ##__VA_ARGS__)
#define CyTrace(fmt, ...)    CyDbgPrint(CYPORT_TRACE,   fmt, ##__VA_ARGS__)
#define CyVerbose(fmt, ...)  CyDbgPrint(CYPORT_VERBOSE, fmt, ##__VA_ARGS__)

/* PAGED_CODE — marks a function as running at PASSIVE_LEVEL only.
 * The DDK checked build verifies IRQL at runtime. OpenWatcom's DDK
 * headers may not define this macro. */
#ifndef PAGED_CODE
#define PAGED_CODE()
#endif


/* ====================================================================
 * ASSERT MACROS
 * ====================================================================
 * CYPORT_ASSERT(cond) — checked (debug) builds only. If cond is FALSE,
 *   fires KeBugCheckEx with our bugcheck code. The driver stops dead.
 *   Use for "this must never happen" invariants.
 *
 * CYPORT_VERIFY(cond) — always evaluates the condition (side effects
 *   are preserved), but only asserts in checked builds. Use when the
 *   condition expression has necessary side effects.
 *
 * CYPORT_ASSERT_AT_IRQL(irql) — verifies current IRQL matches.
 * CYPORT_ASSERT_PASSIVE() — must be at PASSIVE_LEVEL.
 * CYPORT_ASSERT_DISPATCH() — must be at <= DISPATCH_LEVEL.
 * ==================================================================== */

/* Our bugcheck code — unique to this driver so crash dumps are
 * identifiable. Format: 'CY' in hex + subcode.
 * 0xCY000001 = assertion failure
 * 0xCY000002 = IRQL violation
 * 0xCY000003 = invalid state
 * 0xCY000004 = hardware fault                                         */
#define CYPORT_BUGCHECK_ASSERT      0xCE000001
#define CYPORT_BUGCHECK_IRQL        0xCE000002
#define CYPORT_BUGCHECK_STATE       0xCE000003
#define CYPORT_BUGCHECK_HARDWARE    0xCE000004

#if DBG  /* Checked (debug) build */

#define CYPORT_ASSERT(cond)                                            \
    do {                                                               \
        if (!(cond)) {                                                 \
            DbgPrint("CYPORT ASSERT FAILED: %s\n"                     \
                     "  File: %s  Line: %d\n",                        \
                     #cond, __FILE__, __LINE__);                       \
            KeBugCheckEx(CYPORT_BUGCHECK_ASSERT,                      \
                         (ULONG_PTR)__LINE__,                          \
                         (ULONG_PTR)__FILE__,                          \
                         0, 0);                                        \
        }                                                              \
    } while (0)

#define CYPORT_VERIFY(cond) CYPORT_ASSERT(cond)

#define CYPORT_ASSERT_AT_IRQL(expected)                                \
    do {                                                               \
        KIRQL _cur = KeGetCurrentIrql();                               \
        if (_cur != (expected)) {                                      \
            DbgPrint("CYPORT IRQL VIOLATION: expected %d, got %d\n"   \
                     "  File: %s  Line: %d\n",                        \
                     (int)(expected), (int)_cur,                       \
                     __FILE__, __LINE__);                               \
            KeBugCheckEx(CYPORT_BUGCHECK_IRQL,                        \
                         (ULONG_PTR)_cur,                              \
                         (ULONG_PTR)(expected),                        \
                         (ULONG_PTR)__LINE__, 0);                      \
        }                                                              \
    } while (0)

#define CYPORT_ASSERT_PASSIVE()  CYPORT_ASSERT_AT_IRQL(PASSIVE_LEVEL)
#define CYPORT_ASSERT_DISPATCH()                                       \
    do {                                                               \
        KIRQL _cur = KeGetCurrentIrql();                               \
        if (_cur > DISPATCH_LEVEL) {                                   \
            DbgPrint("CYPORT IRQL VIOLATION: expected <= DISPATCH, "   \
                     "got %d\n  File: %s  Line: %d\n",                \
                     (int)_cur, __FILE__, __LINE__);                    \
            KeBugCheckEx(CYPORT_BUGCHECK_IRQL,                        \
                         (ULONG_PTR)_cur,                              \
                         (ULONG_PTR)DISPATCH_LEVEL,                    \
                         (ULONG_PTR)__LINE__, 0);                      \
        }                                                              \
    } while (0)

#else  /* Free (release) build */

#define CYPORT_ASSERT(cond)         ((void)0)
#define CYPORT_VERIFY(cond)         ((void)(cond))
#define CYPORT_ASSERT_AT_IRQL(x)    ((void)0)
#define CYPORT_ASSERT_PASSIVE()     ((void)0)
#define CYPORT_ASSERT_DISPATCH()    ((void)0)

#endif /* DBG */


/* ====================================================================
 * REGISTER ACCESS TRACING
 * ====================================================================
 * CyTraceRead/CyTraceWrite log every register access at VERBOSE level.
 * The register name is resolved from the offset for readability.
 * ==================================================================== */

/* Register name lookup — maps offset to human-readable name.
 * Defined in cyhw.c. Returns "UNKNOWN" for unrecognized offsets. */
extern const char *CyRegName(ULONG offset);

#define CyTraceRead(chip, chan, reg, val)                               \
    CyVerbose("RD chip=%d chan=%d reg=%s(0x%03X) -> 0x%02X\n",        \
              (int)(chip), (int)(chan), CyRegName(reg),                 \
              (unsigned)(reg), (unsigned)(val))

#define CyTraceWrite(chip, chan, reg, val)                              \
    CyVerbose("WR chip=%d chan=%d reg=%s(0x%03X) <- 0x%02X\n",        \
              (int)(chip), (int)(chan), CyRegName(reg),                 \
              (unsigned)(reg), (unsigned)(val))


/* ====================================================================
 * STATE VALIDATION
 * ====================================================================
 * CyAssertState checks that a port extension is in a valid state
 * for the requested operation. Use at the top of every IRP handler.
 * ==================================================================== */

#define CYPORT_STATE_UNINITIALIZED  0
#define CYPORT_STATE_INITIALIZED    1
#define CYPORT_STATE_OPEN           2
#define CYPORT_STATE_CLOSING        3
#define CYPORT_STATE_REMOVED        4

#if DBG
#define CyAssertState(ext, expectedState)                              \
    do {                                                               \
        if ((ext)->DeviceState != (expectedState)) {                   \
            CyError("State mismatch: expected %d, got %d "            \
                    "(chip=%d chan=%d)\n",                              \
                    (int)(expectedState), (int)(ext)->DeviceState,     \
                    (int)(ext)->ChipIndex, (int)(ext)->Channel);       \
        }                                                              \
    } while (0)
#else
#define CyAssertState(ext, state) ((void)0)
#endif


/* ====================================================================
 * POOL ALLOCATION TRACKING (debug builds)
 * ====================================================================
 * Every pool allocation uses a tag so leaks are identifiable in
 * !poolused output.
 *
 * Tags (4-char, little-endian):
 *   'CyPt' — port extension data
 *   'CyRx' — receive buffer
 *   'CyTx' — transmit buffer
 *   'CySy' — symbolic link string
 *   'CyIr' — IRP-related allocations
 * ==================================================================== */

#define CYPORT_TAG_PORT     'tPyC'  /* 'CyPt' */
#define CYPORT_TAG_RXBUF    'xRyC'  /* 'CyRx' */
#define CYPORT_TAG_TXBUF    'xTyC'  /* 'CyTx' */
#define CYPORT_TAG_SYMLINK  'ySyC'  /* 'CySy' */
#define CYPORT_TAG_IRP      'rIyC'  /* 'CyIr' */


#endif /* CYDEBUG_H */
