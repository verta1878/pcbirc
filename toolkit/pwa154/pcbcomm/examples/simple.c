/* simple.c — minimal pcbdcom SDK example.
 * Opens COM1 at 9600 8N1, echoes RX to TX.
 * Build: BCC -ml simple.c PCBDCOM_BL.OBJ  ->  simple.exe */
#include <stdio.h>
#include "PCBDCOM.H"

int main(void)
{
    struct port_param pp;
    unsigned char b;

    if (ser_rs232_init() != RS232ERR_NONE) return 1;

    pp.baud = 9600; pp.parity = 'N'; pp.data_bits = 8;
    pp.stop_bits = 1; pp.flow = 0; pp.buf_size = 512;
    if (ser_rs232_setup(1, &pp) != RS232ERR_NONE) return 2;

    ser_rs232_dtr_on(1);
    ser_rs232_rts_on(1);
    printf("Echoing COM1. Ctrl-C to exit.\n");
    for (;;)
        if (ser_rs232_getbyte(1, &b) == RS232ERR_NONE)
            ser_rs232_putbyte(1, &b);
}
