/* sys/select.h — OW cross-compilation stub for QFront */
#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <sys/types.h>

/* fd_set — simplified for single-fd use */
#ifndef FD_SETSIZE
#define FD_SETSIZE 64
#endif

typedef struct {
    unsigned long fds_bits[FD_SETSIZE / (8 * sizeof(unsigned long))];
} fd_set;

#define FD_ZERO(set)      memset((set), 0, sizeof(fd_set))
#define FD_SET(fd, set)   ((set)->fds_bits[(fd) / (8*sizeof(unsigned long))] |= (1UL << ((fd) % (8*sizeof(unsigned long)))))
#define FD_CLR(fd, set)   ((set)->fds_bits[(fd) / (8*sizeof(unsigned long))] &= ~(1UL << ((fd) % (8*sizeof(unsigned long)))))
#define FD_ISSET(fd, set) ((set)->fds_bits[(fd) / (8*sizeof(unsigned long))] & (1UL << ((fd) % (8*sizeof(unsigned long)))))

struct timeval {
    long tv_sec;
    long tv_usec;
};

/* select() — declared here, linked from libc or FOSSIL layer */
extern int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);

#endif /* _SYS_SELECT_H */
