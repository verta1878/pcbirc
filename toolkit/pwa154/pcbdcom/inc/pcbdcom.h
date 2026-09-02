/* ============================================================================
 * PCBDCOM.H — pcbdcom SDK public API
 *
 * For applications that link against PCBDCOM.OBJ as a drop-in replacement
 * for WCSC's COMMDRV.OBJ. Provides the 13-function ser_rs232_* API used by
 * PCBoard's MODEMDRV.C and other COMM-DRV-aware software.
 *
 * License: GPLv3.
 * ==========================================================================*/
#ifndef PCBDCOM_SDK_H
#define PCBDCOM_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#define RS232ERR_NONE   0
#define RS232ERR_BUSY   1
#define RS232ERR_PARAM  2
#define RS232ERR_NOPORT 3

struct port_param {
    unsigned int  baud;
    unsigned char parity;
    unsigned char data_bits;
    unsigned char stop_bits;
    unsigned char flow;
    unsigned int  buf_size;
    unsigned char misc[16];
};

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

#endif
