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
/*								   NEWSCR.HPP								  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*					Data and functions for the cSCRIPT class				  */
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

#ifndef ___NEWSCR_HPP___

#define ___NEWSCR_HPP___

/******************************************************************************/

/* Pragmas */

/******************************************************************************/

/* Included Files */

#include	<dos.h>

#include	"pcbmisc.hpp"

#include	"stack.hpp"

#include	"label.hpp"
#include	"var.hpp"

#if defined(DBASE) && !defined(___COMP___)
#include	"dbase.hpp"
#endif

/******************************************************************************/

/* Defined Macros */

#ifdef	PCB_DEMO
#define SCR_BUF_SIZE	(1024-1)
#else
#define SCR_BUF_SIZE	(65279L-1)  /* 65535-256 is the magic number */
#endif

#define SCR_VER 		1530L
#define STACK_CEILING	6022


#if defined(PCB_DEMO) && defined(___COMP___)
#define TOK_DEMO_BIT	0x8000
#else
#define TOK_DEMO_BIT	0x0000
#endif

	/*--------------------------------------------------------------------*/

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

#define NUM_UNIQUE_OPS	279

#define MAX_LINE_SIZE	2048

#define OP_PRI_CPAR 	9
#define OP_PRI_UNARY	8
#define OP_PRI_EXP		7
#define OP_PRI_MULT 	6
#define OP_PRI_ADD		5
#define OP_PRI_EQUALITY 4
#define OP_PRI_NOT		3
#define OP_PRI_AND		2
#define OP_PRI_OR		1
#define OP_PRI_OPAR 	0

#endif

	/*--------------------------------------------------------------------*/

#ifdef	___EXEC___

#define MAX_FILE_CHAN	8

#define MAX_DBF_CHANNELS 8
/* #define LA_NONE		   0 */
/* #define LA_READ		   1 */
/* #define LA_WRITE 	   2 */

#define NO_USER_GOT 	-1
#define CUR_USER_GOT	0

#define VID_U_EXPERT	1
#define VID_U_FSE		2
#define VID_U_FSEP		3
#define VID_U_CLS		4
#define VID_U_EXPDATE	5
#define VID_U_SEC		6
#define VID_U_PAGELEN	7
#define VID_U_EXPSEC	8
#define VID_U_CITY		9
#define VID_U_BDPHONE	10
#define VID_U_HVPHONE	11
#define VID_U_TRANS 	12
#define VID_U_CMNT1 	13
#define VID_U_CMNT2 	14
#define VID_U_PWD		15
#define VID_U_SCROLL	16
#define VID_U_LONGHDR	17
#define VID_U_DEF79 	18
#define VID_U_ALIAS 	19
#define VID_U_VER		20
#define VID_U_ADDR		21
#define VID_U_NOTES 	22
#define VID_U_PWDEXP	23
#define VID_U_ACCOUNT	24

#endif

	/*--------------------------------------------------------------------*/

#define TOK_OP_END		   0

#define TOK_OP_CPAR 	   -1

#define TOK_OP_UPLUS	   -2
#define TOK_OP_UMINUS	   -3

#define TOK_OP_EXP		   -4

#define TOK_OP_TIMES	   -5
#define TOK_OP_DIVIDE	   -6
#define TOK_OP_MOD		   -7

#define TOK_OP_PLUS 	   -8
#define TOK_OP_MINUS	   -9

#define TOK_OP_EQ		   -10
#define TOK_OP_NE		   -11
#define TOK_OP_LT		   -12
#define TOK_OP_LE		   -13
#define TOK_OP_GT		   -14
#define TOK_OP_GE		   -15

#define TOK_OP_NOT		   -16
#define TOK_OP_AND		   -17
#define TOK_OP_OR		   -18

#define TOK_OP_OPAR 	   -19

#define TOK_OP_LEN		   -20
#define TOK_OP_LOWER	   -21
#define TOK_OP_UPPER	   -22
#define TOK_OP_MID		   -23
#define TOK_OP_LEFT 	   -24
#define TOK_OP_RIGHT	   -25
#define TOK_OP_SPACE	   -26
#define TOK_OP_FERR 	   -27
#define TOK_OP_CHR		   -28
#define TOK_OP_ASC		   -29
#define TOK_OP_INSTR	   -30
#define TOK_OP_ABORT	   -31
#define TOK_OP_LTRIM	   -32
#define TOK_OP_RTRIM	   -33
#define TOK_OP_TRIM 	   -34
#define TOK_OP_RANDOM	   -35
#define TOK_OP_DATE 	   -36
#define TOK_OP_TIME 	   -37
#define TOK_OP_U_NAME	   -38
#define TOK_OP_U_LDATE	   -39
#define TOK_OP_U_LTIME	   -40
#define TOK_OP_U_LDIR	   -41
#define TOK_OP_U_LOGONS    -42
#define TOK_OP_U_FUL	   -43
#define TOK_OP_U_FDL	   -44
#define TOK_OP_U_BDLDAY    -45
#define TOK_OP_U_TIMEON    -46
#define TOK_OP_U_BDL	   -47
#define TOK_OP_U_BUL	   -48
#define TOK_OP_YEAR 	   -49
#define TOK_OP_MONTH	   -50
#define TOK_OP_DAY		   -51
#define TOK_OP_DOW		   -52
#define TOK_OP_HOUR 	   -53
#define TOK_OP_MIN		   -54
#define TOK_OP_SEC		   -55
#define TOK_OP_TIMEAP	   -56
#define TOK_OP_VER		   -57
#define TOK_OP_NOCHAR	   -58
#define TOK_OP_YESCHAR	   -59
#define TOK_OP_STRIPATX    -60
#define TOK_OP_REPLACE	   -61
#define TOK_OP_STRIP	   -62
#define TOK_OP_INKEY	   -63
#define TOK_OP_TOSTRING    -64
#define TOK_OP_MASKPWD	   -65
#define TOK_OP_MASKALF	   -66
#define TOK_OP_MASKNUM	   -67
#define TOK_OP_MASKAN	   -68
#define TOK_OP_MASKFIL	   -69
#define TOK_OP_MASKPTH	   -70
#define TOK_OP_MASKASC	   -71
#define TOK_OP_CURCONF	   -72
#define TOK_OP_PCBDAT	   -73
#define TOK_OP_PPEPATH	   -74
#define TOK_OP_VALDATE	   -75
#define TOK_OP_VALTIME	   -76
#define TOK_OP_U_MSGRD	   -77
#define TOK_OP_U_MSGWR	   -78
#define TOK_OP_PCBNODE	   -79
#define TOK_OP_READLINE    -80
#define TOK_OP_SYSOPSEC    -81
#define TOK_OP_ONLOCAL	   -82
#define TOK_OP_UN_STAT	   -83
#define TOK_OP_UN_NAME	   -84
#define TOK_OP_UN_CITY	   -85
#define TOK_OP_UN_OPER	   -86
#define TOK_OP_CURSEC	   -87
#define TOK_OP_GETTOKEN    -88
#define TOK_OP_MINLEFT	   -89
#define TOK_OP_MINON	   -90
#define TOK_OP_GETENV	   -91
#define TOK_OP_CALLID	   -92
#define TOK_OP_REGAL	  (-93^TOK_DEMO_BIT)
#define TOK_OP_REGAH	  (-94^TOK_DEMO_BIT)
#define TOK_OP_REGBL	  (-95^TOK_DEMO_BIT)
#define TOK_OP_REGBH	  (-96^TOK_DEMO_BIT)
#define TOK_OP_REGCL	  (-97^TOK_DEMO_BIT)
#define TOK_OP_REGCH	  (-98^TOK_DEMO_BIT)
#define TOK_OP_REGDL	  (-99^TOK_DEMO_BIT)
#define TOK_OP_REGDH	  (-100^TOK_DEMO_BIT)
#define TOK_OP_REGAX	  (-101^TOK_DEMO_BIT)
#define TOK_OP_REGBX	  (-102^TOK_DEMO_BIT)
#define TOK_OP_REGCX	  (-103^TOK_DEMO_BIT)
#define TOK_OP_REGDX	  (-104^TOK_DEMO_BIT)
#define TOK_OP_REGSI	  (-105^TOK_DEMO_BIT)
#define TOK_OP_REGDI	  (-106^TOK_DEMO_BIT)
#define TOK_OP_REGF 	  (-107^TOK_DEMO_BIT)
#define TOK_OP_REGCF	  (-108^TOK_DEMO_BIT)
#define TOK_OP_REGDS	  (-109^TOK_DEMO_BIT)
#define TOK_OP_REGES	  (-110^TOK_DEMO_BIT)
#define TOK_OP_B2W		   -111
#define TOK_OP_PEEKB	  (-112^TOK_DEMO_BIT)
#define TOK_OP_PEEKW	  (-113^TOK_DEMO_BIT)
#define TOK_OP_MKADDR	  (-114^TOK_DEMO_BIT)
#define TOK_OP_EXIST	   -115
#define TOK_OP_I2S		   -116
#define TOK_OP_S2I		   -117
#define TOK_OP_CARRIER	   -118
#define TOK_OP_TOKENSTR    -119
#define TOK_OP_CDON 	   -120
#define TOK_OP_LANGEXT	   -121
#define TOK_OP_ANSION	   -122
#define TOK_OP_VALCC	   -123
#define TOK_OP_FMTCC	   -124
#define TOK_OP_CCTYPE	   -125
#define TOK_OP_GETX 	   -126
#define TOK_OP_GETY 	   -127
#define TOK_OP_BAND 	   -128
#define TOK_OP_BOR		   -129
#define TOK_OP_BXOR 	   -130
#define TOK_OP_BNOT 	   -131
#define TOK_OP_U_PWDHST    -132
#define TOK_OP_U_PWDLC	   -133
#define TOK_OP_U_PWDTC	   -134
#define TOK_OP_U_STAT	   -135
#define TOK_OP_DEFCOLOR    -136
#define TOK_OP_ABS		   -137
#define TOK_OP_GRAFMODE    -138
#define TOK_OP_PSA		   -139
#define TOK_OP_FILEINF	   -140
#define TOK_OP_PPENAME	   -141
#define TOK_OP_MKDATE	   -142
#define TOK_OP_CURCOLOR    -143
#define TOK_OP_KINKEY	   -144
#define TOK_OP_MINKEY	   -145
#define TOK_OP_MAXNODE	   -146
#define TOK_OP_SLPATH	   -147
#define TOK_OP_HELPPATH    -148
#define TOK_OP_TEMPPATH    -149
#define TOK_OP_MODEM	   -150
#define TOK_OP_LOGGEDON    -151
#define TOK_OP_CALLNUM	   -152
#define TOK_OP_MGETBYTE    -153
#define TOK_OP_TOKCOUNT    -154
#define TOK_OP_U_RECNUM    -155
#define TOK_OP_U_INCONF    -156
#define TOK_OP_PEEKDW	  (-157^TOK_DEMO_BIT)
#define TOK_OP_DBGLEVEL    -158
#define TOK_OP_SCRTEXT	   -159
#define TOK_OP_SHOWSTAT    -160
#define TOK_OP_PAGESTAT    -161

/* The following were added for PPL 2.0 */

#define TOK_OP_REPSTR	   -162
#define TOK_OP_STRSTR	   -163
#define TOK_OP_TOBIGSTR    -164
#define TOK_OP_TOBOOLEAN   -165
#define TOK_OP_TOBYTE	   -166
#define TOK_OP_TODATE	   -167
#define TOK_OP_TODREAL	   -168
#define TOK_OP_TOEDATE	   -169
#define TOK_OP_TOINTEGER   -170
#define TOK_OP_TOMONEY	   -171
#define TOK_OP_TOREAL	   -172
#define TOK_OP_TOSBYTE	   -173
#define TOK_OP_TOSWORD	   -174
#define TOK_OP_TOTIME	   -175
#define TOK_OP_TOUNSIGNED  -176
#define TOK_OP_TOWORD	   -177
#define TOK_OP_MIXED	   -178
#define TOK_OP_ALIAS	   -179
#define TOK_OP_CONFREG	   -180
#define TOK_OP_CONFEXP	   -181
#define TOK_OP_CONFSEL	   -182
#define TOK_OP_CONFSYS	   -183
#define TOK_OP_CONFMW	   -184
#define TOK_OP_LPRINTED    -185
#define TOK_OP_ISNONSTOP   -186
#define TOK_OP_ERRCORRECT  -187
#define TOK_OP_CONFALIAS   -188
#define TOK_OP_USERALIAS   -189
#define TOK_OP_CURUSER	   -190
#define TOK_OP_U_LMR	   -191
#define TOK_OP_CHATSTAT    -192
#define TOK_OP_DEFANS	   -193
#define TOK_OP_LASTANS	   -194
#define TOK_OP_MEGANUM	   -195
#define TOK_OP_EVTTIMEADJ  -196
#define TOK_OP_ISBITSET    -197
#define TOK_OP_FMTREAL	   -198
#define TOK_OP_FLAGCNT	   -199
#define TOK_OP_KBDBUFSIZE  -200
#define TOK_OP_PPLBUFSIZE  -201
#define TOK_OP_KBDFILUSED  -202
#define TOK_OP_LOMSGNUM    -203
#define TOK_OP_HIMSGNUM    -204

/* The following were added for PPL 2.0+1 */

#define TOK_OP_DRIVESPACE  -205
#define TOK_OP_OUTBYTES    -206
#define TOK_OP_HICONFNUM   -207
#define TOK_OP_INBYTES	   -208
#define TOK_OP_CRC32	   -209
#define TOK_OP_PCBMAC	   -210
#define TOK_OP_ACTMSGNUM   -211
#define TOK_OP_STACKLEFT   -212
#define TOK_OP_STACKERR    -213

/* The following were added for PPL 3.0 */

#define TOK_OP_DGETALIAS   -214
#define TOK_OP_DBOF 	   -215
#define TOK_OP_DCHANGED    -216
#define TOK_OP_DDECIMALS   -217
#define TOK_OP_DDELETED    -218
#define TOK_OP_DEOF 	   -219
#define TOK_OP_DERR 	   -220
#define TOK_OP_DFIELDS	   -221
#define TOK_OP_DLENGTH	   -222
#define TOK_OP_DNAME	   -223
#define TOK_OP_DRECCOUNT   -224
#define TOK_OP_DRECNO	   -225
#define TOK_OP_DTYPE	   -226
#define TOK_OP_FNEXT	   -227
#define TOK_OP_DNEXT	   -228
#define TOK_OP_TODDATE	   -229
#define TOK_OP_DCLOSEALL   -230  /*FIRST DUP */
#define TOK_OP_DOPEN	   -231
#define TOK_OP_DCLOSE	   -232
#define TOK_OP_DSETALIAS   -233
#define TOK_OP_DPACK	   -234
#define TOK_OP_DLOCKF	   -235
#define TOK_OP_DLOCK	   -236
#define TOK_OP_DLOCKR	   -237
#define TOK_OP_DUNLOCK	   -238
#define TOK_OP_DNOPEN	   -239
#define TOK_OP_DNCLOSE	   -240
#define TOK_OP_DNCLOSEALL  -241
#define TOK_OP_DNEW 	   -242
#define TOK_OP_DADD 	   -243
#define TOK_OP_DAPPEND	   -244
#define TOK_OP_DTOP 	   -245
#define TOK_OP_DGO		   -246
#define TOK_OP_DBOTTOM	   -247
#define TOK_OP_DSKIP	   -248
#define TOK_OP_DBLANK	   -249
#define TOK_OP_DDELETE	   -250
#define TOK_OP_DRECALL	   -251
#define TOK_OP_DTAG 	   -252
#define TOK_OP_DSEEK	   -253
#define TOK_OP_DFBLANK	   -254
#define TOK_OP_DGET 	   -255
#define TOK_OP_DPUT 	   -256
#define TOK_OP_DFCOPY	   -257
#define TOK_OP_DSELECT	   -258
#define TOK_OP_DCHKSTAT    -259

#define TOK_OP_PCBACCOUNT  -260
#define TOK_OP_PCBACCSTAT  -261
#define TOK_OP_DERRMSG	   -262
#define TOK_OP_ACCOUNT	   -263
#define TOK_OP_SCANMSGHDR  -264
#define TOK_OP_CHECKRIP    -265
#define TOK_OP_RIPVER	   -266
#define TOK_OP_QWKLIMITS   -267
#define TOK_OP_FINDFIRST   -268
#define TOK_OP_FINDNEXT    -269
#define TOK_OP_USELMRS	   -270
#define TOK_OP_CONFINFO    -271
#define TOK_OP_TINKEY	   -272
#define TOK_OP_CWD		   -273
#define TOK_OP_INSTRR	   -274
#define TOK_OP_FDORDAKA    -275
#define TOK_OP_FDORDORG    -276
#define TOK_OP_FDORDAREA   -277
#define TOK_OP_FDOQRD	   -278
#define TOK_OP_GETDRIVE    -279
#define TOK_OP_SETDRIVE    -280
#define TOK_OP_BS2I 	   -281
#define TOK_OP_BD2I 	   -282
#define TOK_OP_I2BS 	   -283
#define TOK_OP_I2BD 	   -284
#define TOK_OP_FTELL	   -285
#define TOK_OP_OS		   -286


#define MAX_OP_TOK			286

/* CONFINF()  -> return a piece of information about the conference */
/* UN_BROAD() -> return broadcast information */

	/*--------------------------------------------------------------------*/

#define TOK_DREAL		(-1*vtDREAL)
#define TOK_BIGSTR		(-1*vtBIGSTR)
#define TOK_SWORD		(-1*vtSWORD)
#define TOK_SBYTE		(-1*vtSBYTE)
#define TOK_WORD		(-1*vtWORD)
#define TOK_BYTE		(-1*vtBYTE)
#define TOK_TIME		(-1*vtTIME)
#define TOK_STRING		(-1*vtSTRING)
#define TOK_REAL		(-1*vtREAL)
#define TOK_MONEY		(-1*vtMONEY)
#define TOK_INTEGER 	(-1*vtINTEGER)
#define TOK_EDATE		(-1*vtEDATE)
#define TOK_DATE		(-1*vtDATE)
#define TOK_UNSIGNED	(-1*vtUNSIGNED)
#define TOK_BOOLEAN 	(-1*vtBOOLEAN)
#define TOK_FUNCT		(-1*vtFUNCTION)
#define TOK_DDATE		(-1*vtDDATE)
#define TOK_END 		1
#define TOK_CLS 		2
#define TOK_CLREOL		3
#define TOK_MORE		4
#define TOK_WAIT		5
#define TOK_COLOR		6
#define TOK_GOTO		7
#define TOK_LET 		8
#define TOK_PRINT		9
#define TOK_PRINTLN 	10
#define TOK_IF			11
#define TOK_CONFFLAG	12
#define TOK_CONFUNFLAG	13
#define TOK_DISPFILE	14
#define TOK_INPUT		15
#define TOK_FCREATE 	16
#define TOK_FOPEN		17
#define TOK_FAPPEND 	18
#define TOK_FCLOSE		19
#define TOK_FGET		20
#define TOK_FPUT		21
#define TOK_FPUTLN		22
#define TOK_RESETDISP	23
#define TOK_STARTDISP	24
#define TOK_FPUTPAD 	25
#define TOK_HANGUP		26
#define TOK_GETUSER 	27
#define TOK_PUTUSER 	28
#define TOK_DEFCOLOR	29
#define TOK_DELETE		30
#define TOK_DELUSER 	31
#define TOK_ADJTIME 	32
#define TOK_LOG 		33
#define TOK_INPUTSTR	34
#define TOK_INPUTYN 	35
#define TOK_INPUTMONEY	36
#define TOK_INPUTINT	37
#define TOK_INPUTCC 	38
#define TOK_INPUTDATE	39
#define TOK_INPUTTIME	40
#define TOK_GOSUB		41
#define TOK_RETURN		42
#define TOK_PROMPTSTR	43
#define TOK_DTRON		44
#define TOK_DTROFF		45
#define TOK_CDCHKON 	46
#define TOK_CDCHKOFF	47
#define TOK_DELAY		48
#define TOK_SENDMODEM	49
#define TOK_INC 		50
#define TOK_DEC 		51
#define TOK_NEWLINE 	52
#define TOK_NEWLINES	53
#define TOK_TOKENIZE	54
#define TOK_GETTOKEN	55
#define TOK_SHELL	   (56^TOK_DEMO_BIT)
#define TOK_DISPTEXT	57
#define TOK_STOP		58
#define TOK_INPUTTEXT	59
#define TOK_BEEP		60
#define TOK_PUSH	   (61^TOK_DEMO_BIT)
#define TOK_POP 	   (62^TOK_DEMO_BIT)
#define TOK_KBDSTUFF	63
#define TOK_CALL	   (64^TOK_DEMO_BIT)
#define TOK_JOIN		65
#define TOK_QUEST		66
#define TOK_BLT 		67
#define TOK_DIR 		68
#define TOK_KBDFILE 	69
#define TOK_BYE 		70
#define TOK_GOODBYE 	71
#define TOK_BROADCAST	72
#define TOK_WAITFOR 	73
#define TOK_KBDCHKON	74
#define TOK_KBDCHKOFF	75
#define TOK_OPTEXT		76
#define TOK_DISPSTR 	77
#define TOK_RDUNET		78
#define TOK_WRUNET		79
#define TOK_DOINTR		80
#define TOK_VARSEG	   (81^TOK_DEMO_BIT)
#define TOK_VAROFF	   (82^TOK_DEMO_BIT)
#define TOK_POKEB	   (83^TOK_DEMO_BIT)
#define TOK_POKEW	   (84^TOK_DEMO_BIT)
#define TOK_VARADDR    (85^TOK_DEMO_BIT)
#define TOK_ANSIPOS 	86
#define TOK_BACKUP		87
#define TOK_FORWARD 	88
#define TOK_FRESHLINE	89
#define TOK_WRUSYS	   (90^TOK_DEMO_BIT)
#define TOK_RDUSYS	   (91^TOK_DEMO_BIT)
#define TOK_NEWPWD		92
#define TOK_OPENCAP 	93
#define TOK_CLOSECAP	94
#define TOK_MESSAGE 	95
#define TOK_SAVESCRN	96
#define TOK_RESTSCRN	97
#define TOK_SOUND		98
#define TOK_CHAT		99
#define TOK_SPRINT		100
#define TOK_SPRINTLN	101
#define TOK_MPRINT		102
#define TOK_MPRINTLN	103
#define TOK_RENAME		104
#define TOK_FREWIND 	105
#define TOK_POKEDW	   (106^TOK_DEMO_BIT)
#define TOK_DBGLEVEL	107
#define TOK_SHOWON		108
#define TOK_SHOWOFF 	109
#define TOK_PAGEON		110
#define TOK_PAGEOFF 	111

/* The following were added for PPL 2.0 */

#define TOK_FSEEK		112
#define TOK_FFLUSH		113
#define TOK_FREAD		114
#define TOK_FWRITE		115
#define TOK_FDEFIN		116
#define TOK_FDEFOUT 	117
#define TOK_FDGET		118
#define TOK_FDPUT		119
#define TOK_FDPUTLN 	120
#define TOK_FDPUTPAD	121
#define TOK_FDREAD		122
#define TOK_FDWRITE 	123
#define TOK_ADJBYTES	124
#define TOK_KBDSTRING	125
#define TOK_ALIAS		126
#define TOK_REDIM		127
#define TOK_APPEND		128
#define TOK_COPY		129
#define TOK_KBDFLUSH	130
#define TOK_MDMFLUSH	131
#define TOK_KEYFLUSH	132
#define TOK_LASTIN		133
#define TOK_FLAG		134
#define TOK_DOWNLOAD	135
#define TOK_WRUSYSDOOR (136^TOK_DEMO_BIT)
#define TOK_GETALTUSER	137
#define TOK_ADJDBYTES	138
#define TOK_ADJTBYTES	139
#define TOK_ADJTFILES	140
#define TOK_LANG		141
#define TOK_SORT		142
#define TOK_MOUSEREG	143
#define TOK_SCRFILE 	144
#define TOK_SEARCHINIT	145
#define TOK_SEARCHFIND	146
#define TOK_SEARCHSTOP	147
#define TOK_PRFOUND 	148
#define TOK_PRFOUNDLN	149
#define TOK_TPAGET		150
#define TOK_TPAPUT		151
#define TOK_TPACGET 	152
#define TOK_TPACPUT 	153
#define TOK_TPAREAD 	154
#define TOK_TPAWRITE	155
#define TOK_TPACREAD	156
#define TOK_TPACWRITE	157
#define TOK_BITSET		158
#define TOK_BITCLEAR	159
#define TOK_BRAG		160
#define TOK_FREALTUSER	161

/* the following were added by Stan after 02/14/94 */

#define TOK_SETLMR		162
#define TOK_SETENV		163
#define TOK_FCLOSEALL	164
#define TOK_DECLARE 	165
#define TOK_FUNCTION	166
#define TOK_PROCEDURE	167
#define TOK_PCALL		168
#define TOK_FPCLR		169
#define TOK_BEGIN		170
#define TOK_FEND		171
#define TOK_STATIC		172
#define TOK_STACKABORT	173

/* The following were added for PPL 3.0 */

#define TOK_DCREATE 	174  /* dbfchan,name,exclusive, fieldinfo, taginfo */
#define TOK_DOPEN		175  /* dbfchan,name, exclusive */
#define TOK_DCLOSE		176  /* dbfchan */
#define TOK_DSETALIAS	177  /* dbfchan,name */
#define TOK_DPACK		178  /* dbfchannel */
#define TOK_DCLOSEALL	179 /* */
#define TOK_DLOCK		180  /* dbfchannel */
#define TOK_DLOCKF		180  /* dbfchannel	 dlock & dlockf do the same thing */
#define TOK_DLOCKR		181  /* dbfchannel, recno */
#define TOK_DLOCKG		182  /* dbfchannel, recnos,count */
#define TOK_DUNLOCK 	183  /* dbfchannel */
#define TOK_DNCREATE	184  /* dbfchannel,name,taginfo */
#define TOK_DNOPEN		185  /* dbfchannle, name */
#define TOK_DNCLOSE 	186  /* dbfchannel, name */
#define TOK_DNCLOSEALL	187  /* channel */
#define TOK_DNEW		188  /* channel */
#define TOK_DADD		189  /* channel */
#define TOK_DAPPEND 	190  /* channel */
#define TOK_DTOP		191  /* channel */
#define TOK_DGO 		192  /* channel, recno */
#define TOK_DBOTTOM 	193  /* channel */
#define TOK_DSKIP		194  /* channel, number */
#define TOK_DBLANK		195  /* channel */
#define TOK_DDELETE 	196  /* channel */
#define TOK_DRECALL 	197  /* channel */
#define TOK_DTAG		198  /* channel, name */
#define TOK_DSEEK		199  /* channel, expression */
#define TOK_DFBLANK 	200  /* channel, name */
#define TOK_DGET		201  /* channel, name , var */
#define TOK_DPUT		202  /* channel, name, expression */
#define TOK_DFCOPY		203  /* channel, name, channel, name */

#define TOK_EVAL		204
#define TOK_ACCOUNT 	205
#define TOK_RECORDUSAGE 206
#define TOK_MSGTOFILE	207
#define TOK_QWKLIMITS	208
#define TOK_COMMAND 	209
#define TOK_USELMRS 	210
#define TOK_CONFINFO	211
#define TOK_ADJTUBYTES	212
#define TOK_GRAFMODE	213
#define TOK_ADDUSER 	214
#define TOK_KILLMSG 	215
#define TOK_CHDIR		216
#define TOK_MKDIR		217
#define TOK_REDIR		218
#define TOK_FDOWRAKA	219
#define TOK_FDOADDAKA	220
#define TOK_FDOWRORG	221
#define TOK_FDOADDORG	222
#define TOK_FDOQMOD 	223
#define TOK_FDOQADD 	224
#define TOK_FDOQDEL 	225
#define TOK_SOUNDDELAY	226

#define MAX_TOK 		226

/* INPUTFMT color,var,text,mask,valid */
/* color and var are as expected */
/* text would be the default string with literals "(   )    -    " */
/* mask would be the input mask 				  "LNNNLLNNNLNNNN" */
/*	 L = literal,		A = alpha		w/force upper, a = alpha */
/*	 N = numeric,		C = alnum		w/force upper, c = alnum */
/*	 O = numeric+space, X = any 		w/force upper, x = any */
/*						B = alpha+space w/force upper, b = alpha+space */
/*						D = alnum+space w/force upper, d = alnum+space */
/* valid is as expected */

/******************************************************************************/

/* Types */

typedef unsigned char byte;

	/*--------------------------------------------------------------------*/

class cSCRIPT;

struct sKEYWORD
{
	char		* token;
	int 		  tokenVal;
	byte		  reqArgs;
	byte		  maxArgs;
	bool		  useVarArgs;
#if   defined(___COMP___)
	int LIBENTRY (* tokenFunc)(cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
#elif defined(___INFO___) || defined(___SCAN___)
	void LIBENTRY (* tokenFunc)(cSCRIPT & obj, sKEYWORD & inf);
#endif
};

	/*--------------------------------------------------------------------*/

struct sOPERATOR
{
	char * token;
	int    tokenVal;
	byte   tokenPri;
	bool   isBinary;
	int    reqArgs;
};

	/*--------------------------------------------------------------------*/

union uDEFVALS
{
	tDREAL		realval;
	tINTEGER	integerval;
	tSTRING 	stringval;
};

	/*--------------------------------------------------------------------*/

#ifdef	___EXEC___

struct sFILEINFO
{
	bool	inUse;
	DOSFILE hand;
	bool	errStat;
	byte	lastAcc;
};

#endif

	/*--------------------------------------------------------------------*/

typedef cSTACK<int> 	   tTS;
typedef cISTACK<cVARVAL>   tVS;
typedef struct {  /* structure to hold select case info on a nested statement */
	char * old;
	int   cntr;
} ss;

	/*--------------------------------------------------------------------*/

typedef cVAR * cVARPTR;

	/*--------------------------------------------------------------------*/

class cSCRIPT
{
	public:

/* Script interface functions */

		cSCRIPT  (int numToks);

		~cSCRIPT (void);

		int LIBENTRY compile (char * f);
		int LIBENTRY save	 (char * f);
		int LIBENTRY load	 (char * f);
		int LIBENTRY execute (char * af);



/* Execute support data */

#ifdef	___EXEC___

		bool				ansAppended;

#endif

	private:

/* Compile support data */

#ifdef	___COMP___

		tVARLST 			varLst;
		tLABLST 			labLst;

		static sKEYWORD 	kwArray[];
		static sOPERATOR	opArray[];

		static char *		nextToken;

		DOSFILE 			fs;

		int 				curVarID;

		bool				genSysLabel;

		tTS *				tokenStack;
		cISTACK<tTS>		tokenStkStk;

		cISTACK<long>		argStack;
		cISTACK<char>		opStack;

		cSTACK<int> 		cntrlStruct;
		int 				destLabelCount;

		cSTACK<long>		startStruct;

		cSTACK<int> 		endStruct;
		int 				endLabelCount;

		cSTACK<int> 		loopStruct;
		int 				loopLevel;

		cSTACK<char *>		forStepExpr;

		int 				sysLabelCount;

		cSTACK<ss *>		  selectStack;
/*cSTACK<int>		caseStack; */
		bool				inCase;




#endif

/* Execute support data */

#ifdef	___EXEC___

		bool				altUserAvailable;

		int 				ppeVer;

		cVAR * *			varLst;

		int 				defInChan;
		int 				defOutChan;

		sFILEINFO			fileArr[MAX_FILE_CHAN];

#ifdef DBASE

		cDBF				dbfArr[MAX_DBF_CHANNELS];

#endif

/*cSTACK<char _FAR_ *>	retAddr; */
		cSTACK<long>		retAddr;

		tVS *				varStack;

		cISTACK<tVS>		varStkStk;
		cISTACK<tVS>		prgProcStkStk;

		tVS 				prgVarStack;
		tVS 				prgProcVarStack;
		tVS 				prgProcGlobVarStack;
		tVS 				currProcStack;
		cVARVAL *			currProc;

		cVARVAL 			finalResult;

		cVARVAL 			tmpBoolean;
		cVARVAL 			tmpDate;
		cVARVAL 			tmpInteger;
		cVARVAL 			tmpString;
		cVARVAL 			tmpBigStr;
		cVARVAL 			tmpDReal;

		cSCRIPT *			tmpScript;

		char				tokenBuf			[ MAX_STR_LEN+1 ];
		int 				tokenCnt;

		char				PPEPath 			[ MAX_STR_LEN+1 ];
		char				PPEName 			[ MAX_STR_LEN+1 ];

		char				readLineFile		[ MAX_STR_LEN+1 ];
		DOSFILE 			readLineStream;
		long				readLineLine;

		nodetype			unInfo;

		#ifndef __OS2__  /* may need to finish (REGS and SREGS are not defined) */
		REGS				intrREGS;
		SREGS				intrSEGS;
		#endif

		long				lastUserGot;

		UData * 			ptrUData;
		char  * 			ptrUReg;
		long  * 			ptrULMR;

		bool				searchAvailable;
		bool				searchInUse;
		int 				searchTokens;
		bool				searchFound;
		char				searchInput 		[ MAX_STR_LEN+1 ];
		char				searchCodedInput	[ MAX_STR_LEN+1 ];

		bool				tmpRipMode;
		bool				tmpGraphMode;
		bool				tmpUseAnsi;

		int 				breakFlag;
		bool				stackErrFlag;
		bool				stackErrContFlag;
		bool				altUserLMRFlag;


		char				str1[1+MAX_STR_LEN+1];
		char				str2[1+MAX_STR_LEN+1];
		char				str3[1+MAX_STR_LEN+1];

		char				buf[MAX_BIGSTR_LEN+1+2];
		bool				swapped;
		bool				rled;
#endif

/* Info & Scan Support Data */

#if defined(___INFO___) || defined(___SCAN___)

		static sKEYWORD 	kwArray[];
		static sOPERATOR	opArray[];

		int 				ppeVer;

		cVAR * *			varLst;

#endif

/* Other data */

		char _FAR_ *		scriptBuffer;
		char _FAR_ *		scriptPointer;
		uint				scriptSize;  /* DWT: changed from long to uint */
		int 				swapScriptPointer;
		uint				swapScriptSize;  /* DWT: changed from long to uint */
		long				swapOffset;

		sint				varCount;
		int *				wvErr;
		int *				wvHand;
		int 				numCalls;



/* Compile support functions */

#ifdef	___COMP___

		static int	  LIBENTRY tokenize 		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass, int LIBENTRY (* func)(int,int,cSCRIPT&,char*,int,int));

		static int	  LIBENTRY subTokARGS		   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokARGV		   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokVAREXPR	   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokEXPRVAR	   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokVIDEXPR	   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokEEVID		   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokVIDS		   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokREDIM		   (int n, int t, cSCRIPT & o, char * a, int l, int p);
		static int	  LIBENTRY subTokSORT		   (int n, int t, cSCRIPT & o, char * a, int l, int p);

		static int	  LIBENTRY tokenARGS		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenARGV		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenVAREXPR 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenEXPRVAR 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenVIDEXPR 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenEEVID		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenREDIM		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenVIDS		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenSORT		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenLET 		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenGOX 		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenIFWHILE 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenFOR 		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenSELECT		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenCASE		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenDEFAULT 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenSLCTEND 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenFUNC		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenENDFUNC 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenDECL		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);
		static int	  LIBENTRY tokenBEGIN		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenDBFPROC 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenSTATIC		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenFCLSALL 	   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenTYPE		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenBRANCH		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY tokenVARS		   (cSCRIPT & obj, sKEYWORD & inf, char * args, int line, int pass);

		static int	  LIBENTRY isCall			   (cSCRIPT & obj, char * s, int line, int pass, bool procOnly, bool funcOnly,int *err);  /* added by stan */

		static char * LIBENTRY standardizeLine	   (char * l, int & q, int & p);

		static void   LIBENTRY stripSpaces		   (char * s);

		static int	  LIBENTRY argCount 		   (char * s, int funcstyle = FALSE);
		static char * LIBENTRY getArgument		   (char * s);

		static int			 cmpOP				 (const void * _item, const void * _key);
		static int			 cmpSYMS			 (const void * _item, const void * _key);

		static bool   LIBENTRY startStructError    (cSCRIPT & obj, char at, char dt, char * ts, int l);

		int 		  LIBENTRY anop 			   (char * e);
		int 		  LIBENTRY isConst			   (char * s);

		void		  LIBENTRY popStartStructInfo  (char & t, int & n);
		void		  LIBENTRY pushStartStructInfo (char t, int n);

		bool		  LIBENTRY isIfDestStatement   (char * p);
		bool		  LIBENTRY isIfEndStatement    (char * p);

		int 		  LIBENTRY addTokenToBuffer    (int tokval);

		int 		  LIBENTRY compPass 		   (int pass);
		int 		  LIBENTRY compLine 		   (int pass, char * sBuf, int & curLine);

		int 		  LIBENTRY doLabel			   (char * t, int curLine, int issys = FALSE);
		int 		  LIBENTRY doVariable		   (int type, char * args, int curLine, int pass, int issys = FALSE, uDEFVALS * defval = NULL);

		int 		  LIBENTRY getLabel 		   (char * name, int line, int pass);
		cVAR		* LIBENTRY getVariable		   (char * name, int line, int dovarflag = FALSE);

		int 		  LIBENTRY findTypeAndVal	   (char * s, int & type, uDEFVALS & val);

		int 		  LIBENTRY processOp		   (char * s, int token, int & lastTokenVar, char * & e, int line, int pass);
		int 		  LIBENTRY processArg		   (char * tmpsym, int legal, int line, int pass);

		void		  LIBENTRY eceSub			   (char * invops, int & err);
		int 		  LIBENTRY evalConstExpr	   (char * e, int line, int & retVal);

		int 		  LIBENTRY wrVIDSUB 		   (char * t, int l, int p, int issys = FALSE);
		int 		  LIBENTRY wrVID			   (char * t, int l, int p, int issys = FALSE);

		int 		  LIBENTRY in2post			   (char * e, int line, int pass);

		int 		  LIBENTRY legalName		   (char * s);

		long		  LIBENTRY availBytes		   (void)	  { return (SCR_BUF_SIZE-scriptSize); }

		void		  LIBENTRY preprocess		   (char * s);

		void		  LIBENTRY defineSymbol 	   (void);
		void		  LIBENTRY undefSymbol		   (void);

		int 		  LIBENTRY processComment	   (int pass, int line);

		int 		  LIBENTRY parseArg 		   (cSCRIPT & obj, cVAR * t,char ** arg, char * func, int pnum, int line);

#endif

/* Execute support functions */

#ifdef	___EXEC___

		void		  LIBENTRY msgtofile		   (void);

		void		  LIBENTRY getUserVars		   (long user, int & err);
		void		  LIBENTRY putUserVars		   (int & err);

		void		  LIBENTRY execTPA			   (int token, int & err);
		void		  LIBENTRY execFile 		   (int token, int & err);
		void		  LIBENTRY execDBF			   (int token, int & err);

		void		  LIBENTRY openChan 		   (int chan, char * name, int	 openFlags);
		void		  LIBENTRY closeChan		   (int chan);
		void		  LIBENTRY rewindChan		   (int chan);
		void		  LIBENTRY getChan			   (int chan, char * str,  int	 size);
		void		  LIBENTRY putChan			   (int chan, char * str);
		void		  LIBENTRY seekChan 		   (int chan, long	 pos,  char  meth);
		void		  LIBENTRY flushChan		   (int chan);
		void		  LIBENTRY readChan 		   (int chan, char * strBuf,  int & size);
		void		  LIBENTRY writeChan		   (int chan, char * strBuf,  int	size);

		cVARVAL *	  LIBENTRY aVarValPtr		   (int varID, int v = 0, int m = 0, int c = 0);

		cVARVAL *	  LIBENTRY aVarSubPtr		   (int varID);
		cVAR	*	  LIBENTRY aVarPtr			   (int varID);

		cVARVAL *	  LIBENTRY curVarSubPtr 	   (void);

		cVARVAL *	  LIBENTRY getVarSubPtr 	   (void);
		cVAR	*	  LIBENTRY getVarPtr		   (void);

		int 		  LIBENTRY curVID			   (void);
		int 		  LIBENTRY getVID			   (void);

		cVARVAL *	  LIBENTRY getSub			   (cVAR * p);
		cVARVAL *	  LIBENTRY evalArg			   (void);
		cVARVAL *	  LIBENTRY evalPostFix		   (void);
        cVARVAL *     LIBENTRY AccountSub          (cVARVAL *p1);
        cVARVAL *     LIBENTRY PcbAccountSub       (cVARVAL *p1);
        cVARVAL *     LIBENTRY ScanMsgHdrSub       (cVARVAL *p1, cVARVAL *p2, cVARVAL *p3, cVARVAL *p4);

		int 		  LIBENTRY doForItems		   (int LIBENTRY (* itemfunc)(cSCRIPT & obj, int num));
		static int	  LIBENTRY redimArray		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY printVal 		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY prFoundVal		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY sprintVal		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY mprintVal		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY fputVal			   (cSCRIPT & obj, int num);
		static int	  LIBENTRY fdputVal 		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY pushVars 		   (cSCRIPT & obj, int num);
		static int	  LIBENTRY popVars			   (cSCRIPT & obj, int num);

		char *		  LIBENTRY expandMask		   (char * msk);

		char *		  LIBENTRY readLine 		   (char * fn, long ln, char * strBuf);

		int 		  LIBENTRY setnewpwd		   (char * Str);

		void		  LIBENTRY searchInit		   (char * Scriteria, bool caseSensitive);
		void		  LIBENTRY searchFind		   (char * text, int len);
		void		  LIBENTRY searchPrint		   (char * str);
		void		  LIBENTRY searchStop		   (void);

		unsigned long LIBENTRY calcCRC32		   (bool f, char * buf, unsigned len);
		int 		  LIBENTRY stkinit			   (cVARVAL * o);
		int 		  LIBENTRY stkclean 		   (cVARVAL * o);
		int 		  LIBENTRY initLocals		   (cVARVAL * o);
		int 		  LIBENTRY clearProc		   (cVARVAL * fpptr);

		void		  LIBENTRY execdbf			   (cVARVAL * p1,cVARVAL * p2,cVARVAL * p3,cVARVAL * p4,cVARVAL * & rp, int & err);
		void		  LIBENTRY execdbs			   (int token, int & err);

		int 		  LIBENTRY swapIn			   ( void );
		void		  LIBENTRY swapOut			   ( void );

		cVARVAL *	  LIBENTRY getConfInfo		   (unsigned int confnum, unsigned int field);
		void		  LIBENTRY putConfInfo		   (unsigned int confnum, unsigned int field,cVARVAL * value);


#endif

/* Info & Scan Support Functions */

#if defined(___INFO___) || defined(___SCAN___)

		static void   LIBENTRY tokenize 		   (cSCRIPT & obj, sKEYWORD & inf, void LIBENTRY (* func)(int,cSCRIPT&));

		static void   LIBENTRY subTokARGS		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokARGV		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokVAREXPR	   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokEXPRVAR	   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokVIDEXPR	   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokEEVID		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokREDIM		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokVIDS		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokSORT		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokLET		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokGOX		   (int n, cSCRIPT & o);
		static void   LIBENTRY subTokIFWHILE	   (int n, cSCRIPT & o);

		static void   LIBENTRY tokenARGS		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenARGV		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenVAREXPR 	   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenEXPRVAR 	   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenVIDEXPR 	   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenEEVID		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenREDIM		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenVIDS		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenSORT		   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenLET 		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenGOX 		   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenIFWHILE 	   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenFOR 		   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenSELECT		   (cSCRIPT & obj, sKEYWORD & inf);  /* Added by Stan */
		static void   LIBENTRY tokenCASE		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenDEFAULT 	   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenSLCTEND 	   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenFUNC		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenENDFUNC 	   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenDECL		   (cSCRIPT & obj, sKEYWORD & inf);
		static void   LIBENTRY tokenBEGIN		   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenDBFPROC 	   (cSCRIPT & obj, sKEYWORD & inf);


		static int	  LIBENTRY tokenTYPE		   (cSCRIPT & obj, sKEYWORD & inf);
		static int	  LIBENTRY tokenSTATIC		   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenFCLSALL 	   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenBRANCH		   (cSCRIPT & obj, sKEYWORD & inf);

		static void   LIBENTRY tokenVARS		   (cSCRIPT & obj, sKEYWORD & inf);

		void		  LIBENTRY rdVARSUBS		   (void);

		void		  LIBENTRY rdLABOFF 		   (void);
		void		  LIBENTRY rdVIDSUB 		   (void);
		void		  LIBENTRY rdVID			   (void);
		void		  LIBENTRY rdPOSTFIX		   (void);

		void		  LIBENTRY report			   (void);
		void		  LIBENTRY reportScan		   (void);

#endif

/* Other functions */

		void		  LIBENTRY initialize		   (int pass);

/*		  static void	LIBENTRY countVars			 (cVAR & obj); */
		static void   LIBENTRY writeVars		   (cVAR & obj);

};

/******************************************************************************/

/* Constants */

/******************************************************************************/

/* Variables */

/******************************************************************************/

/* Function Prototypes */

/******************************************************************************/

/* Inline Functions */

/******************************************************************************/

#endif

