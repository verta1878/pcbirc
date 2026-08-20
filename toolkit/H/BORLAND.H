/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#ifndef H_BORLAND
#define H_BORLAND

/* these functions exist in Borland's run-time library - this header lets */
/* MSC and WATCOM compiled code make use of the same functions.           */
/*                                                                        */
/* Be sure to link with BORLAND.LIB in order to pull in the functions     */
/* found at the bottom of this file.                                      */

#ifdef __WATCOMC__
  /* Pull in full Borland→Watcom compatibility layer — hexadecimal/pcbrevival */
  #include "watcompat.h"
#endif

#define asm       _asm
#define disable   _disable
#define enable    _enable

#ifndef __WATCOMC__
  /* Watcom: coreleft mapped in watcompat.h to return large value */
  #define coreleft  _memavl
#endif

#define getvect   _dos_getvect
#define setvect   _dos_setvect

#ifdef _MSC_VER
  #define ffblk                   _find_t
#elif defined(__WATCOMC__)
  #define ffblk                   find_t
#endif

#define ff_name                   name
#define ff_fsize                  size
#define ff_fdate                  wr_date
#define ff_ftime                  wr_time
#define ff_attrib                 attrib
#define findfirst(str,dta,attr)   _dos_findfirst(str,attr,dta)
#define findnext(dta)             _dos_findnext(dta)

#define FA_ARCH                   _A_ARCH
#define FA_NORMAL                 _A_NORMAL
#define FA_HIDDEN                 _A_HIDDEN
#define FA_RDONLY                 _A_RDONLY
#define FA_SYSTEM                 _A_SYSTEM
#define FA_DIREC                  _A_SUBDIR
#define FA_SUBDIR                 _A_SUBDIR

#ifndef __WATCOMC__
  /* Watcom: farmalloc/farfree mapped in watcompat.h to malloc/free */
  #ifndef farmalloc
    #define farmalloc _fmalloc
    #define farfree   _ffree
  #endif
#endif

/* these function prototypes are for functions found in BORLAND.LIB */

#ifdef _MSC_VER
  int  __cdecl getcbrk(void);
  int  __cdecl setcbrk(int cbrkvalue);
  void __cdecl nosound(void);
  void __cdecl sound(unsigned frequency);
  unsigned long __cdecl farcoreleft(void);
#endif

#endif /* ifndef H_BORLAND */
