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
/*                                 FILE.HPP                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*          Functions for file I/O and clearing of the text buffer          */
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

#ifndef	___FILE_HPP___

#define	___FILE_HPP___

/****************************************************************************/

// Included Files

#include    <misc.h>

/****************************************************************************/

// Defined Macros

#define	PN_SIZE			(3+128+1+12)

/****************************************************************************/

// Types

/****************************************************************************/

// Variables

extern int  dialogAttr,    dialogHLAttr,  dialogFileAttr,
            dialogDrvAttr, dialogDirAttr, dialogOptAttr;

extern char bakFlag;
extern char ioFlag;

extern char gFlag;

/****************************************************************************/

// Function Prototypes

int  pascal getFN      (char * p, int f);
void pascal doClearMem (void);
void pascal clearMem   (void);
void pascal fileErr    (char * s, char * f);
void pascal doLoadFile (void);
void pascal loadFile   (void);
void pascal doSaveFile (void);
void pascal saveFile   (void);
void pascal killFile   (void);

/****************************************************************************/

// Inline Functions

inline int exist(char * s)
{
    unsigned char rv = fileexist(s);
    return ((rv != 255) && ((rv & 0x18) == 0));
}

/****************************************************************************/

#endif
