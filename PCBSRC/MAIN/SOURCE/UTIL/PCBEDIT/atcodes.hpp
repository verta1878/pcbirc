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
/*                                ATCODES.HPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Functions for checking, finding and evaluation @Codes and hex digits    */
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

#ifndef ___ATCODES_HPP___

#define ___ATCODES_HPP___

/******************************************************************************/

// Included Files

#include    <miscedit.hpp>

/******************************************************************************/

// Defined Macros

// These may change if more macros are added
#define ATCLREOL     13
#define ATCLS        14
#define ATDELAY      24
#define ATENV        29
#define ATPOS        72
#define ATXCODE     105

// #define doIsATCode(s) ((*s == '@') ? isATCode(s) : 0)

/******************************************************************************/

// Variables

extern char * atVarLst          [];
extern char * defAtVarValLst    [];
extern char * atVarValLst       [];
extern char   replaceableATCode [];

//extern unsigned char atWidth;
//extern char          atJust;
extern int           atWidth;
extern int           atJust;
extern int           atSize;
extern char          atBuf   [];

extern char          noatFlag;
extern char          noatxFlag;

/******************************************************************************/

// Function Prototypes

int pascal getATX    (char * s, int c, int o);

int pascal isATCode  (char * s);

/******************************************************************************/

// Inline Functions

inline int isHex ( char c )
{
    return (isDIGIT(c) || (isXDIGIT(c) && isUPPER(c)));
}

    /*--------------------------------------------------------------------*/

inline int hex2dec ( char c )
{
    if (isDIGIT(c)) return c-'0';
    if (!isXDIGIT(c) || isLOWER(c)) return -1;
    return c-'A'+10;
}

    /*--------------------------------------------------------------------*/

inline int isATX ( char s [] )
{
    return (!noatFlag && (s[0] == '@') && (s[1] == 'X') &&
        isHex(s[2]) && isHex(s[3]));
}

    /*--------------------------------------------------------------------*/

inline int doIsATCode ( char s [] )
{
    atSize = atWidth = atJust = 0; *atBuf = NUL;
    return
        ((!noatFlag && (*s == '@')) ?
            (isATX(s) ?
                (atSize = 4, ATXCODE) :
                isATCode(s))
            : 0);
}

/******************************************************************************/

#endif

