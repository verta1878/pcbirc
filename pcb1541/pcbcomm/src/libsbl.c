/* ============================================================================
 * libsbl.c — COMM-DRV Serial Base Library replacement
 *
 * Clean-room implementation of WCSC's libsbl.lib. Based on analysis of
 * what PCBoard links against: MODEMDRV.C references port_param, opcb_block,
 * and aux_pcb structures but only accesses them through the 13 ser_rs232_*
 * functions in commdrbl.lib.
 *
 * libsbl.lib appears to be the "base" or "support" library that commdrbl.lib
 * depends on — common types, error handling, and low-level I/O primitives.
 * Since we implement everything in commdrbl.c using INT 14h calls (which
 * the TSR handles), libsbl.lib in our implementation provides only the
 * shared data structures and utility functions.
 *
 * Build:
 *   bcc -ml -c libsbl.c
 *   tlib libsbl.lib +libsbl.obj
 *
 * pcbirc crew (hexadecimal + sysop/0), GPLv3.
 * ==========================================================================*/

#include "comm.h"
#include <dos.h>

/* ---- Error description table ------------------------------------------- */

static const char *rs232_errors[] = {
    "No error",                    /* RS232ERR_NONE  */
    "Port busy",                   /* RS232ERR_BUSY  */
    "Invalid parameter",           /* RS232ERR_PARAM */
    "Port not found",              /* RS232ERR_NOPORT */
    "Initialization failed",       /* RS232ERR_INIT  */
    "Insufficient memory"          /* RS232ERR_NOMEM */
};

const char * LIBENTRY ser_rs232_strerror(int err)
{
    if (err < 0 || err > RS232ERR_NOMEM)
        return "Unknown error";
    return rs232_errors[err];
}

/* ---- FOSSIL detection -------------------------------------------------- */
/* Utility function: check if a FOSSIL driver is loaded on a given port.
 * Returns 1 if FOSSIL signature 0x1954 is returned, 0 otherwise.
 * Does not open/init the port — just probes. */

int LIBENTRY ser_rs232_detect(int port)
{
    union REGS r;
    r.h.ah = 0x04;       /* FOSSIL init */
    r.h.al = 0;
    r.x.dx = port;
    r.x.bx = 0;
    int86(0x14, &r, &r);

    if (r.x.ax == 0x1954) {
        /* Deinit immediately — we were just probing */
        r.h.ah = 0x05;
        r.x.dx = port;
        int86(0x14, &r, &r);
        return 1;
    }
    return 0;
}

/* ---- Baud rate table --------------------------------------------------- */
/* Maps baud rate to FOSSIL divisor index (bits 7-5 of function 00h AL).
 * FOSSIL encoding differs from BIOS INT 14h. */

int LIBENTRY ser_rs232_baud_to_divisor(long baud)
{
    switch (baud) {
        case   300L: return 0x02;
        case   600L: return 0x03;
        case  1200L: return 0x04;
        case  2400L: return 0x05;
        case  4800L: return 0x06;
        case  9600L: return 0x07;
        case 19200L: return 0x00;
        case 38400L: return 0x01;
        default:     return 0x01;  /* default to 38400 */
    }
}

long LIBENTRY ser_rs232_divisor_to_baud(int divisor)
{
    static const long table[8] = {
        19200L, 38400L, 300L, 600L, 1200L, 2400L, 4800L, 9600L
    };
    return table[divisor & 0x07];
}

/* ---- Card name table --------------------------------------------------- */
/* Maps CARD_* constants to display names. Used by DRVSETUP-compatible
 * configuration tools. */

const char * LIBENTRY ser_rs232_cardname(int cardtype)
{
    switch (cardtype) {
        case CARD_NONE:      return "NONE";
        case CARD_8250:      return "8250";
        case CARD_BOCA:      return "BOCA";
        case CARD_BOCA16:    return "BOCA16";
        case CARD_CYCLOM:    return "CYCLOM";
        case CARD_DIGPCXE:   return "DIGI_PCXE";
        case CARD_DIGPCXI:   return "DIGI_PCXI";
        case CARD_DIGACCEL:  return "DIGI_ACCEL";
        case CARD_DIGCXI:    return "DIGI_CXI";
        case CARD_ROCKET:    return "ROCKET";
        case CARD_EASYIO:    return "EASYIO";
        case CARD_ARNET:     return "ARNET_SPP";
        case CARD_HUB6:      return "HUB6";
        case CARD_GTEK:      return "GTEK";
        case CARD_STALLION:  return "STALLION";
        case CARD_CHASE:     return "CHASE";
        case CARD_EQUINOX:   return "EQUINOX";
        default:             return "UNKNOWN";
    }
}

/* ---- Port parameter defaults ------------------------------------------- */

void LIBENTRY ser_rs232_defaults(struct port_param *pp)
{
    if (!pp) return;
    pp->baud     = 0x01;         /* 38400 divisor */
    pp->lngth    = LENGTH_8;
    pp->parity   = PARITY_NONE;
    pp->protocol = PROT_RTSRTS;
    pp->outbuf_len = 4096;
    pp->cardtype = CARD_8250;
    pp->error    = 0;
    pp->opcb     = (struct opcb_block *)0;
    pp->auxpcb   = (struct aux_pcb *)0;
}
