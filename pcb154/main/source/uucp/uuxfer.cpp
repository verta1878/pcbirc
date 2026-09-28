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



#include    <conio.h>
#include    <ctype.h>
#include    <dir.h>
#include    <dos.h>
#include    <setjmp.h>

#include    <iostream.h>
#include    <iomanip.h>
#include    <constrea.h>

#include    <uucp.hpp>

#include    "gprot.hpp"

extern "C" void pascal openport   ( void );
//extern "C" void pascal openmodem  ( int type );
//extern "C" void pascal closemodem ( int type );

//const int HIDE = 1;

const int NOHANG = 0;
const int HANG   = 1;

const int errNone         = 0;
const int errChatFailure  = 1;
const int errLostCarrier  = 2;
const int errSiteNotFound = 3;
const int errInvSystem    = 4;
const int errNoMsg        = 5;
const int errBadMsg       = 6;
const int errNoGProt      = 7;
const int errGProt        = 8;

/******************************************************************************/

extern char volatile CDokay;
extern char VerifyCDLoss;

/******************************************************************************/

constream conscr;
constream csnout;
constream cpiout;
constream cpoout;
constream cmiout;
constream cmoout;
constream conout;
constream conerr;

cDOSFILE  logFile;

/******************************************************************************/

static jmp_buf lostCDTask;

char buf [ 8192 ];

/******************************************************************************/

class cSYSFILE : protected cDOSFILE
{

    public:

        cSYSFILE ( void ) { }

        cSYSFILE ( char * site, char * buf, size_t size )
        {
            getSite(site,buf,size);
        }

        char * pascal getSite ( char * site, char * buf, size_t size );

    protected:

    private:

};

char * pascal cSYSFILE::getSite ( char * site, char * buf, size_t size )
{
    if (size == 0) return NULL;

    buf[0] = NUL;

    if (site[0] == NUL) return NULL;

    char fileName [ 128 + 1 ];
    buildstr(fileName,PcbData.uucpPath,"SYSTEMS",NULL);
    if (open(fileName,OPEN_READ|OPEN_DENYNONE) != 0) return NULL;

    int sitelen = strlen(site);

    while (getln(buf,size) == 0)
    {
        stripleft(buf,'\t');
        stripleft(buf,' ');

        if (buf[0] == '#') buf[0] = NUL;
        if (buf[0] == NUL) continue;

        if ((memicmp(buf,site,strlen(site)) == 0) &&
            ((buf[sitelen] == '\t') || (buf[sitelen] == ' ')))
            break;

        buf[0] = NUL;
    }

    close();

    return (buf[0] ? buf : NULL);
}

/******************************************************************************/

const char DLE = '\x10';

static int near pascal doGProtocol(char * spool, char * sname, char * prot)
{
 // if (uucp7E1)
 // {
 //  // while (OutBytes > 0) giveup();
 //  // tickdelay(18);
 //     Asy.DataBits = 8;
 //     setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
 // }
    csnout << endl << "Performing UUCP G Protocol File Transfers for " <<
        sname;

    logFile.putln("-------------------------------------------------------------------------------");
    logFile.putln("Beginning UUCP G Protocol");

    cGPROT gProt(spool,sname,prot);
    return ((gProt.run() == 0) ? errNone : errGProt);
}

static int near pascal getMessage(char * m, int ms)
{
    if (m == NULL) return -1;

    char * s = m;

    bool bldMsg = FALSE;
    int  bytes  = 0;

    settimer(4,uucpResTime*182L/10);

    long lastGetTimer = gettimer(4);

    while (gettimer(4) > 0)
    {
        if (gettimer(4)+18 < lastGetTimer)
        {
            if (!cdstillup() || kbhit())
                longjmp(lostCDTask,1);
            lastGetTimer = gettimer(4);
        }
        int c = comminkey();
        if (c == -1)
        {
            giveup();
        }
        else if (bldMsg)
        {
            *(m++) = char(c);
            if (char(c) == NUL)
            {
                cmiout << endl << s;
                logFile.printf("Received: %s\r\n",s);
                return bytes;
            }
            else if (bytes == ms)
                return -1;
            ++bytes;
        }
        else if (c == DLE)
        {
            bldMsg = TRUE;
        }
    }

    logFile.putln("*** Message not received in time allowed!");

    return -1;
}

static void near pascal putMessage(char * m)
{
    if (!cdstillup() || kbhit()) longjmp(lostCDTask,1);
    sendbyte(DLE);
    sendstr(m,strlen(m)+1); // Send string with terminating NUL
    cmoout << endl << m;
    logFile.printf("    Sent: %s\r\n",m);
}

static int near pascal doShereExchange(char * spool, char * sname, char * prot)
{
    if (uucp7E1)
    {
     // while (OutBytes > 0) giveup();
     // tickdelay(18);
        Asy.DataBits = 8;
        setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
    }

    csnout << endl << "Negotiating Shere Exchange for " << sname;

    logFile.putln("-------------------------------------------------------------------------------");
    logFile.putln("Beginning Shere Exchange");

    char msg [ 128 + 1 ];
    int  size;

    size = getMessage(msg,sizeof(msg));
    if (size <= 0)
    {
        conerr << endl << "Error finding Shere message ...";
        return errNoMsg;
    }
    else if (memcmp(msg,"Shere",5) != 0)
    {
        conerr << endl << "Bad message!";
        return errBadMsg;
    }

    buildstr(msg,"S",uucpName,NULL);
    if (uucpGrade && isascii(uucpGrade) && isalnum(uucpGrade))
        sprintf(msg+strlen(msg)," -p%c -v%c=%c",
            uucpGrade,uucpGrade,uucpGrade);
    putMessage(msg);

    size = getMessage(msg,sizeof(msg));
    if (size <= 0)
    {
        conerr << endl << "Error finding status message ...";
        return errNoMsg;
    }
    else if (strcmp(msg,"ROK") != 0)
    {
        conerr << endl << "Answerer didn't accept call!";
        return errBadMsg;
    }

    size = getMessage(msg,sizeof(msg));
    if (size <= 0)
    {
        conerr << endl << "Error finding protocol list message ...";
        return errNoMsg;
    }
    else if (msg[0] != 'P')
    {
        conerr << endl << "Bad message!";
        return errBadMsg;
    }
    else if (strchr(msg+1,'g') == NULL)
    {
        conerr << endl << "UUCP G Protocol Not Available!";
        return errNoGProt;
    }

    // Note: These three lines are only for use with the UUCP G Protocol
    //       Must confirm that it is available first
    msg[0] = 'U';
    msg[1] = tolower(prot[0]);
    msg[2] = NUL;
    putMessage(msg);
    int rv = doGProtocol(spool,sname,prot);

    logFile.putln("-------------------------------------------------------------------------------");
    logFile.putln("Shutting Down Shere Exchange");

    if (cdstillup())
    {
        putMessage("OOOOOO");
        getMessage(msg,sizeof(msg));
    }

    return rv;
}

static int near pascal isodigit(char c)
{
    return ((c >= '0') && (c <= '7'));
}

const char suppressCR = '\xFE';
const char delay2Sec  = '\xFD';
const char sendBREAK  = '\xFC';
const char sendNUL    = '\xFB';
const char delayFSec  = '\xFA';
const char subSep     = '\xF9';
const char shift7E1   = '\xF8';
const char shift8N1   = '\xF7';

static char * near pascal xlateStr(char * s, bool expect)
{
    if (s == NULL) return s;

    stripboth(s,'\"');
    if (s[0] == NUL) return s;

    char * o = s;
    char * p = s;

    while (*s)
    {
        if (*s == '\\')
        {
            ++s;
            switch (*s)
            {
                case '8':  *(p++) = shift8N1;   ++s; break;
                case 'b':  *(p++) = '\b';       ++s; break;
                case 'c':  *(p++) = suppressCR; ++s; break;
                case 'd':  *(p++) = delay2Sec;  ++s; break;
                case 'e':  *(p++) = shift7E1;   ++s; break;
                case 'K':  *(p++) = sendBREAK;  ++s; break;
                case 'n':  *(p++) = '\n';       ++s; break;
                case 'N':  *(p++) = sendNUL;    ++s; break;
                case 'p':  *(p++) = delayFSec;  ++s; break;
                case 'q':                       ++s; break;
                case 'r':  *(p++) = '\r';       ++s; break;
                case 's':  *(p++) = ' ';        ++s; break;
                case 't':  *(p++) = '\t';       ++s; break;
                case '\\': *(p++) = '\\';       ++s; break;

                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                {
                    int i = 0, d = 0;
                    while (isodigit(*s) && (d++ < 3)) i = (i*8)+(*(s++)-'0');
                    *(p++) = (char(i) ? char(i) : sendNUL);
                    break;
                }

                case NUL:
                    break;

                default:
                    *(p++) = *(s++);
                    break;
            }
        }
        else if (expect && (*s == '-'))
        {
            *(p++) = subSep;
            ++s;
        }
        else
        {
            *(p++) = *(s++);
        }
    }

    *p = NUL;

    return o;
}

const char EOT = '\x04';

static char saveBuf [ 256 + 1 ];

static void near pascal doSend(char * s)
{
    if (s == NULL) s = "";

    maxstrcpy(saveBuf,s,sizeof(saveBuf));

    cmoout << endl << /* "Send: " << */
        (((s[0] == '\\') && (s[1] == 'q')) ? "<******>" : s);

    logFile.printf("    Sent: %s\r\n",s);

    tickdelay(18);

    if (strcmp(s,"BREAK") == 0)
    {
        sendbyte(NUL);
        return;
    }

    if (strcmp(s,"EOT") == 0)
    {
        sendbyte(EOT);
        return;
    }

    xlateStr(s,FALSE);

    bool doCR = TRUE;

    while (*s)
    {
        switch (*s)
        {
            case shift7E1:
                Asy.DataBits = 7;
                setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
                break;

            case shift8N1:
                Asy.DataBits = 8;
                setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
                break;

            case suppressCR:
                doCR = FALSE;
                break;

            case delay2Sec:
                tickdelay(2*182/10);
                break;

            case delayFSec:
                tickdelay(4);
                break;

            case sendBREAK:
            case sendNUL:
                sendbyte(NUL);
                break;

            default:
                sendbyte(*s);
                tickdelay(2);
                break;
        }

        ++s;
    }

    if (doCR) sendbyte('\r');
}

static int abortFlag = FALSE;

static int near pascal doWaitFor(char * s)
{
    cmiout << endl << /* "Expect: " << */ s;

    logFile.printf("  Expect: %s\r\n",s?s:"<nothing>");

LOOP:

    if (s == NULL) return 0;

    xlateStr(s,TRUE);
    if (s[0] == NUL) return 0;

    char buf [ 1024 ];
    memset(buf,0,sizeof(buf));

    char * nextSeq = strchr(s,subSep);
    if (nextSeq) *(nextSeq++) = NUL;

    int slen = strlen(s);

    settimer(4,uucpResTime*182L/10);

    long lastGetTimer = gettimer(4);

    while (gettimer(4) > 0)
    {
        if (gettimer(4)+18 < lastGetTimer)
        {
            if (kbhit())
                switch (getch())
                {
                    case 27: abortFlag = TRUE; return 0;
                    case  0: getch();          break;
                }
            lastGetTimer = gettimer(4);
        }

        int c = comminkey();
        if (c == -1)
        {
            giveup();
        }
        else
        {
            conout << char(c);
            memcpy(buf,buf+1,slen-1);
            buf[slen-1] = char(c);
            if (memicmp(buf,s,slen) == 0) return 0;
            if ((memicmp(s,    "CONNECT",7) == 0) &&
                (
                (memicmp(buf+3,"BUSY",   4) == 0) ||
                (memicmp(buf+0,"NO DIAL",7) == 0) // ||
             // (memicmp(buf+3,"RING",   4) == 0)
                ))
            {
                static int tries = 0;
                if (tries++ == uucpBusyTries)
                {
                    logFile.putln("*** Expect/Sub-Expect sequence not "
                        "found before timeout!");
                    return -1;
                }
                char tmpBuf [ sizeof(saveBuf) ];
                strcpy(tmpBuf,saveBuf);
                doSend(tmpBuf);
                tickdelay(18);
                settimer(4,uucpResTime*182L/10);
            }
        }
    }

    if (nextSeq)
    {
        s = strchr(nextSeq,subSep);
        if (s) *(s++) = NUL;
        doSend(nextSeq);
        goto LOOP;
    }

    logFile.putln("*** Expect/Sub-Expect sequence not found before timeout!");

    return -1;
}

static void near pascal doHangup(int ticks)
{
    turnoffdtr();
    tickdelay(ticks);
    turnondtr();
 // logFile.putln("*** Expect/Sub-Expect sequence not found before timeout!");
}

static int near pascal doLoginScript(char * spool, char * sname)
{
    int cdval;
    cdval = setjmp(lostCDTask);
    if (cdval == 1)
    {
        conerr << endl << "Lost Carrier Detect, aborting ...";
        return errLostCarrier;
    }

    char sysLine [ 1024 + 1 ];
    cSYSFILE system(sname,sysLine,sizeof(sysLine));

    if (sysLine[0] == NUL)
    {
        conerr << endl << "Unable to locate " << sname << " in systems file, "
            "aborting ...";
        logFile.printf("*** Site %s not found in systems file.\r\n",sname);
        return errSiteNotFound;
    }

    char * site   = strtok(sysLine," \t"); // Used
    char * times  = strtok(NULL,   " \t"); // Used
    char * dev    = strtok(NULL,   " \t"); // Ignored
    char * hwtype = strtok(NULL,   " \t"); // Ignored
    char * speed  = strtok(NULL,   " \t"); // Ignored
    char * prot   = strtok(NULL,   " \t"); // Used

    if ((site   == NULL) || (times == NULL) || (dev  == NULL) ||
        (hwtype == NULL) || (speed == NULL) || (prot == NULL) ||
        (toupper(prot[0]) != 'G'))
    {
        conerr << endl << "Invalid systems file entry for " << sname <<
            ", aborting ...";
        logFile.printf("*** Site %s systems file entry invalid.\r\n",sname);
        return errInvSystem;
    }

    csnout << endl << "Processing Chat Script for " << sname;

    logFile.printf("SPOOL = %s\r\n",sname);
    logFile.putln("-------------------------------------------------------------------------------");
    logFile.putln("Beginning Chat Script");

    char * expect;
    char * send;

    do {

        expect = strtok(NULL," \t");
        send   = strtok(NULL," \t");

        if (expect && doWaitFor(expect) != 0)
        {
            conerr << endl << "Expected input not found in " << uucpResTime <<
                " seconds, aborting ...";
            return errChatFailure;
        }

        if (send) doSend(send);

    } while ((expect != NULL) && (send != NULL) && !abortFlag);

    if (!abortFlag && online()) CDokay = VerifyCDLoss = TRUE;

    if (!abortFlag && cdstillup())
    {
        csnout << endl << "Connected to " << sname;
        logFile.putln("Connection Established");
    }
    else
    {
        conerr << endl << "Unable to connect to " << sname;
        logFile.putln("*** Connection Failure");
        doHangup(5*182/10);
        return errChatFailure;
    }

    conout.clrscr();

    int rv = doShereExchange(spool,sname,prot);

    doHangup(5*182/10);

    return rv;
}

static int near pascal doIfWorkQueued(char * spool, char * sname)
{
    char spath [ 128 + 1 ];
    buildstr(spath,spool,sname,"\\*.CMD",NULL);

    int cmdFound = FALSE;

    ffblk sFFB;
    int done = findfirst(spath,&sFFB,0);
    while (!done)
    {
        cmdFound = TRUE;
        done = findnext(&sFFB);
    }

    return (cmdFound ? doLoginScript(spool,sname) : 0);
}

static int near pascal doSpools(void)
{
    int rv = 0;

    char spool [ 128 + 1 ];

    if ((stricmp(uucpSystem,"all") != 0) && (stricmp(uucpSystem,"any") != 0))
    {
        buildstr(spool,PcbData.uucpSpoolPath,uucpSystem,NULL);
        strupr(spool);
        char tmp [ 128 + 1 ];
        if (validatedir(spool,tmp) != 0)
        {
            conerr << endl << "Unable to validate/create " << spool;
            logFile.printf("*** Unable to validate/create %s\r\n",spool);
            return 1;
        }
    }

    buildstr(spool,PcbData.uucpSpoolPath,"*.*",NULL);

    ffblk sFFB;
    int done = findfirst(spool,&sFFB,FA_DIREC);
    while (!done)
    {
        csnout << endl << "Scanning Spools";
        if ((sFFB.ff_attrib & FA_DIREC) && (sFFB.ff_name[0] != '.'))
        {
            csnout << endl << "Scanning Spool " << sFFB.ff_name;
            if ((stricmp(sFFB.ff_name,uucpSystem) == 0) ||
                (stricmp("all",       uucpSystem) == 0))
            {
                int trv = doLoginScript(PcbData.uucpSpoolPath,sFFB.ff_name);
                if (trv > rv) rv = trv;
            }
            else if (stricmp("any",uucpSystem) == 0)
            {
                int trv = doIfWorkQueued(PcbData.uucpSpoolPath,sFFB.ff_name);
                if (trv > rv) rv = trv;
            }
        }
        done = findnext(&sFFB);
    }
    csnout << endl;

    return rv;
}

static int near pascal initModem(void)
{
 // Asy.Online = REMOTE;
    Asy.CarrierSpeed = Asy.ConnectSpeed = Asy.ModemSpeed = PcbData.ModemSpeed;
    initport();

    // Ought to check com port number here and abort if 0
    if (Asy.ComPortNumber == 0) return -1;

    Asy.DataBits = (uucp7E1 ? 7 : 8);
    setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
 // openport();
    openmodem(HIDE);
 // online();
 // if (!cdstillup()) Asy.Online = LOCAL;

    return 0;
}

static void near pascal deinitModem(void)
{
    if (uucpOffHook)
    {
        doHangup(18);
        tickdelay(2*182/10);
        slowsendtomodem("AT\r");
        slowsendtomodem("AT\r");
        slowsendtomodem(PcbData.ModemOff);
        slowsendtomodem("\r");
    }

    closemodem(uucpOffHook ? NOHANG : HANG);
}

int main(void)
{
    #ifdef MEMCHECK
      mc_startcheck(erf_standard);
    #endif

    initialize();

    settimer(2,0);

    int outTop  =    2;
    int outBot  =   19;
    int errTop  =   21;
    int errBot  =   24;

    int outAttr = 0x07;
    int errAttr = 0x0F;

    int crRatio =    2;

    text_info ti;
    gettextinfo(&ti);

    switch (ti.currmode)
    {
        case C4350:
            if (ti.screenheight == 43)
            {
                outTop =  2;
                outBot = 34;
                errTop = 36;
                errBot = 42;
            }
            else if (ti.screenheight == 50)
            {
                outTop =  2;
                outBot = 39;
                errTop = 41;
                errBot = 49;
            }
            crRatio = 1;
            // Fall through for color selection

        case C40:
        case C80:
            outAttr = 0x1F;
            errAttr = 0x4F;
            break;

        case BW40:
        case BW80:
        case MONO:
        default:
            break;
    }

    conscr << setattr(outAttr);
    conscr.clrscr();

    int i;

    for (i = 0; i < (ti.screenwidth*ti.screenheight); ++i)
    {
        buf[i*2+0] = ' ';
        buf[i*2+1] = outAttr;
    }

    for (i = 0; i < ti.screenheight; ++i)
    {
        buf[(i*ti.screenwidth+(                  1 -1))*2] =
        buf[(i*ti.screenwidth+((ti.screenwidth/2+0)-1))*2] =
        buf[(i*ti.screenwidth+((ti.screenwidth/2+1)-1))*2] =
        buf[(i*ti.screenwidth+( ti.screenwidth     -1))*2] = '³';
    }

    for (i = 0+1; i < ti.screenwidth-1; ++i)
    {
        char ch;
        if (i == 0)
            ch = 'Ã';
        else if (i == (ti.screenwidth-1))
            ch = '´';
        else
            ch = 'Ä';
        buf[((outTop- 1-1)*ti.screenwidth+i)*2] =
        buf[((outTop+ 1-1)*ti.screenwidth+i)*2] =
        buf[((outTop+ 5-1)*ti.screenwidth+i)*2] =
        buf[((outTop+ 9-1)*ti.screenwidth+i)*2] =
        buf[((outBot+ 1-1)*ti.screenwidth+i)*2] =
        buf[((errBot+ 1-1)*ti.screenwidth+i)*2] = ch;
    }

    buf[((              1-1)*ti.screenwidth+(             1-1))*2] = 'Ú';
    buf[((              1-1)*ti.screenwidth+(ti.screenwidth-1))*2] = '¿';
    buf[((ti.screenheight-1)*ti.screenwidth+(             1-1))*2] = 'À';
    buf[((ti.screenheight-1)*ti.screenwidth+(ti.screenwidth-1))*2] = 'Ù';

    puttext(1,1,ti.screenwidth,ti.screenheight,buf);

    conscr << setxy(13,1)               <<
        " UUXFER Version 1.30 -- UUCP/UUCICO Utility for PCBoard ";
    conscr << setxy(11,ti.screenheight) <<
        " Copyright (C) 1994-1996 -- Clark Development Company, Inc. ";

    csnout.window(                 1+crRatio,outTop+ 0,ti.screenwidth    -crRatio,outTop+ 0);
    csnout << setattr(outAttr); csnout.clrscr();

    cpiout.window(                 1+crRatio,outTop+ 2,ti.screenwidth/2+0-crRatio,outTop+ 4);
    cpiout << setattr(outAttr); cpiout.clrscr(); cpiout.setf(ios::uppercase);

    cpoout.window(ti.screenwidth/2+1+crRatio,outTop+ 2,ti.screenwidth    -crRatio,outTop+ 4);
    cpoout << setattr(outAttr); cpoout.clrscr(); cpoout.setf(ios::uppercase);

    cmiout.window(                 1+crRatio,outTop+ 6,ti.screenwidth/2+0-crRatio,outTop+ 8);
    cmiout << setattr(outAttr); cmiout.clrscr();

    cmoout.window(ti.screenwidth/2+1+crRatio,outTop+ 6,ti.screenwidth    -crRatio,outTop+ 8);
    cmoout << setattr(outAttr); cmoout.clrscr();

    conout.window(                 1+crRatio,outTop+10,ti.screenwidth    -crRatio,outBot);
    conout << setattr(outAttr); conout.clrscr();

    conerr.window(                 1+crRatio,errTop,   ti.screenwidth    -crRatio,errBot);
    conerr << setattr(errAttr); conerr.clrscr();

    int rv = 99;

    if (initModem() == 0)
    {
        char logName [ 128 + 1 ];
        buildstr(logName,PcbData.uucpLogPath,"UUXFER.LOG",NULL);
        logFile.open(logName,OPEN_WRIT|OPEN_DENYWRIT|OPEN_APPEND);
        logFile.putln("*******************************************************************************");
        strcpy(logName,"START = DDDDDDDD TTTTTTTT");
        datestr (logName+ 8);
        timestr1(logName+17);
        logName[16] = ' ';
        logFile.putln(logName);

        rv = doSpools();

        logFile.putln("*******************************************************************************");
        logFile.printf("Exiting with errorlevel %d\r\n",rv);
        strcpy(logName,"END   = DDDDDDDD TTTTTTTT");
        datestr (logName+ 8);
        timestr1(logName+17);
        logName[16] = ' ';
        logFile.putln(logName);

        logFile.close();

        csnout << endl << "Processing complete, exiting ...";

        deinitModem();
    }

    deinitialize();

    conscr << setattr(0x07);
    conscr.clrscr();

    #ifdef MEMCHECK
      mc_endcheck();
    #endif

    return rv;
}

