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
/*                                  EXEC.HPP                                 */
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


/* Defined Macros */

#ifdef ERRBRK

/*#define EVAL(a,b,c)            {b = evalarg(); c(b); a = *b;} */
#define EVAL(a,b,c)         { (a = *evalArg()); }

#define nullErrReturn(a)  if (a == NULL) { /*err = TRUE;*/ return; }
#define nullErrBreak(a)   if (a == NULL) { /*err = TRUE;*/ break; }
#define falseErrBreak(a)  if (!a) {/* err = TRUE;*/ break; }
#define ERR 				err

#else

#define EVAL(a,b,c)         { (a = *evalArg()); }

#define nullErrReturn(a)
#define nullErrBreak(a)
#define falseErrBreak(a)
#define ERR

#endif

#ifdef LIB

#define mask_message		mask_alphanum
#define mask_filename		mask_alphanum

#define DatFile 			"C:\\PCB\\PCBOARD.DAT"

#define PCB_MAXNODES		250

#else

#define CurrConf             Status.CurConf

#endif

#define MAX_COLS			80
#define MAX_ROWS			(Scrn_BottomRow+1)

/* Macro to ensure that v is in the range l through h inclusive */
#define inRange(v,l,h) if (v < l) v = l; else if (v > h) v = h

#define HIWORD(L) ((unsigned int)(((unsigned long)(L) >> 16) & 0xFFFF))
#define LOWORD(L) ((unsigned int)(L))

#define SCR_ERR_SOFLOW		52
