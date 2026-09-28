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
/*                                 DISP.HPP                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                   Functions used to update the display                   */
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

#ifndef	___DISP_HPP___

#define	___DISP_HPP___

/****************************************************************************/

// Included Files

/****************************************************************************/

// Defined Macros

/****************************************************************************/

// Types

/****************************************************************************/

// Variables

/****************************************************************************/

// Function Prototypes

void pascal dispScrn (int udStat);
void pascal dispStat (void);
void pascal dispCurs (void);
void pascal chkAttrs (int forceAll = 0);

/****************************************************************************/

// Inline Functions

/****************************************************************************/

#endif
