/* pcbcp_compat.h — OpenWatcom compatibility for PCBCP */
#ifndef PCBCP_COMPAT_H
#define PCBCP_COMPAT_H

#ifndef __cplusplus
typedef unsigned char bool;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

/* Watcom doesn't have alloc.h */
#ifdef __WATCOMC__
#include <stdlib.h>
#include <malloc.h>
#endif

#endif
