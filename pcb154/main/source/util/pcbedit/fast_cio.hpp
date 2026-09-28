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
/*                                FAST_CIO.HPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                       Functions for fast console I/O                       */
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

#ifndef	___FAST_CIO_HPP___

#define	___FAST_CIO_HPP___

/******************************************************************************/

// Defined Macros

#define F_ALL       0x0001
#define F_ALPHA     0x0002
#define F_NUM       0x0004
#define F_FILE      0x0008
#define F_PATH      0x0010
#define F_LOWER     0x0020
#define F_UPPER     0x0040
#define F_WILD      0x0080

#define F_VALID     0x00FE

/******************************************************************************/

// Variables

extern int				NOCURS;
extern int				SOLIDCURS;
extern int				NORMALCURS;

#ifndef __OS2__

extern int				curAttr;

extern char *			curScrPtr;

extern unsigned long	baseScrnAddr;
extern int				snowWatch;

#endif

extern int              cursType;

/******************************************************************************/

// Function Prototypes

int  pascal getStr      (char * buf, int size, int flags);
void pascal setAttr     (int attr);
void pascal cls         (void);
void pascal cle         (void);
int  pascal wrChar      (int ch);
int  pascal wrStr       (char * s);
int         fastprintf  (const char * f, ...);
void pascal setXY       (int x, int y);
void pascal setcurstype (int type);
void pascal putline     (int line, char * buf);
int  pascal movetxt     (int t, int b, int dt);
int  pascal puttxt      (void * c);
void pascal setWin      (int l, int t, int r, int b);
int  pascal curx        (void);
int  pascal cury        (void);

/******************************************************************************/

#endif

