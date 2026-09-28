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

// Available Debug Macros (define to generate a specific type of debug info)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// debugLogInvalidHeaders               debugForceInvalidHeader
// debugLogForceInvalidHeader           debugLogBadPacketChecksum
// debugUnexpectedDataString            debugLogUnexpectedData
// debugLogOutOfSequence                debugDispCmdFile
// debugDispFileCreateError             debugLogCmdFile
// debugLogCmdFile

/******************************************************************************/

#include    <conio.h>
#include    <ctype.h>
#include    <dir.h>

#include    <iostream.h>
#include    <iomanip.h>
#include    <constrea.h>

#include    <gprot.hpp>

/******************************************************************************/

extern int      ExtendedError;
extern char     ExtendedAction;
extern char     ExtendedClass;
extern char     ExtendedLocus;

/******************************************************************************/

extern constream conscr;
extern constream csnout;
extern constream cpiout;
extern constream cpoout;
extern constream cmiout;
extern constream cmoout;
extern constream conout;
extern constream conerr;

extern cDOSFILE  logFile;

/******************************************************************************/

char * pktTypeText [] = { " CP: ", "ACDP ", "LDB: ", "SDB: " };

char * pktCPTText  [] = { "?????", "CLOSE", "NAK  ", "SRJ  ", "ACK  ",
                                   "INITC", "INITB", "INITA" };

/******************************************************************************/

const int timeout = 30*182/10;

/******************************************************************************/

const char DLE = '\x10';

const unsigned char kMIN    = 1;
const unsigned char k32     = 1;
const unsigned char k64     = 2;
const unsigned char k128    = 3;
const unsigned char k256    = 4;
const unsigned char k512    = 5;
const unsigned char k1024   = 6;
const unsigned char k2048   = 7;
const unsigned char k4096   = 8;
const unsigned char kCP     = 9;
const unsigned char kMAX    = 9;

const unsigned char tfCP    = 0000;
const unsigned char tfACDP  = 0100;
const unsigned char tfLDB   = 0200;
const unsigned char tfSDB   = 0300;
const unsigned char tfMASK  = 0300;

const unsigned char xfCLOSE = 0010;
const unsigned char xfNAK   = 0020;
const unsigned char xfSRJ   = 0030;
const unsigned char xfACK   = 0040;
const unsigned char xfINITC = 0050;
const unsigned char xfINITB = 0060;
const unsigned char xfINITA = 0070;
const unsigned char xfMASK  = 0070;

const unsigned char yfMASK  = 0007;

const unsigned MAGIC = 0125252;

const int maxDataSize = 4096;

int kDBSTable [] =
{
    0,
    maxDataSize >> 7, // kDBSTable[  k32] =   32
    maxDataSize >> 6, // kDBSTable[  k64] =   64
    maxDataSize >> 5, // kDBSTable[ k128] =  128
    maxDataSize >> 4, // kDBSTable[ k256] =  256
    maxDataSize >> 3, // kDBSTable[ k512] =  512
    maxDataSize >> 2, // kDBSTable[k1024] = 1024
    maxDataSize >> 1, // kDBSTable[k2048] = 2048
    maxDataSize >> 0, // kDBSTable[k4096] = 4096
    0
};

const int maxWinSize = 7;
const int maxDataBlk = k4096;

/******************************************************************************/

static int near pascal chksum(unsigned char * s, int n)
{
    short sum;
    unsigned short t;
    short x;

    sum = -1;
    x = 0;

    do {

        if (sum < 0)
        {
            sum <<= 1;
            ++sum;
        }
        else
        {
            sum <<= 1;
        }

        t = sum;
        sum += *(s++) & 0377;
        x += sum ^ n;
        if ((unsigned short)sum <= t)
            sum ^= x;

    } while (--n > 0);

    return sum;
}

/******************************************************************************/

cGPDL::cGPDL(char * _prot) :
    initADone(FALSE),
    initBDone(FALSE),
    initCDone(FALSE),
    closeDone(FALSE),
    negWinSize(3),
    negDataBlk(k64),
    pktSSeq(0),
    pktRSeq(0),
    pktRAck(0),
    consecutiveNAKS(0),
    abortFlag(FALSE)
{
    for (int i = 0; i < maxWinNodes; ++i)
    {
        pktWin[i].acked = TRUE;
        pktWin[i].data = NULL;
    }

    memset(prot,0,sizeof(prot));
    maxstrcpy(prot,_prot,sizeof(prot));

    int tmpWinSize = atoi(prot+1);
    int tmpDataBlk = 0;

    if (tmpWinSize > 9)
    {
        tmpDataBlk = tmpWinSize%10;
        tmpWinSize /= 10;
    }

    if (tmpWinSize > 0) negWinSize = tmpWinSize;
    if (tmpDataBlk > 0) negDataBlk = tmpDataBlk;

    if (negWinSize > maxWinSize) negWinSize = maxWinSize;
    if (negDataBlk > maxDataBlk) negDataBlk = maxDataBlk;

    if (negWinSize < 1) negWinSize = 1;
    if (negDataBlk < 1) negDataBlk = 1;
}

cGPDL::~cGPDL(void)
{
    for (int i = 0; i < maxWinNodes; ++i)
        if (pktWin[i].data != NULL)
        {
            free(pktWin[i].data);
            pktWin[i].data = NULL;
        }
}

int pascal cGPDL::run(void)
{
    int rv = -1;

    if (init() == 0)
    {
        rv = app();
        deinit();
    }

    return (abortFlag ? -1 : rv);
}

int pascal cGPDL::init(void)
{
    if (!initADone) sendPacket(xfINITA,negWinSize);
 // sendPacket(xfINITB,negDataBlk-1);
 // sendPacket(xfINITC,negWinSize);

    int status = 0;

    while (!initADone || !initBDone || !initCDone)
    {
        if (abortObj()) return -1;

        unsigned char pktType, xField, yField;
        int size;

        if (waitReceivePacket(timeout,pktType,xField,yField,size) == 0)
        {
            if (pktType == tfCP)
                switch (xField)
                {
                    case xfINITA: // break
                    case xfINITB: // break
                    case xfINITC: break;
                    default:      return -1;
                }

            status = initADone+initBDone+initCDone;
        }

        switch (status)
        {
            case 0: sendPacket(xfINITA,negWinSize);   break;
            case 1: sendPacket(xfINITB,negDataBlk-1); break;
            case 2: sendPacket(xfINITC,negWinSize);   break;
        }
    }

    return ((initADone && initBDone && initCDone) ? 0 : -1);
}

void pascal cGPDL::deinit(void)
{
    sendPacket(tfCP,xfCLOSE);

    int closeTryCnt = 0;

    while (!closeDone)
    {
        if (abortObj()) return;

        unsigned char pktType, xField, yField;
        int size;

        waitReceivePacket(timeout,pktType,xField,yField,size);

        if (!closeDone) sendPacket(tfCP,xfCLOSE);

        if (++closeTryCnt > 3) break;
    }
}

bool pascal cGPDL::abortObj(void)
{
    if (gettimer(2) < 0)
    {
        if (!abortFlag)
        {
            if (!cdstillup())
            {
                abortFlag = TRUE;
            }
            else if (kbhit())
            {
                int key = getch();
                if (key ==  0) key = getch()+0x0100;
                if (key == 27) abortFlag = TRUE;
            }
        }
        settimer(2,182/10);
    }

    return abortFlag || closeDone;
}

int pascal cGPDL::sendPacket(unsigned char xf, unsigned char yf)
{
    return sendPacket(kCP,tfCP,xf,yf,NULL);
}

int pascal cGPDL::sendPacket(char * ds, bool ll)
{
    return sendPacket(strlen(ds),ds,ll);
}

int pascal cGPDL::sendPacket(int sz, char * ds, bool ll)
{
    char buf [ maxDataSize ];
    #ifdef MEMCHECK
      mc_register(buf,sizeof(buf));
    #endif

    // Initialize header information assuming a long data block
    // Need only be done once because the
    // last one will abort after reassignment
    unsigned char tf = tfLDB;

    // Loop for as many data blocks as needed
    for (int i = 0; ((i < sz) || (i == 0)) && !abortObj();
        i += kDBSTable[negDataBlk])
    {
        // Assume the temp size will be a full block and initialize data
        int tsz = kDBSTable[negDataBlk];
        memset(buf,0,tsz);

        // If the actual size isn't full size, adjust, then copy data
        if (tsz > (sz-i)) tsz = sz-i;
        if (ds != NULL) memcpy(buf,ds+i,tsz);

        // If not last block long and we aren't working with a full block
        if (!ll && (tsz < kDBSTable[negDataBlk]))
        {
            // Change to a short data block and calculate the difference
            // in size between this and a long data block
            tf = tfSDB;
            int diff = kDBSTable[negDataBlk]-tsz;

            // If the difference is less or equal to 127 then one byte
            // adjustment is needed
            if (diff <= 127)
            {
                memmove(buf+1,buf,tsz);
                buf[0] = (unsigned char) diff;
            }
            // Otherwise a two byte adjustment is needed
            else
            {
                memmove(buf+2,buf,tsz);
                ((int *) buf)[0] = (((diff & 0x7F80) << 1) | 0x0080 |
                    (diff & 0x007F));
            }
        }

        // Send the packet, checking for errors
        if (sendPacket(negDataBlk,tf,((++pktSSeq)&0x07)<<3,pktRSeq,buf) != 0)
        {
            #ifdef MEMCHECK
              mc_unregister(buf);
            #endif
            return -1;
        }
    }

    #ifdef MEMCHECK
      mc_unregister(buf);
    #endif

    // Return success or failure based on abort status
    return (abortObj() ? -1 : 0);
}

/*
Data In
                      10 03 62 73 D0 C2 11 55 20 64             bs   U d
    61 65 6D 6F 6E 20 64 69 72 65 63 74 0A 5A 0A 52     aemon direct Z R
    20 6F 77 6E 65 72 2D 72 65 63 72 75 69 74 65 72      owner-recruiter
    2D 6E 65 77 73 2D 64 69 67 65 73 74 40 4F 6E 72     -news-digest@Onr
    61 6D 70 2E 4E 45 54 0A 46 20 44 2E 64 69 72 65     amp.NET F D.dire
    63 74 53 48 62 7A 33 0A 49 20 44 2E 64 69 72 65     ctSHbz3 I D.dire
    63 74 53 48 62 7A 33 0A 43 20 72 6D 61 69 6C 20     ctSHbz3 C rmail
    64 61 6E 65 2E 63 61 6E 6E 6F 6E 0A 0A 20 20 20     dane.cannon
    20 20 20 20 20 20 20 20 20 20 20 20 10 01 9D 13
    D8 57 20 90 04 00 66 20 72 65 63 72 75 69 74 65      W    f recruite
    72 2D 6E 65 77 73 2D 64 69 67 65 73 74 20 56 32     r-news-digest V2
    20 23 10 03 62 73 D0 C2 11 55                        #  bs   U
Data Out
    10 09 99 AA 11 2B                                        +
*/

int pascal cGPDL::receivePacket(unsigned char & tf, unsigned char & xf,
    unsigned char & yf, int & sz, char * ds)
{
    sGPKTHDR * hdrptr = (sGPKTHDR *) getCommPtr(sizeof(*hdrptr));
    if (hdrptr == NULL) return -1;

    sGPKTHDR & hdr = *hdrptr;

    static long consecutiveINVHDRS = 0;

    short ck = MAGIC - (0xFF & hdr.ctl);
    if ((hdr.dle != DLE) || (hdr.k < kMIN) || (hdr.k > kMAX) ||
        ((hdr.k ^ hdr.cklo ^ hdr.ckhi ^ hdr.ctl) != hdr.xor) ||
        ((hdr.k == kCP) && (memcmp(&hdr.cklo,&ck,sizeof(ck)) != 0)))
    {
        if (consecutiveINVHDRS++ == 0)
        {
            conerr << endl << "Invalid Header Encountered!";
           #ifdef debugLogInvalidHeaders
            logFile.printf("Invalid Header Encountered!\r\n");
            logFile.printf("Header Info:  %03o %03o %03o %03o %03o %03o\r\n",
                (unsigned int) hdr.dle,
                (unsigned int) hdr.k,
                (unsigned int) hdr.cklo,
                (unsigned int) hdr.ckhi,
                (unsigned int) hdr.ctl,
                (unsigned int) hdr.xor);
            logFile.printf("   Expected:  %03o %03o %03o %03o %03o %03o\r\n",
                (unsigned int) DLE,
                (unsigned int) hdr.k,
                (unsigned int) (ck&0xFF),
                (unsigned int) (((ck&0xFF00)>>8)&0xFF),
                (unsigned int) hdr.ctl,
                (unsigned int) (hdr.k ^ (ck&0xFF) ^ (((ck&0xFF00)>>8)&0xFF) ^ hdr.ctl));
           #endif
        }
        ungetCommPtr(1);
        syncCommPtr();
        return -1;
    }

    consecutiveINVHDRS = 0;

    tf = hdr.ctl & tfMASK;
    xf = hdr.ctl & xfMASK;
    yf = hdr.ctl & yfMASK;

    sz = 0;

    if (hdr.k == kCP)
    {
        switch (xf)
        {
            case xfCLOSE:
                closeDone = TRUE;
                break;

            case xfNAK:
                resendWindow(yf);
                break;

            case xfACK:
                ackPackets(yf);
                break;

            case xfINITC:
                if (yf < negWinSize) negWinSize = yf;
                initCDone = TRUE;
                break;

            case xfINITB:
                ++yf;
                if (yf < negDataBlk) negDataBlk = yf;
                --yf;
                initBDone = TRUE;
                break;

            case xfINITA:
                if (yf < negWinSize) negWinSize = yf;
                initADone = TRUE;
                break;
        }

        cpiout << endl << pktTypeText[tf >> 6] << pktCPTText[xf >> 3] <<
            ' ' << int(yf);

        goto SUCCESS;
    }

    {
        sz = kDBSTable[hdr.k];
        char * buf = (char *) getCommPtr(sz);
        if (buf == NULL)
        {
            ungetCommPtr(0);
            goto FAILURE;
        }

       #ifdef debugForceInvalidHeader
        static long tmpPktNum = 0;
        if (++tmpPktNum == 10)
        {
            conerr << endl << "Forced Invalid Header!";
           #ifdef debugLogForceInvalidHeader
            logFile.printf("Forced Invalid Header!\r\n");
           #endif
            ungetCommPtr(1);
            goto FAILURE;
        }
       #endif

        ck = MAGIC - (chksum(buf,sz) ^ (0xFF & hdr.ctl));
        if (memcmp(&hdr.cklo,&ck,sizeof(ck)) != 0)
        {
            if ((consecutiveNAKS++ % (negWinSize*2+1)) == 0)
                sendPacket(xfNAK,pktRSeq);
            conerr << endl << "Bad Packet Checksum!  " << consecutiveNAKS;
           #ifdef debugLogBadPacketChecksum
            logFile.printf("Bad Packet Checksum!  %d\r\n",consecutiveNAKS);
            logFile.printf("Header Info:  %03o %03o %03o %03o %03o %03o\r\n",
                (unsigned int) hdr.dle,
                (unsigned int) hdr.k,
                (unsigned int) hdr.cklo,
                (unsigned int) hdr.ckhi,
                (unsigned int) hdr.ctl,
                (unsigned int) hdr.xor);
            logFile.printf("Packet Size:  %04X\r\n",sz);
            logFile.printf("Packet Data:\r\n");
            const dispWidth = 16;
            for (int i = 0; i < sz; i += dispWidth)
            {
                logFile.printf("    ");
                for (int j = 0; (j < dispWidth) && (i+j < sz); ++j)
                {
                    logFile.printf("%03o ",(unsigned char) buf[i+j]);
                }
                logFile.printf("\r\n");
            }
            logFile.printf("   Checksum: MAGIC                                       = %06o\r\n",MAGIC);
            logFile.printf("             chksum(buf,sz)                              = %06o\r\n",chksum(buf,sz));
            logFile.printf("             hdr.ctl                                     = %06o\r\n",(unsigned char) hdr.ctl);
            logFile.printf("             MAGIC - (chksum(buf,sz) ^ (0xFF & hdr.ctl)) = %06o\r\n",ck);
           #endif
            ungetCommPtr(1);
            goto FAILURE;
        }

        if (tf == tfSDB)
        {
            unsigned int diff = *((unsigned char *) buf);
            if (diff & 0x80)
            {
                diff = *((unsigned int *) buf);
                diff = ((diff & 0xFF00) >> 1) | (diff & 0x007F);
            }

            sz -= diff;
            memcpy(buf,buf+((diff<=127)?1:2),sz);
        }

        if ((ds != NULL) &&
            (((pktRSeq+1)%maxWinNodes) == ((xf>>3)%maxWinNodes)))
        {
            memcpy(ds,buf,sz);
            pktRSeq = xf >> 3;
            sendPacket(xfACK,pktRSeq);
            if (consecutiveNAKS > 0) --consecutiveNAKS; // consecutiveNAKS = 0;
            pktRAck = yf;
            ackPackets(pktRAck);
        }
        else
        {
            if (ds == NULL)
            {
                conerr << endl << "Not expecting a data packet!";
               #ifdef debugUnexpectedDataString
                conerr << endl << maxstr(buf,64);
               #endif
               #ifdef debugLogUnexpectedData
                logFile.printf("Not expecting a data packet!\r\n");
               #ifdef debugUnexpectedDataString
                logFile.printf("%-64.64s",buf);
               #endif
               #endif
            }
            if (((pktRSeq+1)%maxWinNodes) != ((xf>>3)%maxWinNodes))
            {
                conerr << endl << "Out of sequence data packet!  " <<
                    consecutiveNAKS << ' ' << (int(pktRSeq+1)%8) << ' ' <<
                    (int(xf)>>3);
               #ifdef debugLogOutOfSequence
                logFile.printf("Out of sequence data packet!  %d %d %d\r\n",
                    consecutiveNAKS,(int(pktRSeq+1)%8),(int(xf)>>3));
                logFile.printf("Header Info:  %03o %03o %03o %03o %03o %03o\r\n",
                    (unsigned int) hdr.dle,
                    (unsigned int) hdr.k,
                    (unsigned int) hdr.cklo,
                    (unsigned int) hdr.ckhi,
                    (unsigned int) hdr.ctl,
                    (unsigned int) hdr.xor);
               #endif
            }
            if ((consecutiveNAKS++ % (negWinSize*2+1)) == 0)
                sendPacket(xfNAK,pktRSeq);
            // Do I need to ungetCommPtr for either of the previous if blocks
            goto FAILURE;
        }
    }

    cpiout << endl << pktTypeText[tf >> 6] <<
        hex << setw(4) << setfill('0') << kDBSTable[hdr.k] << ' ' <<
        dec << (int(xf) >> 3) << ' ' << int(yf);

SUCCESS:

    if (consecutiveNAKS > 0) --consecutiveNAKS; // consecutiveNAKS = 0;
    syncCommPtr();
    return 0;

FAILURE:

    syncCommPtr();
    return -1;
}

int pascal cGPDL::waitReceivePacket(long tk, unsigned char & tf,
    unsigned char & xf, unsigned char & yf, int & sz, char * ds)
{
    static int waitFailCnt = 0;

    int rv = -1;

    settimer(4,tk);

    while ((gettimer(4) > 0) && (rv == -1))
    {
        if (abortObj()) return -1;
        rv = receivePacket(tf,xf,yf,sz,ds);
        if (rv == -1) giveup();
    }

    if (rv == -1)
    {
        if (++waitFailCnt >= 3)
            abortFlag = TRUE;
        else
            resendWindow(pktRAck);
    }
    else
        waitFailCnt = 0;

    return rv;
}

int pascal cGPDL::sendPacket(unsigned char k, unsigned char tf,
    unsigned char xf, unsigned char yf, char * ds)
{
    if (k != kCP)
        while (fullWindow())
        {
            if (abortObj()) return -1;
            unsigned char pktType, xField, yField;
            int size;
            receivePacket(pktType,xField,yField,size);
        }

    sGPKTHDR hdr = { DLE, k, 0, 0, tf|xf|yf, 0 };

    if (k != kCP)
    {
        pktWin[xf>>3].acked = FALSE;
        pktWin[xf>>3].hdr = hdr;
        pktWin[xf>>3].data = (char *) malloc(kDBSTable[k]);
        if (pktWin[xf>>3].data) memcpy(pktWin[xf>>3].data,ds,kDBSTable[k]);
    }

    return sendPacket(hdr,ds);
}

int pascal cGPDL::sendPacket(sGPKTHDR & hdr, char * ds)
{
    if (hdr.k == kCP)
        switch (hdr.ctl & xfMASK)
        {
            case xfACK:
            case xfNAK:
                hdr.ctl = (hdr.ctl & (tfMASK|xfMASK)) | pktRSeq;
                break;
        }
    else
        hdr.ctl = (hdr.ctl & (tfMASK|xfMASK)) | pktRSeq;

    short ck;

    if (hdr.k == kCP)
        ck = MAGIC - (0xFF & hdr.ctl);
    else
        ck = MAGIC - (chksum(ds,kDBSTable[hdr.k]) ^ (0xFF & hdr.ctl));

    memcpy(&hdr.cklo,&ck,sizeof(ck));

    hdr.xor = hdr.k ^ hdr.cklo ^ hdr.ckhi ^ hdr.ctl;

    cpoout << endl << pktTypeText[(hdr.ctl&tfMASK)>>6];
    if (hdr.k == kCP)
        cpoout <<
            pktCPTText[(hdr.ctl&xfMASK)>>3] << ' ' << int(hdr.ctl&yfMASK);
    else
        cpoout <<
            hex << setw(4) << setfill('0') << kDBSTable[hdr.k] << ' ' <<
            dec << (int(hdr.ctl&xfMASK)>>3) << ' ' <<
                    int(hdr.ctl&yfMASK);

    putCommBuf(&hdr,sizeof(hdr));
    if (ds) putCommBuf(ds,kDBSTable[hdr.k]);

    return (abortObj() ? -1 : 0);
}

void pascal cGPDL::resendWindow(int lastAcked)
{
    ackPackets(lastAcked);

    for (int i = 0; i < maxWinNodes; ++i)
    {
        int pn = (lastAcked+i+1)%maxWinNodes;
        if (pktWin[pn].acked) break;
        sendPacket(pktWin[pn].hdr,pktWin[pn].data);
    }
}

void pascal cGPDL::ackPackets(int lastAcked)
{
    pktRAck = lastAcked;
    int i;

    for (i = 0; i < maxWinNodes; ++i)
    {
        int pn = (lastAcked-i+maxWinNodes)%maxWinNodes;
        if (pktWin[pn].acked) break;
        pktWin[pn].acked = TRUE;
        if (pktWin[pn].data != NULL)
        {
            free(pktWin[pn].data);
            pktWin[pn].data = NULL;
        }
    }

    static int level = 0;
    if ((level++ == 0) && (i == 0)) resendWindow(lastAcked);
    --level;
}

bool pascal cGPDL::fullWindow(void)
{
    int nodes = 0;

    for (int i = 0; i < maxWinNodes; ++i)
        if (!pktWin[i].acked)
            ++nodes;

    return (nodes >= negWinSize);
}

/******************************************************************************/

int pascal cGPAL::sendMessage(char * msg)
{
    cmoout << endl << maxstr(msg,32);
    logFile.printf("SentMsg: %s\r\n",msg);
    return sendPacket(msg);
}

int pascal cGPAL::sendMessage(char * msg, char * file)
{
    if (sendMessage(msg) != 0) return -1;

    int rv = -1;

    char rmsg [ maxDataSize ];
    #ifdef MEMCHECK
      mc_register(rmsg,sizeof(rmsg));
    #endif

    if (receiveMessage(rmsg) == 0)
    {
        if (msg[0] == 'S')
        {
            if (stricmp(rmsg,"SY")  == 0)
            {
                rv = sendFile(file,TRUE,rmsg);
            }
            else if (stricmp(rmsg,"SN2") == 0)
            {
            }
            else if (stricmp(rmsg,"SN4") == 0)
            {
            }
            else
            {
            }
        }
        else if (msg[0] == 'R')
        {
            if (memicmp(rmsg,"RY",2)  == 0)
            {
                rv = receiveFile(file,TRUE,rmsg);
            }
            else if (stricmp(rmsg,"RN2") == 0)
            {
            }
            else
            {
            }
        }
    }

    #ifdef MEMCHECK
      mc_unregister(rmsg);
    #endif

    return rv;
}

int pascal cGPAL::receiveMessage(char * msg)
{
    char * p = msg;

    unsigned char pktType, xField, yField;
    int size;

    while (!abortObj())
    {
        if (waitReceivePacket(timeout,pktType,xField,yField,size,msg) != 0)
            return -1;

        if (pktType == tfCP) continue;

        if (memchr(msg,NUL,size) != NULL)
            break;

        msg += size;
    }

    msg = p;

    int binFlag = 0;

    while (!binFlag && *msg) binFlag = !isascii(*(msg++));

    if (!abortObj() && !binFlag)
    {
        cmiout << endl << maxstr(p,32);
        logFile.printf("ReceivedMsg: %s\r\n",p);
    }

    return ((abortObj() || binFlag) ? -1 : 0);
}

#pragma argsused
int pascal cGPAL::sendFile(char * file, bool master, char * buf)
{
    long packet = 0;
    long size   = 0;

    conout << endl << endl << "Uploading " << file;

    logFile.printf("SentFile:  %s\r\n",file);

    cDOS f;

    if (f.open(file,OPEN_READ|OPEN_DENYNONE) == 0)
    {
        long sticks = getticks();

        int rb;

        conout << endl;

        while ((rb = f.read(buf,maxDataSize)) > 0)
        {
            conout << '\r' << "Packet " << ++packet << ", "
                "Size " << (size += rb);
            if (sendPacket(rb,buf,FALSE) == -1)
                break;
        }

        f.close();

        if (!abortObj())
        {
            long eticks = getticks();
            long tticks = eticks-sticks;
            if (tticks < 0) tticks = eticks+(24L*60L*60L*182L/10L)-sticks;

            long tsecs = tticks*10L/182L;
            if (tsecs <= 0) tsecs = 1;

            conout << endl << "File Transfer Complete!  "
                "Average CPS = " << size/tsecs;

            logFile.printf("  Transfer Successful -- File Size %ld -- "
                "Avg CPS %ld\r\n",size,size/tsecs);

            unlink(file);
        }
        else
        {
            conerr << endl << "Error in sending file!";

            logFile.putln("  Transfer Failed");
        }
    }
    else
    {
        logFile.putln("  Error Opening File");
    }

    sendPacket(0,NULL,FALSE);

    while (!abortObj())
    {
        if (receiveMessage(buf) == 0)
        {
            if (stricmp(buf,"CY")  == 0)
            {
                break;
            }
            else if (stricmp(buf,"CN5") == 0)
            {
                return -1;
            }
            else
            {
                return -1;
            }
        }
    }

    return (abortObj() ? -1 : 0);
}

int pascal cGPAL::receiveFile(char * file, bool master, char * buf)
{
    long packet = 0;
    long size   = 0;

    bool err = TRUE;

    conout << endl << endl << "Downloading " << file;

    logFile.printf("ReceivedFile:  %s\r\n",file);

    cDOS f;

    if (f.create(file,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL) == 0)
    {
        if (!master) sendMessage("SY");

        err = FALSE;

        unsigned char pktType, xField, yField;

        int rb;

        long sticks = getticks();

        conout << endl;

        while (!abortObj())
        {
            if (waitReceivePacket(timeout,pktType,xField,yField,rb,buf) == 0)
            {
                if (pktType != tfCP)
                {
                    if ((rb > 0) && (f.write(buf,rb) == rb))
                    {
                        conout << '\r' << "Packet " << ++packet << ", "
                            "Size " << (size += rb);
                    }
                    else if (rb > 0)
                    {
                        conerr << endl << "Error in writing file! (" <<
                            int(ExtendedError)  << ':' <<
                            int(ExtendedClass)  << ':' <<
                            int(ExtendedLocus)  << ':' <<
                            int(ExtendedAction) << ')';

                        logFile.printf("  Error in writing file! "
                            "(%d:%d:%d:%d)\r\n",
                            int(ExtendedError), int(ExtendedClass),
                            int(ExtendedLocus), int(ExtendedAction));

                        err = TRUE;
                        break;
                    }
                    else
                    {
                        long eticks = getticks();
                        long tticks = eticks-sticks;
                        if (tticks < 0)
                            tticks = eticks+
                                (24L*60L*60L*182L/10L)-
                                sticks;

                        long tsecs = tticks*10L/182L;
                        if (tsecs <= 0) tsecs = 1;

                        conout << '\r' << "File Transfer Complete!  " <<
                            packet << " Packets - " << size << " Bytes - "
                            "Average CPS = " << size/tsecs;

                        logFile.printf("  Transfer Successful -- "
                            "Avg CPS %ld\r\n", size/tsecs);

                        break;
                    }
                }
            }
        }

        f.close();

        if (err || abortObj())
        {
            sendMessage("CN5");

            unlink(file);

            if (abortObj()) logFile.putln("  Transfer Aborted");
        }
        else
            sendMessage("CY");
    }
    else
    {
        if (!master) sendMessage("SN4");
       #ifdef debugDispFileCreateError
        conerr << endl <<
            " File Create Error! (" << int(ExtendedError)  << ':' <<
                                       int(ExtendedClass)  << ':' <<
                                       int(ExtendedLocus)  << ':' <<
                                       int(ExtendedAction) << ')';
       #endif
        logFile.putln("  Error Creating File");
    }

    return ((abortObj() || err) ? -1 : 0);
}

/******************************************************************************/

int pascal cGPPL::app(void)
{
    char rmsg [ maxDataSize ];
    #ifdef MEMCHECK
      mc_register(rmsg,sizeof(rmsg));
    #endif

    while (!abortObj())
    {
        csnout << endl << "Performing UUCP G Protocol File Transfers for " <<
            sname << " (Master Mode)";
        conout << endl << endl << "Sending Outbound Files ...";
        master();

        if (abortObj()) continue;

        conout << endl << endl << "Requesting Hang Up ...";
        sendMessage("H");
        while (receiveMessage(rmsg) == 0)
        {
            if (stricmp(rmsg,"HY") == 0)
            {
                conout << endl << "Hanging Up ...";
                sendMessage("HY");
                break;
            }
            else if (stricmp(rmsg,"HN") == 0)
            {
                conout << endl << "Reversing Channels ...";
                break;
            }
        }

        if (stricmp(rmsg,"HY") == 0) break;

        if (abortObj()) continue;

        csnout << endl << "Performing UUCP G Protocol File Transfers for " <<
            sname << " (Slave Mode)";
        conout << endl << endl << "Receiving Inbound Files ...";
        while (receiveMessage(rmsg) == 0)
        {
            if (rmsg[0] == 'S')
            {
                char * ptr = strtok(rmsg," ");
                ptr = strtok(NULL," ");
                ptr = strtok(NULL," ");

                char file [ 128 + 1 ];
                buildstr(file,spool,sname,"\\",NULL);

                munge(file+strlen(file),ptr,sname);

                receiveFile(file,FALSE,rmsg);
            }
            else if (rmsg[0] == 'R')
            {
                sendMessage("RN2");
            }
            else if (rmsg[0] == 'H')
            {
                break;
            }
        }

        if (abortObj()) continue;

        conout << endl << endl << "Hanging Up ...";
        sendMessage("HY");
        receiveMessage(rmsg);
        break;
    }

    #ifdef MEMCHECK
      mc_unregister(rmsg);
    #endif

    return (abortObj() ? -1 : 0);
}

int pascal cGPPL::master(void)
{
    char spath [ 128 + 1 ];
    buildstr(spath,spool,sname,"\\*.CMD",NULL);

    ffblk sFFB;
    int done = findfirst(spath,&sFFB,0);
    while (!done)
    {
        if (!abortObj()) doCmdFile(sFFB.ff_name);
        done = findnext(&sFFB);
    }

    return 0;
}

int pascal cGPPL::doCmdFile(char * name)
{
    char cname [ 128 + 1 ];
    buildstr(cname,spool,sname,"\\",name,NULL);

    cDOSFILE cfile;

    if (cfile.open(cname,OPEN_READ|OPEN_DENYNONE) == 0)
    {
        char cmd  [ 128 + 1 ];
        char file [ 128 + 1 ];

        int rv = 0;

        while (cfile.getuln(cmd,sizeof(cmd)) == 0)
        {
            buildstr(file,spool,sname,"\\",cmd+2,NULL);
            strTrunc(file,' ');

           #ifdef debugDispCmdFile
            conout << endl << cmd;
            conout << endl << file;
           #ifdef debugLogCmdFile
            logFile.printf("%s\r\n",cmd);
            logFile.printf("%s\r\n",file);
           #endif
           #endif

            rv = sendMessage(cmd,file);
            if (rv != 0) break;
        }

        cfile.close();

        if (rv == 0) unlink(cname);
    }

    return 0;
}

/******************************************************************************/

