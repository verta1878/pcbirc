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
/*									      */
/*                                CLEARBUF.HPP                                */
/*									      */
/*----------------------------------------------------------------------------*/
/*									      */
/*    Template functions to set a buffer of a fixed size to all NUL bytes     */
/*									      */
/*============================================================================*/
/*									      */
/*			 Written by Scott Dale Robison			      */
/*									      */
/*----------------------------------------------------------------------------*/
/*									      */
/*	      Copyright (C) 1995, Clark Development Company, Inc.	      */
/*									      */
/******************************************************************************/

#ifndef ___CLEARBUF_HPP___

#define ___CLEARBUF_HPP___

/******************************************************************************/

// Included Files

#include    <mem.h>

#include    <types.hpp>

/******************************************************************************/

// Clear Buffer Functions

template <class T>
inline void clearBuffer ( T & t )
{
    memset(&t,NUL,sizeof(t));
}

inline void clearBuffer ( void * p, unsigned s )
{
    memset(p,NUL,s);
}

/******************************************************************************/

#endif // ___CLEARBUF_HPP___

