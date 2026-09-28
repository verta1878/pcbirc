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



#include    <umwf.hpp>

#include    "msgbase.hpp"

#include    <constream.h>

#include    <uucp.hpp>

/******************************************************************************/

extern constream conerr;

extern cDOSFILE logFile;

/******************************************************************************/

struct sMSGBASE
{
    long        confNum;
    pcbconftype confDat;
    msgbasetype msgBase;
    long        start;
    int         isLmeHmn;
};

/******************************************************************************/

const int mbListSize = 2;

static int mbMaxOpenTime = 20 * 182 / 10 / 2;

static sMSGBASE mbList [ mbListSize ];

/******************************************************************************/

long lastSavedConfNum = 0;
long lastSavedMsgNum  = 0;
long lastSavedMsgOff  = 0;

/******************************************************************************/

#ifdef ___TRACK_CACHE_STATS___

static unsigned mbOpenCnt     = 0;
static unsigned mbCloseCnt    = 0;
static unsigned mbOpenTooLong = 0;
static unsigned mbCacheHit    = 0;
static unsigned mbCacheMiss   = 0;
static unsigned mbCacheFound  = 0;
static unsigned mbCacheFlush  = 0;

#define incCnt(v) (++v)

#else

#define incCnt(v)

#endif

/******************************************************************************/

static sMSGBASE * near pascal openMsgBase(int i, unsigned c)
{
    getconfrecord(c,&mbList[i].confDat);
    int rv = -1;
    for (int retry = 0; (rv != 0) && (retry < 3); ++retry)
        rv = openmessagebase(c,&mbList[i].confDat,&mbList[i].msgBase,
            RDWRLOCK);
    if (rv == 0)
    {
        incCnt(mbOpenCnt);
        mbList[i].confNum = c;
        mbList[i].start = getticks();
        mbList[i].isLmeHmn = (mbList[i].confDat.ExportPtr ==
            mbList[i].msgBase.Stats.HighMsgNum);
    }
    return ((rv == 0) ? &mbList[i] : NULL);
}

static void near pascal closeMsgBase(int i)
{
    incCnt(mbCloseCnt);
    if (mbList[i].confNum != -1L)
    {
        if (mbList[i].isLmeHmn)
        {
            mbList[i].confDat.ExportPtr = mbList[i].msgBase.Stats.HighMsgNum;
            putconfrecord((unsigned)mbList[i].confNum,&mbList[i].confDat);
        }
        closemessagebase(&mbList[i].msgBase);
        mbList[i].confNum = -1L;
    }
}

static void near pascal closeMsgBase(sMSGBASE * mb)
{
    incCnt(mbCloseCnt);
    for (int i = 0; i < mbListSize; ++i)
        if (&mbList[i] == mb)
        {
            closeMsgBase(i);
            break;
        }
}

static sMSGBASE * near pascal getMsgBase(unsigned confNum)
{
    int i;

    // close anything open too long
    for (i = 0; i < mbListSize; ++i)
        if (getticks()-mbList[i].start > mbMaxOpenTime)
        {
            incCnt(mbOpenTooLong);
            closeMsgBase(i);
            // if the desired conference matches msgbase i delay for two seconds
            if (confNum == mbList[i].confNum) tickdelay(2*182/10);
        }

    // see if it is already open (hopefully)
    for (i = 0; i < mbListSize; ++i)
        if (mbList[i].confNum == confNum)
        {
            incCnt(mbCacheHit);
            return &mbList[i];
        }

    incCnt(mbCacheMiss);

    // look for an available entry
    for (i = 0; i < mbListSize; ++i)
        if (mbList[i].confNum == -1)
        {
            incCnt(mbCacheFound);
            return openMsgBase(i,confNum);
        }

    // we have to free one up now (oldest)
    int oldest;
    for (oldest = 0, i = 1; i < mbListSize; ++i)
        if (mbList[i].start < mbList[oldest].start)
            oldest = i;

    incCnt(mbCacheFlush);

    closeMsgBase(oldest);
    return openMsgBase(oldest,confNum);
}

void pascal initMsgBaseList(void)
{
    static bool mbListInitialized = FALSE;
    if (!mbListInitialized)
    {
        mbMaxOpenTime = PcbData.NetTimeout * 182 / 10 / 2;
        memset(mbList,0xFF,sizeof(mbList));
        mbListInitialized = TRUE;
    }
}

void pascal deinitMsgBaseList(void)
{
    for (int i = 0; i < mbListSize; ++i) closeMsgBase(i);
}

/******************************************************************************/

static void near pascal buildExtHdrs(char * name, char * info)
{
    char * tmpinf = (info ? strdup(info) : NULL);

    int segCnt = 0;

    if (tmpinf != NULL)
    {
        stripall(tmpinf,' ');
        while (strlen(tmpinf) > 0)
        {
            char tmpbuf [ 60 + 1 ];
            sprintf(tmpbuf,"%02d",++segCnt);
            maxstrcpy(tmpbuf+2,tmpinf,sizeof(tmpbuf)-2);
            buildgenextheader(name,tmpbuf);
            strcpy(tmpinf,tmpinf+strlen(tmpbuf+2));
        }
    }

    sstrfre(tmpinf);
}

/******************************************************************************/

static void pascal capMsgInfo(unsigned short ConfNum, long MsgNum,
    long MsgOffset)
{
    lastSavedConfNum = ConfNum;
    lastSavedMsgNum  = MsgNum;
    lastSavedMsgOff  = MsgOffset;
}

static int near pascal saveMessage(unsigned cnum, msgheadertype & msgHdr,
    char * buf)
{
    sMSGBASE * mb = getMsgBase(cnum);
    if (mb == NULL) return -1;

    switch (mb->confDat.ConfType)
    {
        case IN_EMAIL:
        case IN_JUNK:
        case UN_MOD_NEWS:
        case UN_PUB_NEWS:
            break;

        default:
            closeMsgBase(mb);
            return -1;
    }

    lastSavedConfNum = lastSavedMsgNum = lastSavedMsgOff = -1;

    return savetomsgbase(cnum,&mb->confDat,&mb->msgBase,&msgHdr,buf,0,0,
        capMsgInfo);
}

    /*--------------------------------------------------------------------*/

int pascal postMessage(
    unsigned   cnum,   char * buf,   unsigned   size,       char   stat,
    char     * date,   char * sTime, char     * to,         char * from,
    char     * subj,   char * lto,   char     * lfrom,      char * lsubj,
    char     * origin, char * id,    char     * followupto, char * newsgroups,
    char     * attach )
{
    char nameBuf [ 120 + 1 ];
    char * nameBuf2 = nameBuf + 60;

    sprintf(nameBuf,"%-120.120s",lto);   stripright(nameBuf2,' ');
    if (nameBuf2[0]   != NUL) buildextheader(EXTHDR_TO2,    nameBuf2,0);
    nameBuf2[0] = NUL;                   stripright(nameBuf, ' ');
    if (nameBuf [0]   != NUL) buildextheader(EXTHDR_TO,     nameBuf, HDR_TO);

    sprintf(nameBuf,"%-120.120s",lfrom); stripright(nameBuf2,' ');
    if (nameBuf2[0]   != NUL) buildextheader(EXTHDR_FROM2,  nameBuf2,0);
    nameBuf2[0] = NUL;                   stripright(nameBuf, ' ');
    if (nameBuf [0]   != NUL) buildextheader(EXTHDR_FROM,   nameBuf, HDR_FROM);

    if (lsubj[0]      != NUL) buildextheader(EXTHDR_SUBJECT,lsubj+ 0,HDR_SUBJ);
    if (strlen(lsubj) > 60)   buildextheader(EXTHDR_SUBJ2,  lsubj+60,0);

    if (origin[0]     != NUL) buildextheader(EXTHDR_ORIGIN, origin,  0);

    if (id[0]         != NUL) buildgenextheader("UMSGID",id);

    if (followupto[0] != NUL) buildExtHdrs("UFOLLOW",followupto);
    if (newsgroups[0] != NUL) buildExtHdrs("UNEWSGR",newsgroups);

    if (attach != NULL)
    {
        pcbconftype confinf;
        getconfrecord(cnum,&confinf);

        // Does directory exist?
        if (confinf.AttachLoc[0] != NUL)
            confinf.AttachLoc[strlen(confinf.AttachLoc)-1] = NUL;
        if (fileexist(confinf.AttachLoc) != 255)
        {
            char dname [ 128 + 1 ];
            cDOS dfile;

         // buildstr(dname,confinf.AttachLoc,"\\",attach,NULL);
            strcpy(dname,confinf.AttachLoc);
            strcat(dname,"\\");
            strcat(dname,attach);

            char * seqname = strrchr(dname,'\\');

            if (seqname)
                ++seqname;
            else
                seqname = dname;

            strTrunc(seqname,'.');

            if (strlen(seqname) > 8) seqname[8] = NUL;

            int dseq = 0;

            do {

                strTrunc(seqname,'.');
                sprintf(seqname+strlen(seqname),".%03d",dseq++);

            } while (((fileexist(dname) != 255) ||
                (dfile.create(dname,OPEN_WRIT|OPEN_DENYRDWR,0) != 0)) &&
                (dseq < 1000));
                // 1000 = Magic number, max attached files + 1 with same name

            if (dfile.isOpen())
            {
                long fSize = 0;

                char sname [ 128 + 1 ];
             // buildstr(sname,PcbData.TmpLoc,attach,NULL);
                strcpy(sname,PcbData.TmpLoc);
                strcat(sname,attach);

                cDOS sfile;
                if (sfile.open(sname,OPEN_READ|OPEN_DENYWRIT) == 0)
                {
                    int gbsize = 16384;
                    char * gbuf = NULL;

                    while ((gbuf == NULL) && (gbsize >= 512))
                    {
                        gbuf = new char [ gbsize ];
                        if (gbuf == NULL) gbsize >>= 1;
                    }

                    if (gbuf)
                    {
                        int rb;

                        while ((rb = sfile.read(gbuf,gbsize)) > 0)
                        {
                            fSize += rb;
                            dfile.write(gbuf,rb);
                        }

                        delete [] gbuf;
                    }

                    sfile.close();
                }

                dfile.close();

                char attachInfo [ 60 + 1 ];
                sprintf(attachInfo,"%s (%ld) %s",attach,fSize,seqname);
                memset(attachInfo+strlen(attachInfo),' ',
                    sizeof(attachInfo)-strlen(attachInfo));
                attachInfo[sizeof(attachInfo)-1] = NUL;

                buildextheader(EXTHDR_ATTACH, attachInfo, HDR_FILE);
            }
            else
            {
                strTrunc(seqname,'.');
                strcat(seqname,".###");
                conerr << "Attach Create Failed -- " << seqname << endl;
                logFile.printf("Attach Create Failed -- %s\r\n",seqname);
            }
        }
        else // No, log it
        {
            conerr << "Attach Dir (" << cnum << ") Not Found -- " <<
                confinf.AttachLoc << endl;
            logFile.printf("Attach Dir (%u) Not Found -- %s\r\n",
                cnum, confinf.AttachLoc);
        }
    }

    msgheadertype msgHdr;
    makemsgheader(&msgHdr,stat,size,date,sTime,to,0,0,"",' ',from,subj,"",
        FALSE);

    int rv = saveMessage(cnum,msgHdr,buf);

    freehdrs();

    if (stricmp(to,"ALL") != 0) addUMWF(to,cnum);

    return rv;
}

/******************************************************************************/

