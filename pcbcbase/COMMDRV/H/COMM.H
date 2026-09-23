/* ============================================================================
 * comm.h — COMM-DRV/DOS compatible API header
 *
 * Clean-room reconstruction of WCSC COMM-DRV v15 SDK header.
 * Derived from the public API surface visible in Clark's MODEMDRV.C
 * (PCBoard 15.3/15.4 source) and the COMM-DRV runtime distribution
 * at pcb1541/install/dist/target/COMMDRV/.
 *
 * This header defines the types, constants, and function prototypes
 * needed to compile MODEMDRV.C and link against commdrbl.lib.
 *
 * pcbirc crew (hexadecimal + sysop/0), GPLv3.
 * ==========================================================================*/

#ifndef COMM_H
#define COMM_H

/* ---- Error codes ------------------------------------------------------- */

#define RS232ERR_NONE     0     /* success */
#define RS232ERR_BUSY     1     /* port busy / no data available */
#define RS232ERR_PARAM    2     /* invalid parameter */
#define RS232ERR_NOPORT   3     /* port not found / not configured */
#define RS232ERR_INIT     4     /* initialization failed */
#define RS232ERR_NOMEM    5     /* insufficient memory */

/* ---- Card type identifiers --------------------------------------------- */
/* These match the cardtype field in port_param and opcb.
 * Values derived from COMMDRV.EXE's DRVSETUP card selection menu
 * and the nine COMMDV00-08.DRV board drivers. */

#define CARD_NONE          0    /* port disabled */
#define CARD_8250          1    /* standard 8250/16450/16550 COM port */
#define CARD_BOCA          2    /* Boca BB-1004/1008 */
#define CARD_BOCA16        3    /* Boca BB-2016 (16-port) */
#define CARD_CYCLOM        4    /* Cyclades Cyclom-Y */
#define CARD_DIGPCXE       5    /* DigiBoard PC/Xe */
#define CARD_DIGPCXI       6    /* DigiBoard PC/Xi */
#define CARD_DIGACCEL      7    /* DigiBoard AccelePort */
#define CARD_DIGCXI        8    /* DigiBoard COM/Xi (80186-based) */
#define CARD_ROCKET        9    /* Comtrol RocketPort */
#define CARD_EASYIO       10    /* Stallion EasyIO */
#define CARD_ARNET        11    /* Arnet SmartPort Plus */
#define CARD_HUB6         12    /* Intel HUB-6 */
#define CARD_GTEK          13    /* GTEK BBS-550/8Fx */
#define CARD_STALLION     14    /* Stallion Brumby/ONboard */
#define CARD_CHASE        15    /* Chase Research IOLAN */
#define CARD_EQUINOX      16    /* Equinox SST */

/* ---- Line parameters --------------------------------------------------- */

#define LENGTH_5           5
#define LENGTH_6           6
#define LENGTH_7           7
#define LENGTH_8           8

#define PARITY_NONE        0
#define PARITY_ODD         1
#define PARITY_EVEN        2
#define PARITY_MARK        3
#define PARITY_SPACE       4

#define STOPBIT_1          1
#define STOPBIT_2          2

/* ---- Flow control protocol --------------------------------------------- */

#define PROT_NONE          0    /* no flow control */
#define PROT_XONXOFF       1    /* XON/XOFF software flow control */
#define PROT_RTSRTS        2    /* RTS/CTS hardware flow control */

/* ---- opcb flags -------------------------------------------------------- */

#define XMTOFF_STATE    0x01    /* transmit halted */

/* ---- Auxiliary PCB (error counters) ------------------------------------ */

struct aux_pcb {
    unsigned int aux_frmint;    /* framing error count */
    unsigned int aux_ovrint;    /* overrun error count */
    unsigned int aux_parint;    /* parity error count */
};

/* ---- Output Control Block (read-only for caller) ----------------------- */
/* Maintained by the COMM-DRV TSR. Caller reads these via pcb.opcb->. */

struct opcb_block {
    unsigned char  msr_reg;         /* modem status register (CTS/DSR/RI/DCD) */
    unsigned int   inbuf_count;     /* bytes waiting in RX buffer */
    unsigned int   outbuf_count;    /* bytes waiting in TX buffer */
    unsigned short cardtype;        /* card type (CARD_* constant) */
    unsigned char  flag;            /* driver flags (XMTOFF_STATE etc) */
};

/* ---- Port Parameter Block ---------------------------------------------- */
/* Passed to ser_rs232_setup(). The opcb and auxpcb pointers are filled
 * by the driver; the caller sets the configuration fields. */

struct port_param {
    unsigned int       baud;        /* baud rate divisor (from bauddivisor()) */
    unsigned char      lngth;       /* data bits (LENGTH_5..LENGTH_8) */
    unsigned char      parity;      /* parity (PARITY_NONE..PARITY_SPACE) */
    unsigned char      protocol;    /* flow control (PROT_*) */
    unsigned int       outbuf_len;  /* output buffer size reported by driver */
    unsigned short     cardtype;    /* card type to configure (CARD_*) */
    int                error;       /* error code from last operation */
    unsigned char      block[16];   /* card-specific configuration block */
    struct opcb_block *opcb;        /* → output control block (driver-owned) */
    struct aux_pcb    *auxpcb;      /* → auxiliary PCB with error counters */
};

/* ---- Calling convention ------------------------------------------------- */
/* PCBoard declares all library entries with LIBENTRY, which expands to
 * 'pascal' on Borland (uppercase symbols, callee-cleans-stack, @…$Q…
 * OMF mangling). Without it, BC 3.1 produces C-style _lowercase names
 * and none of the 13 symbols link. */

#ifndef LIBENTRY
# if defined(__BORLANDC__) || defined(__TURBOC__)
#  define LIBENTRY pascal
# elif defined(_MSC_VER)
#  define LIBENTRY _pascal
# elif defined(__WATCOMC__)
#  define LIBENTRY __pascal
# else
#  define LIBENTRY
# endif
#endif

/* ---- Function prototypes ----------------------------------------------- */
/* These are the 13 ser_rs232_* functions that MODEMDRV.C calls. */

#ifdef __cplusplus
extern "C" {
#endif

int  LIBENTRY ser_rs232_init       (void);
int  LIBENTRY ser_rs232_setup      (int port, struct port_param *pp);
int  LIBENTRY ser_rs232_getport    (int port, struct port_param *pp);
int  LIBENTRY ser_rs232_getbyte    (int port, unsigned char *b);
int  LIBENTRY ser_rs232_putbyte    (int port, unsigned char *b);
int  LIBENTRY ser_rs232_getpacket  (int port, int n, unsigned char *buf);
int  LIBENTRY ser_rs232_putpacket  (int port, int n, unsigned char *buf);
int  LIBENTRY ser_rs232_viewpacket (int port, int n, unsigned char *buf);
int  LIBENTRY ser_rs232_flush      (int port, int which);  /* 0=RX, 1=TX, 2=both */
int  LIBENTRY ser_rs232_dtr_on     (int port);
int  LIBENTRY ser_rs232_dtr_off    (int port);
int  LIBENTRY ser_rs232_rts_on     (int port);
int  LIBENTRY ser_rs232_rts_off    (int port);

#ifdef __cplusplus
}
#endif

#endif /* COMM_H */
