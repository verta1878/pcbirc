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


#ifndef FIDOMSG_HPP
#define FIDOMSG_HPP

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include "pcboard.h"
#include "pcbmsgs.hpp"
#include "msgstub.hpp"

class cFIDOMSG
{
public:
/*             cFIDOMSG(); */
/*            ~cFIDOMSG(){message.freeMsgBuf();} */
bool         MsgtoFile(long msgNum);
bool         FiletoMsg(const char * filename);
bool         fillMSGHdr(msgheadertype & Hdr);
bool         writeBody(char * messBody,DOSFILE & fblk);
bool         getFidoArea(char *areaname);
bool         fillPCBHeader(msgheadertype & Hdr, DOSFILE & MSGfile, char * file,long filesize);
bool         checkAttaches(const char * files);
void         processMessBody(char * buffer);
void         getHighMsgNo(long & himsgnum, long & lmx);
void         start_at_1MSG(void);
void         handleAttach(const char * messBuf);
long         findHiMSG(char * dir);
bool         createFile(const char * fromAddr,const char * toAddr,const char * Subject);
void         updatePackInfo(const char * fname, PACKMSGNFO & pknfo);


static bool           preventFile;

private:

    STORED_FIDO_MSG   MSGHdr;
    AREA_STRUCT       this_area;
    pcbconftype       confrec;
    bool              KFS;

};
#endif
