/* termios stubs for DOS — qfconfig keyboard input */
#ifdef __WATCOMC__
#include "termios.h"
#include <conio.h>

int tcgetattr(int fd, struct termios *t) {
    (void)fd; (void)t;
    return 0;
}

int tcsetattr(int fd, int act, const struct termios *t) {
    (void)fd; (void)act; (void)t;
    return 0;
}

int tcflush(int fd, int queue) {
    (void)fd; (void)queue;
    return 0;
}

speed_t cfsetispeed(struct termios *t, speed_t s) { (void)t; return s; }
speed_t cfsetospeed(struct termios *t, speed_t s) { (void)t; return s; }
#endif
