/* ====================================================================
 * comport_compat.c — COM Port Handling Across All Windows Versions
 * ====================================================================
 * Platform-specific serial port access. One file handles:
 *
 *   Win95/98/ME:
 *     - CreateFileA("COM1") works but limited
 *     - No overlapped I/O (FILE_FLAG_OVERLAPPED unreliable on 9x)
 *     - VCOMM VxD API preferred (from VxD driver, not user-mode)
 *     - 16-byte FIFO default on original UARTs
 *     - No SetupDi enumeration
 *
 *   NT4/2000:
 *     - CreateFileA("\\\\.\\COM1") — must use \\.\COMn for ports > 9
 *     - Overlapped I/O works
 *     - DCB defaults differ from 9x (fAbortOnError=TRUE by default!)
 *     - SetupComm for buffer sizes
 *     - No WMI COM enumeration
 *
 *   XP/Vista/7:
 *     - Same as NT4 but SetupDi available for port enumeration
 *     - WMI available for port enumeration
 *     - USB-serial adapters common (COM numbers > 4)
 *
 *   Win8/8.1:
 *     - Same as 7 but Connected Standby can close ports
 *     - Power management events affect serial
 *
 *   Win10/11 (x86/x64):
 *     - Same API but app manifest may be needed for some USB ports
 *     - USBSER.SYS is inbox driver (no 3rd party driver needed)
 *     - COM port > 256 possible with USB hubs
 *     - ARM64 builds possible but rare for serial
 *
 * Key differences this file handles:
 *   1. Port name format: "COM1" vs "\\\\.\\COM1" vs "\\\\.\\COM256"
 *   2. Overlapped I/O: disabled on 9x, enabled on NT+
 *   3. DCB defaults: fAbortOnError varies by platform
 *   4. Port enumeration: direct access (9x), SetupDi (NT+)
 *   5. Buffer sizes: SetupComm on NT+, FIFO control on 9x
 *   6. Handle size: 32-bit HANDLE on i386, 64-bit on x64
 *   7. USB-serial: high COM numbers need \\.\COMn format
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "wf_core.h"

/* Forward declaration — from registry_compat.c */
extern int wf_compat_platform(void);
extern int wf_compat_is_64bit(void);

#define WF_PLATFORM_9X   1
#define WF_PLATFORM_NT   2
#define WF_PLATFORM_MOD  3


/* ================================================================
 * PORT NAME FORMATTING
 * ================================================================
 * Win95/98: "COM1" through "COM4" works directly.
 * NT+: Must use "\\\\.\\COMn" format for ALL ports.
 *      Without the \\.\\ prefix, ports > COM9 SILENTLY FAIL.
 *      Even COM1-COM4 should use it for consistency.
 * USB adapters: Can be COM5, COM12, COM256 — always need \\.\\ .
 * ================================================================ */

static void format_port_name(char *out, int out_size, const char *name)
{
    int platform = wf_compat_platform();

    if (platform == WF_PLATFORM_9X) {
        /* Win95/98: use bare name "COM1" */
        strncpy(out, name, out_size - 1);
        out[out_size - 1] = '\0';
    } else {
        /* NT+: use \\.\COMn format */
        if (strncmp(name, "\\\\.\\", 4) == 0) {
            /* Already formatted */
            strncpy(out, name, out_size - 1);
            out[out_size - 1] = '\0';  /* Missing here previously — the
                                        * WF_PLATFORM_9X branch two lines
                                        * up NUL-terminates after its
                                        * strncpy, this branch didn't,
                                        * leaving `out` unterminated
                                        * whenever `name` was >= out_size
                                        * bytes with no NUL in that
                                        * range. Classic strncpy footgun;
                                        * found during WF-1 verification
                                        * (didn't match WF-1's own
                                        * description exactly, but is a
                                        * real, same-class bug in the
                                        * same file). */
        } else {
            snprintf(out, out_size, "\\\\.\\%s", name);
        }
    }
}


/* ================================================================
 * OPEN COM PORT
 * ================================================================ */

int wfp_com_open(WfPort *p, const char *name, uint32_t baud)
{
    char portname[32];
    HANDLE hCom;
    DCB dcb;
    COMMTIMEOUTS timeouts;
    int platform = wf_compat_platform();
    DWORD flags;

    format_port_name(portname, sizeof(portname), name);

    /* Overlapped I/O: reliable on NT+, problematic on 9x */
    if (platform == WF_PLATFORM_9X)
        flags = 0;                  /* Synchronous on Win95/98       */
    else
        flags = FILE_FLAG_OVERLAPPED;

    hCom = CreateFileA(portname,
                       GENERIC_READ | GENERIC_WRITE,
                       0,           /* Exclusive access              */
                       NULL,
                       OPEN_EXISTING,
                       flags,
                       NULL);

    if (hCom == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        wfp_log("COM open failed: %s (error %lu)", portname, (unsigned long)err);

        /* Common errors:
         * 2  = FILE_NOT_FOUND — port doesn't exist
         * 5  = ACCESS_DENIED — port in use by another app
         * 31 = GEN_FAILURE — driver problem (USB unplugged)
         * 87 = INVALID_PARAMETER — bad port name format */
        if (err == ERROR_FILE_NOT_FOUND)
            wfp_log("  Port %s does not exist", portname);
        else if (err == ERROR_ACCESS_DENIED)
            wfp_log("  Port %s is in use by another application", portname);
        else if (err == ERROR_GEN_FAILURE)
            wfp_log("  Port %s hardware error (USB disconnected?)", portname);

        return -1;
    }

    p->hCom = (void *)hCom;

    /* ---- Configure DCB ----
     * CRITICAL: GetCommState first to get platform defaults,
     * then override. NT4 sets fAbortOnError=TRUE by default
     * which causes reads/writes to fail on parity/framing errors.
     * Win95 sets it FALSE. We always want FALSE. */

    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hCom, &dcb);

    dcb.BaudRate     = baud;
    dcb.ByteSize     = 8;
    dcb.Parity       = NOPARITY;
    dcb.StopBits     = ONESTOPBIT;
    dcb.fBinary      = TRUE;

    /* Flow control — OFF by default.
     * Original WinFOSSIL: no flow control until app requests it.
     * CRITICAL on all platforms: if CTS flow is on and modem
     * doesn't assert CTS, all writes block forever. */
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fOutX        = FALSE;       /* No XON/XOFF (breaks Zmodem)  */
    dcb.fInX         = FALSE;

    /* DTR/RTS — raise both on open.
     * Modem needs DTR to respond to AT commands.
     * RTS needed for hardware handshaking setup. */
    dcb.fDtrControl  = DTR_CONTROL_ENABLE;
    dcb.fRtsControl  = RTS_CONTROL_ENABLE;

    /* Error handling — MUST be FALSE on all platforms.
     * NT4 default is TRUE which causes serial errors to abort I/O.
     * Win95 default is FALSE. Force FALSE everywhere. */
    dcb.fAbortOnError   = FALSE;

    /* Signal sensitivity */
    dcb.fDsrSensitivity = FALSE;    /* Don't ignore data if DSR low */
    dcb.fNull           = FALSE;    /* Don't discard null bytes      */
    dcb.fErrorChar      = FALSE;    /* Don't replace error chars     */

    /* XON/XOFF characters (standard values) */
    dcb.XonChar  = 0x11;            /* Ctrl-Q                        */
    dcb.XoffChar = 0x13;            /* Ctrl-S                        */
    dcb.XonLim   = 2048;
    dcb.XoffLim  = 2048;

    if (!SetCommState(hCom, &dcb)) {
        wfp_log("SetCommState failed: error %lu", (unsigned long)GetLastError());
        CloseHandle(hCom);
        p->hCom = NULL;
        return -1;
    }

    /* ---- Buffer sizes ----
     * SetupComm: sets driver-level buffers (not our ring buffers).
     * Win95: may be ignored by some UART drivers.
     * NT+: works reliably. 4K matches original WinFOSSIL. */
    SetupComm(hCom, (DWORD)p->cfg.rx_buf_size, (DWORD)p->cfg.tx_buf_size);

    /* ---- Timeouts ----
     * Different strategies per platform:
     *
     * Win95/98 (synchronous I/O):
     *   Short read timeout so we don't block the thread.
     *   Write timeout generous (modem may be slow).
     *
     * NT+ (overlapped I/O):
     *   ReadIntervalTimeout=MAXDWORD with Multiplier=0 and
     *   Constant=0 means "return immediately with whatever
     *   is available" — we rely on our own ring buffer. */

    memset(&timeouts, 0, sizeof(timeouts));

    if (platform == WF_PLATFORM_9X) {
        /* Win95/98: synchronous mode, short timeouts */
        timeouts.ReadIntervalTimeout         = 50;
        timeouts.ReadTotalTimeoutMultiplier   = 0;
        timeouts.ReadTotalTimeoutConstant     = 100;
        timeouts.WriteTotalTimeoutMultiplier  = 10;
        timeouts.WriteTotalTimeoutConstant    = 1000;
    } else {
        /* NT+: overlapped mode, return immediately */
        timeouts.ReadIntervalTimeout         = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier   = 0;
        timeouts.ReadTotalTimeoutConstant     = 0;
        timeouts.WriteTotalTimeoutMultiplier  = 0;
        timeouts.WriteTotalTimeoutConstant    = 5000;
    }

    SetCommTimeouts(hCom, &timeouts);

    /* Purge any stale data in driver buffers */
    PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR |
                    PURGE_RXABORT | PURGE_TXABORT);

    /* Clear any pending errors */
    {
        DWORD errors;
        COMSTAT comstat;
        ClearCommError(hCom, &errors, &comstat);
    }

    p->dtr_on = 1;
    p->rts_on = 1;

    wfp_log("COM opened: %s @ %lu baud (%s mode, %s)",
            portname, (unsigned long)baud,
            (platform == WF_PLATFORM_9X) ? "synchronous" : "overlapped",
            wf_compat_is_64bit() ? "x64" : "x86");

    return 0;
}


/* ================================================================
 * CLOSE COM PORT
 * ================================================================ */

void wfp_com_close(WfPort *p)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;

    /* Drop DTR to signal disconnect to modem */
    EscapeCommFunction(hCom, CLRDTR);

    /* Flush pending writes */
    FlushFileBuffers(hCom);

    /* Purge buffers */
    PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR |
                    PURGE_RXABORT | PURGE_TXABORT);

    CloseHandle(hCom);
    p->hCom = NULL;

    wfp_log("COM closed: port %d", p->index);
}


/* ================================================================
 * READ FROM COM PORT
 * ================================================================
 * Win95: synchronous ReadFile (blocks up to timeout).
 * NT+: overlapped ReadFile (returns immediately or waits).
 * ================================================================ */

int wfp_com_read(WfPort *p, void *buf, int len)
{
    HANDLE hCom = (HANDLE)p->hCom;
    DWORD bytesRead = 0;
    int platform = wf_compat_platform();

    if (!hCom || hCom == INVALID_HANDLE_VALUE) return -1;

    if (platform == WF_PLATFORM_9X) {
        /* Synchronous read */
        if (!ReadFile(hCom, buf, (DWORD)len, &bytesRead, NULL)) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                /* Clear the error and continue */
                DWORD errors;
                ClearCommError(hCom, &errors, NULL);
                return 0;
            }
        }
    } else {
        /* Overlapped read */
        OVERLAPPED ov;
        memset(&ov, 0, sizeof(ov));
        ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!ov.hEvent) return -1;

        if (ReadFile(hCom, buf, (DWORD)len, &bytesRead, &ov)) {
            /* Completed immediately */
            CloseHandle(ov.hEvent);
            return (int)bytesRead;
        }

        if (GetLastError() == ERROR_IO_PENDING) {
            /* Wait up to 100ms for data */
            DWORD wait = WaitForSingleObject(ov.hEvent, 100);
            if (wait == WAIT_OBJECT_0)
                GetOverlappedResult(hCom, &ov, &bytesRead, FALSE);
            else
                CancelIo(hCom);     /* Timeout — cancel pending I/O */
        } else {
            /* Real error */
            DWORD errors;
            ClearCommError(hCom, &errors, NULL);
        }

        CloseHandle(ov.hEvent);
    }

    return (int)bytesRead;
}


/* ================================================================
 * WRITE TO COM PORT
 * ================================================================ */

int wfp_com_write(WfPort *p, const void *buf, int len)
{
    HANDLE hCom = (HANDLE)p->hCom;
    DWORD bytesWritten = 0;
    int platform = wf_compat_platform();

    if (!hCom || hCom == INVALID_HANDLE_VALUE) return -1;

    if (platform == WF_PLATFORM_9X) {
        /* Synchronous write */
        if (!WriteFile(hCom, buf, (DWORD)len, &bytesWritten, NULL)) {
            DWORD errors;
            ClearCommError(hCom, &errors, NULL);
            return 0;
        }
    } else {
        /* Overlapped write */
        OVERLAPPED ov;
        memset(&ov, 0, sizeof(ov));
        ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!ov.hEvent) return -1;

        if (WriteFile(hCom, buf, (DWORD)len, &bytesWritten, &ov)) {
            CloseHandle(ov.hEvent);
            return (int)bytesWritten;
        }

        if (GetLastError() == ERROR_IO_PENDING) {
            DWORD wait = WaitForSingleObject(ov.hEvent, 5000);
            if (wait == WAIT_OBJECT_0)
                GetOverlappedResult(hCom, &ov, &bytesWritten, FALSE);
            else {
                CancelIo(hCom);
                p->perf_tx_timeouts++;
            }
        } else {
            DWORD errors;
            ClearCommError(hCom, &errors, NULL);
        }

        CloseHandle(ov.hEvent);
    }

    return (int)bytesWritten;
}


/* ================================================================
 * STATUS / CONTROL
 * ================================================================ */

int wfp_com_status(WfPort *p)
{
    HANDLE hCom = (HANDLE)p->hCom;
    DWORD modem_status = 0;
    DWORD errors;
    COMSTAT comstat;
    int result = 0;

    if (!hCom || hCom == INVALID_HANDLE_VALUE) return 0;

    ClearCommError(hCom, &errors, &comstat);
    GetCommModemStatus(hCom, &modem_status);

    /* Map Win32 modem status to FOSSIL status bits */
    if (modem_status & MS_CTS_ON)  result |= WF_ST_CTS;
    if (modem_status & MS_DSR_ON)  result |= WF_ST_DSR;
    if (modem_status & MS_RING_ON) result |= WF_ST_RI;
    if (modem_status & MS_RLSD_ON) result |= WF_ST_DCD;

    /* Line status from errors */
    if (errors & CE_OVERRUN)  result |= WF_ST_OVRN;
    if (errors & CE_RXPARITY) result |= WF_ST_PRTY;
    if (errors & CE_FRAME)    result |= WF_ST_FRME;
    if (errors & CE_BREAK)    result |= WF_ST_BREAK;

    /* Data available */
    if (comstat.cbInQue > 0) result |= WF_ST_RDA;

    /* TX ready (always if not flow-blocked) */
    if (!(comstat.fCtsHold || comstat.fDsrHold || comstat.fXoffHold))
        result |= WF_ST_THRE | WF_ST_TSRE;

    return result;
}

void wfp_com_set_baud(WfPort *p, uint32_t baud)
{
    HANDLE hCom = (HANDLE)p->hCom;
    DCB dcb;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;

    GetCommState(hCom, &dcb);
    dcb.BaudRate = baud;
    SetCommState(hCom, &dcb);
    wfp_log("COM%d: baud → %lu", p->index + 1, (unsigned long)baud);
}

void wfp_com_set_dtr(WfPort *p, int on)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;
    EscapeCommFunction(hCom, on ? SETDTR : CLRDTR);
}

void wfp_com_set_rts(WfPort *p, int on)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;
    EscapeCommFunction(hCom, on ? SETRTS : CLRRTS);
}

void wfp_com_set_break(WfPort *p, int on)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;
    if (on) SetCommBreak(hCom);
    else ClearCommBreak(hCom);
}

void wfp_com_set_flow(WfPort *p, int xon, int cts)
{
    HANDLE hCom = (HANDLE)p->hCom;
    DCB dcb;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;

    GetCommState(hCom, &dcb);

    dcb.fOutX = xon ? TRUE : FALSE;
    dcb.fInX  = xon ? TRUE : FALSE;

    dcb.fOutxCtsFlow = cts ? TRUE : FALSE;
    dcb.fRtsControl  = cts ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;

    SetCommState(hCom, &dcb);
}

void wfp_com_flush(WfPort *p)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;
    FlushFileBuffers(hCom);
}

void wfp_com_purge_rx(WfPort *p)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;
    PurgeComm(hCom, PURGE_RXCLEAR | PURGE_RXABORT);
}

void wfp_com_purge_tx(WfPort *p)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;
    PurgeComm(hCom, PURGE_TXCLEAR | PURGE_TXABORT);
}

void wfp_com_setup_buffers(WfPort *p, int rx_size, int tx_size)
{
    HANDLE hCom = (HANDLE)p->hCom;
    if (!hCom || hCom == INVALID_HANDLE_VALUE) return;

    /* SetupComm sets the DRIVER-level buffers (not our ring buffers).
     * On Win95 some drivers ignore this. On NT+ it works. */
    SetupComm(hCom, (DWORD)rx_size, (DWORD)tx_size);
}


/* ================================================================
 * PORT ENUMERATION
 * ================================================================
 * Lists available COM ports on the system.
 * Method varies by platform:
 *   Win95/98: Try opening COM1-COM16, check for errors
 *   NT+: QueryDosDevice or SetupDi (if available)
 * ================================================================ */

int wf_enum_ports(char ports[][WF_PORT_NAME_LEN], int max_ports)
{
    int count = 0;
    int platform = wf_compat_platform();

    if (platform == WF_PLATFORM_9X) {
        /* Win95/98: brute force — try each port */
        int i;
        for (i = 1; i <= 16 && count < max_ports; i++) {
            char name[16];
            HANDLE h;
            snprintf(name, sizeof(name), "COM%d", i);
            h = CreateFileA(name, GENERIC_READ | GENERIC_WRITE,
                           0, NULL, OPEN_EXISTING, 0, NULL);
            if (h != INVALID_HANDLE_VALUE) {
                CloseHandle(h);
                strncpy(ports[count], name, WF_PORT_NAME_LEN - 1);
                ports[count][WF_PORT_NAME_LEN - 1] = '\0';
                count++;
            } else if (GetLastError() == ERROR_ACCESS_DENIED) {
                /* Port exists but is in use */
                strncpy(ports[count], name, WF_PORT_NAME_LEN - 1);
                ports[count][WF_PORT_NAME_LEN - 1] = '\0';
                count++;
            }
        }
    } else {
        /* NT+: QueryDosDevice for COM ports */
        char buf[65535];
        DWORD len = QueryDosDeviceA(NULL, buf, sizeof(buf));
        if (len > 0) {
            char *p = buf;
            while (*p && count < max_ports) {
                if (strncmp(p, "COM", 3) == 0 && p[3] >= '0' && p[3] <= '9') {
                    /* p comes from QueryDosDeviceA's own buffer, not a
                     * caller-controlled string like format_port_name's
                     * `name` — practically always short — but explicit
                     * termination costs nothing and matches the fix
                     * applied to the other strncpy sites in this file. */
                    strncpy(ports[count], p, WF_PORT_NAME_LEN - 1);
                    ports[count][WF_PORT_NAME_LEN - 1] = '\0';
                    count++;
                }
                p += strlen(p) + 1;
            }
        }
    }

    return count;
}

#endif /* _WIN32 */
