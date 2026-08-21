/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* WATCOMPAT.H - Borland C++ → OpenWatcom compatibility macros              */
/* Allows PCBoard source to compile with both Borland and Watcom compilers.  */
/* Written by: hexadecimal — pcbrevival project                             */
/* Version: 1.2 — Phase 0 additions for pcb-util                           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_WATCOMPAT
#define H_WATCOMPAT

#ifdef __WATCOMC__

/*------------------------------------------------------------------------*/
/* Calling convention and memory model macros                              */
/*------------------------------------------------------------------------*/
#define far
#define near
#define huge
#define _Cdecl
#define cdecl
#define pascal
#define interrupt __interrupt
#undef _FARDATA_
#define _FARDATA_
#define _FAR_
#define _FARCALL_
#define _NEAR_
#define _FAR_CALL_
#define _FAR_DATA_

#ifndef LIBENTRY
#define LIBENTRY
#endif

/*------------------------------------------------------------------------*/
/* Borland header equivalents                                              */
/*------------------------------------------------------------------------*/
#define farmalloc(n)      malloc(n)
#define farfree(p)        free(p)
#define farmemset(p,v,n)  memset(p,v,n)
#define farcoreleft()     (1024L*1024L)
#define coreleft()        (1024L*1024L)

/*------------------------------------------------------------------------*/
/* Borland inline register pseudo-variables                                */
/*------------------------------------------------------------------------*/
#include <i86.h>
#include <dos.h>
#ifndef _BORLAND_REGS_DEFINED
#define _BORLAND_REGS_DEFINED
static union REGS _borland_regs;
static unsigned short _ES_dummy;
#endif
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

/*------------------------------------------------------------------------*/
/* Borland types                                                           */
/*------------------------------------------------------------------------*/
#ifndef __cplusplus
typedef unsigned char bool;
#endif
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef atexit_t
typedef void (*atexit_t)(void);
#endif

/*------------------------------------------------------------------------*/
/* Borland DOS functions                                                   */
/*------------------------------------------------------------------------*/
#include <direct.h>
#define fnmerge(p,dr,di,f,e) _makepath(p,dr,di,f,e)
#define fnsplit(p,dr,di,f,e) _splitpath(p,dr,di,f,e)
#define MAXPATH _MAX_PATH
#define MAXDRIVE _MAX_DRIVE
#define MAXDIR _MAX_DIR
#define MAXFILE _MAX_FNAME
#define MAXEXT _MAX_EXT
#define WILDCARDS  0x01
#define EXTENSION  0x02
#define FILENAME   0x04
#define DIRECTORY  0x08
#define DRIVE      0x10

/*------------------------------------------------------------------------*/
/* Borland I/O                                                             */
/*------------------------------------------------------------------------*/
#include <fcntl.h>
#include <share.h>
#ifndef O_DENYALL
#define O_DENYALL  SH_DENYRW
#define O_DENYREAD SH_DENYRD
#define O_DENYNONE SH_DENYNO
#endif
#ifndef OPEN_RDONLY
#define OPEN_RDONLY  O_RDONLY
#define OPEN_WRONLY  O_WRONLY
#define OPEN_RDWR    O_RDWR
#define OPEN_NORMAL  0
#endif

/*------------------------------------------------------------------------*/
/* Borland CRT/conio compatibility                                         */
/*------------------------------------------------------------------------*/
#ifndef clrscr
#define clrscr() cls()
#endif

#ifndef BLACK
#define BLACK        0
#define BLUE         1
#define GREEN        2
#define CYAN         3
#define RED          4
#define MAGENTA      5
#define BROWN        6
#define LIGHTGRAY    7
#define DARKGRAY     8
#define LIGHTBLUE    9
#define LIGHTGREEN  10
#define LIGHTCYAN   11
#define LIGHTRED    12
#define LIGHTMAGENTA 13
#define YELLOW      14
#define WHITE       15
#endif

#ifndef _NOCURSOR
#define _NOCURSOR      0x2000
#define _NORMALCURSOR  0x0607
#define _SOLIDCURSOR   0x0007
#endif

#ifndef C80
#define C80    3
#define BW80   2
#define C4350  43
#endif


#ifndef textmode
#define textmode(m) /* no-op in 32-bit flat model */
#endif

/* textcolor/textbackground — global attribute tracking */
#ifndef _WATCOM_TEXTCOLOR
#define _WATCOM_TEXTCOLOR
static unsigned char _wc_textattr = 7;
#define textcolor(c)       (_wc_textattr = (_wc_textattr & 0xF0) | ((c) & 0x0F))
#define textbackground(c)  (_wc_textattr = (_wc_textattr & 0x0F) | (((c) & 0x0F) << 4))
#define textattr(a)        (_wc_textattr = (a))
#endif

/* text_info / gettextinfo */
#ifndef _TEXT_INFO_DEFINED
#define _TEXT_INFO_DEFINED
typedef struct text_info {
    unsigned char winleft, wintop, winright, winbottom;
    unsigned char attribute, normattr;
    unsigned char currmode;
    unsigned char screenheight, screenwidth;
    unsigned char curx, cury;
} text_info;
#define gettextinfo(ti) do { \
    (ti)->screenheight = 25; (ti)->screenwidth = 80; \
    (ti)->curx = 1; (ti)->cury = 1; (ti)->currmode = 3; \
    (ti)->attribute = 7; (ti)->normattr = 7; \
    (ti)->winleft = 1; (ti)->wintop = 1; \
    (ti)->winright = 80; (ti)->winbottom = 25; } while(0)
#endif

/*------------------------------------------------------------------------*/
/* Borland searchpath                                                      */
/*------------------------------------------------------------------------*/
#ifndef _WATCOM_SEARCHPATH
#define _WATCOM_SEARCHPATH
/* Borland searchpath */
#include <stdlib.h>
#define searchpath _watcom_searchpath
static char _sp_buf[260];
static char *_watcom_searchpath(const char *file) {
    _searchenv(file, "PATH", _sp_buf);
    return _sp_buf[0] ? _sp_buf : (char *)0;
}
#endif

/*------------------------------------------------------------------------*/
/* Borland getvect/setvect                                                 */
/*------------------------------------------------------------------------*/
#ifndef getvect
#define getvect(n)    _dos_getvect(n)
#define setvect(n,h)  _dos_setvect(n, (void (__interrupt *)())h)
#endif

/*------------------------------------------------------------------------*/
/* Borland dfree → Watcom _diskfree_t                                      */
/*------------------------------------------------------------------------*/
#ifndef dfree
#define dfree _diskfree_t
#define df_avail avail_clusters
#define df_total total_clusters
#define df_bsec  bytes_per_sector
#define df_sclus sectors_per_cluster
#define getdfree(drive, dtable) _dos_getdiskfree(drive, dtable)
#endif

/*------------------------------------------------------------------------*/
/* Borland values.h → limits.h                                             */
/*------------------------------------------------------------------------*/
#ifndef MAXINT
#include <limits.h>
#define MAXINT INT_MAX
#endif

/*------------------------------------------------------------------------*/
/* Borland _version                                                        */
/*------------------------------------------------------------------------*/
#include <stdlib.h>
#define _version ((_osminor << 8) | _osmajor)

/*------------------------------------------------------------------------*/
/* DOSFILE type (from dosfunc.h — cannot include dosfunc.h in C mode       */
/* because it pulls in types.hpp which has C++ class keyword)              */
/*------------------------------------------------------------------------*/
#ifndef _DOSFILE_DEFINED
#define _DOSFILE_DEFINED
typedef struct {
    int      handle;
    char     *buffer;
    unsigned bufsize;
    unsigned offset;
    unsigned end;
    char     status;
    long     curpos;
    char     last_op;
} DOSFILE;
#endif

#endif /* __WATCOMC__ */


/* Borland DOS functions */
#ifdef __WATCOMC__
#ifndef unlink
#include <io.h>
#define unlink(f) remove(f)
#endif
#ifndef getdisk
#define getdisk() (_getdrive() - 1)
#define setdisk(d) _chdrive((d) + 1)
#endif
#endif


/* Borland getverify/getcurdir */
#ifdef __WATCOMC__
#ifndef getverify
#define getverify() 0
#endif
#ifndef getcurdir
#define getcurdir(d,buf) _getdcwd(d, buf, 260)
#endif
#endif

#ifndef getcbrk
#define getcbrk() 0
#define setcbrk(v) /* no-op */
#endif

/* Borland window() — sets text window coordinates */
#ifdef __WATCOMC__
#ifndef _BORLAND_WINDOW
#define _BORLAND_WINDOW
static unsigned char _wc_winx1=1, _wc_winy1=1, _wc_winx2=80, _wc_winy2=25;
static void window(int left, int top, int right, int bottom) {
    _wc_winx1 = (unsigned char)left;
    _wc_winy1 = (unsigned char)top;
    _wc_winx2 = (unsigned char)right;
    _wc_winy2 = (unsigned char)bottom;
}
#endif
#endif

#endif /* H_WATCOMPAT */

/* Borland ftime compatibility — getftime/setftime */
#ifndef _FTIME_COMPAT
#define _FTIME_COMPAT
#include <dos.h>
struct ftime {
    unsigned ft_tsec  : 5;   /* two-second intervals */
    unsigned ft_min   : 6;
    unsigned ft_hour  : 5;
    unsigned ft_day   : 5;
    unsigned ft_month : 4;
    unsigned ft_year  : 7;   /* years since 1980 */
};

#ifdef __cplusplus
extern "C" {
#endif
static int getftime(int handle, struct ftime *ftimep) {
    unsigned date, time;
    if (_dos_getftime(handle, &date, &time) != 0) return -1;
    ftimep->ft_tsec  = time & 0x1F;
    ftimep->ft_min   = (time >> 5) & 0x3F;
    ftimep->ft_hour  = (time >> 11) & 0x1F;
    ftimep->ft_day   = date & 0x1F;
    ftimep->ft_month = (date >> 5) & 0x0F;
    ftimep->ft_year  = (date >> 9) & 0x7F;
    return 0;
}
static int setftime(int handle, struct ftime *ftimep) {
    unsigned date, time;
    time = ftimep->ft_tsec | (ftimep->ft_min << 5) | (ftimep->ft_hour << 11);
    date = ftimep->ft_day | (ftimep->ft_month << 5) | (ftimep->ft_year << 9);
    return (_dos_setftime(handle, date, time) == 0) ? 0 : -1;
}
#ifdef __cplusplus
}
#endif
#endif /* _FTIME_COMPAT */
