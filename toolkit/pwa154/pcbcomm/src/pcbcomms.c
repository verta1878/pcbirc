/* nopcbdcom_stub.c — empty ser_rs232_* implementations.
 * Build into NOPCBDCOM.OBJ for use when you need the symbols but not
 * the driver.
 * License: GPLv3. */
#define RS232ERR_NOPORT 3
struct port_param;

int  ser_rs232_init(void) { return RS232ERR_NOPORT; }
int  ser_rs232_setup(unsigned int p, struct port_param *pp)   { (void)p;(void)pp; return RS232ERR_NOPORT; }
int  ser_rs232_getport(unsigned int p, struct port_param *pp) { (void)p;(void)pp; return RS232ERR_NOPORT; }
int  ser_rs232_getbyte(unsigned int p, unsigned char *b)      { (void)p;(void)b;  return RS232ERR_NOPORT; }
int  ser_rs232_putbyte(unsigned int p, unsigned char *b)      { (void)p;(void)b;  return RS232ERR_NOPORT; }
int  ser_rs232_getpacket(unsigned int p, unsigned int n, unsigned char *b)  { (void)p;(void)n;(void)b; return RS232ERR_NOPORT; }
int  ser_rs232_putpacket(unsigned int p, unsigned int n, unsigned char *b)  { (void)p;(void)n;(void)b; return RS232ERR_NOPORT; }
int  ser_rs232_viewpacket(unsigned int p, unsigned int n, unsigned char *b) { (void)p;(void)n;(void)b; return RS232ERR_NOPORT; }
int  ser_rs232_flush(unsigned int p, unsigned int w)          { (void)p;(void)w;  return RS232ERR_NOPORT; }
int  ser_rs232_dtr_on(unsigned int p)  { (void)p; return RS232ERR_NOPORT; }
int  ser_rs232_dtr_off(unsigned int p) { (void)p; return RS232ERR_NOPORT; }
int  ser_rs232_rts_on(unsigned int p)  { (void)p; return RS232ERR_NOPORT; }
int  ser_rs232_rts_off(unsigned int p) { (void)p; return RS232ERR_NOPORT; }
