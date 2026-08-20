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
/*																			  */
/*								   NEWSCR.CPP								  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*				Data, types and functions for the cSCRIPT class.			  */
/*																			  */
/*============================================================================*/
/*																			  */
/*								   Written by								  */
/*							   Scott Dale Robison							  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*			  Copyright (C) 1994 - Clark Development Company, Inc.			  */
/*																			  */
/******************************************************************************/

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

// CHECK_TABLES is used to conditionally include/exclude code to check the
// kwArray (statement table) and opArray (function and operator table)

//#define CHECK_TABLES
#define CRYPT

#if (defined(___COMP___) || defined(___SCAN___)) && defined(CHECK_TABLES)
#include	<conio.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	"lrand.hpp"
#include	"newscr.hpp"

#include	"scrmisc.hpp"


#ifdef DEBUGSCR
#include	<memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

#define HDR_TXT "PCBoard Programming Language Executable  3.40\x0D\x0A\x1A"  /* 15.4: bumped from 3.30 */
#define HDR_SIZ 48

#define CUR_PPE_VER 	 340   // 15.4: bumped from 330.  For PCB 15.23/15.3 was 3.30.
#define MAX_PPE_VER 	9999

/******************************************************************************/

// Types

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Constants

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

// The following statements are disabled in the DEMO version of PPL:
//	  CALL,   POKE,  POKEB,   POKEDW, POKEW,  POP,	 PUSH,
//	  RDUSYS, SHELL, VARADDR, VAROFF, VARSEG, WRUSYS

sKEYWORD cSCRIPT::kwArray[] =
{
	{ "\x08""ACCOUNT ",    TOK_ACCOUNT,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""ADDUSER ",    TOK_ADDUSER,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""ADJBYTES ",   TOK_ADJBYTES,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""ADJDBYTES ",  TOK_ADJDBYTES,   1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""ADJTBYTES ",  TOK_ADJTBYTES,   1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""ADJTFILES ",  TOK_ADJTFILES,   1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""ADJTIME ",    TOK_ADJTIME,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0B""ADJTUBYTES ", TOK_ADJTUBYTES,  1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""ALIAS ",      TOK_ALIAS,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""ANSIPOS ",    TOK_ANSIPOS,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""APPEND ",     TOK_APPEND,      2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""BACKUP ",     TOK_BACKUP,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""BEEP ",       TOK_BEEP,        0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""BEGIN ",      TOK_BEGIN,       0,  0, FALSE, cSCRIPT::tokenBEGIN   },
	{ "\x07""BIGSTR ",     TOK_BIGSTR,      1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x09""BITCLEAR ",   TOK_BITCLEAR,    2,  2, FALSE, cSCRIPT::tokenVAREXPR },
	{ "\x07""BITSET ",     TOK_BITSET,      2,  2, FALSE, cSCRIPT::tokenVAREXPR },
	{ "\x04""BLT ",        TOK_BLT,         1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""BOOLEAN ",    TOK_BOOLEAN,     1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""BRAG ",       TOK_BRAG,        0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""BROADCAST ",  TOK_BROADCAST,   3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x04""BYE ",        TOK_BYE,         0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""BYTE ",       TOK_BYTE,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""CALL ",       TOK_CALL,        1,  1, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x09""CDCHKOFF ",   TOK_CDCHKOFF,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""CDCHKON ",    TOK_CDCHKON,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""CHAT ",       TOK_CHAT,        0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""CHDIR ",      TOK_CHDIR,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""CLOSECAP ",   TOK_CLOSECAP,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""CLREOL ",     TOK_CLREOL,      0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x04""CLS ",        TOK_CLS,         0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""COLOR ",      TOK_COLOR,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""COMMAND ",    TOK_COMMAND,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""CONFFLAG ",   TOK_CONFFLAG,    2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""CONFINFO ",   TOK_CONFINFO,    3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0B""CONFUNFLAG ", TOK_CONFUNFLAG,  2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""COPY ",       TOK_COPY,        2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""DADD ",       TOK_DADD,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DAPPEND ",    TOK_DAPPEND,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""DATE ",       TOK_DATE,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x09""DBGLEVEL ",   TOK_DBGLEVEL,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DBLANK ",     TOK_DBLANK,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DBOTTOM ",    TOK_DBOTTOM,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DCLOSE ",     TOK_DCLOSE,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""DCLOSEALL ",  TOK_DCLOSEALL,   0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DCREATE ",    TOK_DCREATE,     4,  4, FALSE, cSCRIPT::tokenDBFPROC },
	{ "\x06""DDATE ",      TOK_DDATE,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x08""DDELETE ",    TOK_DDELETE,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x04""DEC ",        TOK_DEC,         1,  1, FALSE, cSCRIPT::tokenARGV    },
	{ "\x08""DECLARE ",    TOK_DECLARE,     2,  2, FALSE, cSCRIPT::tokenDECL    }, // added by stan to support DECLARE FUNCTION statements
	{ "\x09""DEFCOLOR ",   TOK_DEFCOLOR,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DELAY ",      TOK_DELAY,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DELETE ",     TOK_DELETE,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DELUSER ",    TOK_DELUSER,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DFBLANK ",    TOK_DFBLANK,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DFCOPY ",     TOK_DFCOPY,      4,  4, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""DGET ",       TOK_DGET,        3,  3, FALSE, cSCRIPT::tokenDBFPROC },
	{ "\x04""DGO ",        TOK_DGO,         2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x04""DIR ",        TOK_DIR,         1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""DISPFILE ",   TOK_DISPFILE,    2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DISPSTR ",    TOK_DISPSTR,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""DISPTEXT ",   TOK_DISPTEXT,    2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DLOCK ",      TOK_DLOCK,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DLOCKF ",     TOK_DLOCKF,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DLOCKG ",     TOK_DLOCKG,      3,  3, FALSE, cSCRIPT::tokenDBFPROC },
	{ "\x07""DLOCKR ",     TOK_DLOCKR,      2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DNCLOSE ",    TOK_DNCLOSE,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0B""DNCLOSEALL ", TOK_DNCLOSEALL,  1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""DNCREATE ",   TOK_DNCREATE,    3,  3, FALSE, cSCRIPT::tokenDBFPROC },
	{ "\x05""DNEW ",       TOK_DNEW,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DNOPEN ",     TOK_DNOPEN,      2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DOINTR ",     TOK_DOINTR,     10, 10, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DOPEN ",      TOK_DOPEN,       3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""DOWNLOAD ",   TOK_DOWNLOAD,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DPACK ",      TOK_DPACK,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""DPUT ",       TOK_DPUT,        3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DREAL ",      TOK_DREAL,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x08""DRECALL ",    TOK_DRECALL,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DSEEK ",      TOK_DSEEK,       2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""DSETALIAS ",  TOK_DSETALIAS,   2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DSKIP ",      TOK_DSKIP,       2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""DTAG ",       TOK_DTAG,        2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""DTOP ",       TOK_DTOP,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DTROFF ",     TOK_DTROFF,      0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""DTRON ",      TOK_DTRON,       0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""DUNLOCK ",    TOK_DUNLOCK,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""EDATE ",      TOK_EDATE,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""EVAL ",       TOK_EVAL,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""FAPPEND ",    TOK_FAPPEND,     4,  4, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""FCLOSE ",     TOK_FCLOSE,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""FCLOSEALL ",  TOK_FCLOSEALL,   0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""FCREATE ",    TOK_FCREATE,     4,  4, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""FDEFIN ",     TOK_FDEFIN,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""FDEFOUT ",    TOK_FDEFOUT,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""FDGET ",      TOK_FDGET,       1,  1, FALSE, cSCRIPT::tokenVAREXPR },
	{ "\x09""FDOWRAKA ",   TOK_FDOWRAKA,    2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""FDOADDAKA ",  TOK_FDOADDAKA,   3,  3, FALSE, cSCRIPT::tokenARGS },
	{ "\x09""FDOWRORG ",   TOK_FDOWRORG,    2,  2, FALSE, cSCRIPT::tokenARGS },
	{ "\x0A""FDOADDORG ",  TOK_FDOADDORG,   3,  3, FALSE, cSCRIPT::tokenARGS },
	{ "\x08""FDOQADD ",    TOK_FDOQADD,     3,  3, FALSE, cSCRIPT::tokenARGS },
	{ "\x08""FDOQDEL ",    TOK_FDOQDEL,     1,  1, FALSE, cSCRIPT::tokenARGS },
	{ "\x08""FDOQMOD ",    TOK_FDOQMOD,     4,  4, FALSE, cSCRIPT::tokenARGS },
	{ "\x06""FDPUT ",      TOK_FDPUT,       1, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x08""FDPUTLN ",    TOK_FDPUTLN,     0, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x09""FDPUTPAD ",   TOK_FDPUTPAD,    2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""FDREAD ",     TOK_FDREAD,      2,  2, FALSE, cSCRIPT::tokenVAREXPR },
	{ "\x08""FDWRITE ",    TOK_FDWRITE,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""FFLUSH ",     TOK_FFLUSH,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""FGET ",       TOK_FGET,        2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x05""FLAG ",       TOK_FLAG,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""FOPEN ",      TOK_FOPEN,       4,  4, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""FORWARD ",    TOK_FORWARD,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""FPUT ",       TOK_FPUT,        2, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x07""FPUTLN ",     TOK_FPUTLN,      1, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x08""FPUTPAD ",    TOK_FPUTPAD,     3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""FREAD ",      TOK_FREAD,       3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0B""FREALTUSER ", TOK_FREALTUSER,  0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""FRESHLINE ",  TOK_FRESHLINE,   0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""FREWIND ",    TOK_FREWIND,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""FSEEK ",      TOK_FSEEK,       3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""FUNCTION ",   TOK_FUNCTION,    1,  1, FALSE, cSCRIPT::tokenFUNC    },
	{ "\x07""FWRITE ",     TOK_FWRITE,      3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0B""GETALTUSER ", TOK_GETALTUSER,  1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""GETTOKEN ",   TOK_GETTOKEN,    1,  1, FALSE, cSCRIPT::tokenARGV    },
	{ "\x08""GETUSER ",    TOK_GETUSER,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""GOODBYE ",    TOK_GOODBYE,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""GOSUB ",      TOK_GOSUB,       1,  1, FALSE, cSCRIPT::tokenGOX     },
	{ "\x05""GOTO ",       TOK_GOTO,        1,  1, FALSE, cSCRIPT::tokenGOX     },
	{ "\x09""GRAFMODE ",   TOK_GRAFMODE,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""HANGUP ",     TOK_HANGUP,      0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x03""IF ",         TOK_IF,          2,  2, FALSE, cSCRIPT::tokenIFWHILE },
	{ "\x04""INC ",        TOK_INC,         1,  1, FALSE, cSCRIPT::tokenARGV    },
	{ "\x06""INPUT ",      TOK_INPUT,       2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x08""INPUTCC ",    TOK_INPUTCC,     3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0A""INPUTDATE ",  TOK_INPUTDATE,   3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x09""INPUTINT ",   TOK_INPUTINT,    3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0B""INPUTMONEY ", TOK_INPUTMONEY,  3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x09""INPUTSTR ",   TOK_INPUTSTR,    6,  6, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0A""INPUTTEXT ",  TOK_INPUTTEXT,   4,  4, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0A""INPUTTIME ",  TOK_INPUTTIME,   3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x08""INPUTYN ",    TOK_INPUTYN,     3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x08""INTEGER ",    TOK_INTEGER,     1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""JOIN ",       TOK_JOIN,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""KBDCHKOFF ",  TOK_KBDCHKOFF,   0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""KBDCHKON ",   TOK_KBDCHKON,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""KBDFILE ",    TOK_KBDFILE,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""KBDFLUSH ",   TOK_KBDFLUSH,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""KBDSTRING ",  TOK_KBDSTRING,   1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""KBDSTUFF ",   TOK_KBDSTUFF,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""KEYFLUSH ",   TOK_KEYFLUSH,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""KILLMSG ",    TOK_KILLMSG,     2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""LANG ",       TOK_LANG,        1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""LASTIN ",     TOK_LASTIN,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x04""LET ",        TOK_LET,         2,  2, FALSE, cSCRIPT::tokenLET     },
	{ "\x04""LOG ",        TOK_LOG,         2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""MDMFLUSH ",   TOK_MDMFLUSH,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""MESSAGE ",    TOK_MESSAGE,     9,  9, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""MKDIR ",      TOK_MKDIR,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""MONEY ",      TOK_MONEY,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""MORE ",       TOK_MORE,        0,  0, FALSE, cSCRIPT::tokenARGS    },
	/* 15.4: MOVEMSG conf message movetype (movetype: TRUE=move, FALSE=copy) */
	{ "\x08""MOVEMSG ",    TOK_MOVEMSG,     3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""MOUSEREG ",   TOK_MOUSEREG,   10, 10, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""MPRINT ",     TOK_MPRINT,      1, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x09""MPRINTLN ",   TOK_MPRINTLN,    0, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x0A""MSGTOFILE ",  TOK_MSGTOFILE,   3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""NEWLINE ",    TOK_NEWLINE,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""NEWLINES ",   TOK_NEWLINES,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""NEWPWD ",     TOK_NEWPWD,      2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x08""OPENCAP ",    TOK_OPENCAP,     2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x07""OPTEXT ",     TOK_OPTEXT,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""PAGEOFF ",    TOK_PAGEOFF,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""PAGEON ",     TOK_PAGEON,      0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""POKEB ",      TOK_POKEB,       2,  2, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x07""POKEDW ",     TOK_POKEDW,      2,  2, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x06""POKEW ",      TOK_POKEW,       2,  2, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x04""POP ",        TOK_POP,         1, 99, TRUE,  cSCRIPT::tokenARGV    }, // * NO DEMO
	{ "\x08""PRFOUND ",    TOK_PRFOUND,     1, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x0A""PRFOUNDLN ",  TOK_PRFOUNDLN,   0, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x06""PRINT ",      TOK_PRINT,       1, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x08""PRINTLN ",    TOK_PRINTLN,     0, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x0A""PROCEDURE ",  TOK_PROCEDURE,   1,  1, FALSE, cSCRIPT::tokenFUNC    },
	{ "\x0A""PROMPTSTR ",  TOK_PROMPTSTR,   5,  5, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x05""PUSH ",       TOK_PUSH,        1, 99, TRUE,  cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x08""PUTUSER ",    TOK_PUTUSER,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""QUEST ",      TOK_QUEST,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""QWKLIMITS ",  TOK_QWKLIMITS,   2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""RDUNET ",     TOK_RDUNET,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""RDUSYS ",     TOK_RDUSYS,      0,  0, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x05""REAL ",       TOK_REAL,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x0C""RECORDUSAGE ",TOK_RECORDUSAGE, 5,  5, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""REDIM ",      TOK_REDIM,       1,  4, TRUE,  cSCRIPT::tokenREDIM   },
	{ "\x06""REDIR ",      TOK_REDIR,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""RENAME ",     TOK_RENAME,      2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""RESETDISP ",  TOK_RESETDISP,   0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""RESTSCRN ",   TOK_RESTSCRN,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""RETURN ",     TOK_RETURN,      0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""SAVESCRN ",   TOK_SAVESCRN,    0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""SCRFILE ",    TOK_SCRFILE,     2,  2, FALSE, cSCRIPT::tokenARGV    },
	{ "\x0B""SEARCHFIND ", TOK_SEARCHFIND,  2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0B""SEARCHINIT ", TOK_SEARCHINIT,  2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0B""SEARCHSTOP ", TOK_SEARCHSTOP,  0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""SENDMODEM ",  TOK_SENDMODEM,   1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""SETENV ",     TOK_SETENV,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""SETLMR ",     TOK_SETLMR,      2,  2, FALSE, cSCRIPT::tokenARGS    },
	/* 15.4: SETBANKVAL idx value — write a banking PSA field by index */
	{ "\x0B""SETBANKVAL ", TOK_SETBANKVAL,  2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""SHELL ",      TOK_SHELL,       4,  4, FALSE, cSCRIPT::tokenEXPRVAR }, // * NO DEMO
	{ "\x06""SHORT ",      TOK_SBYTE,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	/* 15.4: SHORTDESC value — write the current user's short-desc mode */
	{ "\x0A""SHORTDESC ",  TOK_SHORTDESCSTMT,1, 1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""SHOWOFF ",    TOK_SHOWOFF,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""SHOWON ",     TOK_SHOWON,      0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""SORT ",       TOK_SORT,        2,  2, FALSE, cSCRIPT::tokenSORT    },
	{ "\x06""SOUND ",      TOK_SOUND,       1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0B""SOUNDDELAY ", TOK_SOUNDDELAY,  2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""SPRINT ",     TOK_SPRINT,      1, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x09""SPRINTLN ",   TOK_SPRINTLN,    0, 99, TRUE,  cSCRIPT::tokenARGS    },
	{ "\x0B""STACKABORT ", TOK_STACKABORT,  1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x0A""STARTDISP ",  TOK_STARTDISP,   1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""STATIC ",     TOK_STATIC,      0,  0, FALSE, cSCRIPT::tokenSTATIC  },
	{ "\x05""STOP ",       TOK_STOP,        0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""STRING ",     TOK_STRING,      1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x04""INT ",        TOK_SWORD,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""TIME ",       TOK_TIME,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x09""TOKENIZE ",   TOK_TOKENIZE,    1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""TPACGET ",    TOK_TPACGET,     3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x08""TPACPUT ",    TOK_TPACPUT,     3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""TPACREAD ",   TOK_TPACREAD,    3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x0A""TPACWRITE ",  TOK_TPACWRITE,   3,  3, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""TPAGET ",     TOK_TPAGET,      2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x07""TPAPUT ",     TOK_TPAPUT,      2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""TPAREAD ",    TOK_TPAREAD,     2,  2, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x09""TPAWRITE ",   TOK_TPAWRITE,    2,  2, FALSE, cSCRIPT::tokenARGS    },
	{ "\x09""UNSIGNED ",   TOK_UNSIGNED,    1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x08""USELMRS ",    TOK_USELMRS,     1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x08""VARADDR ",    TOK_VARADDR,     2,  2, FALSE, cSCRIPT::tokenARGV    }, // * NO DEMO
	{ "\x07""VAROFF ",     TOK_VAROFF,      2,  2, FALSE, cSCRIPT::tokenARGV    }, // * NO DEMO
	{ "\x07""VARSEG ",     TOK_VARSEG,      2,  2, FALSE, cSCRIPT::tokenARGV    }, // * NO DEMO
	{ "\x08""WAITFOR ",    TOK_WAITFOR,     3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x05""WORD ",       TOK_WORD,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x07""WRUNET ",     TOK_WRUNET,      6,  6, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""WRUSYS ",     TOK_WRUSYS,      0,  0, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x0B""WRUSYSDOOR ", TOK_WRUSYSDOOR,  1,  1, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO

#if defined(___COMP___)

	{ "\x06""BREAK ",      0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x09""CONTINUE ",   0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x05""CASE ",       0,               1, 99, TRUE,  cSCRIPT::tokenCASE    },
	{ "\x08""DEFAULT ",    0,               0,  0, FALSE, cSCRIPT::tokenDEFAULT },
	{ "\x08""ELSE IF ",    0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x07""ELSEIF ",     0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x08""END FOR ",    0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x07""END IF ",     0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x0B""END SELECT ", 0,               0,  0, FALSE, cSCRIPT::tokenSLCTEND }, //Added by Stan
	{ "\x0A""END WHILE ",  0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x07""ENDFOR ",     0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x09""END FUNC ",   0,               0,  0, FALSE, cSCRIPT::tokenENDFUNC },
	{ "\x08""ENDFUNC ",    0,               0,  0, FALSE, cSCRIPT::tokenENDFUNC },
	{ "\x09""END PROC ",   0,               0,  0, FALSE, cSCRIPT::tokenENDFUNC },
	{ "\x08""ENDPROC ",    0,               0,  0, FALSE, cSCRIPT::tokenENDFUNC },
	{ "\x06""ENDIF ",      0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x0A""ENDSELECT ",  0,               0,  0, FALSE, cSCRIPT::tokenSLCTEND }, //Added by stan
	{ "\x09""ENDWHILE ",   0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x04""FOR ",        0,               0,  0, FALSE, cSCRIPT::tokenFOR     },
	{ "\x05""LOOP ",       0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x05""NEXT ",       0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x05""QUIT ",       0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },
	{ "\x06""WHILE ",      0,               0,  0, FALSE, cSCRIPT::tokenIFWHILE },
	{ "\x06""UBYTE ",      TOK_BYTE,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x06""ERASE ",      TOK_DELETE,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x07""DOUBLE ",     TOK_DREAL,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x07""GO SUB ",     TOK_GOSUB,       1,  1, FALSE, cSCRIPT::tokenGOX     },
	{ "\x06""GO TO ",      TOK_GOTO,        1,  1, FALSE, cSCRIPT::tokenGOX     },
	{ "\x05""LONG ",       TOK_INTEGER,     1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x07""SDWORD ",     TOK_INTEGER,     1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x08""LAST IN ",    TOK_LASTIN,      1,  1, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""POKE ",       TOK_POKEB,       2,  2, FALSE, cSCRIPT::tokenARGS    }, // * NO DEMO
	{ "\x0B""PUTALTUSER ", TOK_PUTUSER,     0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x06""FLOAT ",      TOK_REAL,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x06""SBYTE ",      TOK_SBYTE,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x07""SELECT ",     0,               0,  0, FALSE, cSCRIPT::tokenSELECT  }, //Added by Stan
	{ "\x06""SWORD ",      TOK_SWORD,       1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x06""DWORD ",      TOK_UNSIGNED,    1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x07""UDWORD ",     TOK_UNSIGNED,    1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x09""WAIT FOR ",   TOK_WAITFOR,     3,  3, FALSE, cSCRIPT::tokenEXPRVAR },
	{ "\x06""UWORD ",      TOK_WORD,        1, 99, TRUE,  cSCRIPT::tokenVARS    },
	{ "\x05""TYPE ",       0,               0,  0, FALSE, cSCRIPT::tokenTYPE    },
	{ "\x09""END TYPE ",   0,               0,  0, FALSE, cSCRIPT::tokenTYPE    },
	{ "\x08""ENDTYPE ",    0,               0,  0, FALSE, cSCRIPT::tokenTYPE    },
#endif

	{ "\x04""END ",        TOK_END,         0,  0, FALSE, cSCRIPT::tokenARGS    },
	{ "\x05""WAIT ",       TOK_WAIT,        0,  0, FALSE, cSCRIPT::tokenARGS    },

#if defined(___COMP___)

	{ "\x05""ELSE ",       0,               0,  0, FALSE, cSCRIPT::tokenBRANCH  },

#endif

	{ NULL, 			   0,				0,	0, FALSE, NULL					}
};

	/*--------------------------------------------------------------------*/

// The following functions are disabled in the DEMO version of PPL:
//	  MKADDR, PEEK,  PEEKB, PEEKDW, PEEKW, REGAH, REGAL, REGAX, REGBH, REGBL,
//	  REGBX,  REGCF, REGCH, REGCL,	REGCX, REGDH, REGDI, REGDL, REGDS, REGDX,
//	  REGES,  REGF,  REGSI

sOPERATOR cSCRIPT::opArray[] =
{
	{ "\x06""ABORT(",      TOK_OP_ABORT,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""ABS(",        TOK_OP_ABS,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""ACCOUNT(",    TOK_OP_ACCOUNT,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""ACTMSGNUM(",  TOK_OP_ACTMSGNUM,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""ALIAS(",      TOK_OP_ALIAS,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""AND(",        TOK_OP_BAND,       OP_PRI_UNARY,    FALSE,  2 },
	{ "\x07""ANSION(",     TOK_OP_ANSION,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""ASC(",        TOK_OP_ASC,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x04""B2W(",        TOK_OP_B2W,        OP_PRI_UNARY,    FALSE,  2 },
	{ "\x05""BD2I(",       TOK_OP_BD2I,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""BS2I(",       TOK_OP_BS2I,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""CALLID(",     TOK_OP_CALLID,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""CALLNUM(",    TOK_OP_CALLNUM,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""CARRIER(",    TOK_OP_CARRIER,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""CCTYPE(",     TOK_OP_CCTYPE,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""CDON(",       TOK_OP_CDON,       OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""CHATSTAT(",   TOK_OP_CHATSTAT,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""CHECKRIP(",   TOK_OP_CHECKRIP,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""CHR(",        TOK_OP_CHR,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""CONFALIAS(",  TOK_OP_CONFALIAS,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""CONFEXP(",    TOK_OP_CONFEXP,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""CONFINFO(",   TOK_OP_CONFINFO,   OP_PRI_UNARY,    FALSE,  2 },
	{ "\x07""CONFMW(",     TOK_OP_CONFMW,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""CONFREG(",    TOK_OP_CONFREG,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""CONFSEL(",    TOK_OP_CONFSEL,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""CONFSYS(",    TOK_OP_CONFSYS,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""CRC32(",      TOK_OP_CRC32,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x09""CURCOLOR(",   TOK_OP_CURCOLOR,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""CURCONF(",    TOK_OP_CURCONF,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""CURSEC(",     TOK_OP_CURSEC,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""CURUSER(",    TOK_OP_CURUSER,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""CWD(",        TOK_OP_CWD,        OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""DADD(",       TOK_OP_DADD,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""DAPPEND(",    TOK_OP_DAPPEND,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""DATE(",       TOK_OP_DATE,       OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""DAY(",        TOK_OP_DAY,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""DBGLEVEL(",   TOK_OP_DBGLEVEL,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""DBLANK(",     TOK_OP_DBLANK,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""DBOF(",       TOK_OP_DBOF,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""DBOTTOM(",    TOK_OP_DBOTTOM,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""DCHANGED(",   TOK_OP_DCHANGED,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""DCHKSTAT(",   TOK_OP_DCHKSTAT,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""DCLOSE(",     TOK_OP_DCLOSE,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""DCLOSEALL(",  TOK_OP_DCLOSEALL,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0A""DDECIMALS(",  TOK_OP_DDECIMALS,  OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""DDELETE(",    TOK_OP_DDELETE,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""DDELETED(",   TOK_OP_DDELETED,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""DEFANS(",     TOK_OP_DEFANS,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""DEFCOLOR(",   TOK_OP_DEFCOLOR,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""DEOF(",       TOK_OP_DEOF,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""DERR(",       TOK_OP_DERR,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""DERRMSG(",    TOK_OP_DERRMSG,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""DFBLANK(",    TOK_OP_DFBLANK,    OP_PRI_UNARY,    FALSE,  2 },
	{ "\x07""DFCOPY(",     TOK_OP_DFCOPY,     OP_PRI_UNARY,    FALSE,  4 },
	{ "\x08""DFIELDS(",    TOK_OP_DFIELDS,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""DGET(",       TOK_OP_DGET,       OP_PRI_UNARY,    FALSE,  2 },
	{ "\x0A""DGETALIAS(",  TOK_OP_DGETALIAS,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x04""DGO(",        TOK_OP_DGO,        OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""DLENGTH(",    TOK_OP_DLENGTH,    OP_PRI_UNARY,    FALSE,  2 },
	{ "\x06""DLOCK(",      TOK_OP_DLOCK,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""DLOCKF(",     TOK_OP_DLOCKF,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""DLOCKR(",     TOK_OP_DLOCKR,     OP_PRI_UNARY,    FALSE,  2 },
	{ "\x06""DNAME(",      TOK_OP_DNAME,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""DNCLOSE(",    TOK_OP_DNCLOSE,    OP_PRI_UNARY,    FALSE,  2 },
	{ "\x0B""DNCLOSEALL(", TOK_OP_DNCLOSEALL, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""DNEW(",       TOK_OP_DNEW,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""DNEXT(",      TOK_OP_DNEXT,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""DNOPEN(",     TOK_OP_DNOPEN,     OP_PRI_UNARY,    FALSE,  2 },
	{ "\x06""DOPEN(",      TOK_OP_DOPEN,      OP_PRI_UNARY,    FALSE,  3 },
	{ "\x04""DOW(",        TOK_OP_DOW,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""DPACK(",      TOK_OP_DPACK,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""DPUT(",       TOK_OP_DPUT,       OP_PRI_UNARY,    FALSE,  3 },
	{ "\x08""DRECALL(",    TOK_OP_DRECALL,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""DRECCOUNT(",  TOK_OP_DRECCOUNT,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""DRECNO(",     TOK_OP_DRECNO,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0B""DRIVESPACE(", TOK_OP_DRIVESPACE, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""DSEEK(",      TOK_OP_DSEEK,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""DSELECT(",    TOK_OP_DSELECT,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""DSETALIAS(",  TOK_OP_DSETALIAS,  OP_PRI_UNARY,    FALSE,  2 },
	{ "\x06""DSKIP(",      TOK_OP_DSKIP,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x05""DTAG(",       TOK_OP_DTAG,       OP_PRI_UNARY,    FALSE,  2 },
	{ "\x05""DTOP(",       TOK_OP_DTOP,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""DTYPE(",      TOK_OP_DTYPE,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""DUNLOCK(",    TOK_OP_DUNLOCK,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0B""ERRCORRECT(", TOK_OP_ERRCORRECT, OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""EVTTIMEADJ(", TOK_OP_EVTTIMEADJ, OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""EXIST(",      TOK_OP_EXIST,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""FDOQRD(",     TOK_OP_FDOQRD,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""FDORDAKA(",   TOK_OP_FDORDAKA,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""FDORDAREA(",  TOK_OP_FDORDAREA,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""FDORDORG(",   TOK_OP_FDORDORG,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""FERR(",       TOK_OP_FERR,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""FILEINF(",    TOK_OP_FILEINF,    OP_PRI_UNARY,    FALSE,  2 },
	{ "\x0A""FINDFIRST(",  TOK_OP_FINDFIRST,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""FINDNEXT(",   TOK_OP_FINDNEXT,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""FLAGCNT(",    TOK_OP_FLAGCNT,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""FMTCC(",      TOK_OP_FMTCC,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""FMTREAL(",    TOK_OP_FMTREAL,    OP_PRI_UNARY,    FALSE,  3 },
	{ "\x06""FNEXT(",      TOK_OP_FNEXT,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""FTELL(",      TOK_OP_FTELL,      OP_PRI_UNARY,    FALSE,  1 },
	/* 15.4 additions — sorted alphabetically per convention */
	{ "\x0B""GETBANKVAL(", TOK_OP_GETBANKVAL, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""GETDRIVE(",   TOK_OP_GETDRIVE,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""GETENV(",     TOK_OP_GETENV,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""GETMSGHDR(",  TOK_OP_GETMSGHDR,  OP_PRI_UNARY,    FALSE,  3 },
	{ "\x09""GETTOKEN(",   TOK_OP_GETTOKEN,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""GETX(",       TOK_OP_GETX,       OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""GETY(",       TOK_OP_GETY,       OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""GRAFMODE(",   TOK_OP_GRAFMODE,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""HELPPATH(",   TOK_OP_HELPPATH,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0A""HICONFNUM(",  TOK_OP_HICONFNUM,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""HIMSGNUM(",   TOK_OP_HIMSGNUM,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""HOUR(",       TOK_OP_HOUR,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""I2BD(",       TOK_OP_I2BD,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""I2BS(",       TOK_OP_I2BS,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x04""I2S(",        TOK_OP_I2S,        OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""INBYTES(",    TOK_OP_INBYTES,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""INKEY(",      TOK_OP_INKEY,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""INSTR(",      TOK_OP_INSTR,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x07""INSTRR(",     TOK_OP_INSTRR,     OP_PRI_UNARY,    FALSE,  2 },
	{ "\x09""ISBITSET(",   TOK_OP_ISBITSET,   OP_PRI_UNARY,    FALSE,  2 },
	{ "\x0A""ISNONSTOP(",  TOK_OP_ISNONSTOP,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""KBDBUFSIZE(", TOK_OP_KBDBUFSIZE, OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""KBDFILUSED(", TOK_OP_KBDFILUSED, OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""KINKEY(",     TOK_OP_KINKEY,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""LANGEXT(",    TOK_OP_LANGEXT,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""LASTANS(",    TOK_OP_LASTANS,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""LEFT(",       TOK_OP_LEFT,       OP_PRI_UNARY,    FALSE,  2 },
	{ "\x04""LEN(",        TOK_OP_LEN,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""LOGGEDON(",   TOK_OP_LOGGEDON,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""LOMSGNUM(",   TOK_OP_LOMSGNUM,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""LOWER(",      TOK_OP_LOWER,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""LPRINTED(",   TOK_OP_LPRINTED,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""LTRIM(",      TOK_OP_LTRIM,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x0B""MASK_ALNUM(", TOK_OP_MASKAN,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""MASK_ALPHA(", TOK_OP_MASKALF,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""MASK_ASCII(", TOK_OP_MASKASC,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0A""MASK_FILE(",  TOK_OP_MASKFIL,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""MASK_NUM(",   TOK_OP_MASKNUM,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0A""MASK_PATH(",  TOK_OP_MASKPTH,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""MASK_PWD(",   TOK_OP_MASKPWD,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""MAXNODE(",    TOK_OP_MAXNODE,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""MEGANUM(",    TOK_OP_MEGANUM,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""MGETBYTE(",   TOK_OP_MGETBYTE,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""MID(",        TOK_OP_MID,        OP_PRI_UNARY,    FALSE,  3 },
	{ "\x04""MIN(",        TOK_OP_MIN,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""MINKEY(",     TOK_OP_MINKEY,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""MINLEFT(",    TOK_OP_MINLEFT,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""MINON(",      TOK_OP_MINON,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""MIXED(",      TOK_OP_MIXED,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""MKADDR(",     TOK_OP_MKADDR,     OP_PRI_UNARY,    FALSE,  2 }, // * NO DEMO
	{ "\x07""MKDATE(",     TOK_OP_MKDATE,     OP_PRI_UNARY,    FALSE,  3 },
	{ "\x06""MODEM(",      TOK_OP_MODEM,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""MONTH(",      TOK_OP_MONTH,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""NOCHAR(",     TOK_OP_NOCHAR,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""NOT(",        TOK_OP_BNOT,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""ONLOCAL(",    TOK_OP_ONLOCAL,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x03""OR(",         TOK_OP_BOR,        OP_PRI_UNARY,    FALSE,  2 },
	{ "\x03""OS(",         TOK_OP_OS,         OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""OUTBYTES(",   TOK_OP_OUTBYTES,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""PAGESTAT(",   TOK_OP_PAGESTAT,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""PCBACCOUNT(", TOK_OP_PCBACCOUNT, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0B""PCBACCSTAT(", TOK_OP_PCBACCSTAT, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""PCBDAT(",     TOK_OP_PCBDAT,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""PCBMAC(",     TOK_OP_PCBMAC,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""PCBNODE(",    TOK_OP_PCBNODE,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""PEEK(",       TOK_OP_PEEKB,      OP_PRI_UNARY,    FALSE,  1 }, // * NO DEMO
	{ "\x06""PEEKB(",      TOK_OP_PEEKB,      OP_PRI_UNARY,    FALSE,  1 }, // * NO DEMO
	{ "\x07""PEEKDW(",     TOK_OP_PEEKDW,     OP_PRI_UNARY,    FALSE,  1 }, // * NO DEMO
	{ "\x06""PEEKW(",      TOK_OP_PEEKW,      OP_PRI_UNARY,    FALSE,  1 }, // * NO DEMO
	{ "\x08""PPENAME(",    TOK_OP_PPENAME,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""PPEPATH(",    TOK_OP_PPEPATH,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0B""PPLBUFSIZE(", TOK_OP_PPLBUFSIZE, OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""PSA(",        TOK_OP_PSA,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""QWKLIMITS(",  TOK_OP_QWKLIMITS,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""RANDOM(",     TOK_OP_RANDOM,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""READLINE(",   TOK_OP_READLINE,   OP_PRI_UNARY,    FALSE,  2 },
	{ "\x06""REGAH(",      TOK_OP_REGAH,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGAL(",      TOK_OP_REGAL,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGAX(",      TOK_OP_REGAX,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGBH(",      TOK_OP_REGBH,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGBL(",      TOK_OP_REGBL,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGBX(",      TOK_OP_REGBX,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGCF(",      TOK_OP_REGCF,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGCH(",      TOK_OP_REGCH,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGCL(",      TOK_OP_REGCL,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGCX(",      TOK_OP_REGCX,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGDH(",      TOK_OP_REGDH,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGDI(",      TOK_OP_REGDI,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGDL(",      TOK_OP_REGDL,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGDS(",      TOK_OP_REGDS,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGDX(",      TOK_OP_REGDX,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGES(",      TOK_OP_REGES,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x05""REGF(",       TOK_OP_REGF,       OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x06""REGSI(",      TOK_OP_REGSI,      OP_PRI_UNARY,    FALSE,  0 }, // * NO DEMO
	{ "\x08""REPLACE(",    TOK_OP_REPLACE,    OP_PRI_UNARY,    FALSE,  3 },
	{ "\x0B""REPLACESTR(", TOK_OP_REPSTR,     OP_PRI_UNARY,    FALSE,  3 },
	{ "\x06""RIGHT(",      TOK_OP_RIGHT,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x07""RIPVER(",     TOK_OP_RIPVER,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""RTRIM(",      TOK_OP_RTRIM,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x04""S2I(",        TOK_OP_S2I,        OP_PRI_UNARY,    FALSE,  2 },
	{ "\x0B""SCANMSGHDR(", TOK_OP_SCANMSGHDR, OP_PRI_UNARY,    FALSE,  4 },
	{ "\x08""SCRTEXT(",    TOK_OP_SCRTEXT,    OP_PRI_UNARY,    FALSE,  4 },
	{ "\x04""SEC(",        TOK_OP_SEC,        OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""SETDRIVE(",   TOK_OP_SETDRIVE,   OP_PRI_UNARY,    FALSE,  1 },
	/* 15.4 addition */
	{ "\x0A""SETMSGHDR(",  TOK_OP_SETMSGHDR,  OP_PRI_UNARY,    FALSE,  4 },
	{ "\x0A""SHORTDESC(",  TOK_OP_SHORTDESC,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""SHOWSTAT(",   TOK_OP_SHOWSTAT,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""SLPATH(",     TOK_OP_SLPATH,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""SPACE(",      TOK_OP_SPACE,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""STACKERR(",   TOK_OP_STACKERR,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0A""STACKLEFT(",  TOK_OP_STACKLEFT,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""STRING(",     TOK_OP_TOSTRING,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""STRIP(",      TOK_OP_STRIP,      OP_PRI_UNARY,    FALSE,  2 },
	{ "\x09""STRIPATX(",   TOK_OP_STRIPATX,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""STRIPSTR(",   TOK_OP_STRSTR,     OP_PRI_UNARY,    FALSE,  2 },
	{ "\x09""SYSOPSEC(",   TOK_OP_SYSOPSEC,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""TEMPPATH(",   TOK_OP_TEMPPATH,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x05""TIME(",       TOK_OP_TIME,       OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""TIMEAP(",     TOK_OP_TIMEAP,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""TINKEY(",     TOK_OP_TINKEY,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""TOBIGSTR(",   TOK_OP_TOBIGSTR,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""TOBOOLEAN(",  TOK_OP_TOBOOLEAN,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""TOBYTE(",     TOK_OP_TOBYTE,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""TODATE(",     TOK_OP_TODATE,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TODDATE(",    TOK_OP_TODDATE,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""TODOUBLE(",   TOK_OP_TODREAL,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TODREAL(",    TOK_OP_TODREAL,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TODWORD(",    TOK_OP_TOUNSIGNED, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOEDATE(",    TOK_OP_TOEDATE,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOFLOAT(",    TOK_OP_TOREAL,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x06""TOINT(",      TOK_OP_TOSWORD,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""TOINTEGER(",  TOK_OP_TOINTEGER,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""TOKCOUNT(",   TOK_OP_TOKCOUNT,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""TOKENSTR(",   TOK_OP_TOKENSTR,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""TOLONG(",     TOK_OP_TOINTEGER,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOMONEY(",    TOK_OP_TOMONEY,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""TOREAL(",     TOK_OP_TOREAL,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOSBYTE(",    TOK_OP_TOSBYTE,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""TOSDWORD(",   TOK_OP_TOINTEGER,  OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOSHORT(",    TOK_OP_TOSBYTE,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""TOSTRING(",   TOK_OP_TOSTRING,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOSWORD(",    TOK_OP_TOSWORD,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""TOTIME(",     TOK_OP_TOTIME,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOUBYTE(",    TOK_OP_TOBYTE,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""TOUDWORD(",   TOK_OP_TOUNSIGNED, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0B""TOUNSIGNED(", TOK_OP_TOUNSIGNED, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""TOUWORD(",    TOK_OP_TOWORD,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""TOWORD(",     TOK_OP_TOWORD,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x05""TRIM(",       TOK_OP_TRIM,       OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""UN_CITY(",    TOK_OP_UN_CITY,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""UN_NAME(",    TOK_OP_UN_NAME,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""UN_OPER(",    TOK_OP_UN_OPER,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""UN_STAT(",    TOK_OP_UN_STAT,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""UPPER(",      TOK_OP_UPPER,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""USELMRS(",    TOK_OP_USELMRS,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x0A""USERALIAS(",  TOK_OP_USERALIAS,  OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""U_BDL(",      TOK_OP_U_BDL,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""U_BDLDAY(",   TOK_OP_U_BDLDAY,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""U_BUL(",      TOK_OP_U_BUL,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""U_FDL(",      TOK_OP_U_FDL,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""U_FUL(",      TOK_OP_U_FUL,      OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""U_INCONF(",   TOK_OP_U_INCONF,   OP_PRI_UNARY,    FALSE,  2 },
	{ "\x08""U_LDATE(",    TOK_OP_U_LDATE,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""U_LDIR(",     TOK_OP_U_LDIR,     OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""U_LMR(",      TOK_OP_U_LMR,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""U_LOGONS(",   TOK_OP_U_LOGONS,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""U_LTIME(",    TOK_OP_U_LTIME,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""U_MSGRD(",    TOK_OP_U_MSGRD,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""U_MSGWR(",    TOK_OP_U_MSGWR,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x07""U_NAME(",     TOK_OP_U_NAME,     OP_PRI_UNARY,    FALSE,  0 },
	/* 15.4 addition — U_PERSONAL(idx) reads the personal PSA fields */
	{ "\x0B""U_PERSONAL(", TOK_OP_U_PERSONAL, OP_PRI_UNARY,    FALSE,  1 },
	{ "\x0A""U_PWDHIST(",  TOK_OP_U_PWDHST,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""U_PWDLC(",    TOK_OP_U_PWDLC,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x08""U_PWDTC(",    TOK_OP_U_PWDTC,    OP_PRI_UNARY,    FALSE,  0 },
	{ "\x09""U_RECNUM(",   TOK_OP_U_RECNUM,   OP_PRI_UNARY,    FALSE,  1 },
	{ "\x07""U_STAT(",     TOK_OP_U_STAT,     OP_PRI_UNARY,    FALSE,  1 },
	{ "\x09""U_TIMEON(",   TOK_OP_U_TIMEON,   OP_PRI_UNARY,    FALSE,  0 },
	{ "\x06""VALCC(",      TOK_OP_VALCC,      OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""VALDATE(",    TOK_OP_VALDATE,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""VALTIME(",    TOK_OP_VALTIME,    OP_PRI_UNARY,    FALSE,  1 },
	{ "\x04""VER(",        TOK_OP_VER,        OP_PRI_UNARY,    FALSE,  0 },
	{ "\x04""XOR(",        TOK_OP_BXOR,       OP_PRI_UNARY,    FALSE,  2 },
	{ "\x05""YEAR(",       TOK_OP_YEAR,       OP_PRI_UNARY,    FALSE,  1 },
	{ "\x08""YESCHAR(",    TOK_OP_YESCHAR,    OP_PRI_UNARY,    FALSE,  0 },

	{ NULL, 			   0,				  0,			   0,	  -1 },

	{ "\x02""==",          TOK_OP_EQ,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""!=",          TOK_OP_NE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""<>",          TOK_OP_NE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""><",          TOK_OP_NE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""<=",          TOK_OP_LE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""=<",          TOK_OP_LE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02"">=",          TOK_OP_GE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""=>",          TOK_OP_GE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x02""&&",          TOK_OP_AND,        OP_PRI_AND,      TRUE,  -1 },
	{ "\x02""||",          TOK_OP_OR,         OP_PRI_OR,       TRUE,  -1 },
	{ "\x02""**",          TOK_OP_EXP,        OP_PRI_EXP,      TRUE,  -1 },

	{ "\x01"")",           TOK_OP_CPAR,       OP_PRI_CPAR,     TRUE,  -1 },

//	{ "\x01""+",           TOK_OP_UPLUS,      OP_PRI_UNARY,    FALSE, -1 },
//	{ "\x01""-",           TOK_OP_UMINUS,     OP_PRI_UNARY,    FALSE, -1 },

	{ "\x01""^",           TOK_OP_EXP,        OP_PRI_EXP,      TRUE,  -1 },

	{ "\x01""*",           TOK_OP_TIMES,      OP_PRI_MULT,     TRUE,  -1 },
	{ "\x01""/",           TOK_OP_DIVIDE,     OP_PRI_MULT,     TRUE,  -1 },
	{ "\x01""%",           TOK_OP_MOD,        OP_PRI_MULT,     TRUE,  -1 },

	{ "\x01""+",           TOK_OP_PLUS,       OP_PRI_ADD,      TRUE,  -1 },
	{ "\x01""+",           TOK_OP_UPLUS,      OP_PRI_UNARY,    FALSE, -1 },
	{ "\x01""-",           TOK_OP_MINUS,      OP_PRI_ADD,      TRUE,  -1 },
	{ "\x01""-",           TOK_OP_UMINUS,     OP_PRI_UNARY,    FALSE, -1 },

	{ "\x01""=",           TOK_OP_EQ,         OP_PRI_EQUALITY, TRUE,  -1 },
//	{ "\x02""<>",          TOK_OP_NE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x01""<",           TOK_OP_LT,         OP_PRI_EQUALITY, TRUE,  -1 },
//	{ "\x02""<=",          TOK_OP_LE,         OP_PRI_EQUALITY, TRUE,  -1 },
	{ "\x01"">",           TOK_OP_GT,         OP_PRI_EQUALITY, TRUE,  -1 },
//	{ "\x02"">=",          TOK_OP_GE,         OP_PRI_EQUALITY, TRUE,  -1 },

	{ "\x01""!",           TOK_OP_NOT,        OP_PRI_NOT,      FALSE, -1 },

	{ "\x01""&",           TOK_OP_AND,        OP_PRI_AND,      TRUE,  -1 },

	{ "\x01""|",           TOK_OP_OR,         OP_PRI_OR,       TRUE,  -1 },

	{ "\x01""(",           TOK_OP_OPAR,       OP_PRI_OPAR,     FALSE, -1 },

	{ NULL, 			   0,				  0,			   0,	  -1 }
};

#endif

/******************************************************************************/

// Variables

#ifdef	___COMP___
char * cSCRIPT::nextToken = NULL;
#endif

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)
#pragma argsused
#endif
cSCRIPT::cSCRIPT(int numToks) :
	varCount(0),
	wvErr(NULL),
	wvHand(NULL)
#if defined(___COMP___)
	,
	labLst(DD_YES,FALSE),
//	  varLst(DD_YES,FALSE),
	varLst(2048),
	tokenStack(NULL),
	tokenStkStk(DD_YES,FALSE),
	argStack(DD_YES,FALSE),
	opStack(DD_YES,FALSE)
#endif
#if defined(___EXEC___)
	,
	altUserAvailable(TRUE),
	defInChan(0),
	defOutChan(0),
	ppeVer(MAX_PPE_VER),
	varLst(NULL),
	tokenCnt(numToks),
	varStack(NULL),
	varStkStk(DD_YES,FALSE),
	prgProcStkStk(DD_YES,FALSE),
	currProcStack(DD_NO,FALSE),
	prgVarStack(DD_NO,FALSE),
	prgProcVarStack(DD_YES,FALSE),
	prgProcGlobVarStack(DD_NO,FALSE),
	finalResult(vtSTRING),
	tmpBoolean(vtBOOLEAN),
	tmpDate(vtDATE),
	tmpInteger(vtINTEGER),
	tmpString(vtSTRING),
	tmpBigStr(vtBIGSTR),
	tmpDReal(vtDREAL),
	tmpScript(NULL),
	ansAppended(FALSE),
	lastUserGot(NO_USER_GOT),
	searchAvailable(FALSE),
	searchInUse(FALSE),
	searchTokens(0),
	searchFound(FALSE),
	tmpRipMode(Control.RipMode),
	tmpGraphMode(Control.GraphicsMode),
	tmpUseAnsi(UseAnsi),
	ptrUData(&UsersData),
	ptrUReg(ConfReg),
	ptrULMR(MsgReadPtr)
#endif
#if defined(___INFO___) || defined(___SCAN___)
	,
	ppeVer(MAX_PPE_VER),
	varLst(NULL)
#endif
{
#ifdef	___COMP___
	scriptBuffer = (char far *) farmalloc(SCR_BUF_SIZE);
	inCase			  = FALSE;
#else
	scriptBuffer = NULL;
#endif
   int i;

#ifdef	___EXEC___

    for(i = 0; i < MAX_FILE_CHAN; i++)
	  memset(&fileArr[i],0,sizeof(fileArr[i]));

	*readLineFile = '\0';
	memset(&readLineStream,0,sizeof(readLineStream));
	readLineLine = 0;

	readusernetrecord(PcbData.NodeNum,&unInfo);

	#ifndef __OS2__  // may need to finish (but may not be implemented)
	  memset(&intrREGS,0,sizeof(intrREGS));
	  memset(&intrSEGS,0,sizeof(intrSEGS));
	#endif

	altUserAvailable  = (TempRead == NULL);

	searchAvailable   = !insearch();
	*searchInput	  = '\0';
	*searchCodedInput = '\0';
	numCalls		  = 0;				// Numbar of calls to ::execute
	breakFlag		  = 0;				// When this is set execution of a PPE is completly aborted
	currProc		  = NULL;			// Current process (function or procedure within a PPE
	stackErrFlag	  = FALSE;			// Indicates a stack error has occured while executing a PPE
	stackErrContFlag  = TRUE;			// A flag the PPE programmer can set so they can eithercontinue execution or abort execution when a stack error occures within a PPE
	altUserLMRFlag	  = TRUE;			// Tells Getuservars if it needs to init LMR data
	swapped 		  = FALSE;
	rled			  = TRUE;

	varStkStk.push(NULL);
	prgProcStkStk.push(NULL);
	prgVarStack.push(NULL);
	prgProcVarStack.push(NULL);
	prgProcGlobVarStack.push(NULL);
	currProcStack.push(NULL);

	#ifdef DBASE
      for(i = 0; i < MAX_DBF_CHANNELS; i++)
		  dbfArr[i].myChannel = i;
	#endif

	slrand(exacttime());

#endif

#if defined(___COMP___) && defined(CHECK_TABLES)

	printf("\n\n\n\nChecking Tables ...\n\n");

	int fail = 0, failFlag = FALSE;

    for (i = 0; kwArray[i].token != NULL; ++i)
	{
		int oldFail = fail;

		printf("Checking statement \"%s\" ... ",kwArray[i].token+1);
		if (*kwArray[i].token != strlen(kwArray[i].token+1))
		{
			++fail;
			printf("LENGTH ERROR!\n");
		}
		else if (kwArray[i].useVarArgs && (kwArray[i].maxArgs <= kwArray[i].reqArgs))
		{
			++fail;
			printf("VARIABLE ARGUMENTS ERROR!\n");
		}
		else if (!kwArray[i].useVarArgs && (kwArray[i].maxArgs != kwArray[i].reqArgs))
		{
			++fail;
			printf("VARIABLE ARGUMENTS ERROR!\n");
		}

		for (int j = i+1; kwArray[j].token != NULL; ++j)
			if (strncmp(kwArray[i].token+1,kwArray[j].token+1,
				*kwArray[i].token) == 0)
			{
				++fail;
				printf("DUPLICATE START SUB-STRING ERROR!\n");
				break;
			}

		if (oldFail == fail)
			printf("\r                          %*.*s\r",
				*kwArray[i].token,*kwArray[i].token,"");
	}

	if (fail) printf("\n");
	printf("%d Statements Scanned, %d Statements Failed\n\n",i,fail);

	if (fail) failFlag = TRUE;

	i = 0;
	fail = 0;

	while (opArray[i].token != NULL)
	{
		printf("Checking function \"%s\" ... ",opArray[i].token+1);
		if (*opArray[i].token != strlen(opArray[i].token+1))
		{
			++fail;
			printf("LENGTH ERROR!\n");
		}
		else if ((strcmp(opArray[i].token+1,opArray[i+1].token+1) > 0) &&
			(opArray[i+1].token != NULL))
		{
			++fail;
			printf("ORDER ERROR!\n");
		}
		else if ((strcmp(opArray[i].token+1,opArray[i+1].token+1) == 0) &&
			(opArray[i+1].token != NULL))
		{
			++fail;
			printf("DUPLICATE ERROR!\n");
		}
		else
		{
			printf("\r                         %*.*s\r",*opArray[i].token,*opArray[i].token,"");
		}
		++i;
//		  if (getch() == 0) getch();
	}

	if (fail) printf("\n");
	printf("%d Functions Scanned, %d Functions Failed\n\n",i,fail);

	if (fail) failFlag = TRUE;

	if (i < NUM_UNIQUE_OPS)
	{
		failFlag = TRUE;
		printf("Not enough functions found in table, update NUM_UNIQUE_OPS\n\n");
	}
	else if (i > NUM_UNIQUE_OPS)
	{
		failFlag = TRUE;
		printf("Too many functions found in table, update NUM_UNIQUE_OPS\n\n");
	}

	int funcOff = i++;
	fail = 0;

	while (opArray[i].token != NULL)
	{
		printf("Checking operator \"%s\" ... ",opArray[i].token+1);
		if (*opArray[i].token != strlen(opArray[i].token+1))
		{
			++fail;
			printf("LENGTH ERROR!\n");
		}
		else
		{
			printf("\r                         %*.*s\r",*opArray[i].token,*opArray[i].token,"");
		}
		++i;
//		  if (getch() == 0) getch();
	}

	if (fail) printf("\n");
	printf("%d Operators Scanned, %d Operators Failed\n\n",i-funcOff,fail);

	if (fail) failFlag = TRUE;

	if (failFlag)
	{
		printf("Hit a key to continue ... ");
		if (getch() == 0) getch();
	}


#endif

}

	/*--------------------------------------------------------------------*/

cSCRIPT::~cSCRIPT(void)
{
    int i;

	#ifndef __OS2__
	  nosound(); // Ensure the speaker isn't left on because of a PPE
	#endif

	if (scriptBuffer != NULL) free(scriptBuffer);

#if defined(___EXEC___) || defined(___INFO___)

	if (varLst)
	{
        for (i = 0; i < varCount; ++i)
			if (varLst[i]) delete varLst[i];
		delete varLst;
	}

#endif

#ifdef	___EXEC___

    for (i = 0; i < MAX_FILE_CHAN; ++i)
		if (fileArr[i].inUse)
			dosfclose(&(fileArr[i].hand));

	#ifdef DBASE
	  for (i = 0; i < MAX_DBF_CHANNELS; ++i)
		  if(dbfArr[i].inUse) dbfArr[i].dbfClose();
	#endif

	if (readLineStream.handle != 0) dosfclose(&readLineStream);

	Display.ShowOnScreen = TRUE; // Ensure writing to screen is on at exit
	Control.RipMode 	 = tmpRipMode;
	Control.GraphicsMode = tmpGraphMode;
	UseAnsi 			 = tmpUseAnsi;

	if (tmpScript != NULL) delete tmpScript;

	if (altUserAvailable)
		if (TempRead != NULL)
			tempuserdealloc();

	if (searchInUse) stopsearch();


#endif

#ifdef	___COMP___

	labLst.flush(DD_YES);
//	  varLst.flush(DD_YES);

	while (!tokenStkStk.isEmpty())
	{
		tokenStack = tokenStkStk.pop();
		if (tokenStack != NULL)
		{
			delete tokenStack;
			tokenStack = NULL;
		}
	}

	while (!forStepExpr.isEmpty())
	{
		char * p = forStepExpr.pop();
		if (p != NULL) free(p);
	}

#endif

#ifdef	___EXEC___
	while (!varStkStk.isEmpty())
	{
		varStack = varStkStk.pop();
		if (varStack != NULL)
		{
			while (!varStack->isEmpty())
			{
				cVARVAL * v = varStack->pop();
				delete v;
			}
			delete varStack;
			varStack = NULL;
		}
	}

	tVS * tmp;
	while((tmp = prgProcStkStk.pop()) != NULL)
	{
		 while(!tmp->isEmpty())
		   tmp->pop();
		 if(tmp != NULL)
		   delete tmp;
	}


#endif
}

	/*--------------------------------------------------------------------*/

#ifdef	___COMP___

void LIBENTRY cSCRIPT::initialize(int pass)
{
	if (pass == 1)
	{
		if (scriptBuffer != NULL)
			_fmemset(scriptBuffer,0,size_t(SCR_BUF_SIZE));
		labLst.flush(DD_YES);
//		  varLst.flush(DD_YES);
		curVarID = 0;
	}

	startStruct.flush();
	cntrlStruct.flush();
	endStruct.flush();
	loopStruct.flush();

	destLabelCount = 0;
	endLabelCount = 0;
	loopLevel = 0;
	sysLabelCount = 0;
	genSysLabel = FALSE;

	scriptPointer = scriptBuffer;
	scriptSize = 0;
}

#endif

	/*--------------------------------------------------------------------*/

#ifdef ___COMP___

// int * tmpVC;
int * tmpER;
int * tmpHN;

//#pragma argsused
//void LIBENTRY cSCRIPT::countVars(cVAR & obj)
//{
//	  ++(*tmpVC);
//}
//
//	  /*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::writeVars(cVAR & obj)
{
	char tmpBuf[80];

	memcpy(tmpBuf,&obj.info,sizeof(obj.info));
#ifdef CRYPT
	encrypt2(tmpBuf,sizeof(obj.info));
	encrypt3(tmpBuf,sizeof(obj.info));		  // Added for 15.3
#endif
	*tmpER = *tmpER || (writecheck(*tmpHN,tmpBuf/*&obj.info*/,sizeof(obj.info)) == -1);

	if (!*tmpER)
	{
		if (obj.data->type == vtSTRING)
		{
			char * ptr = nul2mty(*obj.data);
			short  len = strlen(ptr)+1; 	  // DWT changed from int to short
#ifdef CRYPT
			encrypt2(ptr,len);
			encrypt3(ptr,len);	  // added for 15.3
#endif
			*tmpER = (writecheck(*tmpHN,&len,sizeof(len)) == -1) ||
					 (writecheck(*tmpHN,ptr,len)		  == -1);
#ifdef CRYPT

			decrypt3(ptr,len);	 // Added for 15.3
			decrypt2(ptr,len);
#endif
		}

		else
		{
			memcpy(tmpBuf,obj.data,sizeof(cVARVAL));
#ifdef CRYPT
			encrypt2(tmpBuf,sizeof(cVARVAL));
			encrypt3(tmpBuf,sizeof(cVARVAL));	// Added for 15.3
#endif
			*tmpER = (writecheck(*tmpHN,tmpBuf/*obj.data*/,sizeof(cVARVAL)) == -1);
		}
	}
}

	/*--------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::save(char * f)
{
	if (scriptBuffer == NULL)
	{
		scriptErr(0,-1,SCR_ERR_MEM,"SCRIPT BUFFER");
		return TRUE;
	}

	char filename[3+128+1+12+1];
	strcpy(filename,f);

	char * ptr = strrchr(filename,'\\');
	ptr = (ptr ? ++ptr : filename);

	char * dot = strchr(ptr,'.');
	ptr = (dot ? dot : (ptr+strlen(ptr)));

	strcpy(ptr,".PPE");

	int hand = doscreatecheck(filename,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);
	if (hand == -1) return TRUE;

	int err = (writecheck(hand,HDR_TXT,HDR_SIZ) == -1);

	if (!err)
	{
		varLst.sortIDReverse();

		varCount = varLst.used();
		err = writecheck(hand,&varCount,sizeof(varCount));

		tmpER = wvErr  = &err;
		tmpHN = wvHand = &hand;
		varLst.forEach(cSCRIPT::writeVars);
	}

	err = err || (writecheck(hand,&scriptSize,2/*sizeof(scriptSize)*/) == -1);

	// Make temporary buffer to RLE scriptBuffer into and RLE entire scriptBuffer
#ifdef CRYPT
	char * tmpScriptBuffer = (char * ) farmalloc(65000U);
	if(tmpScriptBuffer == NULL)
	{
		scriptErr(0,-1,SCR_ERR_MEM,"SCRIPT BUFFER");
		return TRUE;
	}
	uint newScriptSize = 0;
	bool rle = FALSE;
	newScriptSize = doRLE(tmpScriptBuffer,scriptBuffer,size_t(scriptSize));
	//scriptSize = doRLE(tmpScriptBuffer,scriptBuffer,size_t(scriptSize));
	if(newScriptSize < scriptSize)
	{
	  farfree(scriptBuffer);
	  realloc(tmpScriptBuffer,size_t(newScriptSize));
	  scriptBuffer = tmpScriptBuffer;
	  scriptSize = newScriptSize;
	  rle = TRUE;
	}
	else
	{
	  farfree(tmpScriptBuffer);
	  rle = FALSE;
	}
#endif

	if (!err)
	{
		char tmpBuf[2048];
		long tmpSize = 0;
		bool writeLastByte = FALSE;
		while (!err && (tmpSize < scriptSize))
		{
			long thisSize = scriptSize-tmpSize;
			if (thisSize > 2048) thisSize = 2048-1;
			_fmemcpy(tmpBuf,scriptBuffer+size_t(tmpSize),size_t(thisSize));
			if( (tmpBuf[int(thisSize)-1] == '\x0') && rle)
			  writeLastByte = TRUE;
#ifdef CRYPT
			encrypt2(tmpBuf,size_t(thisSize));
			encrypt3(tmpBuf,size_t(thisSize));		// added for 15.3
#endif
			err = (writecheck(hand,tmpBuf,size_t(thisSize)) == -1);
			if(writeLastByte)
			{
			   writecheck(hand,scriptBuffer+size_t(tmpSize)+size_t(thisSize),size_t(1));
			   writeLastByte = FALSE;
			   tmpSize++;
			}
			tmpSize += thisSize;
		}
	}

	dosclose(hand);

	return err;
}

#endif

	/*--------------------------------------------------------------------*/

#if defined(___EXEC___) || defined(___INFO___) || defined(___SCAN___)

int LIBENTRY cSCRIPT::load(char * f)
{
	DOSFILE File;
	if (dosfopen(f,OPEN_READ|OPEN_DENYWRIT,&File) == -1) return TRUE;

	char buf[80+1];
	char decryptBuf[80+1];
	long endOffset;
//	  memset(buf,0,sizeof(buf));

	// seek now instead of later so that we don't end up flushing the buffer
	endOffset = dosfseek(&File,0,SEEK_END);
	dosrewind(&File);

	int err = ((dosfread(buf,HDR_SIZ,&File) != HDR_SIZ) ||
			   (dosfread(&varCount,sizeof(varCount),&File) != sizeof(varCount)));

	err = err || (memcmp(buf,HDR_TXT,HDR_SIZ-9) != 0);

	if (!err)
	{
		if (buf[40] == ' ') buf[40] = '0';
		ppeVer = atoi(buf+40)*100+atoi(buf+43);
	}

	if (ppeVer > CUR_PPE_VER) err = TRUE;

	int i;

#if !defined(___SCAN___)
	if (!err) varLst = new cVARPTR[varCount];
err = err || (varLst == NULL);

	if (varLst)
		for (i = 0; i < varCount; ++i)
			varLst[i] = NULL;

#endif

	// DWT: varSize used to be set to sizeof(cVARVAL) but for compatibility we'll
	// DWT: hardcode a value of 12 here because on disk it is stored as 12 bytes
	int varSize = 12;

	// If it is an version 1.00 PPE file, then the var size is
	// sizeof(int)+sizeof(tINTEGER)+2 (magic number, meaning unknown)
	if (ppeVer == 100) varSize = sizeof(sint)+sizeof(tINTEGER)+2;	// DWT: changed from sizeof(int) to sizeof(sint)

	for (i = 0; !err && (i < varCount); ++i)
	{
		sVARINFO varInfo;

		err = err || (dosfread(decryptBuf/*&varInfo*/,sizeof(varInfo),&File) !=
			sizeof(varInfo));

#ifdef CRYPT
		// for PCBoard 15.3+ a new encryption was added on top of the existing

		if(ppeVer == 330)  decrypt3(decryptBuf,sizeof(varInfo)); // 15.3

		if(ppeVer >= 301)  decrypt2(decryptBuf,sizeof(varInfo));
#endif
		memcpy(&varInfo,decryptBuf,sizeof(varInfo));

#if !defined(___SCAN___)

		cVAR * v = new cVAR(NULL,varInfo);

		// If error creating variable, disp error and return
		if ((v == NULL) || (v->data == NULL))
		{
			if (v != NULL) delete v;
			scriptErr(0,-1,SCR_ERR_MEM,"VARIABLE DECLARATION");
			err = TRUE;
			break;
		}

#endif

		switch (varInfo.type)
		{
			case vtSTRING:
			{
				short len;				  // DWT:  changed from int to short
				char str[MAX_STR_LEN+1];
				err = (dosfread(&len,sizeof(len),&File) != sizeof(len)) ||
					  (dosfread(str,len,&File)			!= len);
#ifdef CRYPT

				if(ppeVer == 330) decrypt3(str,len); // 15.3

				if(ppeVer >= 301) decrypt2(str,len);
#endif
#if !defined(___SCAN___)
				*v->getVal(0,0,0) = str;
#endif
				break;
			}

			// NOTE:  Version 2.00 doesn't support vtBIGSTR constants,
			//		  so no special processing is required for that type

			default:
			{
				cVARVAL tmp(vtBOOLEAN);
				err = (dosfread(decryptBuf/*&tmp*/,varSize,&File) != varSize);
#ifdef CRYPT

				if(ppeVer == 330)  decrypt3(decryptBuf,varSize); //15.3

				if(ppeVer >= 301)  decrypt2(decryptBuf,varSize);
#endif

				#ifdef __OS2__
				  #ifdef __BORLANDC__
					memcpy(&tmp.type,decryptBuf+2,varSize-2);  // DWT:	skip over the virtual table
				  #else
					memcpy(&tmp,decryptBuf+2,varSize-2);  // DWT:  skip over the virtual table
				  #endif
				#else
				  memcpy(&tmp,decryptBuf,varSize);
				#endif
#if !defined(___SCAN___)
				*v->getVal(0,0,0) = tmp;
#endif
			}
		}

#if !defined(___SCAN___)
//		  varLst[varCount-i-1] = v;
		varLst[varInfo.id-1] = v;
#endif
	}

	scriptSize = 0;
	err = err || (dosfread(&scriptSize,2/*sizeof(scriptSize)*/,&File) !=
		2/*sizeof(scriptSize)*/);

	if (scriptSize > SCR_BUF_SIZE)
	{
		err = TRUE;
#ifdef PCB_DEMO
		printcls();
		print("This PPE file is too large for the DEMO version of PCBoard");
		newline();
		moreprompt(PRESSENTER);
		printcls();
		return TRUE;
#endif
	}

	if (!err)
	{
		if (scriptBuffer != NULL) free(scriptBuffer);
		scriptBuffer = (char *) malloc(scriptSize);

		if (scriptBuffer == NULL)
		{
			scriptErr(0,-1,SCR_ERR_MEM,"SCRIPT BUFFER");
			//err = TRUE;
			dosfclose(&File);
			return TRUE;
		}

		char tmpBuf[2048];
		long tmpSize = 0,codeSize=0;

		swapOffset = dosfseek(&File,0,SEEK_CUR);
		codeSize   = (endOffset - swapOffset);

		if(codeSize == scriptSize) rled = FALSE;

		int bytesRead;
		while ((bytesRead = dosfread(tmpBuf,size_t(sizeof(tmpBuf)-1),&File)) > 0)
		{
			if(ppeVer >= 301)
			{
#ifdef CRYPT

				if(ppeVer == 330) decrypt3(tmpBuf,bytesRead); // 15.3

				decrypt2(tmpBuf,bytesRead);

				// handle boundry condition if last byte is 0
				if(rled && tmpBuf[bytesRead-1] == '\x0')
				  bytesRead += dosfread(&tmpBuf[bytesRead],size_t(1),&File);
				if(rled)
					tmpSize += unRLE(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
				else
				{
					memcpy(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
					tmpSize += bytesRead;
				}

#else
				memcpy(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
				tmpSize += bytesRead;
#endif

			}
			else
			{
			  memcpy(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
			  tmpSize += bytesRead;
			}

		}
	}
	dosfclose(&File);

#if defined(___EXEC___)

	strcpy(PPEPath,f);
	char * ptr = ((PPEPath[1] == ':') ? PPEPath+2 : PPEPath);
	char * bs = strrchr(ptr,'\\');
	ptr = (bs ? ++bs : ptr);
	strcpy(PPEName,ptr);
	*ptr = '\0';

	ptr = strchr(PPEName,'.');
	if (ptr) *ptr = '\0';

#endif

	return err;
}


#endif
#ifdef ___EXEC___
void LIBENTRY cSCRIPT::swapOut(void)
{
	// Save current scriptpointer
	swapScriptPointer = (sint) (scriptPointer-scriptBuffer);  // DWT:  changed int to sint
	swapScriptSize	  = scriptSize;
	//far free scriptbuffer
	if (scriptBuffer != NULL) free(scriptBuffer);
	scriptBuffer = NULL;
}

int LIBENTRY cSCRIPT::swapIn(void)
{
	char f[MAX_STR_LEN+1];
	DOSFILE File;
	char tmpBuf[2048];
	long tmpSize = 0;

	sprintf(f,"%s%s.PPE",PPEPath,PPEName);
	if (dosfopen(f,OPEN_READ|OPEN_DENYWRIT,&File) == -1) return TRUE;
	dossetbuf(&File,2048);

	// far malloc scriptbuffer
	if (scriptBuffer != NULL) free(scriptBuffer);
	scriptBuffer = (char *) malloc(swapScriptSize);

	if (scriptBuffer == NULL)
	{
		scriptErr(0,-1,SCR_ERR_MEM,"SCRIPT BUFFER");
		return TRUE ;
	}
	dosfseek(&File,swapOffset,SEEK_SET);
	// read in code
	int bytesRead = 0;
	do
	{
		//long thisSize = scriptSize-tmpSize;
		//if (thisSize > 2048) thisSize = 2048;
		//err = (dosfread(tmpBuf,size_t(tmpBuf),&File) != thisSize);
		bytesRead = dosfread(tmpBuf,size_t(sizeof(tmpBuf)-1),&File);
		if(ppeVer >= 301 && bytesRead)
		{
#ifdef CRYPT

			if(ppeVer == 330) decrypt3(tmpBuf,bytesRead); // 15.3

			decrypt2(tmpBuf,bytesRead);

			// handle boundry condition if last byte is 0
			if(rled && tmpBuf[bytesRead-1] == '\x0')
			  bytesRead += dosfread(&tmpBuf[bytesRead],size_t(1),&File);

			// If the file has been RLE'd, unRLE it. If not just copy it.
			if(rled)
			   tmpSize += unRLE(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
			else
			   memcpy(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
#else
			memcpy(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
			tmpSize += bytesRead;
#endif

		}
		else
		{
		  if(bytesRead)
		  {
			memcpy(scriptBuffer+size_t(tmpSize),tmpBuf,size_t(bytesRead));
			tmpSize += bytesRead;
		  }
		}

	}while(bytesRead != 0);
	scriptPointer = (char *) (scriptBuffer+swapScriptPointer);
	dosfclose(&File);
	return FALSE;
}
#endif
/******************************************************************************/


