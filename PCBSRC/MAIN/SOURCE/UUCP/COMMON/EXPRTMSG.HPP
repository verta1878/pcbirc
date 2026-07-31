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



#ifndef ___EXPRTMSG_HPP___

#define ___EXPRTMSG_HPP___

#include    <pcbmsgs.hpp>

const int msgTypeEmail = 0;
const int msgTypeNews  = 1;

extern unsigned nextSeq;

class cTMPMSG;

void pascal exportMessage(
    int msgType, char * sp, char * line1, char * to, char * from,
    char * subj, char * ng, char * ft, char * msgid, char * date,
    char * distrib, char * org, char * replyto,
    cMSG * message, cDOSFILE * textfile, char * attachPath, char * refs,
    char * buf, int size, cTMPMSG * msgobj, char * sender );

void pascal getSeqNum ( void );
void pascal putSeqNum ( void );

#endif

