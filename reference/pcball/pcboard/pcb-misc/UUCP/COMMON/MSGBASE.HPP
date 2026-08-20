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



#ifndef ___MSGBASE_HPP___

#define ___MSGBASE_HPP___

/******************************************************************************/

extern long lastSavedConfNum;
extern long lastSavedMsgNum;
extern long lastSavedMsgOff;

/******************************************************************************/

void pascal initMsgBaseList   ( void );
void pascal deinitMsgBaseList ( void );

int pascal postMessage(
	unsigned   cnum,   char * buf,	unsigned   size,	   char   stat,
	char	 * date,   char * time, char	 * to,		   char * from,
	char	 * subj,   char * lto,	char	 * lfrom,	   char * lsubj,
    char     * origin, char * id,   char     * followupto, char * newsgroups,
    char     * attach );

/******************************************************************************/

#endif

