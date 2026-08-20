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


/****************************************************************************/
/*                                                                          */
/*                                 MISC.HPP                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                 Miscellaneous functions used by PCBEdit.                 */
/*                                                                          */
/*==========================================================================*/
/*                                                                          */
/*                      Written by Scott Dale Robison                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*           Copyright (C) 1993, Clark Development Company, Inc.            */
/*                                                                          */
/****************************************************************************/

#ifndef	___MISC_HPP___

#define	___MISC_HPP___

/****************************************************************************/

// Included Files

#include    <stddef.h>

#include    <pcbedit.hpp>

#include    <pcbtypes.hpp>

/****************************************************************************/

// Defined Macros

/****************************************************************************/

// Types

/****************************************************************************/

// Variables

extern char hexDigits [];

extern char nodosFlag;

/****************************************************************************/

// Function Prototypes

int  pascal selectColor    (int sbl);
void pascal doDOS          (char * cmd = NULL, int pause = FALSE);
void pascal dispHelp       (void);
void pascal selectChar     (void);
void pascal selectFKeyChar (void);
void pascal selectMacro    (void);
void pascal selectATVar    (void);
int  pascal confirm        (char * s, void pascal (* func)(void) = NULL,
                            char key = '\0');
int  pascal confirmExit    (void);
void pascal loadMacros     (char * s);
void pascal loadFKeyChar   (char * s);

/****************************************************************************/

// Template Functions

template <class t>
void swap(t & a, t & b)
{
    t c = a;
    a = b;
    b = c;
}

/****************************************************************************/

#endif
