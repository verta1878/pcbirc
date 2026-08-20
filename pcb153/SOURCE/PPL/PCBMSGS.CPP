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


#include    <string.h>

#include    "pcbmsgs.hpp"

#ifndef LIB
#include    "newdata.h"
#include    "messages.h"
#include    "pcboard.h"
#include    "pcboard.ext"
#endif

#ifdef DEBUG
#include <memcheck.h>
#endif


// const char NUL = '\0';

cMSG::cMSG ( void ) : msgBuf(NULL), msgBufSize(0) { }

cMSG::~cMSG ( void ) { freeMsgBuf(); }

void LIBENTRY cMSG::freeMsgBuf(void)
{
    if (msgBuf)
    {
     // delete [] msgBuf;
        free(msgBuf);
        msgBuf = NULL;
        msgBufSize = 0;
    }
}

void LIBENTRY cMSG::reallocMsgBuf(int size)
{
    if (size > msgBufSize)
    {
        void * msgBufTmp = realloc(msgBuf,size);
        if (msgBufTmp)
        {
            msgBuf = (char *) msgBufTmp;
            msgBufSize = size;
        }
    }
}

int LIBENTRY cMSG::open(char * name)
{
    if (cDOSFILE::open(name,OPEN_RDWR|OPEN_DENYNONE) != 0)
        return -1;
    else
        cDOSFILE::setbuf(0x1000);

    if (aliasThis.alias(*this) != 0)
        return -1;

    return 0;
}

void LIBENTRY cMSG::close(void)
{
    aliasThis.close();
    cDOSFILE::close();
}

long LIBENTRY cMSG::lowMsgNum(void)
{
    cBSREAL tmpLMN;
    long pos = aliasThis.pos();
    if (aliasThis.seek(4L,SEEK_SET)            != 4L)             return -1;
    if (aliasThis.read(&tmpLMN,sizeof(tmpLMN)) != sizeof(tmpLMN)) return -1;
    aliasThis.pos(pos);
    return tmpLMN;
}

long LIBENTRY cMSG::highMsgNum(void)
{
    cBSREAL tmpHMN;
    long pos = aliasThis.pos();
    if (aliasThis.seek(0L,SEEK_SET)            != 0L)             return -1;
    if (aliasThis.read(&tmpHMN,sizeof(tmpHMN)) != sizeof(tmpHMN)) return -1;
    aliasThis.pos(pos);
    return tmpHMN;
}

int LIBENTRY cMSG::readHdr(long off)
{
    msgHdrOff = -1;

    if (off < 0) return -1;

    if (cDOSFILE::seek(off,SEEK_SET)           != off)            return -1;
    if (cDOSFILE::read(&msgHdr,sizeof(msgHdr)) != sizeof(msgHdr)) return -1;

    msgHdrOff = off;

    return 0;
}
/*
int LIBENTRY cMSG::readHdr(void)
{
    msgHdrOff = -1;

    long off = cDOSFILE::s.curpos;

    if (cDOSFILE::read(&msgHdr,sizeof(msgHdr)) != sizeof(msgHdr)) return -1;

    msgHdrOff = off;

    return 0;
}
*/
int LIBENTRY cMSG::readBody(unsigned size)
{
    msgSize = ((unsigned(msgHdr.blocks)-1U)*128U);
    size = (((size+128)/128)+1)*128;
    if (msgSize > size) size = msgSize;

 // if (msgBuf) free(msgBuf); // delete [] msgBuf;
 // msgBuf = (char*) malloc(size+128U); // msgBuf = new char[size+128U];
    reallocMsgBuf(size+128U);

    if ((msgBuf == NULL) || (msgBufSize == 0) || (msgBufSize < size+128U))
        return -1;

    memset(msgBuf,' ',msgSize+128U);
    if (cDOSFILE::read(msgBuf,msgSize) != msgSize) return -1;

    msgSize = size;

    msgExtHdr = (sMSGEXTHDR *) msgBuf;
    countExtHdrs();

    msgBody = msgBuf + msgExtHdrs * sizeof(sMSGEXTHDR);
    msgSize -= msgExtHdrs * sizeof(sMSGEXTHDR);
    msgLineStart = msgLineEnd = msgBuf;
    inGetLine = FALSE;

    sortExtHdrs();

    return 0;
}

int LIBENTRY cMSG::writeHdr(long off)
{
    // Grab default offset if needed
    if (off == -1) off = msgHdrOff;

    msgHdrOff = -1;

    if (off < 0) return -1;

    int rv = 0;

    long pos = aliasThis.pos();
    if (aliasThis.seek(off,SEEK_SET) != off)
        rv = -1;
    else
    {
        if (aliasThis.write(&msgHdr,sizeof(msgHdr)) != sizeof(msgHdr))
            rv = -1;
    }
    aliasThis.pos(pos);

    msgHdrOff = off;

    return rv;
}

void LIBENTRY cMSG::countExtHdrs(void)
{
    for (msgExtHdrs = 0;
        (msgExtHdrs < MAXEXTHDRS) && validExtHdr(msgExtHdrs);
        ++msgExtHdrs)
        ;
}

bool LIBENTRY cMSG::validExtHdr(int i)
{
    if (msgExtHdr[i].id    != 0x40FF)        return FALSE;
    if (msgExtHdr[i].colon != ':')           return FALSE;

    if ((msgExtHdr[i].stat != EXTSTAT_NONE) &&
        (msgExtHdr[i].stat != EXTSTAT_READ))
        return FALSE;

    if (msgExtHdr[i].cr    != LineSeparator) return FALSE;

    return TRUE;
}

int cMSG::cmpMsgExtHdrs(const void * l, const void * r)
{
    return memcmp(l,r,sizeof(sMSGEXTHDR));
}

void LIBENTRY cMSG::sortExtHdrs(void)
{
    if (msgExtHdrs == 0) return;
    qsort(msgExtHdr,msgExtHdrs,sizeof(sMSGEXTHDR),cmpMsgExtHdrs);
}

int cMSG::chkMsgExtHdrName(const void * l, const void * r)
{
    char       * lp = (char       *) l;
    sMSGEXTHDR * rp = (sMSGEXTHDR *) r;
    return memcmp(lp,rp->funct,sizeof(rp->funct));
}

void LIBENTRY cMSG::getExtHdr(char * name, void * buf)
{
    if (msgExtHdrs == 0) return;
    sMSGEXTHDR * extHdr = (sMSGEXTHDR *) bsearch(name,msgExtHdr,
        msgExtHdrs,sizeof(sMSGEXTHDR),chkMsgExtHdrName);
    if (extHdr) memcpy(buf,extHdr->desc,sizeof(extHdr->desc));
}

void LIBENTRY cMSG::getExtHdrs(char * name, char * buf)
{
    unsigned size = 0;
    for (int i = 0; i < msgExtHdrs; ++i)
        if (memcmp(name,msgExtHdr[i].funct,
            sizeof(msgExtHdr[i].funct)) == 0)
        {
            memcpy(buf+size,msgExtHdr[i].desc,
                sizeof(msgExtHdr[i].desc));
            size += sizeof(msgExtHdr[i].desc);
            buf[size] = NUL;
            stripright(buf,' ');
            size = strlen(buf);
        }
}

sMSGEXTHDR * LIBENTRY cMSG::getExtHdr(int num)
{
    if (num >= msgExtHdrs) return NULL;
    return &(msgExtHdr[num]);
}

int LIBENTRY cMSG::digitsToInt(char * s, int o)
{
    char buf [ 2 + 1 ];
    buf[0] = s[o+0];
    buf[1] = s[o+1];
    buf[2] = NUL;
    return atoi(buf);
}

int LIBENTRY cMSG::minute(void)
{
    return digitsToInt(msgHdr.tmsg,3);
}

int LIBENTRY cMSG::hour(void)
{
    return digitsToInt(msgHdr.tmsg,0);
}

int LIBENTRY cMSG::day(void)
{
    return digitsToInt(msgHdr.dmsg,3);
}

int LIBENTRY cMSG::month(void)
{
    return digitsToInt(msgHdr.dmsg,0);
}

int LIBENTRY cMSG::year(void)
{
    return digitsToInt(msgHdr.dmsg,6);
}

void LIBENTRY cMSG::resetLine(void)
{
    if (inGetLine && (msgLineEnd != NULL))
        msgLineEnd[0] = LineSeparator;

    msgLineStart = msgLineEnd = msgBuf;
    inGetLine = FALSE;
}

char * LIBENTRY cMSG::getLine(void)
{
    if (msgLineEnd == NULL) return NULL;

    if (inGetLine)
        msgLineEnd[0] = LineSeparator;
    else
        msgLineEnd = msgBody - 1;

    inGetLine = TRUE;

    msgLineStart = msgLineEnd + 1;

    msgLineEnd = (char *) memchr(msgLineStart,LineSeparator,
        unsigned(msgSize-(msgLineStart-msgBody)));

    if (msgLineEnd == NULL)
        msgLineStart = NULL;
    else
        msgLineEnd[0] = NUL;

    return msgLineStart;
}
/*
int LIBENTRY cMSG::addLine(char * line)
{
    char * p = msgBody;
    int plen = msgSize;

    char * t = (char *) memchr(p,LineSeparator,plen);
    while (t != NULL)
    {
        ++t;
        plen -= int(t-p);
        p = t;
        t = (char *) memchr(p,LineSeparator,plen);
    }

    int llen = strlen(line);

    if (plen <= llen) return -1;

    memcpy(p,line,llen);
    p[llen] = LineSeparator;

    return 0;
}
*/
void LIBENTRY cMSGIDX::setStart(long num)
{
    idxStart = num;
}

int LIBENTRY cMSGIDX::seekIdx(long num)
{
    long idxOff = (num-idxStart);
    if (idxOff < 0) idxOff = 0;
    idxOff *= sizeof(sMSGIDX);

    if (seek(idxOff,SEEK_SET) != idxOff) return -1;

    return 0;
}

long LIBENTRY cMSGIDX::readIdx(long num)
{
    if ((num > 0) && (seekIdx(num) != 0)) return -1L;

    if (read(&idxRec,sizeof(idxRec)) != sizeof(idxRec)) return -1L;

    return idxRec.num;
}

int LIBENTRY cPCBMSGS::open ( const char * name )
{
    char tmp [ 128 + 1 ];

    strcpy(tmp,name);
    if (cMSG::open(tmp) != 0) return -1;

    strcat(tmp,".IDX");
    if (cMSGIDX::open(tmp,OPEN_RDWR|OPEN_DENYNONE) != 0) return -1;

    setStart(lowMsgNum());

    return 0;
}

void LIBENTRY cPCBMSGS::close ( void )
{
    cMSGIDX::close();
    cMSG::close();
}

