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
/*                               INITREST.HPP                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*       Functions used during program initialization and restoration       */
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

#ifndef	___INITREST_HPP___

#define	___INITREST_HPP___

/****************************************************************************/

// Included Files

/****************************************************************************/

// Defined Macros

/****************************************************************************/

// Types

/****************************************************************************/

// Variables

extern int origAttr;

/****************************************************************************/

// Function Prototypes

void pascal procArg       (char * a);
void pascal fadeScreen    (int * fgAttr, int * bgAttr, int delLen,
						   int initIndex, int abortIndex, int stepIndex);
void pascal welcomeScreen (int in);
void pascal dispWelcome   (void);
void pascal initPrg       (void);
void pascal restPrg       (void);

/*****************************************************************************/

#endif
