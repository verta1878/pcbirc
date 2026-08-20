#ifndef _TERMIOS_H
#define _TERMIOS_H
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
#define NCCS 32
struct termios {
    tcflag_t c_iflag, c_oflag, c_cflag, c_lflag;
    cc_t c_cc[NCCS];
};
#define B300    300
#define B1200   1200
#define B2400   2400
#define B4800   4800
#define B9600   9600
#define B19200  19200
#define B38400  38400
#define B57600  57600
#define B115200 115200
#define B230400 230400
#define CS8     0x30
#define CLOCAL  0x800
#define CREAD   0x80
#define VMIN    6
#define VTIME   5
#define TCSANOW 0
#define ICANON  0x02
#define ECHO    0x08
#define TCIOFLUSH 2
int tcgetattr(int fd, struct termios *t);
int tcsetattr(int fd, int act, const struct termios *t);
int tcflush(int fd, int queue);
speed_t cfsetispeed(struct termios *t, speed_t s);
speed_t cfsetospeed(struct termios *t, speed_t s);
#endif
