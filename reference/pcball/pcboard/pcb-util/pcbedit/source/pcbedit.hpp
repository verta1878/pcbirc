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
/*                                PCBEDIT.HPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*       A program to allow easy editing of PCBoard files with @ codes.       */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

#ifndef ___PCBEDIT_HPP___

#define ___PCBEDIT_HPP___

/******************************************************************************/

// Included Files

#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

#ifndef __OS2__
#define MAX_LINES       2048
#else
#define MAX_LINES       32768
#endif
#define MAX_LINE_SIZE   2048

#define TRUE            1
#define FALSE           0

// #define MAX_ROWS        25
// #define MAX_COLS        80

/******************************************************************************/

// Types

/******************************************************************************/

// Variables

extern char *   lineList    [];
extern int      attrList    [];
extern int      last00List  [];
extern char     isFFList    [];
extern int      lastCLSList [];

extern int      lastLine;

extern char     curLine    [MAX_LINE_SIZE+1+2];
extern char     tmpLine    [MAX_LINE_SIZE+1+2];
extern char *   curLineChar;

extern char     lastFN     [];
extern char *   lastLineEnd;

extern int      tlOff, slOff, lcOff, scOff;
extern int      ltlOff, llcOff;

extern int      statAttr, ribAttr, ribHLAttr, helpAttr, helpHLAttr;

extern char     quick;

extern int      macroSet;
extern char *   macros     [15][10];

extern int      charSet;
extern char *   graphChars [];

extern int      textChanged;

extern int      statType;
extern int      statOn;

extern int      insMode;

extern int      disableEditKeys;

extern char     buzzFlag;
extern char     nossFlag;

extern int      startBlockLine, endBlockLine;
extern int      startBlockOff,  endBlockOff;
extern int      startBlockLen,  endBlockLen;
extern int      blockType;

extern int      MAX_ROWS;
extern int      MAX_COLS;

/******************************************************************************/

// Function Prototypes

void pascal buzz        (void);

void        editErr     (char * fmt, ...);

void pascal adjColOff   (int newOff);

void pascal procChar    (int);

int  pascal lineLen     (char * s);
void pascal stripSpaces (void);

void pascal joinLines   (void);

void pascal splitLine   (void);

void pascal commitLine  (void);
void pascal cacheLine   (void);

void pascal stripAttrs  (int clOff, int off, int plen);
void pascal udLine      (void);

/******************************************************************************/

// Inline Functions

/******************************************************************************/

#endif

