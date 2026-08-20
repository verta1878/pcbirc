/* sys/wait.h stub for OpenWatcom linux64 target */
#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H
#define WIFEXITED(s)  (((s) & 0xFF) == 0)
#define WEXITSTATUS(s) (((s) >> 8) & 0xFF)
typedef int pid_t;
#endif
