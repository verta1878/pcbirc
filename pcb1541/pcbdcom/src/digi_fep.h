/* ============================================================================
 * digi_fep.h — Shared FEP protocol for DigiBoard PC/Xe + AccelePort
 *
 * Both card families use the same FEP (Front End Processor)
 * shared-memory protocol from Linux drivers/char/epca.c. Only card-
 * detection and (for AccelePort) concentrator handling differ.
 *
 * License: GPLv3 (pcbirc crew) / GPLv2 upstream (Digi International)
 * ==========================================================================*/
#ifndef DIGI_FEP_H
#define DIGI_FEP_H

#include "pcbdcom.h"

/* ----- Card window offsets ----- */
#define CSTART      0x0400L
#define CMAX_OFF    0x0800L
#define ISTART      0x0800L
#define IMAX_OFF    0x0C00L
#define GLOBAL      0x0D10L
#define CHANSTRUCT  0x1000L
#define RXTXBUF     0x4000L
#define NPORT_OFF   0x0C22L
#define CARDTYPE_OFF 0x0C02L    /* AccelePort card-type ID */

/* ----- Mailbox u16 offsets from GLOBAL ----- */
#define MB_CIN      0x00
#define MB_COUT     0x04
#define MB_CMAX     0x0C
#define MB_EIN      0x10
#define MB_EOUT     0x14
#define MB_IMAX     0x1C

/* ----- board_chan struct offsets (per-channel, stride = BC_STRIDE) ----- */
#define BC_STRIDE   0x50
#define BC_TSEG     0x08
#define BC_TIN      0x0A
#define BC_TOUT     0x0C
#define BC_TMAX     0x0E
#define BC_RSEG     0x10
#define BC_RIN      0x12
#define BC_ROUT     0x14
#define BC_RMAX     0x16
#define BC_CFLAG    0x2C
#define BC_MSTAT    0x40

/* ----- FEP command opcodes ----- */
#define PAUSETX     0xE3
#define RESUMETX    0xE4
#define SETMODEM    0xE9
#define SETIFLAGS   0xEA
#define PAUSERX     0xEE
#define RESUMERX    0xEF

/* ----- Event bits ----- */
#define BREAK_IND    0x01
#define LOWTX_IND    0x02
#define EMPTYTX_IND  0x04
#define DATA_IND     0x08
#define MODEMCHG_IND 0x20

#define FEPTIMEOUT  200000L

/* ----- Per-card state (opaque to backends; passed via backend_data) ----- */
typedef struct digi_fep_card {
    unsigned long  addr;           /* pool: card_seg (as ul)               */
    unsigned char  in_use;         /* pool                                 */
    unsigned int   card_seg;
    unsigned char  n_ports;
    unsigned char  card_type;      /* For AccelePort: distinguishes model  */
    pcbdcom_port_t *ports[64];     /* Max concentrator size                */
} digi_fep_card_t;

/* Shared card pool for both PC/Xe + AccelePort backends */
void *digi_fep_card_get(unsigned long card_seg);

/* Board-channel base */
#define BC_BASE(chan) (CHANSTRUCT + (unsigned long)(chan) * BC_STRIDE)

/* ----- Shared FEP API ----- */
unsigned char digi_fep_readb (unsigned int seg, unsigned long off);
void          digi_fep_writeb(unsigned int seg, unsigned long off, unsigned char v);
unsigned int  digi_fep_readw (unsigned int seg, unsigned long off);
void          digi_fep_writew(unsigned int seg, unsigned long off, unsigned int v);
int           digi_fep_cmd   (unsigned int seg, unsigned char chan,
                              unsigned char op, unsigned char a1, unsigned char a2);
int           digi_fep_init_channel(pcbdcom_port_t *p, unsigned char chan);
void          digi_fep_deinit_channel(pcbdcom_port_t *p, unsigned char chan);
void          digi_fep_isr   (pcbdcom_port_t *p);
int           digi_fep_write (pcbdcom_port_t *p, const void *buf, int n);

#endif /* DIGI_FEP_H */
