#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H
#define FIONREAD 0x541B
#define TIOCMGET 0x5415
#define TIOCMSET 0x5418
#define TIOCM_DTR 0x002
#define TIOCM_CD  0x040
int ioctl(int fd, unsigned long request, ...);
#endif
