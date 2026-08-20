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
#ifndef __WATCOMC__
#if ((sizeof(char) != 1) || (CHAR_BIT != 8))
#error The sizeof char is invalid!
#endif
#endif

/* Define cdcSHORT and cdcLONG to the most natural size modifiers for */
/* the system while ensuring cdcSHORT is 2 bytes and cdcLONG is 4 bytes */
#ifndef __WATCOMC__
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
#ifdef __WATCOMC__
#define minSType(t) ((t)((t)(1) << (bitsin(t) - 1)))
#define maxSType(t) ((t)(~((t)((t)(1) << (bitsin(t) - 1)))))
#define minUType(t) ((t)(0))
#define maxUType(t) ((t)(~((t)(0))))
#else
#define minSType(t) t(t(1) << (bitsin(t) - 1))
#define maxSType(t) t(~minSType(t))
/* Macros to calculate the min/max for unsigned types */
#define minUType(t) t(t(0))
#define maxUType(t) t(~minUType(t))
#endif

/* Define NULL if it isn't already defined */
#ifndef NULL
#define NULL 0
#endif

/* LIBENTRY for function calling conventions */
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

// Borland C 5.0 and Watcom C++ define the 'bool' type internally
#if !defined(__WATCOMC__) && (!defined(__BORLANDC__) || __BORLANDC__ < 0x500)
#ifndef __cplusplus
typedef ubyte bool;
#endif
#endif

/*--------------------------------------------------------------------*/

#if defined(__OS2__)

typedef unsigned long handle;

#else

typedef unsigned handle;

#endif

/******************************************************************************/

/* Constants */

const char NUL = '\0';

        /*--------------------------------------------------------------------*/

const int      minInt      = minSType(int);        //lint !e648
const int      maxInt      = maxSType(int);        //lint !e648

const unsigned minUnsigned = minUType(unsigned);
const unsigned maxUnsigned = maxUType(unsigned);

        /*--------------------------------------------------------------------*/

const sbyte    minSByte    = minSType(sbyte);
const sbyte    maxSByte    = maxSType(sbyte);

const ubyte    minUByte    = minUType(ubyte);
const ubyte    maxUByte    = maxUType(ubyte);

const sint     minSInt     = minSType(sint);       //lint !e648
const sint     maxSInt     = maxSType(sint);       //lint !e648

const uint     minUInt     = minUType(uint);
const uint     maxUInt     = maxUType(uint);

const slong    minSLong    = minSType(slong);      //lint !e648
const slong    maxSLong    = maxSType(slong);      //lint !e648

const ulong    minULong    = minUType(ulong);
const ulong    maxULong    = maxUType(ulong);

        /*--------------------------------------------------------------------*/

#if defined(__cplusplus)


// Borland C 5.0 and Watcom C++ define true/false internally
#if !defined(__WATCOMC__) && (!defined(__BORLANDC__) || __BORLANDC__ < 0x500)
const bool true  = 1;
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
const handle nullHandle = NULLHANDLE;
#else
const handle nullHandle = 0;
#endif

#else

const handle nullHandle = 0;

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

