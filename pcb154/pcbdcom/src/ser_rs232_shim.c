/* ============================================================================
 * ser_rs232_shim.c — pcbdcom drop-in replacement for COMMDRV.OBJ
 *
 * Exports the 13-function ser_rs232_* API that PCBoard's MODEMDRV.C links
 * against. Enables link-time substitution: link PCBoard with pcbdcom.OBJ
 * (this shim + backend code) instead of Clark's proprietary COMMDRV.OBJ.
 *
 * Calling convention (Pascal, callee-cleans, uppercase symbols) matches
 * what COMM-DRV shipped — MODEMDRV.C sees no difference.
 *
 * API surface (all functions from PCBoard MODEMDRV.C):
 *   ser_rs232_init          — one-time driver init
 *   ser_rs232_setup         — configure Port using port_param struct
 *   ser_rs232_getport       — read current port_param
 *   ser_rs232_getbyte       — read 1 byte (blocking-ish)
 *   ser_rs232_putbyte       — write 1 byte
 *   ser_rs232_getpacket     — read N bytes (or 0 = flush counters)
 *   ser_rs232_putpacket     — write N bytes (or NULL = flush TX)
 *   ser_rs232_viewpacket    — peek RX buffer without consuming
 *   ser_rs232_flush         — flush queues (0=RX, 1=TX, 2=both)
 *   ser_rs232_dtr_on/_off   — DTR control
 *   ser_rs232_rts_on/_off   — RTS control
 *
 * License: GPLv3
 * ==========================================================================*/
#include "pcbdcom.h"
#include "backend.h"
#include <stddef.h>
#include <conio.h>
#include "compat.h"

#if defined(_MSC_VER)
# define SHIM_OUT(port, val) _outp((port), (val))
# define SHIM_IN(port)       (unsigned char)_inp((port))
#else
# define SHIM_OUT(port, val) outp((port), (val))
# define SHIM_IN(port)       (unsigned char)inp((port))
#endif

/* Return codes matching COMM-DRV public API */
#define RS232ERR_NONE   0
#define RS232ERR_BUSY   1
#define RS232ERR_PARAM  2
#define RS232ERR_NOPORT 3

/* port_param struct matching COMM-DRV's public layout.
 * Field order preserved for binary compatibility with existing PCBoard
 * MODEMDRV.C code that uses this struct. */
struct port_param {
    unsigned int  baud;
    unsigned char parity;      /* 'N', 'E', 'O', 'M', 'S' */
    unsigned char data_bits;   /* 5..8 */
    unsigned char stop_bits;   /* 1 or 2 */
    unsigned char flow;        /* 0=none, 1=RTS/CTS, 2=XON/XOFF */
    unsigned int  buf_size;
    unsigned char misc[16];    /* reserved for future / card-specific */
};

/* External port table from pcbdcom.c */
extern pcbdcom_port_t g_ports[PCBDCOM_MAX_PORTS];
extern int g_n_ports;

static pcbdcom_port_t *port_by_num(unsigned int port_num)
{
    if (port_num == 0 || port_num > (unsigned int)g_n_ports)
        return NULL;
    return &g_ports[port_num - 1];
}

/* -------- API functions -------- */

int ser_rs232_init(void)
{
    /* Driver already initialized by TSR install. This just returns OK. */
    return RS232ERR_NONE;
}

int ser_rs232_setup(unsigned int port, struct port_param *pp)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p || !pp) return RS232ERR_PARAM;

    p->baud      = (long)pp->baud;
    p->parity    = pp->parity;
    p->data_bits = pp->data_bits;
    p->stop_bits = pp->stop_bits;
    p->flow      = pp->flow;

    if (p->backend && p->backend->init)
        return (p->backend->init(p) == 0) ? RS232ERR_NONE : RS232ERR_PARAM;
    return RS232ERR_NONE;
}

int ser_rs232_getport(unsigned int port, struct port_param *pp)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p || !pp) return RS232ERR_PARAM;

    pp->baud      = (unsigned int)p->baud;
    pp->parity    = p->parity;
    pp->data_bits = p->data_bits;
    pp->stop_bits = p->stop_bits;
    pp->flow      = p->flow;
    pp->buf_size  = PCBDCOM_BUF_SIZE;
    return RS232ERR_NONE;
}

int ser_rs232_getbyte(unsigned int port, unsigned char *b)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p || !b) return RS232ERR_PARAM;
    if (!p->backend || !p->backend->read) return RS232ERR_NOPORT;
    return (p->backend->read(p, b, 1) == 1) ? RS232ERR_NONE : RS232ERR_BUSY;
}

int ser_rs232_putbyte(unsigned int port, unsigned char *b)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p || !b) return RS232ERR_PARAM;
    if (!p->backend || !p->backend->write) return RS232ERR_NOPORT;
    return (p->backend->write(p, b, 1) == 1) ? RS232ERR_NONE : RS232ERR_BUSY;
}

int ser_rs232_getpacket(unsigned int port, unsigned int n, unsigned char *buf)
{
    pcbdcom_port_t *p = port_by_num(port);
    int got;
    if (!p) return RS232ERR_PARAM;
    if (!p->backend || !p->backend->read) return RS232ERR_NOPORT;

    /* n == 0 with buf != NULL: refresh state, return buffered count in
     * the first byte of buf (MODEMDRV.C convention). */
    if (n == 0) {
        unsigned char tmp;
        (void)p->backend->read(p, &tmp, 0);
        return RS232ERR_NONE;
    }

    /* n == 32767 with NULL buf: flush RX buffer (MODEMDRV.C convention) */
    if (n == 32767 && buf == NULL) {
        unsigned char scratch[64];
        int rem = 32767;
        while (rem > 0 && (got = p->backend->read(p, scratch, 64)) > 0)
            rem -= got;
        return RS232ERR_NONE;
    }

    if (!buf) return RS232ERR_PARAM;
    got = p->backend->read(p, buf, (int)n);
    return (got >= 0) ? RS232ERR_NONE : RS232ERR_BUSY;
}

int ser_rs232_putpacket(unsigned int port, unsigned int n, unsigned char *buf)
{
    pcbdcom_port_t *p = port_by_num(port);
    int put;
    if (!p) return RS232ERR_PARAM;
    if (!p->backend || !p->backend->write) return RS232ERR_NOPORT;

    /* n == 0, buf == NULL: flush TX (MODEMDRV.C convention) */
    if (n == 0 || buf == NULL)
        return RS232ERR_NONE;

    put = p->backend->write(p, buf, (int)n);
    return (put >= 0) ? RS232ERR_NONE : RS232ERR_BUSY;
}

int ser_rs232_viewpacket(unsigned int port, unsigned int n, unsigned char *buf)
{
    pcbdcom_port_t *p = port_by_num(port);
    unsigned int i;
    if (!p || !buf) return RS232ERR_PARAM;

    /* Peek RX buffer without consuming */
    {
        unsigned int avail = (p->rx_head - p->rx_tail) & (PCBDCOM_RX_RING - 1);
        for (i = 0; i < n && i < avail; i++)
            buf[i] = p->rx_buf[(p->rx_tail + i) & (PCBDCOM_RX_RING - 1)];
    }
    return RS232ERR_NONE;
}

int ser_rs232_flush(unsigned int port, unsigned int which)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;

    if (which == 0 || which == 2) { p->rx_head = p->rx_tail = 0; }
    if (which == 1 || which == 2) { p->tx_head = p->tx_tail = 0; }
    return RS232ERR_NONE;
}

int ser_rs232_dtr_on(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    /* MCR bit 0 = DTR */
    SHIM_OUT(p->base + 4,
                     SHIM_IN(p->base + 4) | 0x01);
    return RS232ERR_NONE;
}

int ser_rs232_dtr_off(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    SHIM_OUT(p->base + 4,
                     SHIM_IN(p->base + 4) & ~0x01);
    return RS232ERR_NONE;
}

int ser_rs232_rts_on(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    /* MCR bit 1 = RTS */
    SHIM_OUT(p->base + 4,
                     SHIM_IN(p->base + 4) | 0x02);
    return RS232ERR_NONE;
}

int ser_rs232_rts_off(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    SHIM_OUT(p->base + 4,
                     SHIM_IN(p->base + 4) & ~0x02);
    return RS232ERR_NONE;
}
