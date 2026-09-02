/* multiport.c — poll 4 ports, print incoming bytes with source ID.
 * Assumes PCBDCOM.CFG has COM1..COM4 configured.
 * Build: BCC -ml multiport.c PCBDCOM_BL.OBJ */
#include <stdio.h>
#include "PCBDCOM.H"

int main(void)
{
    unsigned int i;
    unsigned char b;
    ser_rs232_init();
    printf("Polling COM1..COM4. Ctrl-C to exit.\n");
    for (;;)
        for (i = 1; i <= 4; i++)
            if (ser_rs232_getbyte(i, &b) == RS232ERR_NONE)
                printf("[COM%u] 0x%02x\n", i, b);
}
