/*
 * ASYNC.C — Serial I/O for PCBoard OpenWatcom port (FOSSIL driver)
 * Replaces ASYNC.ASM (1,893 lines TASM).
 * pcbrevival — Phase 3, August 2026
 * Credit: Original ASM by Clark Development Company
 */

#include <dos.h>
#include <string.h>

/* Globals */
char volatile CDokay = 1;
char volatile _CTSokay = 1;
char volatile _RingDetect = 0;
char CDup = 1;
char B8250 = 0, B16550 = 0, B16550A = 0, B16650 = 0;
char RTSoff = 0;
int volatile _InBytes = 0, _OutBytes = 0;
int volatile _OverrunErrors = 0, _ParityErrors = 0, _FramingErrors = 0;

static int ComPort = 0, PortOpen = 0, fossil_ok = 0;

/* FOSSIL INT 14h */
static int fossil_init(int p) {
    union REGS r;
    r.w.ax = 0x0400; r.w.dx = p;
    int386(0x14, &r, &r);
    return (r.w.ax == 0x1954);
}
static void fossil_deinit(int p) {
    union REGS r;
    r.w.ax = 0x0500; r.w.dx = p;
    int386(0x14, &r, &r);
}
static void fossil_send(int p, char c) {
    union REGS r;
    r.h.ah = 0x01; r.h.al = c; r.w.dx = p;
    int386(0x14, &r, &r);
    _OutBytes++;
}
static int fossil_recv(int p) {
    union REGS r;
    r.w.ax = 0x0200; r.w.dx = p;
    int386(0x14, &r, &r);
    if (r.h.ah & 0x80) return -1;
    _InBytes++;
    return r.h.al;
}
static int fossil_stat(int p) {
    union REGS r;
    r.w.ax = 0x0300; r.w.dx = p;
    int386(0x14, &r, &r);
    return r.w.ax;
}
static void fossil_dtr(int p, int on) {
    union REGS r;
    r.h.ah = 0x06; r.h.al = on ? 1 : 0; r.w.dx = p;
    int386(0x14, &r, &r);
}

/* Name mangling */
#pragma aux ASYNC_INIT "*"
#pragma aux ASYNC_OPENCOM "*"
#pragma aux ASYNC_CLOSECOM "*"
#pragma aux ASYNC_ONLINE "*"
#pragma aux ASYNC_CDSTILLUP "*"
#pragma aux ASYNC_CSENDBYTE "*"
#pragma aux ASYNC_CSENDSTR "*"
#pragma aux ASYNC_CGETBUF "*"
#pragma aux ASYNC_CGETSTR "*"
#pragma aux ASYNC_COMMINKEY "*"
#pragma aux ASYNC_COMMGO "*"
#pragma aux ASYNC_COMMSTOP "*"
#pragma aux ASYNC_COMMPAUSE "*"
#pragma aux ASYNC_CHECKCOMM "*"
#pragma aux ASYNC_CLEARINBUF "*"
#pragma aux ASYNC_CLEAROUTBUF "*"
#pragma aux ASYNC_TURNONDTR "*"
#pragma aux ASYNC_TURNOFFDTR "*"
#pragma aux ASYNC_TURNONRTS "*"
#pragma aux ASYNC_TURNOFFRTS "*"
#pragma aux ASYNC_TURNONFIFO "*"
#pragma aux ASYNC_TURNONXMIT "*"
#pragma aux ASYNC_SETPORT "*"

void ASYNC_INIT(int irq, int addr, char *ib, char *ob,
                int isz, int osz, int cts, int share) {
    (void)irq; (void)ib; (void)ob; (void)isz; (void)osz; (void)cts; (void)share;
    switch (addr) {
        case 0x2F8: ComPort = 1; break;
        case 0x3E8: ComPort = 2; break;
        case 0x2E8: ComPort = 3; break;
        default:    ComPort = 0; break;
    }
    fossil_ok = fossil_init(ComPort);
    if (fossil_ok) { B16550A = 1; CDokay = 1; _CTSokay = 1; }
}

void ASYNC_OPENCOM(int bd, int db) {
    (void)bd; (void)db;
    if (!fossil_ok) fossil_ok = fossil_init(ComPort);
    PortOpen = 1; CDokay = 1; _CTSokay = 1; CDup = 1;
}

void ASYNC_CLOSECOM(void) {
    if (fossil_ok) fossil_deinit(ComPort);
    PortOpen = 0;
}

int ASYNC_ONLINE(void) {
    int s;
    if (!fossil_ok) return 1;
    s = fossil_stat(ComPort);
    CDokay = (s & 0x0080) ? 1 : 0;
    return CDokay;
}

int ASYNC_CDSTILLUP(void) { return ASYNC_ONLINE(); }

/*
 * dos_idle() — Yield CPU time to prevent 100% CPU usage.
 * Original ASYNC.ASM had tight polling loops (known bug,
 * 23-reply thread on alt.bbs.pcboard, Dec 2014).
 * INT 28h is the DOS idle interrupt — tells the OS scheduler
 * (DOSBox, Windows NTVDM, OS/2 VDM) to yield the timeslice.
 * INT 2Fh/1680h is the DPMI "release current virtual machine
 * time slice" call — works under DOS4GW/PMODE.
 */
static void dos_idle(void) {
    union REGS r;
    r.w.ax = 0x1680;  /* Release timeslice (DPMI) */
    int386(0x2F, &r, &r);
}

int ASYNC_COMMINKEY(void) {
    int s;
    if (!fossil_ok) return -1;
    s = fossil_stat(ComPort);
    if (!(s & 0x0100)) {
        dos_idle();  /* yield CPU when no data waiting */
        return -1;
    }
    return fossil_recv(ComPort);
}

void ASYNC_CSENDBYTE(int ch) {
    if (fossil_ok) fossil_send(ComPort, (char)ch);
}

void ASYNC_CSENDSTR(char *str, int len) {
    int i;
    if (!fossil_ok) return;
    for (i = 0; i < len; i++) fossil_send(ComPort, str[i]);
}

int ASYNC_CGETBUF(char *buf, int maxlen) {
    int count = 0, ch;
    if (!fossil_ok) return 0;
    while (count < maxlen) {
        ch = ASYNC_COMMINKEY();
        if (ch == -1) break;
        buf[count++] = (char)ch;
    }
    return count;
}

int ASYNC_CGETSTR(char *buf, int maxlen) {
    int count = 0, ch;
    if (!fossil_ok) return 0;
    while (count < maxlen - 1) {
        ch = ASYNC_COMMINKEY();
        if (ch == -1) break;
        buf[count++] = (char)ch;
        if (ch == '\r' || ch == '\n') break;
    }
    buf[count] = 0;
    return count;
}

int ASYNC_CHECKCOMM(void) {
    int s;
    if (!fossil_ok) return 0;
    s = fossil_stat(ComPort);
    if (!(s & 0x0100)) {
        dos_idle();  /* yield CPU when no data waiting */
        return 0;
    }
    return 1;
}

void ASYNC_CLEARINBUF(void)  { _InBytes = 0; }
void ASYNC_CLEAROUTBUF(void) { _OutBytes = 0; }
void ASYNC_COMMGO(void)      {}
void ASYNC_COMMSTOP(void)    {}
void ASYNC_COMMPAUSE(void)   {}
void ASYNC_TURNONDTR(void)   { if (fossil_ok) fossil_dtr(ComPort, 1); }
void ASYNC_TURNOFFDTR(void)  { if (fossil_ok) fossil_dtr(ComPort, 0); }
void ASYNC_TURNONRTS(void)   { RTSoff = 0; }
void ASYNC_TURNOFFRTS(void)  { RTSoff = 1; }
void ASYNC_TURNONFIFO(int t) { (void)t; }
void ASYNC_TURNONXMIT(void)  { _CTSokay = 1; }
void ASYNC_SETPORT(int b, int d) { (void)b; (void)d; }
