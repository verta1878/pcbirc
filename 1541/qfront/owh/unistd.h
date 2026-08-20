#ifndef _UNISTD_H
#define _UNISTD_H
int close(int fd);
int read(int fd, void *buf, unsigned long count);
int write(int fd, const void *buf, unsigned long count);
unsigned int sleep(unsigned int seconds);
int usleep(unsigned long usec);
#endif
