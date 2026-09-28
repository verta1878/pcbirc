/* ====================================================================
 * vmodem_sys.c — VMODEM.SYS Physical Layer Driver for OS/2
 * ====================================================================
 * Registers with SIO2K.SYS as physical driver "VMOD$".
 * Routes serial I/O to/from VMODEM.EXE via shared memory ring buffers.
 *
 *   BBS app → SIO2K.SYS → IDC → VMODEM.SYS ↔ shared mem ↔ VMODEM.EXE
 *                                                              ↓
 *                                                         TCP/IP socket
 *
 * VMODEM.SYS is a thin pipe — no AT parsing, no TCP.
 * Shared memory: \SHAREMEM\SIO2K\VMODEMn (per port)
 *
 * Clean-room reimplementation. GPLv3.
 * ==================================================================== */

#include "driver.h"
#include "sio2k_idc.h"

#define VM_SHM_MAGIC     0x564D4F44  /* 'VMOD' */
#define VM_BUF_SIZE      4096
/* OS/2 device registration name (from original binary) */
#define VM_DEV_NAME      "VMODEM$ "    /* 8-char padded */

#define VM_MAX_PORTS     8

/* Ring buffer in shared memory */
typedef struct {
    volatile unsigned short head;
    volatile unsigned short tail;
    unsigned char           data[VM_BUF_SIZE];
} VM_RINGBUF;

/* Shared control block — one per port, mapped by both driver and EXE */
typedef struct {
    unsigned long   magic;
    unsigned short  version;
    unsigned short  portIndex;
    unsigned char   dtr, rts, breakOn, flowMode;
    unsigned long   baudRate;
    unsigned char   lcr;
    unsigned char   msr, lsr, connected;
    volatile unsigned short events;
    VM_RINGBUF      txbuf;          /* App → VMODEM.EXE */
    VM_RINGBUF      rxbuf;          /* VMODEM.EXE → App */
} VM_SHARED, *PVM_SHARED;

typedef struct {
    int             active;
    PVM_SHARED      pShared;
    unsigned short  portHandle;
} VM_PORT;

static VM_PORT  g_ports[VM_MAX_PORTS];
static int      g_numPorts = 0;

/* Ring buffer helpers */
static int rb_count(VM_RINGBUF *b) {
    int n = b->head - b->tail;
    return (n < 0) ? n + VM_BUF_SIZE : n;
}
static int rb_free(VM_RINGBUF *b) { return VM_BUF_SIZE - 1 - rb_count(b); }
static void rb_put(VM_RINGBUF *b, unsigned char ch) {
    unsigned short next = (b->head + 1) % VM_BUF_SIZE;
    if (next != b->tail) { b->data[b->head] = ch; b->head = next; }
}
static int rb_get(VM_RINGBUF *b) {
    if (b->tail == b->head) return -1;
    unsigned char ch = b->data[b->tail];
    b->tail = (b->tail + 1) % VM_BUF_SIZE;
    return ch;
}
static void rb_clear(VM_RINGBUF *b) { b->head = b->tail = 0; }

/* IDC entry — called by SIO2K for all port operations */
static unsigned short vmodem_idc(unsigned short cmd, unsigned short portIndex,
                                  void *pData, unsigned short cbData)
{
    VM_PORT *p; PVM_SHARED ps;
    if (portIndex >= g_numPorts) return 0xFFFF;
    p = &g_ports[portIndex]; ps = p->pShared;
    if (!ps) return 0xFFFF;

    switch (cmd) {
    case PHYS_CMD_INIT:
        ps->magic = VM_SHM_MAGIC; ps->version = 1; ps->portIndex = portIndex;
        ps->dtr = ps->rts = 0; ps->msr = 0; ps->lsr = 0x60;
        ps->connected = 0; ps->events = 0;
        rb_clear(&ps->txbuf); rb_clear(&ps->rxbuf);
        return 0;
    case PHYS_CMD_OPEN:
        p->active = 1; ps->dtr = ps->rts = 1; return 0;
    case PHYS_CMD_CLOSE:
        p->active = 0; ps->dtr = ps->rts = 0; return 0;
    case PHYS_CMD_SETBAUD:
        if (pData) ps->baudRate = *(unsigned long *)pData; return 0;
    case PHYS_CMD_SETLINE:
        if (pData) ps->lcr = *(unsigned char *)pData; return 0;
    case PHYS_CMD_SETMCR:
        if (pData) { unsigned char m = *(unsigned char *)pData;
            ps->dtr = (m & 1) ? 1 : 0; ps->rts = (m & 2) ? 1 : 0;
            if (!ps->dtr) ps->events |= 0x0100; } return 0;
    case PHYS_CMD_GETMSR:
        if (pData) *(unsigned char *)pData = ps->msr; return 0;
    case PHYS_CMD_GETLSR:
        if (pData) *(unsigned char *)pData = ps->lsr; return 0;
    case PHYS_CMD_TXBYTE:
        if (pData) rb_put(&ps->txbuf, *(unsigned char *)pData); return 0;
    case PHYS_CMD_TXBLOCK:
        { unsigned char *s = (unsigned char *)pData; unsigned short i;
          for (i = 0; i < cbData && rb_free(&ps->txbuf) > 0; i++) rb_put(&ps->txbuf, s[i]);
          return i; }
    case PHYS_CMD_RXBLOCK:
        { unsigned char *d = (unsigned char *)pData; unsigned short i;
          for (i = 0; i < cbData && rb_count(&ps->rxbuf) > 0; i++)
              { int c = rb_get(&ps->rxbuf); if (c >= 0) d[i] = (unsigned char)c; else break; }
          return i; }
    case PHYS_CMD_RXCOUNT:
        if (pData) *(unsigned short *)pData = (unsigned short)rb_count(&ps->rxbuf); return 0;
    case PHYS_CMD_TXCOUNT:
        if (pData) *(unsigned short *)pData = (unsigned short)rb_free(&ps->txbuf); return 0;
    case PHYS_CMD_TXFLUSH: rb_clear(&ps->txbuf); return 0;
    case PHYS_CMD_RXFLUSH: rb_clear(&ps->rxbuf); return 0;
    case PHYS_CMD_BREAKON:  ps->breakOn = 1; return 0;
    case PHYS_CMD_BREAKOFF: ps->breakOn = 0; return 0;
    case PHYS_CMD_SETFLOW:
        if (pData) ps->flowMode = (unsigned char)(*(unsigned short *)pData); return 0;
    case PHYS_CMD_GETINFO:
        if (pData && cbData >= sizeof(PHYS_PORT_INFO)) {
            PHYS_PORT_INFO *pi = (PHYS_PORT_INFO *)pData;
            pi->ioBase = 0; pi->irq = 0; pi->uartType = PHYS_UART_VMODEM;
            pi->fifoSize = VM_BUF_SIZE; pi->crystalFreq = 0;
            pi->maxBaud = 115200; pi->flags = PHYS_FLAG_EXISTS; }
        return 0;
    default: return 0;
    }
}
