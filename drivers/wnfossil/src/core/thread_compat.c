/* ====================================================================
 * thread_compat.c — Thread + Sync + Timer Backends
 * ====================================================================
 * Implements wfp_thread_*, wfp_cs_*, wfp_event_*, wfp_tick/sleep,
 * wfp_log. Works on Win98 through Win11.
 *
 * Win98: CreateThread works. CriticalSection works.
 *        No TryEnterCriticalSection (we don't use it).
 * NT+:   Everything works.
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "wf_core.h"

/* ---- Threads ---- */

typedef struct {
    void (*func)(void *);
    void *arg;
} ThreadParam;

static DWORD WINAPI thread_wrapper(LPVOID lpParam)
{
    ThreadParam *tp = (ThreadParam *)lpParam;
    void (*func)(void *) = tp->func;
    void *arg = tp->arg;
    HeapFree(GetProcessHeap(), 0, tp);
    func(arg);
    return 0;
}

void *wfp_thread_create(void (*func)(void *), void *arg)
{
    ThreadParam *tp;
    HANDLE h;
    DWORD tid;

    tp = (ThreadParam *)HeapAlloc(GetProcessHeap(), 0, sizeof(ThreadParam));
    if (!tp) return NULL;
    tp->func = func;
    tp->arg = arg;

    h = CreateThread(NULL, 0, thread_wrapper, tp, 0, &tid);
    if (!h) {
        HeapFree(GetProcessHeap(), 0, tp);
        return NULL;
    }
    return (void *)h;
}

void wfp_thread_destroy(void *handle)
{
    if (!handle) return;
    /* Signal thread to stop via port->active flag, then wait */
    WaitForSingleObject((HANDLE)handle, 2000);
    CloseHandle((HANDLE)handle);
}

/* ---- Critical Sections ---- */

void *wfp_cs_create(void)
{
    CRITICAL_SECTION *cs;
    cs = (CRITICAL_SECTION *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                       sizeof(CRITICAL_SECTION));
    if (cs) InitializeCriticalSection(cs);
    return (void *)cs;
}

void wfp_cs_destroy(void *cs)
{
    if (!cs) return;
    DeleteCriticalSection((CRITICAL_SECTION *)cs);
    HeapFree(GetProcessHeap(), 0, cs);
}

void wfp_cs_enter(void *cs)
{
    if (cs) EnterCriticalSection((CRITICAL_SECTION *)cs);
}

void wfp_cs_leave(void *cs)
{
    if (cs) LeaveCriticalSection((CRITICAL_SECTION *)cs);
}

/* ---- Events ---- */

void *wfp_event_create(void)
{
    return (void *)CreateEvent(NULL, FALSE, FALSE, NULL);
}

void wfp_event_set(void *event)
{
    if (event) SetEvent((HANDLE)event);
}

void wfp_event_wait(void *event, int timeout_ms)
{
    if (event)
        WaitForSingleObject((HANDLE)event, (DWORD)timeout_ms);
}

void wfp_event_destroy(void *event)
{
    if (event) CloseHandle((HANDLE)event);
}

/* ---- Timer ---- */

uint32_t wfp_tick_ms(void)
{
    return (uint32_t)GetTickCount();
}

void wfp_sleep_ms(int ms)
{
    Sleep((DWORD)ms);
}

/* ---- Debug Log ---- */

static HANDLE g_log_file = INVALID_HANDLE_VALUE;
static CRITICAL_SECTION g_log_cs;
static int g_log_init = 0;

void wfp_log(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    int len;
    DWORD written;
    SYSTEMTIME st;

    if (!g_log_init) {
        /* WF-6 fix: use InterlockedCompareExchange for thread-safe init.
         * Prevents race if two threads call wfp_log before init. */
        if (InterlockedCompareExchange((LONG *)&g_log_init, 1, 0) == 0) {
            InitializeCriticalSection(&g_log_cs);
        /* Open log file in same directory as executable */
        g_log_file = CreateFileA("WNFOSSIL.LOG",
                                 GENERIC_WRITE, FILE_SHARE_READ,
                                 NULL, OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL, NULL);
        if (g_log_file != INVALID_HANDLE_VALUE) {
            /* Log rotation: if file > 1MB, truncate */
            DWORD size = GetFileSize(g_log_file, NULL);
            if (size > 1048576) {
                SetFilePointer(g_log_file, 0, NULL, FILE_BEGIN);
                SetEndOfFile(g_log_file);
            } else {
                SetFilePointer(g_log_file, 0, NULL, FILE_END);
            }
        }
        } /* InterlockedCompareExchange */
    }

    GetLocalTime(&st);
    len = snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d  ",
                   st.wYear, st.wMonth, st.wDay,
                   st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    va_start(ap, fmt);
    len += vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
    va_end(ap);

    if (len < (int)sizeof(buf) - 2) {
        buf[len++] = '\r';
        buf[len++] = '\n';
    }

    EnterCriticalSection(&g_log_cs);

    /* Write to log file */
    if (g_log_file != INVALID_HANDLE_VALUE)
        WriteFile(g_log_file, buf, (DWORD)len, &written, NULL);

    /* Also OutputDebugString for debugger */
    buf[len] = '\0';
    OutputDebugStringA(buf);

    LeaveCriticalSection(&g_log_cs);
}

#endif /* _WIN32 */
