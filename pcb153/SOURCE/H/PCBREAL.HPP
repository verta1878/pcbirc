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
/*                                PCBREAL.HPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*           Wrapper Classes for Automatic Type Conversions between           */
/*                 bassngl/basdble/pasreal and long variables                 */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1994, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

#ifndef ___PCBREAL_HPP___

#define ___PCBREAL_HPP___

/******************************************************************************/

// Included Files

#include    "misc.h"

/******************************************************************************/

// Types

class cBSREAL
{

    public:

        cBSREAL                 ( long i = 0 ) { *this = i; }
        cBSREAL & operator =    ( long i )     { longtobassngl(bs,i);
                                                 return *this; }
                  operator long ( void )       { return bassngltolong(bs); }

    protected:

        bassngl bs;

};

class cBDREAL
{

    public:

        cBDREAL                 ( long i = 0 ) { *this = i; }
        cBDREAL & operator =    ( long i )     { longtobasdble(bd,i);
                                                 return *this; }
                  operator long ( void )       { return basdbletolong(bd); }

    protected:

        basdble bd;

};

#ifdef ___USE_PREAL___

class cPREAL
{

    public:

        cPREAL                  ( long i = 0 ) { *this = i; }
        cPREAL  & operator =    ( long i )     { longtopasreal(p,i);
                                                 return *this; }
                  operator long ( void )       { return pasrealtolong(p); }

    protected:

        pasreal p;

};

#endif

/******************************************************************************/

// Notes

// The following are unused prototypes from MISC.H:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// double LIBENTRY bassngltodouble(bassngl OldNum);
// double LIBENTRY basdbletodouble(basdble OldNum);
// void   LIBENTRY doubletobasdble(basdble *New, double Old);
// void   LIBENTRY doubletobassngl(bassngl *New, double Old);
// long   LIBENTRY doubletolong(void *Num);
// void   LIBENTRY doubletopasreal(pasreal *New, double Old);
// void   LIBENTRY longtodouble(char *F, long Num);
// double LIBENTRY pasrealtodouble(pasreal OldNum);

/******************************************************************************/

#endif

/******************************************************************************/

