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



#include    <dos.h>
#include <conio.h>
#include    <string.h>
#include    <time.h>

#include    <iostream.h>
#include    <iomanip.h>
#include    <constrea.h>

#include    <dosclass.hpp>
#include    <pcbreal.hpp>
#include    <pcbmsgs.hpp>
#include    <msgbase.hpp>

#include    <exprtmsg.hpp>

#include    <uucp.hpp>

/******************************************************************************/

char scrbuf [ 80 * 50 * 2 ];

bool abortFlag = FALSE;

   constream conscr;
// constream conout;
   constream conout0;
   constream conout1;
   constream conout2;
   constream conout3;
   constream conerr;

   cDOSFILE  logFile;

/******************************************************************************/

/*
//   char uucpPath[33];       /* location for UUCP files                        */
//   char uucpSpoolPath[33];  /* location for UUCP Spool files                  */
//   char uucpLogPath[33];    /* location for UUCP Log files                    */
//   char CompBatFile[33];    /* name/loc of compress batch file for UUCP       */
//   char DeCompBatFile[33];  /* name/loc of decompress batch file for UUCP     */
//   char uucpName[17];       /* UUCP Name                                      */
//   char uucpDomainName[33]; /* UUCP Domain Name                               */
//   char uucpEmailHost[17];  /* UUCP Email Host Name                           */
//   char uucpNewsHost[17];   /* UUCP News Host Name                            */
//   char uucpDefDist[17];    /* UUCP Default News Distribution                 */
//   char uucpTimeZone[6];    /* Time Zone - offset from GMT                    */
//   char uucpSeparator[2];   /* UUCP Name Separator                            */
//   char Organization[65];   /* Organization Name                              */
//   char uucpHighAscii;      /* S = Strip, R = Replace, C = Convert            */
//   unsigned uucpEmailConf;  /* number of UUCP Email Conference                */
//   unsigned uucpJunkConf;   /* number of UUCP Junk Conference                 */
//   bool uucpBangDomain;     /* Bang Domain                                    */
//   bool uucpSubDomain;      /* Sub Domain                                     */
// */

/******************************************************************************/

static void near pascal doConf(char * np, char * mp, unsigned cn,
    pcbconftype & cr, char * mn)
{
    cPCBMSGS message;

    if (message.open(cr.MsgFile) == -1)
        return;

    if (cr.ExportPtr > message.highMsgNum())
        cr.ExportPtr = message.highMsgNum();

    message.seekIdx(cr.ExportPtr+1);

    while (message.readIdx() >= 0)
    {
        if (kbhit())
            switch (getch())
            {
                case 27: abortFlag = TRUE; break;
                case  0: getch();          break;
            }

        if (abortFlag) break;

        if (message.idxRec.num > 0)
            conout2 << endl << "    Scanning Message " << message.idxRec.num;

        // If the message is inactive or not present, skip
        if (message.idxRec.off <= 0) continue;

        // If a problem reading the message header, skip
        if (message.readHdr(message.idxRec.off) != 0) continue;

        cr.ExportPtr = message.idxRec.num;

        // If the message is not set to echo, skip
        if (message.msgHdr.echo != 'E') continue;

        // If the message is protected by a group password, skip
        if ((message.msgHdr.status == MSG_GPWD)      ||
            (message.msgHdr.status == MSG_GPWD_READ) ||
            (message.msgHdr.status == MSG_GPWD_ALL))
            continue;

        // If a problem reading the message body, skip
        if (message.readBody() != 0) continue;

        // Read the TO information
        char to [ 99 * (50+1) + 1 ];

        if (memcmp(message.msgHdr.tname,"@LIST@                   ",25) == 0)
        {
            memset(to,NUL,sizeof(to));
            char * p = to;
            for (int i = 0; i < message.getExtHdrs(); ++i)
            {
                sMSGEXTHDR * hdr = message.getExtHdr(i);
                if (memcmp(hdr->funct,"LIST   ",7) == 0)
                {
                    memcpy(p,hdr->desc,50);
                    p[50] = NUL;
                    stripright(p,' ');
                    p += strlen(p);
                    *(p++) = ',';
                    *p = NUL;
                }
            }
            *(--p) = NUL;
        }
        else
        {
            memset(to,' ',sizeof(to));
            to[sizeof(to)-1] = NUL;
            memcpy(to,message.msgHdr.tname,25);
            message.getExtHdr("TO     ",to+ 0);
            message.getExtHdr("TO2    ",to+60);
            stripright(to,' ');
        }

        // Read the short from information (all we need for export)
        char from [ 25 + 1 ];
        memset(from,' ',sizeof(from));
        from[sizeof(from)-1] = NUL;
        memcpy(from,message.msgHdr.fname,25);
        stripright(strupr(from),' ');

        // Convert to lower case and convert spaces to separators
        char lfrom [ 120 + 1 ];
        change(strlwr(strcpy(lfrom,from)),' ',PcbData.uucpSeparator[0]);

        char aliasName [ 128 + 1 ];
        buildstr(aliasName,PcbData.uucpPath,"ALIAS.OUT",NULL);

        cDOSFILE aliasFile;
        if (exist(aliasName) &&
            (aliasFile.open(aliasName,OPEN_READ|OPEN_DENYNONE) == 0))
        {
            aliasName[0] = NUL;

            while ((aliasName[0] == NUL) &&
                (aliasFile.getln(aliasName,sizeof(aliasName)) != -1))
            {
                char name [ 128 + 1 ];
                strcpy(name,aliasName);
                strTrunc(name,',');
                if (stricmp(name,lfrom) == 0)
                {
                    char * p = strchr(aliasName,',');
                    strcpy(aliasName,p ? p+1 : "");
                }
                else
                    aliasName[0] = NUL;
            }

            aliasFile.close();

            if (aliasName[0] != NUL)
            {
                if (aliasName[0] != '%')   // not a list file
                {
                    maxstrcpy(lfrom,aliasName,sizeof(lfrom));
                    if (uucpForce) // Force the short from to the alias as well
                    {
                        maxstrcpy(from,lfrom,sizeof(from));
                        strupr(from);
                        sepsToSpaces(from);
                    }
                    aliasName[0] = NUL;
                }
            }
        }

        // Build the news path (must occur after initial build of lfrom)
        char npath [ 60 + 1 ];
        sprintf(npath,"%s!%s",PcbData.uucpName,lfrom);

        char tmp [ 120 + 1 ];

        // Finish building lfrom
        if (PcbData.uucpBangDomain)
        {
            // host!user@domain
            buildstr(tmp,PcbData.uucpName,"!",lfrom,"@",
                PcbData.uucpDomainName,NULL);
            strcpy(lfrom,tmp);
        }
        else if (PcbData.uucpSubDomain)
        {
            // user%host@domain
            buildstr(tmp,lfrom,"%",PcbData.uucpName,"@",
                PcbData.uucpDomainName,NULL);
            strcpy(lfrom,tmp);
        }
        else
        {
            // user@domain
            addchar(lfrom,'@');
            strcat(lfrom,PcbData.uucpDomainName);
        }

        // Append the "real" name in parentheses after the FQUDN
        strcat(lfrom," (");
        strcat(lfrom,from);
        addchar(lfrom,')');

        // Assume for now that replyto is identical to lfrom
        char replyto [ 120 + 1 ];
        strcpy(replyto,lfrom);

        // Convert message time to a standard format
        tm msgTime = { 0, message.minute(), message.hour(), message.day(),
            message.month()-1, message.year(), 0, 0, 0 };
        mktime(&msgTime);

        // Build the date information
        char date [ 60 + 1 ];
        strftime(date, sizeof(date), "%a, %d %b %y %H:%M:%S ", &msgTime);
        strcat(date, PcbData.uucpTimeZone);

        // Build the alternate date information
        char altdate [ 60 + 1 ];
        strftime(altdate, sizeof(altdate), "%a %b %d %H:%M:%S %Y", &msgTime);

        // Build the mail "From site!user date" line
        char mfrom [ 120 + 1 ];
        buildstr(mfrom,npath," ",altdate,NULL);

        // Build the message id line
        char msgid [ 60 + 1 ];
        sprintf(msgid,"<%04X%03X.%04X%06lX.uuout@%s>",
            (message.year()*12U*31U)+((message.month()-1U)*31U)+
                message.day()-1,
            message.hour()*60+message.minute(),
            cn,message.idxRec.num,PcbData.uucpDomainName);

        // Build the references line
        char refs [ 256 + 1 ];
        refs[0] = NUL;

        // If there is a refer number
        if (message.msgHdr.refer > 0)
        {
            cPCBMSGS refmsg;
            refmsg.open(cr.MsgFile);
            // If we can read it from the index
            if (refmsg.readIdx(message.msgHdr.refer) > 0)
            {
                // Adjust offset if necessary
                if (refmsg.idxRec.off < 0) refmsg.idxRec.off *= -1;
                // If header read ok
                if (refmsg.readHdr(refmsg.idxRec.off) == 0)
                    // If body read ok
                    if (refmsg.readBody() == 0)
                    {
                        // Get the msg id if defined
                        refmsg.getExtHdr("UMSGID ",refs);
                        refs[60] = NUL;
                        stripright(refs,' ');
                        // If not defined build it per UUOUT specs
                        if (refs[0] == NUL)
                            sprintf(refs,"<%04X%03X.%04X%06lX.uuout@%s>",
                                (refmsg.year()*12U*31U)+
                                    ((refmsg.month()-1U)*31U)+refmsg.day()-1,
                                refmsg.hour()*60+refmsg.minute(),
                                cn,refmsg.idxRec.num,PcbData.uucpDomainName);
                    }
            }
        }

        // Build the subject
        char subject [ 4 + 120 + 1 ];
        memset(subject,' ',sizeof(subject));
        subject[sizeof(subject)-1] = NUL;
        memcpy(subject,message.msgHdr.subj,25);
        message.getExtHdr("SUBJECT",subject+ 0);
        message.getExtHdr("SUBJ2  ",subject+60);
        stripright(subject,' ');

        // Build the distribution
        char distrib [ 60 + 1 ];
        memset(distrib,0,sizeof(distrib));
        message.getExtHdr("ROUTE  ",distrib);
        stripright(distrib,' ');
        if (distrib[0] == NUL) strcpy(distrib,PcbData.uucpDefDist);

        // Get the newsgroup & followupto fields
        char newsgroups [ 1024 + 1 ];
        newsgroups[0] = NUL;
        message.getExtHdrs("UNEWSGR",newsgroups);

        char * tmpPtr = strstr(newsgroups,cr.Name);
        if (tmpPtr != NULL)
        {
            int len = strlen(cr.Name);
            if (!(((tmpPtr      == newsgroups) || (tmpPtr[ -1] == ',')) &&
                  ((tmpPtr[len] == NUL)        || (tmpPtr[len] == ','))))
            {
                addchar(newsgroups,',');
                strcat(newsgroups,cr.Name);
            }
        }
        else
        {
            addchar(newsgroups,',');
            strcat(newsgroups,cr.Name);
        }

        stripboth(newsgroups,',');

        char followupto [ 1024 + 1 ];
        followupto[0] = NUL;
        message.getExtHdrs("UFOLLOW",followupto);

        // Check and handle a Reply-To: sub-field in the to field
        tmpPtr = strstr(to,"(Reply-To: ");
        if (tmpPtr)
        {
            strcpy(to,tmpPtr+11);
            strTrunc(to,')');
        }

        int  msgBdyHdrCnt;
        char msgBdyHdr [ 120 + 1 ];

        msgBdyHdrCnt = 0;
        msgBdyHdr[0] = NUL;

        while (TRUE)
        {
 // Reply-To: scott.dale.robison@lcars.com (Scott Dale Robison)
            maxstrcpy(msgBdyHdr,message.getLine(),sizeof(msgBdyHdr));
                 if (memicmp(msgBdyHdr,"TO:",           3) == 0) stripboth((maxstrcpy(to,        msgBdyHdr+  3,sizeof(to)        ),to),        ' ');
            else if (memicmp(msgBdyHdr,"NEWSGROUPS:",  11) == 0) stripboth((maxstrcpy(newsgroups,msgBdyHdr+ 11,sizeof(newsgroups)),newsgroups),' ');
            else if (memicmp(msgBdyHdr,"FOLLOWUP-TO:", 12) == 0) stripboth((maxstrcpy(followupto,msgBdyHdr+ 12,sizeof(followupto)),followupto),' ');
            else if (memicmp(msgBdyHdr,"SUBJECT:",      8) == 0) stripboth((maxstrcpy(subject,   msgBdyHdr+  8,sizeof(subject)   ),subject),   ' ');
            else if (memicmp(msgBdyHdr,"DISTRIBUTION:",13) == 0) stripboth((maxstrcpy(distrib,   msgBdyHdr+ 13,sizeof(distrib)   ),distrib),   ' ');
            else if (memicmp(msgBdyHdr,"REPLY-TO:",     9) == 0) stripboth((maxstrcpy(replyto,   msgBdyHdr+  9,sizeof(replyto)   ),replyto),   ' ');
            else if (memicmp(msgBdyHdr,"REFERENCES:",  11) == 0) stripboth((maxstrcpy(refs,      msgBdyHdr+ 11,sizeof(refs)      ),refs),      ' ');
            else break;
            ++msgBdyHdrCnt;
        }

        if ((refs[0] != NUL) && (memicmp(subject,"Re: ",4) != 0))
        {
            subject[120] = NUL;
            memmove(subject+4,subject,strlen(subject)+1);
            memcpy(subject,"Re: ",4);
        }

        if ((strchr(to,'@') != NULL) ||
            (strchr(to,'!') != NULL) ||
            (strchr(to,'%') != NULL))
        {
            char tmpSName [ 256 + 1 ];
            char tmpLName [ 256 + 1 ];
            parseName(to, tmpSName, sizeof(tmpSName),
                          tmpLName, sizeof(tmpLName));
            strcpy(to,tmpLName);
        }

        message.resetLine();
        while (msgBdyHdrCnt--) message.getLine();

 //     // Check the message body and grab TO: if defined
 //     char mbTO1 [ 120 + 1 ];
 //     char mbTO2 [   1 + 1 ];

 //     maxstrcpy(mbTO1,message.getLine(),sizeof(mbTO1));
 //     maxstrcpy(mbTO2,message.getLine(),sizeof(mbTO2));
 //     stripright(mbTO1,' ');
 //     stripright(mbTO2,' ');

 //     if ((mbTO2[0] == NUL) && (memicmp(mbTO1,"TO:",4) == 0))
 //         stripboth(strcpy(to,mbTO1+3),' ');
 //     else
 //         message.resetLine();

        struct date today;
        getdate(&today);

        struct time now;
        gettime(&now);

     // if ((message.msgHdr.status == MSG_RCVR) &&
     //     (strlen(to) <= 25) &&
        if ((strchr(to,'@') == NULL) &&
            (strchr(to,'!') == NULL) &&
            (strchr(to,'%') == NULL) &&
            ((message.msgHdr.status == MSG_RCVR) ||
             (message.msgHdr.status == MSG_RCVR_READ)))
        {
            // it's local email, convert to name to PCB standards and forward
            char name [ 120 + 1 ];
            strcpy(name,to);

            if ((strlen(name) <= 25) && (finduser(name) >= 0)) continue;

            strupr(strcpy(name,to));
            sepsToSpaces(name);

            if (strcmp(name,"ALL") == 0)
            {
                // warning

                char td [ 8 + 1 ];
                sprintf(td,"%02d/%02d/%02d",today.da_mon,today.da_day,
                    today.da_year%100);

                char tt [ 5 + 1 ];
                sprintf(tt,"%02d:%02d",now.ti_hour,now.ti_min);

                conout3 << endl << "      Bounced: #" <<
                    message.idxRec.num << " To " << maxstr(name,47);

                postMessage(cn,message.msgBody,message.msgSize,
                    MSG_RCVR,td,tt,message.msgHdr.fname,
                    "UUOUT EXPORT DAEMON","Unable to Export (ALL)","","",
                    "Unable to Export Email to ALL","","","","",NULL);

                logFile.printf("  Bounced: #%ld To %s\r\n",
                    message.idxRec.num,name);
            }
            else if ((strlen(name) > 25) || (finduser(name) == -1))
            {
                char td [ 8 + 1 ];
                sprintf(td,"%02d/%02d/%02d",today.da_mon,today.da_day,
                    today.da_year%100);

                char tt [ 5 + 1 ];
                sprintf(tt,"%02d:%02d",now.ti_hour,now.ti_min);

                conout3 << endl << "      Bounced: #" <<
                    message.idxRec.num << " To " << maxstr(name,47);

                postMessage(cn,message.msgBody,message.msgSize,
                    MSG_RCVR,td,tt,message.msgHdr.fname,
                    "UUOUT EXPORT DAEMON","Invalid Local User","",to,
                    "Invalid Local User Name Specified","","","","",NULL);

                logFile.printf("  Bounced: #%ld To %s\r\n",
                    message.idxRec.num,name);
            }
            // if they don't match, then forward
            else if (strcmp(name,to) != 0) // They are different
            {
                // forward message to name without echo

                char lto    [ 120 + 1 ];
                char lfrom  [ 120 + 1 ];
                char lsubj  [  60 + 1 ];
                char origin [  60 + 1 ];

                memset(lto,   0,sizeof(lto));
                memset(lfrom, 0,sizeof(lfrom));
                memset(lsubj, 0,sizeof(lsubj));
                memset(origin,0,sizeof(origin));

                message.getExtHdr("TO     ",lto   + 0);
                message.getExtHdr("TO2    ",lto   +60);
                message.getExtHdr("FROM   ",lfrom + 0);
                message.getExtHdr("FROM2  ",lfrom +60);
                message.getExtHdr("SUBJECT",lsubj);
                message.getExtHdr("ORIGIN ",origin);

                conout3 << endl << "      Forward: #" << message.idxRec.num <<
                    " From " << maxstr(to,20) << " To " << maxstr(name,20);

                postMessage(cn,message.msgBody,message.msgSize,
                    message.msgHdr.status,message.msgHdr.dmsg,
                    message.msgHdr.tmsg,name,message.msgHdr.fname,
                    message.msgHdr.subj,lto,lfrom,lsubj,origin,
                    "","","",NULL);

                logFile.printf("  Forward: #%ld From %s To %s\r\n",
                    message.idxRec.num,to,name);
            }
            else // They are the same
            {
                continue;
            }

            deinitMsgBaseList();
        }
        else if ((message.msgHdr.status == MSG_RCVR) ||
                 (message.msgHdr.status == MSG_RCVR_READ))
        {
            if (aliasName[0] == '%') {
                if (aliasFile.open(aliasName + 1,OPEN_READ|OPEN_DENYNONE) == 0) {
                    to[0] = NUL;
                    while ((to[0]) == NULL && (aliasFile.getln(to,sizeof(to)) != -1))
                    {
                        conout3 << endl << "      Export: #" << message.idxRec.num <<
                            " (Email) From " << maxstr(from,15) << " To " << maxstr(to,20);

                        // it's remote email, so create the CMD/XQT/DAT files for email
                        exportMessage(msgTypeEmail,mp,mfrom,to,lfrom,subject,newsgroups,
                            followupto,msgid,date,distrib,PcbData.Organization,replyto,
                            &message,NULL,cr.AttachLoc,refs,NULL,0,NULL,NULL);

                        logFile.printf("  Export: #%ld (Email) From %s To %s\r\n",
                            message.idxRec.num,from,to);
                        to[0] = NUL;
                    }
                } else {
                  // unable to open the alias file.  Don't send the message out.
                  continue;
                }
            } else {
                conout3 << endl << "      Export: #" << message.idxRec.num <<
                    " (Email) From " << maxstr(from,15) << " To " << maxstr(to,20);

                // it's remote email, so create the CMD/XQT/DAT files for email
                exportMessage(msgTypeEmail,mp,mfrom,to,lfrom,subject,newsgroups,
                    followupto,msgid,date,distrib,PcbData.Organization,replyto,
                    &message,NULL,cr.AttachLoc,refs,NULL,0,NULL,NULL);

                logFile.printf("  Export: #%ld (Email) From %s To %s\r\n",
                    message.idxRec.num,from,to);
            }
        }
        else if (mn[0] != NUL)
        {
            conout3 << endl << "      Export: #" << message.idxRec.num <<
                " (MNews) From " << maxstr(from,15) << " To " << maxstr(mn,20);

            // It's moderated news, create CMD/XQT/DAT files for email
            exportMessage(msgTypeEmail,mp,mfrom,mn,lfrom,subject,newsgroups,
                followupto,msgid,date,distrib,PcbData.Organization,replyto,
                &message,NULL,cr.AttachLoc,refs,NULL,0,NULL,NULL);

            logFile.printf("  Export: #%ld (MNews) From %s To %s\r\n",
                message.idxRec.num,from,mn);
        }
        else
        {
            conout3 << endl << "      Export: #" << message.idxRec.num <<
                " (PNews) From " << from;

            // it's public news, create CMD/XQT/DAT files for news
            exportMessage(msgTypeNews,np,npath,"",lfrom,subject,newsgroups,
                followupto,msgid,date,distrib,PcbData.Organization,replyto,
                &message,NULL,cr.AttachLoc,refs,NULL,0,NULL,NULL);

            logFile.printf("  Export: #%ld (PNews) From %s\r\n",
                message.idxRec.num,from);
        }

        switch (message.msgHdr.status)
        {
            case MSG_RCVR: message.msgHdr.status = MSG_RCVR_READ; break;
            case MSG_CMNT: message.msgHdr.status = MSG_CMNT_READ; break;
            case MSG_SPWD: message.msgHdr.status = MSG_SPWD_READ; break;
            case MSG_PBLC: message.msgHdr.status = MSG_PBLC_READ; break;
        }

        message.msgHdr.dread = long(today.da_year % 100) * 10000L +
                               long(today.da_mon)        *   100L +
                               long(today.da_day);

        char tread [ 5 + 1 ];
        sprintf(tread,"%02d:%02d",now.ti_hour,now.ti_min);
        memcpy(message.msgHdr.tread,tread,5);

        message.writeHdr(message.idxRec.off);
    }

    message.close();
}

static void near pascal doConfs(void)
{
    char mailPath [ 128 + 1 ];
    buildstr(mailPath,PcbData.uucpSpoolPath,PcbData.uucpEmailHost,"\\",NULL);

    char newsPath [ 128 + 1 ];
    buildstr(newsPath,PcbData.uucpSpoolPath,PcbData.uucpNewsHost,"\\",NULL);

    char tmp [ 128 + 1 ];

    if (validatedir(mailPath,tmp) != 0)
    {
        conerr << endl << "Unable to validate/create " << mailPath;
        logFile.printf("*** Unable to validate/create %s\r\n",mailPath);
        return;
    }

    if (validatedir(newsPath,tmp) != 0)
    {
        conerr << endl << "Unable to validate/create " << newsPath;
        logFile.printf("*** Unable to validate/create %s\r\n",newsPath);
        return;
    }

    pcbconftype   conf;
    unsigned      i, l, h;
    char        * p = uucpConfs;

    while ((p != NULL) && !abortFlag)
    {
        char * t = strchr(p,',');
        if (t != NULL) *(t++) = NUL;

        l = 0;
        h = PcbData.NumConf;

        char * hyphen = strchr(p,'-');
        if (hyphen != NULL)
        {
            *(hyphen++) = NUL;
            if (*p      != NUL) l = unsigned(atol(p));
            if (*hyphen != NUL) h = unsigned(atol(hyphen));
        }
        else
        {
            l = h = unsigned(atol(p));
        }

        p = t;

        for (i = l; (i <= h) && !abortFlag; ++i)
        {
            if (kbhit())
                switch (getch())
                {
                    case 27: abortFlag = TRUE; break;
                    case  0: getch();          break;
                }

            if (abortFlag) continue;

            getconfrecord(i,&conf);

            if (conf.Name[0] == NUL) continue;

            defInitStrBuf(modName,120);

            switch (conf.ConfType)
            {
                case IN_EMAIL:
                    conout0 << endl << "Conference Type -- Internet Email";
                    conout1 << endl << "  Number " << i << " -- " <<
                        conf.Name;
                    logFile.printf("Scanning Conference %u -- %s\r\n",
                        i,conf.Name);
                    doConf(newsPath,mailPath,i,conf,modName);
                    break;

                case UN_MOD_NEWS:
                {
                    // look up moderator here for email purposes
                    cDOSFILE modFile;
                    if (exist(PcbData.uucpModFile) &&
                        (modFile.open(PcbData.uucpModFile,
                            OPEN_READ|OPEN_DENYNONE) == 0))
                    {
                        while ((modName[0] == NUL) &&
                            (modFile.getln(modName,sizeof(modName)) == 0))
                        {
                            unsigned conf = unsigned(atol(modName));
                            if (conf == i)
                            {
                                char * p = strchr(modName,',');
                                strcpy(modName,p ? p+1 : "");
                            }
                            else
                                modName[0] = NUL;
                        }
                        modFile.close();
                    }

                    if (modName[0] == NUL)
                    {
                        strcpy(modName,conf.Name);
                        substitute(modName,".","-",sizeof(modName));
                        strcat(modName,"@uunet.uu.net");
                    }

                    // Fall through to public news . . .
                }

                case UN_PUB_NEWS:
                    conout0 << endl << "Conference Type -- Usenet Newsgroup";
                    conout1 << endl << "  Number " << i << " -- " <<
                        conf.Name;
                    logFile.printf("Scanning Conference %u -- %s\r\n",
                        i,conf.Name);
                    doConf(newsPath,mailPath,i,conf,modName);
                    break;

                case IN_JUNK:
                default:
                    break;
            }

            putconfrecord(i,&conf);
        }
    }
}

int main(void)
{
    #ifdef MEMCHECK
      mc_startcheck(erf_standard);
    #endif

    initialize();

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
        scrbuf[i*2+0] = ' ';
        scrbuf[i*2+1] = outAttr;
    }

    for (i = 0; i < ti.screenheight; ++i)
    {
        scrbuf[(i*ti.screenwidth+(             1-1))*2] =
        scrbuf[(i*ti.screenwidth+(ti.screenwidth-1))*2] = '³';
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
        scrbuf[((outTop-1-1)*ti.screenwidth+i)*2] =
        scrbuf[((outTop+1-1)*ti.screenwidth+i)*2] =
        scrbuf[((outTop+3-1)*ti.screenwidth+i)*2] =
        scrbuf[((outTop+5-1)*ti.screenwidth+i)*2] =
        scrbuf[((outBot+1-1)*ti.screenwidth+i)*2] =
        scrbuf[((errBot+1-1)*ti.screenwidth+i)*2] = ch;
    }

    scrbuf[((              1-1)*ti.screenwidth+(             1-1))*2] = 'Ú';
    scrbuf[((              1-1)*ti.screenwidth+(ti.screenwidth-1))*2] = '¿';
    scrbuf[((ti.screenheight-1)*ti.screenwidth+(             1-1))*2] = 'À';
    scrbuf[((ti.screenheight-1)*ti.screenwidth+(ti.screenwidth-1))*2] = 'Ù';

    puttext(1,1,ti.screenwidth,ti.screenheight,scrbuf);

    conscr << setxy(14,1)               <<
        " UUOUT Version 1.30 - UUCP Export Utility for PCBoard ";
    conscr << setxy(11,ti.screenheight) <<
        " Copyright (C) 1994-1996 -- Clark Development Company, Inc. ";

 // conout.window(1,outTop,ti.screenwidth,outBot);
 // conout << setattr(outAttr); conout.clrscr();

    conout0.window(1+crRatio,outTop+0,ti.screenwidth-crRatio,outTop+0);
    conout0 << setattr(outAttr); conout0.clrscr();

    conout1.window(1+crRatio,outTop+2,ti.screenwidth-crRatio,outTop+2);
    conout1 << setattr(outAttr); conout1.clrscr();

    conout2.window(1+crRatio,outTop+4,ti.screenwidth-crRatio,outTop+4);
    conout2 << setattr(outAttr); conout2.clrscr();

    conout3.window(1+crRatio,outTop+6,ti.screenwidth-crRatio,outBot);
    conout3 << setattr(outAttr); conout3.clrscr();

    conerr.window(1+crRatio,errTop,ti.screenwidth-crRatio,errBot);
    conerr << setattr(errAttr); conerr.clrscr();

    initMsgBaseList();

    getSeqNum();

    char logName [ 128 + 1 ];
    buildstr(logName,PcbData.uucpLogPath,"UUOUT.LOG",NULL);
    logFile.open(logName,OPEN_WRIT|OPEN_DENYWRIT|OPEN_APPEND);
    logFile.putln("*******************************************************************************");
    strcpy(logName,"START = DDDDDDDD TTTTTTTT");
    datestr (logName+ 8);
    timestr1(logName+17);
    logName[16] = ' ';
    logFile.putln(logName);

    doConfs();

    strcpy(logName,"END   = DDDDDDDD TTTTTTTT");
    datestr (logName+ 8);
    timestr1(logName+17);
    logName[16] = ' ';
    logFile.putln(logName);

    logFile.close();

    putSeqNum();

    deinitMsgBaseList();
    deinitialize();

    conscr << setattr(0x07);
    conscr.clrscr();

    #ifdef MEMCHECK
      mc_endcheck();
    #endif

    return 0;
}

