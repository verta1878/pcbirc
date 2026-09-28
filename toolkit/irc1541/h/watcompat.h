/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* WATCOMPAT.H - Borland C++ → OpenWatcom compatibility macros              */
/* Allows PCBoard source to compile with both Borland and Watcom compilers.  */
/* Written by: hexadecimal — pcbrevival project                             */
/* Version: 1.1 — Phase 1.1 OpenWatcom port                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_WATCOMPAT
#define H_WATCOMPAT

#ifdef __WATCOMC__

/*------------------------------------------------------------------------*/
/* Memory model qualifiers — no-ops in Watcom flat model                  */
/*------------------------------------------------------------------------*/
#ifndef far
#define far
#endif
#ifndef near
#define near
#endif
#ifndef huge
#define huge
#endif
#undef _FAR_
#define _FAR_
#undef _FARDATA_
#define _FARDATA_

/*------------------------------------------------------------------------*/
/* Far memory functions → flat equivalents                                */
/*------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#define farmalloc(n)      malloc(n)
#define farfree(p)        free(p)
#define farrealloc(p,n)   realloc(p,n)
#define farcalloc(n,s)    calloc(n,s)
#define farcoreleft()     ((unsigned long)0x7FFFFFFFUL)

#ifndef coreleft
#define coreleft()        ((unsigned long)0x7FFFFFFFUL)
#endif

/* Far string/memory — same as near in flat model */
#define _fmemcpy    memcpy
#define _fmemset    memset
#define _fmemcmp    memcmp
#define _fmemmove   memmove
#define _fstrcpy    strcpy
#define _fstrlen    strlen
#define _fstrcmp    strcmp
#define _fstrcat    strcat
#define _fstrchr    strchr
#define _fstrrchr   strrchr

/*------------------------------------------------------------------------*/
/* BIOS keyboard — bioskey() → _bios_keybrd()                            */
/*------------------------------------------------------------------------*/
#include <bios.h>
#define bioskey(cmd)  _bios_keybrd(cmd)

/*------------------------------------------------------------------------*/
/* Disk/drive functions                                                   */
/* Borland: 0=A,1=B,2=C. Watcom: 1=A,2=B,3=C. Adjust by 1.              */
/*------------------------------------------------------------------------*/
#include <direct.h>
#include <dos.h>

static inline int getdisk(void) {
    unsigned d;
    _dos_getdrive(&d);
    return (int)(d - 1);
}
static inline int setdisk(int d) {
    unsigned total;
    _dos_setdrive((unsigned)(d + 1), &total);
    return (int)total;
}

/*------------------------------------------------------------------------*/
/* stpcpy — copies string, returns pointer to terminating null            */
/*------------------------------------------------------------------------*/
static inline char *stpcpy(char *dest, const char *src) {
    while ((*dest = *src) != '\0') { dest++; src++; }
    return dest;
}

/*------------------------------------------------------------------------*/
/* interrupt keyword                                                      */
/*------------------------------------------------------------------------*/
#ifndef interrupt
#define interrupt __interrupt
#endif

/*------------------------------------------------------------------------*/
/* Calling convention                                                     */
/*------------------------------------------------------------------------*/
#ifndef pascal
#define pascal __pascal
#endif

/*------------------------------------------------------------------------*/
/* Borland pragmas — suppress common warnings                             */
/*------------------------------------------------------------------------*/
#pragma disable_message(202)  /* unreferenced parameter */
#pragma disable_message(203)  /* unreferenced variable */

/*------------------------------------------------------------------------*/
/* Interrupt support — include i86.h for int86(), REGS, sound/nosound     */
/* Watcom provides sound()/nosound()/delay() natively in i86.h            */
/*------------------------------------------------------------------------*/
#include <i86.h>

/*------------------------------------------------------------------------*/
/* Pseudo-registers and geninterrupt                                      */
/*                                                                        */
/* NOT macro'd here — too many source files use _AX/_AL etc. as local     */
/* variable names inside inline asm blocks. Defining them as macros would */
/* break those files. The 2 files that use true Borland pseudo-registers  */
/* (GIVEUP.C, MODEMFOS.C) are converted manually to use int86() + REGS.  */
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
/* __emit__() — Borland inline byte emission                              */
/* Used in EVALP.CPP only. Must be converted per-use to Watcom aux pragma.*/
/*------------------------------------------------------------------------*/


/*------------------------------------------------------------------------*/
/* Borland dir.h constants → Watcom limits.h                              */
/*------------------------------------------------------------------------*/
#include <limits.h>
#ifndef MAXPATH
#define MAXPATH   _MAX_PATH
#endif
#ifndef MAXDIR
#define MAXDIR    _MAX_DIR
#endif
#ifndef MAXDRIVE
#define MAXDRIVE  _MAX_DRIVE
#endif
#ifndef MAXFILE
#define MAXFILE   _MAX_FNAME
#endif
#ifndef MAXEXT
#define MAXEXT    _MAX_EXT
#endif

/*------------------------------------------------------------------------*/
/* Borland CRT odds and ends                                              */
/*------------------------------------------------------------------------*/
#define setcbrk(x)    /* no-op on Watcom */
#define getcbrk()     0

/* unlink() — Watcom has it in io.h */
#include <io.h>


/* Borland _argv/_argc → Watcom __argv/__argc */
#define _argv  __argv
#define _argc  __argc

/* O_DENYNONE — Borland share mode constant */
#include <share.h>
#ifndef O_DENYNONE
#define O_DENYNONE  SH_DENYNO
#endif
#ifndef O_DENYALL
#define O_DENYALL   SH_DENYRW
#endif

/* getcurdir — Borland dir.h function */
/* int getcurdir(int drive, char *directory) */
/* Watcom: _getdcwd() is the closest equivalent */


/* randomize() — Borland CRT, seeds srand with current time */
#define randomize()  srand(0)



/* bool for Watcom C mode (wcc386) — C++ has it built-in, C does not   */
/* sysop/0: Clark's TYPES.HPP skips bool typedef under __WATCOMC__,    */
/* but wcc386 (C mode) doesn't define bool natively.                   */
#ifndef __cplusplus
#ifndef _BOOL_DEFINED
#define _BOOL_DEFINED
typedef unsigned char bool;
#define true  1
#define false 0
#define TRUE  1
#define FALSE 0
#endif
#endif


/*------------------------------------------------------------------------*/
/* int86/int86x → int386/int386x                                          */
/* Watcom 32-bit flat model uses int386 instead of int86.                  */
/* int86 is only available for 16-bit targets (_M_I86).                   */
/*------------------------------------------------------------------------*/
#ifndef _M_I86
#define int86(n,i,o)     int386(n,i,o)
#define int86x(n,i,o,s)  int386x(n,i,o,s)
#endif

/* geninterrupt(N) → int386(N, &regs, &regs) via global REGS */
/* Only for the 2 files that use true Borland pseudo-register pattern */


/* _version — Borland DOS version global */
/* Borland: _version = (minor << 8) | major */
#include <stdlib.h>
#define _version ((_osminor << 8) | _osmajor)

#endif /* __WATCOMC__ */
#endif /* H_WATCOMPAT */

/* Borland pseudo-register compatibility for geninterrupt() calls */
#ifdef __WATCOMC__
#include <i86.h>
static union REGS _borland_regs;
static unsigned short _ES_dummy;
#define _AH _borland_regs.h.ah
#define _AL _borland_regs.h.al
#define _AX _borland_regs.w.ax
#define _BH _borland_regs.h.bh
#define _BL _borland_regs.h.bl
#define _BX _borland_regs.w.bx
#define _CH _borland_regs.h.ch
#define _CL _borland_regs.h.cl
#define _CX _borland_regs.w.cx
#define _DH _borland_regs.h.dh
#define _DL _borland_regs.h.dl
#define _DX _borland_regs.w.dx
#define _SI _borland_regs.w.si
#define _DI _borland_regs.w.di
#define _ES _ES_dummy
#define geninterrupt(n) int386(n, &_borland_regs, &_borland_regs)
#ifndef FP_SEG
#define FP_SEG(p) 0
#endif
#ifndef FP_OFF
#define FP_OFF(p) ((unsigned)(p))
#endif
#endif

/* Borland struct date/time → Watcom dosdate_t/dostime_t */
#ifndef _BDATE_COMPAT
#define _BDATE_COMPAT
struct date { int da_year; char da_day; char da_mon; };
struct time { unsigned char ti_min; unsigned char ti_hour; unsigned char ti_hund; unsigned char ti_sec; };
#define getdate(p) { struct dosdate_t _d; _dos_getdate(&_d); (p)->da_year=_d.year; (p)->da_mon=_d.month; (p)->da_day=_d.day; }
#define gettime(p) { struct dostime_t _t; _dos_gettime(&_t); (p)->ti_hour=_t.hour; (p)->ti_min=_t.minute; (p)->ti_sec=_t.second; (p)->ti_hund=_t.hsecond; }
#endif

/* Borland conio text_info / video mode constants */
#ifndef _TEXTINFO_COMPAT
#define _TEXTINFO_COMPAT
#ifndef C40
#define C40 1
#define C80 3
#define C4350 64
#endif
struct text_info {
    unsigned char winleft, wintop, winright, winbottom;
    unsigned char attribute, normattr;
    unsigned char currmode;
    unsigned char screenheight, screenwidth;
    unsigned char curx, cury;
};
#define gettextinfo(p) memset(p, 0, sizeof(struct text_info)); (p)->currmode = C80; (p)->screenheight = 25; (p)->screenwidth = 80
#endif
#ifndef BW40
#define BW40 0
#define BW80 2
#define MONO 7
#endif
#ifndef setattr
#define setattr(a) /* no-op */
#define textattr(a) /* no-op */
#define textmode(m) /* no-op */
#define window(a,b,c,d) /* no-op */
#endif
#ifndef FONT_8x8
#define FONT_8x8 0
#define FONT_8x14 1
#endif
