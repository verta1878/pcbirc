/* TIMER.C — Replaces TIMER.ASM. Timer tick functions. */
#include <dos.h>
#include <time.h>

#pragma aux GETTICKS "*"
#pragma aux SETTIMER "*"
#pragma aux GETTIMER "*"

static long timer_base = 0;

long GETTICKS(void) {
    /* Read BIOS tick count at 0040:006C */
    return *(long far *)MK_FP(0x40, 0x6C);
}

void SETTIMER(void) {
    timer_base = GETTICKS();
}

long GETTIMER(void) {
    return GETTICKS() - timer_base;
}
