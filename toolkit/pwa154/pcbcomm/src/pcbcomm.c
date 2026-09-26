/* ============================================================================
 * ser_rs232_shim.c  pcbdcom drop-in replacement for COMMDRV.OBJ
 *
 * Exports the 13-function ser_rs232_* API that PCBoard's MODEMDRV.C links
 * against. Enables link-time substitution: link PCBoard with pcbdcom.OBJ
 * (this shim + backend code) instead of Clark's proprietary COMMDRV.OBJ.
 *
 * Calling convention (Pascal, callee-cleans, uppercase symbols) matches
 * what COMM-DRV shipped — MODEMDRV.C sees no difference.
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

/* External port table from pcbdcom.c */
extern pcbdcom_port_t g_ports[PCBDCOM_MAX_PORTS];
extern int g_n_ports;

static pcbdcom_port_t *port_by_num(unsigned int port_num)
{
    if (port_num == 0 || port_num > (unsigned int)g_n_ports)
        return NULL;
    return &g_ports[port_num - 1];
}

/* Update compat_opcb from current port state */
static void update_opcb(pcbdcom_port_t *p)
{
    unsigned int rx_avail = (p->rx_head - p->rx_tail) & (p->rx_size - 1);
    unsigned int tx_pend  = (p->tx_head - p->tx_tail) & (p->tx_size - 1);

    p->compat_opcb.inbuf_count  = rx_avail;
    p->compat_opcb.outbuf_count = tx_pend;
    /* Read MSR from hardware if port is open */
    if (p->open && p->base)
        p->compat_opcb.msr_reg = SHIM_IN(p->base + 6);
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

    p->baud      = (unsigned long)pp->baud;
    p->parity    = pp->parity;
    p->data_bits = pp->data_bits;
    p->stop_bits = pp->stop_bits;
    p->flow      = pp->flow;

    /* Store COMMDRV-compatible fields */
    p->compat_opcb.cardtype = pp->cardtype;

    if (p->backend && p->backend->init)
        return (p->backend->init(p) == 0) ? RS232ERR_NONE : RS232ERR_PARAM;
    return RS232ERR_NONE;
}

int ser_rs232_getport(unsigned int port, struct port_param *pp)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p || !pp) return RS232ERR_PARAM;

    /* Update opcb from current state */
    update_opcb(p);

    /* Basic serial parameters */
    pp->baud      = (unsigned int)p->baud;
    pp->parity    = p->parity;
    pp->data_bits = p->data_bits;
    pp->stop_bits = p->stop_bits;
    pp->flow      = p->flow;
    pp->buf_size  = p->rx_size;

    /* COMMDRV-compatible fields */
    pp->lngth      = (p->data_bits == 8) ? LENGTH_8 : LENGTH_7;
    pp->cardtype   = p->compat_opcb.cardtype;
    pp->protocol   = (p->flow == 1) ? PROT_RTSRTS :
                     (p->flow == 2) ? PROT_XONXOFF : 0;
    pp->error      = RS232ERR_NONE;
    pp->outbuf_len = p->tx_size;
    pp->inbuf_len  = p->rx_size;
    pp->block[0]   = 0;
    pp->block[1]   = 0;
    pp->block[2]   = 0;
    pp->block[3]   = 0;

    /* Pointers to embedded compat structs */
    pp->opcb   = &p->compat_opcb;
    pp->auxpcb = &p->compat_auxpcb;

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

    /* n == 0: refresh state (MODEMDRV.C convention) */
    if (n == 0) {
        update_opcb(p);
        return RS232ERR_NONE;
    }

    /* n == 32767 with NULL buf: flush RX buffer (MODEMDRV.C convention) */
    if (n == 32767 && buf == NULL) {
        p->rx_head = p->rx_tail = 0;
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
        unsigned int avail = (p->rx_head - p->rx_tail) & (p->rx_size - 1);
        for (i = 0; i < n && i < avail; i++)
            buf[i] = p->rx_buf[(p->rx_tail + i) & (p->rx_size - 1)];
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
    SHIM_OUT(p->base + 4, SHIM_IN(p->base + 4) | 0x01);
    return RS232ERR_NONE;
}

int ser_rs232_dtr_off(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    SHIM_OUT(p->base + 4, SHIM_IN(p->base + 4) & ~0x01);
    return RS232ERR_NONE;
}

int ser_rs232_rts_on(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    SHIM_OUT(p->base + 4, SHIM_IN(p->base + 4) | 0x02);
    return RS232ERR_NONE;
}

int ser_rs232_rts_off(unsigned int port)
{
    pcbdcom_port_t *p = port_by_num(port);
    if (!p) return RS232ERR_PARAM;
    SHIM_OUT(p->base + 4, SHIM_IN(p->base + 4) & ~0x02);
    return RS232ERR_NONE;
}
