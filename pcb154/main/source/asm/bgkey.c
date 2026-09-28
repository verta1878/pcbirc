/* BGKEY.C — Replaces BGKEY.ASM. Background keyboard check. */
#include <dos.h>
#include <conio.h>

#pragma aux BGETKEY2 "*"

int BGETKEY2(void) {
    if (kbhit())
        return getch();
    return 0;
}
