/* ============================================================================
 * PCBDCOM.H  pcbdcom SDK public API
 *
 * Drop-in replacement for WCSC's COMMDRV.OBJ. Provides the ser_rs232_*
 * API used by PCBoard's MODEMDRV.C. All structs, constants, and function
 * signatures match COMMDRV for binary compatibility.
 *
 * License: GPLv3.
 * ==========================================================================*/
#ifndef PCBDCOM_SDK_H
#define PCBDCOM_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

/* -------- Error codes -------- */
#define RS232ERR_NONE   0
#define RS232ERR_BUSY   1
#define RS232ERR_PARAM  2
#define RS232ERR_NOPORT 3

/* -------- Baud rate divisors (1.8432 MHz / 16 / baud) -------- */
#define BAUD300    384
#define BAUD1200    96
#define BAUD2400    48
#define BAUD4800    24
#define BAUD9600    12
#define BAUD19200    6
#define BAUD38400    3
#define BAUD57600    2
#define BAUD115200   1

/* -------- Data length -------- */
#define LENGTH_7  7
#define LENGTH_8  8

/* -------- Parity -------- */
#define PARITY_NONE  0
#define PARITY_ODD   1
#define PARITY_EVEN  2

/* -------- Flow control protocol -------- */
#define PROT_RTSRTS   1
#define PROT_XONXOFF  2

/* -------- Card types -------- */
#define CARD_8250      0
#define CARD_BOCA      1
#define CARD_CYCLOM    2
#define CARD_DIGPCXE   3
#define CARD_DIGCXI    4
#define CARD_ROCKET    5
#define CARD_EASYIO    6
#define CARD_ARNET     7

/* -------- Open Port Control Block (opcb_type) -------- */
/* MODEMDRV.C accesses opcb->msr_reg, opcb->inbuf_count,
 * opcb->outbuf_count, opcb->cardtype, opcb->flag */
typedef struct {
    unsigned char  msr_reg;        /* modem status register shadow      */
    unsigned char  flag;           /* state flags (XMTOFF_STATE etc.)   */
    unsigned char  cardtype;       /* CARD_* constant                   */
    unsigned char  reserved1;
    unsigned int   inbuf_count;    /* bytes available in RX buffer      */
    unsigned int   outbuf_count;   /* bytes pending in TX buffer        */
    unsigned char  padding[24];
} opcb_type;

/* -------- Auxiliary Port Control Block (auxpcb_type) -------- */
/* MODEMDRV.C reads aux_frmint, aux_ovrint, aux_parint for error stats */
typedef struct {
    int aux_frmint;    /* framing error count   */
    int aux_ovrint;    /* overrun error count   */
    int aux_parint;    /* parity error count    */
} auxpcb_type;

/* -------- Port Parameters (port_param) -------- */
/* Binary-compatible with COMMDRV. MODEMDRV.C accesses all fields. */
struct port_param {
    unsigned int   baud;           /* baud rate divisor (BAUD* constant) */
    unsigned char  parity;         /* PARITY_* constant                  */
    unsigned char  data_bits;      /* 5..8                               */
    unsigned char  stop_bits;      /* 1 or 2                             */
    unsigned char  flow;           /* 0=none, 1=RTS/CTS, 2=XON/XOFF     */
    unsigned int   buf_size;       /* RX buffer size                     */
    unsigned char  lngth;          /* LENGTH_7 or LENGTH_8               */
    unsigned char  cardtype;       /* CARD_* constant                    */
    unsigned char  protocol;       /* PROT_* constant                    */
    int            error;          /* last error code                    */
    unsigned int   outbuf_len;     /* TX buffer size                     */
    unsigned int   inbuf_len;      /* RX buffer size                     */
    unsigned char  block[4];       /* block mode flags                   */
    opcb_type     *opcb;           /* pointer to open port control block */
    auxpcb_type   *auxpcb;         /* pointer to aux control block       */
    unsigned char  misc[16];       /* reserved                           */
};

/* -------- pcbdcom internal port state -------- */
/* Forward-declared for backend.h; full definition here. */

#define PCBDCOM_MAX_PORTS  32
#define PCBDCOM_RX_RING   4096
#define PCBDCOM_TX_RING   2048
#define PCBDCOM_BUF_SIZE  PCBDCOM_RX_RING

struct pcbdcom_backend;  /* forward */

typedef struct pcbdcom_port {
    /* Hardware */
    unsigned int   base;           /* I/O base address                  */
    unsigned char  irq;            /* IRQ number                        */
    unsigned char  subport;        /* sub-port on multiport card        */
    unsigned char  chip;           /* UART chip type (8250/16550)       */
    unsigned char  open;           /* port is open flag                 */
    unsigned char  lcr;            /* line control register shadow      */

    /* Serial parameters */
    unsigned long  baud;
    unsigned char  parity;
    unsigned char  data_bits;
    unsigned char  stop_bits;
    unsigned char  flow;
    unsigned int   buf_size;

    /* Ring buffers (pointers set by pcbdcom.c from static arenas) */
    unsigned char *rx_buf;
    unsigned int   rx_head;
    unsigned int   rx_tail;
    unsigned int   rx_size;
    unsigned char *tx_buf;
    unsigned int   tx_head;
    unsigned int   tx_tail;
    unsigned int   tx_size;

    /* Backend */
    const struct pcbdcom_backend *backend;
    void          *backend_data;   /* per-backend private state          */
    void          *card_state;     /* per-card shared state              */
    void          *isr;            /* saved ISR vector                   */

    /* COMMDRV compatibility — populated by shim for MODEMDRV.C access */
    opcb_type      compat_opcb;    /* embedded opcb for this port        */
    auxpcb_type    compat_auxpcb;  /* embedded auxpcb for this port      */
} pcbdcom_port_t;

/* -------- API functions -------- */
int  ser_rs232_init(void);
int  ser_rs232_setup(unsigned int port, struct port_param *pp);
int  ser_rs232_getport(unsigned int port, struct port_param *pp);
int  ser_rs232_getbyte(unsigned int port, unsigned char *b);
int  ser_rs232_putbyte(unsigned int port, unsigned char *b);
int  ser_rs232_getpacket(unsigned int port, unsigned int n, unsigned char *buf);
int  ser_rs232_putpacket(unsigned int port, unsigned int n, unsigned char *buf);
int  ser_rs232_viewpacket(unsigned int port, unsigned int n, unsigned char *buf);
int  ser_rs232_flush(unsigned int port, unsigned int which);
int  ser_rs232_dtr_on(unsigned int port);
int  ser_rs232_dtr_off(unsigned int port);
int  ser_rs232_rts_on(unsigned int port);
int  ser_rs232_rts_off(unsigned int port);

#ifdef __cplusplus
}
#endif

#endif /* PCBDCOM_SDK_H */
