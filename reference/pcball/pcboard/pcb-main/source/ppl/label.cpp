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
/*                                LABEL.CPP                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                 Data and functions for the cLABEL class.                 */
/*                                                                          */
/*==========================================================================*/
/*                                                                          */
/*                                Written by                                */
/*                            Scott Dale Robison                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*           Copyright (C) 1993 - Clark Development Company, Inc.           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/

// Pragmas

/****************************************************************************/

// Included Files

#include    <label.hpp>

#ifdef DEBUGSCR
#include    <memcheck.h>
#endif

/****************************************************************************/

// Defined Macros

/****************************************************************************/

// Types

/****************************************************************************/

// Constants

/****************************************************************************/

// Variables

/****************************************************************************/

// Function Prototypes

/****************************************************************************/

// Inline Functions

/****************************************************************************/

// Functions

int LIBENTRY cLABEL::labNameMatch(const cLABEL & l, const void * s)
{
	return (strcmp(l.label,(char *) s) == 0);
}

/****************************************************************************/
