/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* WATCOMPAT.H - Borland C++ → OpenWatcom compatibility macros              */
/* Allows PCBoard source to compile with both Borland and Watcom compilers.  */
/* Written by: hexadecimal, v0.036                                          */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_WATCOMPAT
#define H_WATCOMPAT

#ifdef __WATCOMC__

/* Borland headers → Watcom equivalents */
#define alloc_h_included
#include <malloc.h>     /* replaces alloc.h: farmalloc→_fmalloc etc */
#include <direct.h>     /* replaces dir.h */

/* Borland memory functions → Watcom */
#define farmalloc(n)     malloc(n)     /* flat model: no far/near distinction */
#define farfree(p)       free(p)
#define farrealloc(p,n)  realloc(p,n)
#define coreleft()       0

/* Borland far/near qualifiers — no-ops in flat model */
#ifndef far
#define far
#endif
#ifndef near
#define near
#endif
#ifndef huge
#define huge
#endif
#ifndef _FAR_
#define _FAR_
#endif
#ifndef _FARDATA_
#define _FARDATA_
#endif

/* Calling conventions: Borland pascal → Watcom __pascal */
/* In OS/2 flat model, pascal is not used; LIBENTRY is empty */

/* Borland #pragma option → ignore */
#pragma disable_message(202)   /* suppress "unreferenced" warnings */

/* _fmemcpy etc — in flat model, just use memcpy */
#define _fmemcpy   memcpy
#define _fmemset   memset
#define _fmemcmp   memcmp
#define _fstrcpy   strcpy
#define _fstrlen   strlen

/* Borland constrea.h → Watcom has iostream.h */
/* These will need per-file handling */

#endif /* __WATCOMC__ */
#endif /* H_WATCOMPAT */
