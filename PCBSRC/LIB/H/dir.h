/* dir.h — Borland compatibility shim for Watcom */
#ifndef _DIR_H_COMPAT
#define _DIR_H_COMPAT
#ifdef __WATCOMC__
/* Watcom: no struct ffblk, no findfirst/findnext from dir.h.
   PCBoard FidoNet code uses find_t from dos.h instead. */
#ifndef FA_ARCH
#define FA_ARCH _A_ARCH
#endif
#ifndef FA_RDONLY
#define FA_RDONLY _A_RDONLY
#endif
#ifndef FA_DIREC
#define FA_DIREC _A_SUBDIR
#endif
#ifndef FA_HIDDEN
#define FA_HIDDEN _A_HIDDEN
#endif
#ifndef FA_SYSTEM
#define FA_SYSTEM _A_SYSTEM
#endif
#else
/* Borland: dir.h is native — this shim not used */
#endif
#endif
