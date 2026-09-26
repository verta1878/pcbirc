/* tsrless.c — pcbdcom without the TSR.
 * Links driver code directly. Bigger binary, no separate TSR install.
 * Build: BCC -ml tsrless.c PCBDCOM_BL.OBJ */
#include <stdio.h>
#include "PCBDCOM.H"

int main(int argc, char **argv)
{
    unsigned char b;
    (void)argc; (void)argv;
    ser_rs232_init();
    printf("Standalone pcbdcom. Reading COM1...\n");
    while (ser_rs232_getbyte(1, &b) == RS232ERR_NONE)
        putchar(b);
    return 0;
}
