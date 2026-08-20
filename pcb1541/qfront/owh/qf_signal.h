/* signal.h — OW cross-compilation stub for QFront
 * Provides POSIX signal constants for OpenWatcom. */
#ifndef _OWH_SIGNAL_H
#define _OWH_SIGNAL_H

/* Pull in OW's signal.h via the system path */
#include <sig_prt.h>

/* Signal numbers (POSIX) — define if OW didn't */
#ifndef SIGINT
#define SIGINT  2
#endif
#ifndef SIGTERM
#define SIGTERM 15
#endif
#ifndef SIGHUP
#define SIGHUP  1
#endif
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
#ifndef SIG_IGN
#define SIG_IGN ((void (*)(int))1)
#endif

/* signal() prototype if not declared */
#ifndef _SIGNAL_H_INCLUDED
typedef void (*__sighandler_t)(int);
extern __sighandler_t signal(int __sig, __sighandler_t __handler);
#endif

#endif /* _OWH_SIGNAL_H */
