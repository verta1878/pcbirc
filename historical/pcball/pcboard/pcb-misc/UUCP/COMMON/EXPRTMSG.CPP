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



#include    <exprtmsg.hpp>

#include    <uuencode.hpp>

#ifdef BOUNCE
#include    "pcbmsg.hpp"
#endif

#include    <uucp.hpp>

unsigned nextSeq = 0;

static char lookup [] =
{
       ' ',    'O',    'O',    'H',    'D',    'C',    'S',    '.',
       '.', '\x09',    'o',    'o',    'o',    '&',    '&',    '*',
       '>',    '<',    '+',    '!',    'P',    'S',    '_',    '+',
       '^',    'v',    '>',    '<',    '-',    '+',    '^',    'v',
    '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27',
    '\x28', '\x29', '\x2A', '\x2B', '\x2C', '\x2D', '\x2E', '\x2F',
    '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37',
    '\x38', '\x39', '\x3A', '\x3B', '\x3C', '\x3D', '\x3E', '\x3F',
    '\x40', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47',
    '\x48', '\x49', '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F',
    '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57',
    '\x58', '\x59', '\x5A', '\x5B', '\x5C', '\x5D', '\x5E', '\x5F',
    '\x60', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67',
    '\x68', '\x69', '\x6A', '\x6B', '\x6C', '\x6D', '\x6E', '\x6F',
    '\x70', '\x71', '\x72', '\x73', '\x74', '\x75', '\x76', '\x77',
    '\x78', '\x79', '\x7A', '\x7B', '\x7C', '\x7D', '\x7E',    '^',
       'C',    'u',    'e',    'a',    'a',    'a',    'a',    'c',
       'e',    'e',    'e',    'i',    'i',    'i',    'A',    'A',
       'E',    'a',    'A',    'o',    'o',    'o',    'u',    'u',
       'y',    'O',    'U',    'c',    '#',    'Y',    'P',    'f',
       'a',    'i',    'o',    'u',    'n',    'N',    'a',    'o',
       '?',    '-',    '-',    '2',    '4',    '!',    '<',    '>',
       '#',    '#',    '#',    '|',    '+',    '+',    '+',    '+',
       '+',    '+',    '|',    '+',    '+',    '+',    '+',    '+',
       '+',    '+',    '+',    '+',    '-',    '+',    '+',    '+',
       '+',    '+',    '+',    '+',    '+',    '-',    '+',    '+',
       '+',    '+',    '+',    '+',    '+',    '+',    '+',    '+',
       '+',    '+',    '+',    '#',    '_',    '(',    ')',    '~',
       'a',    'B',    'T',    'r',    'E',    'o',    'u',    't',
       '0',    '0',    'O',    'o',    '%',    '0',    'e',    'U',
       '=',    '+',    '>',    '<',    'f',    'f',    '/',    '=',
       'O',    'o',    '.',    'V',    'n',    '2',    '*',    ' '
};

static char * near pascal xlateText(char * s)
{
    if (PcbData.uucpHighAscii == 'N') return s;

    char * p = s;

    while (*s)
    {
        if (*s > 127)
            switch (PcbData.uucpHighAscii)
            {
                case 'S':
                    strcpy(s,s+1);
                    continue;

                case 'R':
                    *s = '.';
                    break;

                case 'C':
                    *s = lookup[*s];
                    break;
            }

        ++s;
    }

    return p;
}

static void near pascal buildFiles (
    int msgType, char * sp, char * line1, char * to, char * from,
    char * subj, char * ng, char * ft, char * msgid, char * date,
    char * distrib, char * org, char * replyto,
    char * refs,
    cDOSFILE & file, int part, int totParts, char * attachInfo,
    char * sender
    )
{
    char base [ 8 + 1 ];
    sprintf(base,"%04u",++nextSeq%10000);

    char cmdName [ 128 + 1 ];
    char xqtName [ 128 + 1 ];
    char datName [ 128 + 1 ];

    buildstr(cmdName,sp,base,NULL);
    strcpy(xqtName,cmdName);
    strcpy(datName,cmdName);

    strcat(cmdName,".CMD");
    strcat(xqtName,".XQT");
    strcat(datName,".DAT");

    char buf [ 1024 + 1 ];

        /*------------------------------------------------------------*/

/*
CMD
~~~
S 0014.DAT D.lcars0014 uucp - 0014.DAT 0666
S 0014.XQT X.lcars0014 uucp - 0014.XQT 0666
*/

    file.open(cmdName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE);

    sprintf(buf,"S %s.DAT D.%1.7s%s uucp - %s.DAT 0666",base,PcbData.uucpName,
        base,base);
    file.putuln(buf);
    sprintf(buf,"S %s.XQT X.%1.7s%s uucp - %s.XQT 0666",base,PcbData.uucpName,
        base,base);
    file.putuln(buf);

    file.close();

        /*------------------------------------------------------------*/

/*
XQT
~~~
U uucp lcars
F D.lcars0014
I D.lcars0014
C rnews
*/

    file.open(xqtName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE);

    buildstr(buf,"U uucp ",PcbData.uucpName,NULL);
    file.putuln(buf);
    sprintf(buf,"F D.%1.7s%s",PcbData.uucpName,base);
    file.putuln(buf);
    buf[0] = 'I';
    file.putuln(buf);

    strcpy(buf,"C ");

    switch (msgType)
    {
        case msgTypeEmail:
            strcat(buf,"rmail ");
            strcat(buf,to);
            break;

        case msgTypeNews:
            strcat(buf,"rnews");
            break;
    }

    file.putuln(buf);

    file.close();

        /*------------------------------------------------------------*/

/*
DAT
~~~
Path: lcars!scott.dale.robison
From: scott.dale.robison@lcars.com (Scott Dale Robison) 
Newsgroups: to.lcars
Subject: test article #1
Message-ID: <1.211.uupcb@lcars.com>
Date: 25 Jun 94 12:18:00 GMT
Distribution: world
Organization: uuPCB - Demonstration Version
Reply-To: scott.dale.robison@lcars.com (Scott Dale Robison) 

this is my first test article to the newsgroup to.lcars

----
LCARS alpha Bulletin Board System - lcars.com - 801-263-8303 - 300-28800 bps
4084 S 300 E Apt A, Murray, Utah, USA, 84107-1680
General Interest & Star Trek(tm) related discussion / files / games
Support for Radio Pro, Inc. / SANDD Software / Soleil Software
-------------------------------------------------------------------------------
From lcars!scott.dale.robison Sat, 25 Jun 94 12:20:00 -0700
To: scott.robison@clarkdev.com
Subject: test message
From: scott.dale.robison@lcars.com (Scott Dale Robison) 
Message-ID: <69.1.uupcb@lcars.com>
Date: Sat, 25 Jun 94 12:20:00 -0700
Organization: uuPCB - Demonstration Version
Reply-To: scott.dale.robison@lcars.com (Scott Dale Robison) 

outbound mail from lcars.com

----
LCARS alpha Bulletin Board System - lcars.com - 801-263-8303 - 300-28800 bps
4084 S 300 E Apt A, Murray, Utah, USA, 84107-1680
General Interest & Star Trek(tm) related discussion / files / games
Support for Radio Pro, Inc. / SANDD Software / Soleil Software
*/

    file.open(datName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE);

    switch (msgType)
    {
        case msgTypeEmail:

            file.puts("From ");          file.putuln(line1);
            file.puts("To: ");           file.putuln(to);

            file.puts("Subject: ");
            if (attachInfo[0] == NUL)
                file.putuln(subj);
            else
                file.printf("%s - Part %d of %d\n",subj,part,totParts);

            file.puts("From: ");         file.putuln(from);

            if (sender && *sender)
            {
                file.puts("Sender: ");
                file.putuln(sender);
            }

            file.puts("Message-ID: ");
            if (attachInfo[0] == NUL)
                file.putuln(msgid);
            else
                file.printf("%c%03d.%s\n",msgid[0],part,msgid+1);

            file.puts("Date: ");         file.putuln(date);
            file.puts("Organization: "); file.putuln(org);
            file.puts("Reply-To: ");     file.putuln(replyto);
            if (refs[0] != NUL)
            {
                file.puts("References: ");
                file.putuln(refs);
            }

            file.putuln("X-Mailreader: PCBoard Version 15.3");
            file.putuln("X-Mailer: PCBoard/UUOUT Version 1.30");

            break;


        case msgTypeNews:

            file.puts("Path: ");         file.putuln(line1);
            file.puts("From: ");         file.putuln(from);
            file.puts("Newsgroups: ");   file.putuln(ng);
            if (ft[0] != NUL)
            {
                file.puts("Followup-To: ");
                file.putuln(ft);
            }

            file.puts("Subject: ");
            if (attachInfo[0] == NUL)
                file.putuln(subj);
            else
                file.printf("%s - Part %d of %d\n",subj,part,totParts);

            file.puts("Message-ID: ");
            if (attachInfo[0] == NUL)
                file.putuln(msgid);
            else
                file.printf("%c%03d.%s\n",msgid[0],part,msgid+1);

            file.puts("Date: ");         file.putuln(date);
            file.puts("Distribution: "); file.putuln(distrib);
            file.puts("Organization: "); file.putuln(org);
            file.puts("Reply-To: ");     file.putuln(replyto);
            if (refs[0] != NUL)
            {
                file.puts("References: ");
                file.putuln(refs);
            }

            file.putuln("X-Newsreader: PCBoard Version 15.3");
            file.putuln("X-Mailer: PCBoard/UUOUT Version 1.30");

            break;
    }

    file.putuln();
}

#ifdef BOUNCE

static char * near pascal getMsgLine(char * b, int s)
{
    static char * buf  = NULL;
    static int    size = 0;
           char * p    = NULL;

    if (b)
        p = buf = b,
        size = s;
    else if (!buf)
        return NULL;
    else
        *buf = LineSeparator,
        p = buf+1;

    buf = (char *) memchr(p,LineSeparator,size);

    if (buf)
    {
        *buf = NUL;
        size -= int(buf-p+1);
    }
    else
        p = NULL;

    return p;
}

static char * near pascal getMsgLine(cTMPMSG * m)
{
    static cTMPMSG * _m = NULL;
    if (m)
    {
        _m = m;
        _m->resetLine();
    }
    char * p = (_m ? _m->getLine() : NULL);
    if (!p) _m = NULL;
    return p;
}

#endif

#pragma argsused
void pascal exportMessage(
    int msgType, char * sp, char * line1, char * to, char * from,
    char * subj, char * ng, char * ft, char * msgid, char * date,
    char * distrib, char * org, char * replyto,
    cMSG * message, cDOSFILE * textfile, char * attachPath, char * refs,
    char * buf, int size, cTMPMSG * msgobj, char * sender )
{
   #ifndef BOUNCE
    char attachInfo [ 60 + 1 ];
    memset(attachInfo,0,sizeof(attachInfo));
    message->getExtHdr("ATTACH ",attachInfo);
    stripright(attachInfo,' ');
   #endif

    long attachSize = 0L;

   #ifndef BOUNCE
    // encode file attachment here
    // "FILENAME (SIZE) STOREDNAME"
    if (attachInfo[0] != NUL)
    {
        char fileName   [ 60 + 1 ];
        char storedName [ 60 + 1 ];

        maxstrcpy(fileName,attachInfo,sizeof(fileName));
        *strchr(fileName,' ') = NUL;
        maxstrcpy(storedName,strrchr(attachInfo,' ')+1,sizeof(storedName));

     // encodeFile(file,fileName,attachPath,storedName);

        char attachName [ 128 + 1 ];
        buildstr(attachName,attachPath,storedName,NULL);

        cDOSFILE afile;
        if (afile.open(attachName,OPEN_READ|OPEN_DENYNONE) == 0)
        {
            attachSize = afile.size();
            afile.close();
        }
    }
   #endif

    int part;
    int totParts = int(attachSize/(uucpFileSize*1024L));
    if (attachSize%(uucpFileSize*1024L)) ++totParts;

    cDOSFILE file;

    to = strtok(to,",");

    do {

        part = 0;

        buildFiles(
            msgType, sp, line1, to, from, subj, ng, ft, msgid, date, distrib,
            org, replyto, refs, file, part, totParts,
           #ifndef BOUNCE
            attachInfo
           #else
            ""
           #endif
            ,sender);

        if (message != NULL)
        {
           #ifndef BOUNCE

            char * line;
            while ((line = message->getLine()) != NULL)
                file.putuln(xlateText(line));

            message->resetLine();

            // encode file attachment here
            // "FILENAME (SIZE) STOREDNAME"
            if (attachInfo[0] != NUL)
            {
                char fileName   [ 60 + 1 ];
                char storedName [ 60 + 1 ];

                maxstrcpy(fileName,attachInfo,sizeof(fileName));
                *strchr(fileName,' ') = NUL;
                maxstrcpy(storedName,strrchr(attachInfo,' ')+1,sizeof(storedName));

             // encodeFile(file,fileName,attachPath,storedName);

                char attachName [ 128 + 1 ];
                buildstr(attachName,attachPath,storedName,NULL);

                cDOSFILE afile;
                if (afile.open(attachName,OPEN_READ|OPEN_DENYNONE) == 0)
                {
                    encodeFileStart(afile);
                    do {
                        file.close();
                        buildFiles(
                            msgType, sp, line1, to, from, subj, ng, ft, msgid,
                            date, distrib, org, replyto, refs,
                            file,++part,totParts,
                           #ifndef BOUNCE
                            attachInfo
                           #else
                            ""
                           #endif
                            ,sender);
                    } while (encodeFilePart(file,afile,fileName) == 0);
                    encodeFileStop();
                    afile.close();
                }
            }

           #endif
        }
        else if (textfile != NULL)
        {
           #ifdef BOUNCE

            char line [ 1024 + 1 ];
            while (textfile->getln(line,sizeof(line)) == 0)
                file.putuln(xlateText(line));

            if (buf || msgobj)
            {
                file.putuln();
                file.putuln("Text from original message follows:");
                file.putuln("-----------------------------------");
                int hdr = 1;
                char * ptr = (buf ? getMsgLine(buf,size) : getMsgLine(msgobj));
                while (ptr)
                {
                    if (hdr)
                    {
                        if (*ptr == '\x01')
                            ++ptr;
                        else if (*ptr == '\x00')
                            hdr = 0;
                    }
                    file.putuln(xlateText(ptr));
                    ptr = (buf ? getMsgLine(NULL,0) : getMsgLine(NULL));
                }
            }

           #endif
        }

        file.close();

    } while ((to = strtok(NULL,",")) != NULL);
}

void pascal getSeqNum ( void )
{
    nextSeq = 0U;

    char name [ 128 + 1 ];
    buildstr(name,PcbData.uucpPath,"UUOUT.SEQ",NULL);
    if (fileexist(name) == 255) return;

    cDOSFILE file;
    if (file.open(name,OPEN_READ|OPEN_DENYRDWR) != 0) return;

    char text [ 128 + 1 ];
    file.getln(text,sizeof(text));

    file.close();

    nextSeq = unsigned(atol(text));
}

void pascal putSeqNum ( void )
{
    char name [ 128 + 1 ];
    buildstr(name,PcbData.uucpPath,"UUOUT.SEQ",NULL);

    cDOSFILE file;
    if (file.open(name,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE) != 0) return;

    char text [ 128 + 1 ];
    sprintf(text,"%u",nextSeq%10000);
    file.putln(text);

    file.close();
}

