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


/******************************************************************************/
/*                                                                            */
/*                                 TYPES.HPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                  Generic data types used by CDC projects.                  */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1995, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

#ifndef ___TYPES_HPP___

#define ___TYPES_HPP___

/******************************************************************************/

/* Included Files */

#include    <limits.h>

/******************************************************************************/

/* Defined Macros */

/* Confirm valid char size and bits in a char */
#if !defined(__WATCOMC__) && !defined(__TURBOC__)
#if ((sizeof(char) != 1) || (CHAR_BIT != 8))
#error The sizeof char is invalid!
#endif
#endif

/* Define cdcSHORT and cdcLONG to the most natural size modifiers for */
/* the system while ensuring cdcSHORT is 2 bytes and cdcLONG is 4 bytes */
#if defined(__TURBOC__)
  /* Turbo C 2.0: 16-bit DOS, int=2 long=4 */
  #define cdcSHORT
  #define cdcLONG     long
#elif !defined(__WATCOMC__)
#if   (sizeof(int) == 2) && (sizeof(long)  == 4)
  #define cdcSHORT
  #define cdcLONG     long
#elif (sizeof(int) == 4) && (sizeof(short) == 2)
  #define cdcSHORT    short
  #define cdcLONG
#else
  #error Unable to determine sizeof short/int/long types!
#endif
#else
  #define cdcSHORT    short
  #define cdcLONG     long
#endif

/* Macro to determine the bits in a given type */
#define bitsin(t) (sizeof(t) * CHAR_BIT)

/* Macros to calculate the min/max for   signed types (twos complement) */
#define minSType(t) t(t(1) << (bitsin(t) - 1))
#define maxSType(t) t(~minSType(t))

/* Macros to calculate the min/max for unsigned types */
#define minUType(t) t(t(0))
#define maxUType(t) t(~minUType(t))

/* Define NULL if it isn't already defined */
#ifndef NULL
#define NULL 0
#endif

/* LIBENTRY for function calling conventions */
/* CDCCONST: file-scope const has internal linkage in C++ but EXTERNAL   */
/* in C, so C builds (Turbo C) need 'static' to avoid multiply-defined   */
/* symbols when a header const is included by many modules.              */
#if defined(__cplusplus)
  #define CDCCONST const
#else
  #define CDCCONST static const
#endif

/* EXTERN_C: 'extern "C"' in C++, nothing in C (Turbo C) */
#if defined(__cplusplus)
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C
#endif

#ifndef LIBENTRY
  #ifdef __WATCOMC__
        #define LIBENTRY
  #else
        #define LIBENTRY pascal
  #endif
#endif

/* _NEAR_ is used for local functions in large code models */
#ifndef _NEAR_
  #ifdef __OS2__
        #define _NEAR_
  #else
        #define _NEAR_ near
  #endif
#endif

/* _FAR_ is used for far data pointers */
#ifndef _FAR_
  #ifdef __OS2__
        #define _FAR_
  #else
        #define _FAR_ far
  #endif
#endif

/******************************************************************************/

/* Types */

typedef   signed          char sbyte;  /*   Signed  8-bit integer */
typedef unsigned          char ubyte;  /* Unsigned  8-bit integer */

typedef   signed cdcSHORT int  sint;   /*   Signed 16-bit integer */
typedef unsigned cdcSHORT int  uint;   /* Unsigned 16-bit integer */

typedef   signed cdcLONG  int  slong;  /*   Signed 32-bit integer */
typedef unsigned cdcLONG  int  ulong;  /* Unsigned 32-bit integer */

/*--------------------------------------------------------------------*/

/* Borland C 5.0 defines the 'bool' type internally (C++ only). */
/* Turbo C / C mode has no built-in bool, so define it there too.   */
#if defined(__cplusplus)
#if __BORLANDC__ < 0x500
typedef ubyte bool;
#endif
#else
typedef ubyte bool;
#endif

/*--------------------------------------------------------------------*/

#if defined(__OS2__)

typedef unsigned long handle;

#else

typedef unsigned handle;

#endif

/******************************************************************************/

/* Constants */

#if defined(__cplusplus)   /* C++-only file-scope constants (unused in C mode) */

const char NUL = '\0';

        /*--------------------------------------------------------------------*/

const int      minInt      = minSType(int);
const int      maxInt      = maxSType(int);

const unsigned minUnsigned = minUType(unsigned);
const unsigned maxUnsigned = maxUType(unsigned);

        /*--------------------------------------------------------------------*/

const sbyte    minSByte    = minSType(sbyte);
const sbyte    maxSByte    = maxSType(sbyte);

const ubyte    minUByte    = minUType(ubyte);
const ubyte    maxUByte    = maxUType(ubyte);

const sint     minSInt     = minSType(sint);
const sint     maxSInt     = maxSType(sint);

const uint     minUInt     = minUType(uint);
const uint     maxUInt     = maxUType(uint);

const slong    minSLong    = minSType(slong);
const slong    maxSLong    = maxSType(slong);

const ulong    minULong    = minUType(ulong);
const ulong    maxULong    = maxUType(ulong);

#endif  /* __cplusplus */

        /*--------------------------------------------------------------------*/

#if defined(__cplusplus)


/* Borland C 5.0 defines the 'true' identifier internally */
#if __BORLANDC__ < 0x500
const bool true  = 1;
#endif

/* Borland C 5.0 defines the 'false' identifier internally */
#if __BORLANDC__ < 0x500
const bool false = 0;
#endif

#ifndef TRUE
const bool TRUE  = 1;
#endif

#ifndef FALSE
const bool FALSE = 0;
#endif

#else

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif

    /*--------------------------------------------------------------------*/

#if defined(__OS2__)

#ifdef NULLHANDLE
CDCCONST handle nullHandle = NULLHANDLE;
#else
CDCCONST handle nullHandle = 0;
#endif

#else

CDCCONST handle nullHandle = 0;

#endif

/******************************************************************************/

/* Minimum/Maximum Value Functions */

#ifdef INCL_MINMAX

template <class T> const T & minimum ( const T & l, const T & r )
    { return ((l < r) ? l : r); }

template <class T> const T & maximum ( const T & l, const T & r )
    { return ((l > r) ? l : r); }

#endif /* ifdef INCL_MINMAX */

/******************************************************************************/

#endif /* ___TYPES_HPP___ */

