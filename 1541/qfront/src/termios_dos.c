/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* termios_dos.c -- POSIX termios stubs for DOS (Watcom)                    */
/*                                                                           */
/* Provides no-op implementations of tcgetattr, tcsetattr, tcflush,         */
/* cfsetispeed, and cfsetospeed so that code written for POSIX terminal      */
/* control compiles under DOS without #ifdef everywhere.                     */
/*                                                                           */
/* On DOS, keyboard input uses conio.h (getch/kbhit) instead of termios.    */
/* These stubs exist only to satisfy the linker.                             */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifdef __WATCOMC__
#include "termios.h"
#include <conio.h>

int tcgetattr(int Fd, struct termios *Term)
{
    (void)Fd; (void)Term;
    return 0;
}

int tcsetattr(int Fd, int Action, const struct termios *Term)
{
    (void)Fd; (void)Action; (void)Term;
    return 0;
}

int tcflush(int Fd, int Queue)
{
    (void)Fd; (void)Queue;
    return 0;
}

speed_t cfsetispeed(struct termios *Term, speed_t Speed)
{
    (void)Term;
    return Speed;
}

speed_t cfsetospeed(struct termios *Term, speed_t Speed)
{
    (void)Term;
    return Speed;
}

#endif
