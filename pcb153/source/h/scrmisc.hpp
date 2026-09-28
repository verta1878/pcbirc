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
/*                                SCRMISC.HPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*       Miscellaneous script support types, variables, functions, etc.       */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

#ifndef ___SCRMISC_HPP___

#define ___SCRMISC_HPP___

/******************************************************************************/

/* Pragmas */

/******************************************************************************/

/* Included Files */

#include    "newscr.hpp"

/******************************************************************************/

/* Defined Macros */

#if defined(___COMP___)

 #define SCR_ERR_GEN      0
 #define SCR_ERR_MEM      1
 #define SCR_ERR_QUOTE    3
 #define SCR_ERR_PAREN1   4
 #define SCR_ERR_PAREN2   5
 #define SCR_ERR_LABBAD   6
 #define SCR_ERR_LABDEF   7
 #define SCR_ERR_CUSR     8
 #define SCR_ERR_LINNUM  10
 #define SCR_ERR_VARBAD  11
 #define SCR_ERR_VARDEF  12
 #define SCR_ERR_TOOBIG  14
 #define SCR_ERR_ARGNE   15
 #define SCR_ERR_VARNF   16
 #define SCR_ERR_NOEXPR  17
 #define SCR_ERR_IFCNF   18
 #define SCR_ERR_IFSNF   19
 #define SCR_ERR_LABNF   20
 #define SCR_ERR_IMVC    21
 #define SCR_ERR_LABMIS  22
 #define SCR_ERR_VARMIS  23
 #define SCR_ERR_SUSR    24
 #define SCR_WRN_ARGTM   26
 #define SCR_ERR_CEXPR   27
 #define SCR_ERR_INVBC   28
 #define SCR_ERR_ENDOP   29
 #define SCR_ERR_CEXPCH  30
 #define SCR_ERR_CONST   33
 #define SCR_ERR_NOEND   34
 #define SCR_ERR_NOBIF   35
 #define SCR_ERR_BSES    36
 #define SCR_ERR_IMOP    37
 #define SCR_WRN_LINNUM  38
 #define SCR_ERR_MISINC  39
 #define SCR_ERR_FOR     40
 #define SCR_ERR_SRTTYP  41
 #define SCR_ERR_SRTARG  42
 #define SCR_ERR_SWEXP   43
 #define SCR_ERR_CASE    44
 #define SCR_ERR_FUNC    45
 #define SCR_ERR_BADCALL 46
 #define SCR_ERR_PARANUM 47
 #define SCR_ERR_DECDEF  48
 #define SCR_ERR_NODEC   49
 #define SCR_ERR_VARFUNC 50
 #define SCR_ERR_VARPASS 51
 #define SCR_ERR_SOFLOW  52
 #define SCR_ERR_MANYVAR 53
 #define SCR_ERR_MANYPS  54
 #define SCR_ERR_NOFNC   55

#elif defined(LIB)

 #define SCR_ERR_GEN      0
 #define SCR_ERR_MEM      1
 #define SCR_ERR_CUSR     8

 #define SCR_ERR_TOKEN    2
 #define SCR_ERR_EUSR     9
 #define SCR_ERR_EXPEVL  13
 #define SCR_ERR_UNAPND  31
 #define SCR_ERR_UNDEL   32

 #define SCR_ERR_LUSR    25

#else

 #define SCR_ERR_GEN     TXT_ERRORSINPPE       /*  0 */
 #define SCR_ERR_MEM     TXT_NOMEMORYFORPPE    /*  1 */
 #define SCR_ERR_CUSR     8

 #define SCR_ERR_TOKEN   TXT_INVALIDTOKENINPPE /*  2 */
 #define SCR_ERR_EUSR    TXT_ERROREXECPPE      /*  9 */
 #define SCR_ERR_EXPEVL  TXT_EVALERRORINPPE    /* 13 */
 #define SCR_ERR_UNAPND  TXT_APPENDERRORINPPE  /* 31 */
 #define SCR_ERR_UNDEL   TXT_DELETEERRORINPPE  /* 32 */

 #define SCR_ERR_LUSR    TXT_ERRORLOADINGPPE   /* 25 */

#endif

/******************************************************************************/

/* Types */

/******************************************************************************/

/* Constants */

/******************************************************************************/

/* Variables */

extern char    * errText[];

extern cSCRIPT * scriptPtr;

/******************************************************************************/

/* Function Prototypes */

char * LIBENTRY formattedLine (char * line);
void   LIBENTRY dispPcbText   (int prompt, char * optext);
void            scriptErr     (int handNum, int lineNum, int textNum, char * format, ...);
int    LIBENTRY unsupported             (char * token);
long            standardizeTicks        (long ticks);


/* extern "C" { */
int    LIBENTRY doScript      (char * filename, char * ansfile, int numtoks);
void   LIBENTRY cleanupScript (void);
/* } */

/******************************************************************************/

/* Inline Functions */

/******************************************************************************/

#endif

