#ifndef _FCNTL_H
#define _FCNTL_H
#define O_RDWR    0x02
#define O_CREAT   0x40
#define O_EXCL    0x80
#define O_NOCTTY  0x100
#define O_NONBLOCK 0x800
#define O_WRONLY  0x01
#define F_SETFL   4
int open(const char *path, int flags, ...);
int fcntl(int fd, int cmd, ...);
int dprintf(int fd, const char *fmt, ...);
#endif
