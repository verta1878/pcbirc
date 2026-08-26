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


#if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#define SDATA 1
#else
#define LDATA 1
#endif

#if defined(__TINY__) || defined(__SMALL__) || defined(__COMPACT__)
#define SCODE 1
#else
#define LCODE 1
#endif

#ifdef _MSC_VER
#ifdef LDATA
  #define NEEDSEG
  #define NEEDSEGPUSHDS       _asm push ds
  #define NEEDSEGPOPDS        _asm pop  ds
  #define NEEDSEGGETDS(var)   _asm mov ax,seg var   _asm mov ds,ax
#else
  #undef  NEEDSEG
  #define NEEDSEGPUSHDS
  #define NEEDSEGPOPDS
  #define NEEDSEGGETDS
#endif
#else
  #undef  NEEDSEG
  #define NEEDSEGPUSHDS
  #define NEEDSEGPOPDS
  #define NEEDSEGGETDS
#endif
