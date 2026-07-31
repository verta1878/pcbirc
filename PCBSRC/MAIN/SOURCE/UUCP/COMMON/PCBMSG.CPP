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



#include    <ctype.h>
#include    <time.h>

#include    <iomanip.h>
#include    <constream.h>

#include    <cnameidx.h>
#include    <uudecode.hpp>

#include    "decode.h"
#include    "uuin.hpp"

#include    <uucp.hpp>

// extern constream conscr;
// extern constream conout;
// extern constream conout0;
// extern constream conout1;
// extern constream conout2;
   extern constream conout3;
// extern constream conerr;

   extern cDOSFILE  logFile;
   extern cDOSFILE  junkGroups;

   extern int       lastArtNum;
   extern long      lastSize;

   extern char    * messageTypeLabel;

// extern int  pascal decode_base64( char *LineIn );
// extern void pascal writeDecoded( int Length );


long starting = 1L;


static inline int isSpace(char c)
{
    return isascii(c) && isspace(c);
}

static char * near pascal stripspaces(char * s)
{
    if (s)
    {
        char * p = s;
        while (isSpace(*p)) ++p;
        strcpy(s,p);
    }

    return s;
}

static char lastGroup [ 60 + 1 ];

static long near pascal findNewsGroup(char * groupList)
{
    char * group = strtok(groupList,",");

    lastGroup[0] = NUL;

    if ((group == NULL) || (group[0] == NUL)) return PcbData.uucpJunkConf;

    maxstrcpy(lastGroup,group,sizeof(lastGroup));

    return getconfbyname(group);
}

static long near pascal findUserConf(char * lname, char * sname,
    char & stat, bool & all)
{
    // Strip domain, sub-domain, and site info from lname
    char tmp [ 120 + 1 ], * ptr;
    maxstrcpy(tmp,lname,sizeof(tmp));
    ptr = strrchr(tmp,'@');
    if (ptr) *ptr = 0;
    ptr = strrchr(tmp,'%');
    if (ptr) *ptr = 0;
    ptr = strrchr(tmp,'!');
    if (ptr) strcpy(tmp,ptr+1);

    // Try to find a conference by the specified name
    long conf = getconfbyname(tmp);
    if (conf != -1L)
    {
        pcbconftype confinfo;
        getconfrecord(unsigned(conf),&confinfo);
        switch (confinfo.ConfType)
        {
            case IN_EMAIL:
                stat = MSG_RCVR;
             // lname[0] = NUL;
             // strcpy(sname,"ALL");
                break;

            case IN_JUNK:
            case UN_MOD_NEWS:
            case UN_PUB_NEWS:
                stat = MSG_PBLC;
                all = 1;
                lname[0] = NUL;
                strcpy(sname,"ALL");
                break;

            default:
                conf = -1L;
                break;
        }
    }

    // If not found, try to find a conference named "site.email.l"
    // where site is your site name (ala saltair or lcars) and
    // l is the first letter of the user name in tmp
    if (conf == -1L)
    {
        char ch = toupper(sname[0]);
        if (ch < 'A') ch = 'A';
        if (ch > 'Z') ch = 'Z';
        sprintf(tmp,"%s.email.%c",PcbData.uucpName,ch);
        conf = getconfbyname(tmp);
    }

    // If not found, try to find a conference named "site.email"
    if (conf == -1L)
    {
        sprintf(tmp,"%s.email",PcbData.uucpName);
        conf = getconfbyname(tmp);
    }

    // Finally, just grab the defined email conference from PCBOARD.DAT
    if (conf == -1L) conf = PcbData.uucpEmailConf;

    return conf;
}

#define ifFoundMonSet(s,n) if (strstr(tmpMon,s) != NULL) mo = n

static void near pascal parseDate(int & yr, int & mo, int & da,
    int & hr, int & mi, int & se, char * s)
{
    if (s == NULL) s = "";

    yr = mo = da = hr = mi = se = 0;
    strupr(s);

    char tmpMon [ 16 + 1 ];
    char tmpZon [ 16 + 1 ];
    char tmpDow [ 16 + 1 ];

    if (s[3] == ',') strcpy(s,s+5);

    // DOW MON DD, YYYY HH:MM:SS ZZZZZ
    if (s[3] == ' ')
        sscanf(s,"%16s %16s %d, %d %d:%d:%d%16s",tmpDow,tmpMon,&da,&yr,&hr,&mi,&se,tmpZon);
    // DD MON YYYY HH:MM:SS ZZZZZ
    else if (strchr(s,':') != NULL)
        sscanf(s,"%d %16s %d %d:%d:%d%16s",&da,tmpMon,&yr,&hr,&mi,&se,tmpZon);
    // DD MON YYYY HHMM ZZZZZ
    else
        sscanf(s,"%d %16s %d %d%16s",&da,tmpMon,&yr,&hr,tmpZon);

         ifFoundMonSet("JAN", 1);
    else ifFoundMonSet("FEB", 2);
    else ifFoundMonSet("MAR", 3);
    else ifFoundMonSet("APR", 4);
    else ifFoundMonSet("MAY", 5);
    else ifFoundMonSet("JUN", 6);
    else ifFoundMonSet("JUL", 7);
    else ifFoundMonSet("AUG", 8);
    else ifFoundMonSet("SEP", 9);
    else ifFoundMonSet("OCT",10);
    else ifFoundMonSet("NOV",11);
    else ifFoundMonSet("DEC",12);

    if (yr == 0) yr = 1980;
    if (mo == 0) mo =    1;
    if (da == 0) da =    1;

    if (hr >= 100)
    {
        mi = hr%100;
        hr /= 100;
    }

    mi += hr*60; // - cvtTZtoMIN(tmpZon) + cvtTZtoMIN(PcbData.uucpTimeZone);

    while (mi < 0)
    {
        mi += 24*60;
        --da;
    }

    while (mi >= (24*60))
    {
        mi -= 24*60;
        ++da;
    }

    hr = mi/60;
    mi %= 60;

    // need to adjust da/mo/yr here (assuming no *wild* times > 1 day diff)
    if (da < 1)
    {
        --yr;
        --mo;
        da += 31;
    }
}

/******************************************************************************/

cNETHDR::cNETHDR ( void )
{
    lastHdrType  = isMime = -1;

    // pointers which will be allocated as needed when reading in the
    // headers to a message.
    apparentlyto = date         = distribution = followupto   =
    from         = msgidmail    = msgidnews    = newsgroups   =
    organization = replyto      = subject      = to           =
    user         = file         = NULL;

    // clear each of the strings.  These are already pre-allocated buffers.
    memset(MimeVersion,0,sizeof(MimeVersion));
    memset(ContentType,0,sizeof(ContentType));
    memset(EncodeType,0,sizeof(EncodeType));
    memset(Disposition,0,sizeof(Disposition));
    memset(Boundary,0,sizeof(Boundary));
    memset(Base64FileName,0,sizeof(Base64FileName));
}

cNETHDR::~cNETHDR ( void )
{
    kill();

    // check to see if 'file' is pointing to 'Base64FileName'.  If so, we
    // are going to just set it to NULL otherwise we will need to free the
    // memory associated with 'file'.
    if (file == Base64FileName)
      file = NULL;
    else
      sstrfre(file);
}

void pascal cNETHDR::kill ( void )
{
    sstrfre(apparentlyto); sstrfre(date);
    sstrfre(distribution); sstrfre(followupto);
    sstrfre(from);         sstrfre(msgidmail);
    sstrfre(msgidnews);    sstrfre(newsgroups);
    sstrfre(organization); sstrfre(replyto);
    sstrfre(subject);      sstrfre(to);
    sstrfre(user);
}

void pascal cNETHDR::combine(char * & _s1, char * _s2)
{
    char * s = (char *) malloc(sstrlen(_s1)+sstrlen(_s2)+1);

    if (s == NULL) return;

    strcpy(s,_s1 ? _s1 : "");
    stripspaces(strcat(s+strlen(s),_s2 ? _s2 : ""));

    sstrfre(_s1);

    _s1 = s;
}

#define ifChkHdrAppend(n,p,t)             \
    if (memicmp(_hdr,n,sizeof(n)-1) == 0) \
    {                                     \
        combine(p,_hdr+sizeof(n)-1);      \
        lastHdrType = t;                  \
    }

#define caseHdrAppend(t,p) case t: combine(p,_hdr); break

void pascal cNETHDR::getInfo(char * _hdr)
{
         ifChkHdrAppend("Apparently-To:",apparentlyto, 0)
    else ifChkHdrAppend("Date:",         date,         1)
    else ifChkHdrAppend("Distribution:", distribution, 2)
    else ifChkHdrAppend("Followup-To:",  followupto,   3)
    else ifChkHdrAppend("From:",         from,         4)
    else ifChkHdrAppend("Message-Id:",   msgidmail,    5)
    else ifChkHdrAppend("Message-ID:",   msgidnews,    6)
    else ifChkHdrAppend("Newsgroups:",   newsgroups,   7)
    else ifChkHdrAppend("Organization:", organization, 8)
    else ifChkHdrAppend("Reply-To:",     replyto,      9)
    else ifChkHdrAppend("Subject:",      subject,     10)
    else ifChkHdrAppend("To:",           to,          11)
    else if (memicmp(_hdr,"MIME-Ver",8) == 0) {
      strcpy(MimeVersion,_hdr+14);
      lastHdrType = 12;
      isMime++;
    } else if (memicmp(_hdr,"Content-Type",12) == 0) {
      strcpy(ContentType,_hdr+14);
      lastHdrType = 13;
    } else if (memicmp(_hdr,"Content-Tran",12) == 0) {
      strcpy(EncodeType,_hdr+27);
      lastHdrType = 14;
    } else if (memicmp(_hdr,"Content-Disp",12) == 0) {
      strcpy(Disposition,_hdr+21);
      lastHdrType = 15;
    }
    else if (isSpace(_hdr[0]))
    {
        switch (lastHdrType)
        {
            caseHdrAppend( 0,apparentlyto);
            caseHdrAppend( 1,date);
            caseHdrAppend( 2,distribution);
            caseHdrAppend( 3,followupto);
            caseHdrAppend( 4,from);
            caseHdrAppend( 5,msgidmail);
            caseHdrAppend( 6,msgidnews);
            caseHdrAppend( 7,newsgroups);
            caseHdrAppend( 8,organization);
            caseHdrAppend( 9,replyto);
            caseHdrAppend(10,subject);
            caseHdrAppend(11,to);
            case 12:  strcat(MimeVersion,_hdr); break;
            case 13:  strcat(ContentType,_hdr); break;
            case 14:  strcat(EncodeType,_hdr);  break;
            case 15:  strcat(Disposition,_hdr); break;
        }
    }
    else
    {
        lastHdrType = -1;
    }
}

void pascal cNETHDR::setToUser(char * _user)
{
    sstrfre(user);
    user = sstrdup(_user);
    if (user)
    {
        strTrunc(user,'@');
        strTrunc(user,'%');
        char * p = strchr(user,'!');
        if (p) strcpy(user,p+1);
    }
}

void pascal cNETHDR::setFileAttach(char * _file)
{
    sstrfre(file);
    file = sstrdup(_file);
}

/******************************************************************************/

char far cPCBMSG::buf [ msgBufSize ];

cPCBMSG::cPCBMSG(char _all, char _stat, cNETHDR * _hdr) :
    size(0),
    part(0),
    msgAll(_all),
    msgStat(_stat),
    nhdr(_hdr),
    decodeMode(FALSE)
{
    memset(buf,' ',sizeof(buf));
    decodeName[0] = NUL;
}

cPCBMSG::~cPCBMSG(void)
{
    if (decodeMode)
    {
        decodeFileStop();
        decodeMode = FALSE;
    }
    if (decodeName[0] != NUL) unlink(decodeName);
}

int pascal cPCBMSG::flushMsg(void)
{
    if (part > 0) ++part;
    return flushMsgSub();
}

int pascal cPCBMSG::flushMsgPart(void)
{
    ++part;
    return flushMsgSub();
}

void pascal cPCBMSG::addLine(char * line, int len)
{
    if (uucpDecode && (decodeName[0] == NUL) && (memcmp(line,"begin ",6) == 0))
    {
        char name [ 128 + 1 ];
        maxstrcpy(name,line+6,sizeof(name));

        stripleft(name,' ');

        if (isdigit(name[0]))
        {
            char * p = name;
            while (isdigit(*(p++))) ;

            stripleft(name,' ');

            strcpy(name,p);

            if (validfile(name) && (decodeFileStart(PcbData.TmpLoc,name) == 0))
            {
                decodeMode = TRUE;
                nhdr->setFileAttach(name);
                buildstr(decodeName,PcbData.TmpLoc,name,NULL);
                return;
            }
        }
    }

    if (decodeMode)
    {
        if (strcmp(line,"end") == 0)
        {
            decodeMode = FALSE;
            decodeFileStop();
            return;
        }
        else
        {
            decodeFileLine(line);
            return;
        }
    }

 // int len = strlen(line);

    if ((size+len+1+(len/78)+128) > sizeof(buf)) flushMsgPart();

    if (line[0] == '\x01') // If the line is hidden there's no need to wrap it
    {
        memcpy(buf+size,line,len);
        size += len;
        buf[size++] = LineSeparator;
        return;
    }

    if (len == 0) buf[size++] = LineSeparator;

    int maxLineLen = 79;

    while (len > 0)
    {
     // int tlen = len;
     // if (tlen > maxLineLen) tlen = maxLineLen;

        char maxLineBuf [ 80 + 1 ];
        dstrcpy(maxLineBuf,line,maxLineLen);
        int maxLineSize = strlen(maxLineBuf);

        memcpy(buf+size,maxLineBuf,maxLineSize);
        size += maxLineSize;
        buf[size++] = LineSeparator;

        line += maxLineSize;
        len -= maxLineSize;

     // if (len > 0)
     // {
     //     maxLineLen = 78;
     //     ++size;
     // }
    }
}

int pascal cPCBMSG::flushMsgSub(void)
{
    int rv = 0;

 // if (abortFlag) return;

    int yr, mo, da, hr, mi, se;
    if (uucpCurDate)
    {
        time_t ttime;
        time(&ttime);
        tm ltime = *localtime(&ttime);
        yr = ltime.tm_year;
        mo = ltime.tm_mon+1;
        da = ltime.tm_mday;
        hr = ltime.tm_hour;
        mi = ltime.tm_min;
        se = ltime.tm_sec;
    }
    else
        parseDate(yr,mo,da,hr,mi,se,nhdr->date);

    char msgDate [ 8 + 1 ]; sprintf(msgDate,"%02d/%02d/%02d",mo%100,da%100,yr%100);
    char msgTime [ 5 + 1 ]; sprintf(msgTime,"%02d:%02d",hr%100,mi%100);

    if (nhdr->to == NULL)
    {
        nhdr->to = nhdr->apparentlyto;
        nhdr->apparentlyto = NULL;
    }

    char msgSTo   [ 25 + 1 ]; char msgLTo   [ 120 + 1 ]; parseName(nhdr->to,     msgSTo,  sizeof(msgSTo),  msgLTo,  sizeof(msgLTo),TRUE);
    char msgSFrom [ 25 + 1 ]; char msgLFrom [ 120 + 1 ]; parseName(nhdr->from,   msgSFrom,sizeof(msgSFrom),msgLFrom,sizeof(msgLFrom));
    char msgSRTo  [ 25 + 1 ]; char msgLRTo  [ 120 + 1 ]; parseName(nhdr->replyto,msgSRTo, sizeof(msgSRTo), msgLRTo, sizeof(msgLRTo));

    if (nhdr->user != NULL)
    {
     // char tmpLTo [ 120 + 1 ];
       #define tmpLTo msgLTo
        parseName(nhdr->user,msgSTo,sizeof(msgSTo),tmpLTo,sizeof(tmpLTo),TRUE);
       #undef  tmpLTo
    }

    char msgSSubj [  25 + 1 ]; sprintf(msgSSubj,"%-25.25s",  nhdr->subject ? nhdr->subject : "<NONE>");
    char msgLSubj [ 120 + 1 ]; sprintf(msgLSubj,"%-120.120s",nhdr->subject ? nhdr->subject : "<NONE>");

    char msgOrigin [ 60 + 1 ];
    if (nhdr->distribution)
        maxstrcpy(msgOrigin,nhdr->distribution,sizeof(msgOrigin));
    else
        msgOrigin[0] = NUL;

    if (msgAll)
    {
        strcpy(msgSTo,"ALL");
        msgLTo[0] = NUL;
    }

    if (msgLFrom[0] == NUL) strcpy(msgLFrom,msgLRTo);

    if ((msgLRTo[0] != NUL) && (strcmp(msgLFrom,msgLRTo) != 0))
    {
        int len = strlen(msgLFrom); maxstrcpy(msgLFrom+len," (Reply-To: ",sizeof(msgLFrom)-len);
            len = strlen(msgLFrom); maxstrcpy(msgLFrom+len,msgLRTo,       sizeof(msgLFrom)-len);
            len = strlen(msgLFrom); maxstrcpy(msgLFrom+len,")",           sizeof(msgLFrom)-len);
    }

    // If the last 120-25 characters are spaces, then the subjects are identical
    if (strspn(msgLSubj+25," ") == 120-25) msgLSubj[0] = NUL;
 // if (strcmp(msgLSubj+25,"                                   ") == 0)
 //     msgLSubj[0] = NUL;

    // If the last 120-60 characters are spaces, then the subjects can fit in one extended header
    if (strspn(msgLSubj+60," ") == 120-60) msgLSubj[60] = NUL;

    if (part > 0)
    {
                                sprintf(msgSSubj+              25-3,"%3d",part);
        if (msgLSubj[0] != NUL) sprintf(msgLSubj+strlen(msgLSubj)-3,"%3d",part);
    }

    char msgID [ 60 + 1 ];
    if (nhdr->msgidmail)
        maxstrcpy(msgID,nhdr->msgidmail,sizeof(msgID));
    else if (nhdr->msgidnews)
        maxstrcpy(msgID,nhdr->msgidnews,sizeof(msgID));
    else
        msgID[0] = NUL;

    char * msgNewsgroups = sstripall(
        sstrdup(msgAll ? nhdr->newsgroups : NULL),' ');

    // if nhdr->distribution is not acceptable, skip loop

    if (memicmp(nhdr->subject,"cmsg cancel ",12) == 0)
    {
        logFile.printf("cmsg cancel message ignored\r\n");
    }
    else
    {
        // loop through newsgroups
        bool posted = FALSE;
        bool tmpMsgAll = msgAll; // I need this in case of a multi-part email message routed to a specific conference
        for (long confNum = (tmpMsgAll ? findNewsGroup(msgNewsgroups) :
            findUserConf(/*nhdr->user?nhdr->user:*/msgLTo,msgSTo,
                msgStat,tmpMsgAll)); ;
            confNum = findNewsGroup(NULL))
        {
            // If the conference wasn't found, try next
            if (confNum == -1)
            {
                if (uucpJunkOut) junkGroups.printf("%s\r\n",lastGroup);
                continue;
            }

            // If already posted not junk ...
            if (posted && (confNum == PcbData.uucpJunkConf)) break;

            // If we've disabled JUNK posting and this is destined for junk ...
            if (uucpNoJunk && (confNum == PcbData.uucpJunkConf)) break;

            // Now we know it was posted ...
            posted = TRUE;

            // ... Unless it is going to the junk conference
            if (confNum == PcbData.uucpJunkConf) posted = FALSE;

            char messageName [ 128 + 1 ];
            buildstr(messageName,PcbData.uucpPath,"BOUNCE.TXT",NULL);

            // If the message is to all or the user is on the system or
            //   it is a special user ...
            if (tmpMsgAll || uucpForce || (finduser(msgSTo) >= 0) ||
                (stricmp(msgSTo,"postmaster") == 0) ||
                (stricmp(msgSTo,"root")       == 0) ||
                (stricmp(msgSTo,"uucp")       == 0) ||
                (stricmp(msgSTo,"sysop")      == 0) ||
                (stricmp(msgSTo,"usenet")     == 0)
                )
            {
                if (decodeMode)
                {
                    decodeFileStop();
                    decodeMode = FALSE;
                }

                // Post the message (whew!)
                if (postMessage(unsigned(confNum),buf,size,msgStat,msgDate,msgTime,
                    msgSTo,msgSFrom,msgSSubj,uucpLongTo ? msgLTo : "",
                    msgLFrom,msgLSubj,msgOrigin,
                    msgID,nhdr->followupto,nhdr->newsgroups,nhdr->file) == 0)
                {
                    if ((uucpDebug == 0) || (uucpDebug >= 20))
                        logFile.printf("      %s %4d Size %6ld ",
                            messageTypeLabel,lastArtNum,lastSize);

                    conout3 << endl << "      " << messageTypeLabel << ' ' <<
                        setw(4) << lastArtNum << " Size " <<
                        setw(6) << lastSize << ' ';

                    if (part > 0)
                    {
                        conout3 << "Part " << setw(5) << int(part) << ' ';

                        if ((uucpDebug == 0) || (uucpDebug >= 20))
                            logFile.printf("Part %5d ",int(part));
                    }
                    else
                    {
                        conout3 << "           ";

                        if ((uucpDebug == 0) || (uucpDebug >= 20))
                            logFile.printf("           ");
                    }

                    conout3 << "Conf " << setw(5) << unsigned(lastSavedConfNum) <<
                               " Msg " << setw(8) << lastSavedMsgNum << ' ';
                    if (!tmpMsgAll)
                        conout3 << endl << "        To = " <<
                            msgSTo;
                    if (nhdr->file != NULL)
                        conout3 << endl << "        Attachment = " <<
                            nhdr->file;

                    if ((uucpDebug == 0) || (uucpDebug >= 20))
                    {
                        logFile.printf("Conf %5u Msg %8ld\r\n",
                            unsigned(lastSavedConfNum),lastSavedMsgNum);
                        if (!tmpMsgAll)
                            logFile.printf("        To = %s\r\n",
                                msgSTo);
                        if (nhdr->file != NULL)
                            logFile.printf("        Attachment = %s\r\n",
                                nhdr->file);
                    }
                }
                else
                {
                    conout3 << endl << "          Error Saving Msg to Conf " <<
                        setw(5) << unsigned(confNum);

                    logFile.printf("          Error Saving Msg to Conf %5u\r\n",
                        unsigned(confNum));

                    rv = -1;
                }
            }
            else if (fileexist(messageName) != 255)
            {
                char mfrom [ 120 + 1 ];
                strcpy(mfrom,makemfrom("postmaster"));

                char date [ 60 + 1 ];
                strcpy(date,makemdate());

                // Setup replyto
                char replyto [ 120 + 1 ];
                strcpy(replyto,makeaddress("postmaster"));

                // Build the message id line
                char msgid [ 60 + 1 ];
                strcpy(msgid,makemsgid("bounce"));

                // Send out a message from postmaster for bounced mail
                cDOSFILE message;
                if (message.open(messageName,OPEN_READ|OPEN_DENYNONE) == 0)
                    exportMessage(msgTypeEmail,mailPath,mfrom,msgLFrom,replyto,
                        "Bounced Mail -> User Not Found",NULL,NULL,msgid,date,
                        NULL,PcbData.Organization,replyto,NULL,&message,NULL,"",
                        buf,size,NULL,NULL);

                logFile.printf("          Bounced Email -- %s (%s)\r\n",
                    msgLTo,msgSTo);
            }
            else
            {
                logFile.printf("          User Not Found -- %s (%s)\r\n",
                    msgLTo,msgSTo);
            }

            // if not posted (it's junk) or not to all (it's email), break
            if (!posted || !tmpMsgAll) break;
        }
    }

    sstrfre(msgNewsgroups);

    // reinitialize the message buffer
    size = 0;
    memset(buf,' ',sizeof(buf));
    if (nhdr->file != NULL) {
      char Temp[128];
      buildstr(Temp,PcbData.TmpLoc,nhdr->file,NULL);
      unlink(Temp);
    }

    return rv;
}

void pascal cPCBMSG::load(char * path, char * name, char * line, int lsize)
{
    char file [ 128 + 1 ];
    strcpy(file,path);
    strcat(file,name);

    cDOSFILE msg;
    if (msg.open(file,OPEN_READ|OPEN_DENYRDWR) == 0)
    {
        long fSize;
        msg.read(&fSize,sizeof(fSize));
        lastSize = fSize;
        msg.seek(fSize,SEEK_CUR);

        readField(nhdr->apparentlyto,msg);
        readField(nhdr->date,msg);
        readField(nhdr->distribution,msg);
        readField(nhdr->followupto,msg);
        readField(nhdr->from,msg);
        readField(nhdr->msgidmail,msg);
        readField(nhdr->msgidnews,msg);
        readField(nhdr->newsgroups,msg);
        readField(nhdr->organization,msg);
        readField(nhdr->replyto,msg);
        readField(nhdr->subject,msg);
        readField(nhdr->to,msg);
        readField(nhdr->user,msg);

        msg.seek(sizeof(fSize),SEEK_SET);

        long fRead = 0;
        while ((fRead < fSize) && (msg.getuln(line,lsize) == 0))
        {
         // if (kbhit() && (getkey() == ESC))
         //     abortFlag = TRUE;

            int llen = strlen(line);
            fRead += llen+1;

            addLine(line,llen);
        }
    }
}

void pascal cPCBMSG::load(cVMDATA & msg)
{
    long  Num = msg.recCount();
    long  i;

    for (i = starting; i <= Num; ++i)
    {
        void * ptr = msg.recIdxGet(i);
        int len = * (int *) ptr;
        char * line = (char *) ptr;
        line += sizeof(len);
        addLine(line,len);
    }
    starting = i;
}

void pascal cPCBMSG::readField(char * & field,cDOSFILE & msgFile)
{
    int fSize;
    msgFile.read(&fSize,sizeof(fSize));
    if (fSize)
    {
        field = (char *) malloc(fSize);
        if (field) msgFile.read(field,fSize);
    }
}

/******************************************************************************/

static void near pascal buildUniqueName ( char * path, char * name,
    cDOSFILE & file )
{
    name[0] = NUL;

    int tries = 0;
    while ((name[0] == NUL) && (++tries < 128))
    {
        sprintf(name,"%03X%03X%02X.%03X",
            random(0x1000),random(0x1000),
            random(0x0100),random(0x1000));

        char temp [ 128 + 1 ];
        strcpy(temp,path);
        strcat(temp,name);

        if ((fileexist(temp) != 255) ||
            (file.open(temp,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE) != 0))
            name[0] = NUL;
    }
}

unsigned short recSizeFunc(void * p)
{
    int * ip = (int *) p;
    return sizeof(*ip)+*ip+1;
}

cTMPMSG::cTMPMSG(char _all, char _stat, cNETHDR * _hdr, char * _spath) :
    msgAll(_all),
    msgStat(_stat),
    nhdr(_hdr),
    isEncoded(FALSE),
    beginFound(FALSE),
    endFound(FALSE),
    lastLine(0)
{
    strcpy(spath,_spath);
    msgData.initRecVarIdxSet(recSizeFunc,8,idxBuf,sizeof(idxBuf));
}

cTMPMSG::~cTMPMSG(void)
{
    msgData.doneSet();
}

void pascal cTMPMSG::addLine(char * line, int len, int hide)
{
    if (hide) ++len;
    char * ptr = (char *) msgData.createRec(sizeof(len)+len+1);
    memcpy(ptr,&len,sizeof(len));
    ptr += sizeof(len);
    if (hide) *(ptr++) = '\x01';
    strcpy(ptr,line);

    if (uucpDecode && !isEncoded && (line[0] == 'M') &&
        ((len == 61) || (len == 62)))
    {
        char tbuf [ 45 ];
        int tsize;
        decodeLine(line,tbuf,tsize);
        isEncoded = (tsize == 45);
    }

    if (!beginFound      &&
        (line[0] == 'b') &&
        (line[1] == 'e') &&
        (line[2] == 'g') &&
        (line[3] == 'i') &&
        (line[4] == 'n') &&
        (line[5] == ' '))
        beginFound = TRUE;

    if (!endFound        &&
        (line[0] == 'e') &&
        (line[1] == 'n') &&
        (line[2] == 'd') &&
        (line[3] == NUL))
        endFound = TRUE;
}

int pascal cTMPMSG::flushMsg(void)
{
    if (beginFound && !isEncoded) beginFound = FALSE;
    if (endFound && !isEncoded) endFound = FALSE;

    if (!isEncoded)
    {
        cPCBMSG minfo(msgAll,msgStat,nhdr);

        ++lastArtNum;

        minfo.load(msgData);
        if (minfo.flushMsg() != 0) return -1;
    }
    else // must be saved for later
    {
        buildUniqueName(spath,msgName,msgFile);

        long size = 0;
        msgFile.write(&size,sizeof(size));

        for (long i = 1; i <= msgData.recCount(); ++i)
        {
            void * ptr = msgData.recIdxGet(i);
            char * line = (char *) ptr;
            line += sizeof(int);
            msgFile.putuln(line);
        }

        size = msgFile.seek(0,SEEK_END);
        size -= sizeof(long);
        msgFile.seek(0,SEEK_SET);
        msgFile.write(&size,sizeof(size));

        msgFile.seek(0,SEEK_END);

        writeField(nhdr->apparentlyto);
        writeField(nhdr->date);
        writeField(nhdr->distribution);
        writeField(nhdr->followupto);
        writeField(nhdr->from);
        writeField(nhdr->msgidmail);
        writeField(nhdr->msgidnews);
        writeField(nhdr->newsgroups);
        writeField(nhdr->organization);
        writeField(nhdr->replyto);
        writeField(nhdr->subject);
        writeField(nhdr->to);
        writeField(nhdr->user);

        conout3 << endl << "      Analyzing:  " << messageTypeLabel << ' ' <<
            setw(4) << ++lastArtNum << " Size " <<
            setw(6) << lastSize << ' ';
    }

    return 0;
}

void pascal cTMPMSG::writeField(char * field)
{
    int size = (field ? strlen(field)+1 : 0);
    msgFile.write(&size,sizeof(size));
    if (size) msgFile.write(field,size);
}

void pascal cTMPMSG::resetLine(void)
{
    lastLine = 0;
}

char * pascal cTMPMSG::getLine(void)
{
    return ((lastLine+1 <= msgData.recCount()) ?
        (((char *) msgData.recIdxGet(++lastLine)) + sizeof(int)) :
        NULL);
}

/******************************************************************************/

void pascal cMSGINFO::load ( char * path )
{
    char name [ 128 + 1 ];
    strcpy(name,path);
    strcat(name,"MSGINFO.DAT");

    cDOSFILE file;

    if ((fileexist(name) != 255) &&
        (file.open(name,OPEN_READ|OPEN_DENYRDWR) == 0))
    {
        close();
        open();

        sMSGINFO msgInfo;

        while (file.read(&msgInfo,sizeof(msgInfo)) == sizeof(msgInfo))
            add(msgInfo);

        file.close();
    }
}

void pascal cMSGINFO::save ( char * path )
{
    char name [ 128 + 1 ];
    strcpy(name,path);
    strcat(name,"MSGINFO.DAT");

    cDOSFILE file;

    if (file.open(name,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE) == 0)
    {
        sMSGINFO msgInfo;

        long records = recCount();

        unsigned today = getjuliandate();

        for (long i = 1; i <= records; ++i)
        {
            get(msgInfo,i);
            if (msgInfo.date+uucpWaitDays+1 <= today)
                msgInfo.isEncoded = false;
            if (!msgInfo.processed)
            {
                if (file.write(&msgInfo,sizeof(msgInfo)) != sizeof(msgInfo))
                    break;
            }
            else
            {
                strcpy(name,path);
                strcat(name,msgInfo.fileName);
                unlink(name);
            }
        }

        file.close();
    }
}

static short cmpMsgInfo ( const void * lp, const void * rp )
{
    sMSGINFO * lmi = (sMSGINFO *) lp;
    sMSGINFO * rmi = (sMSGINFO *) rp;

    if (lmi->isEncoded != rmi->isEncoded)
        return lmi->isEncoded - rmi->isEncoded;

    int len = lmi->subjLen;
    if (rmi->subjLen < len) len = rmi->subjLen;
    int rv = memicmp(lmi->subject,rmi->subject,len);
    if (rv == 0)
        rv = lmi->partNum - rmi->partNum;

    return rv;
}

void pascal cMSGINFO::sort ( void )
{
    sMSGINFO buf [ 112 ];
    if (recCount() > 0)
    {
        cVMDATA::sort(sizeof(sMSGINFO),1,recCount(),VM_FALSE,cmpMsgInfo,
            (VMSortFunc *) qsort, buf, sizeof(buf));
    }
}

void pascal cMSGINFO::add ( sMSGINFO & rec )
{
    sMSGINFO * mip = (sMSGINFO *) createRec(sizeof(sMSGINFO));

    if (mip) memcpy(mip,&rec,sizeof(rec));
}

void pascal cMSGINFO::add ( cTMPMSG & msg )
{
    sMSGINFO rec;

    memset(&rec,0,sizeof(rec));
    rec.processed = FALSE;
    rec.date = getjuliandate();
    rec.all = msg.msgAll;
    rec.stat = msg.msgStat;
    strcpy(rec.fileName,msg.msgName);
    rec.isEncoded = msg.isEncoded;
    rec.beginFound = msg.beginFound;
    rec.endFound = msg.endFound;

    maxstrcpy(rec.subject,msg.nhdr->subject,sizeof(rec.subject));

    // Possible subject formats:
    // text text text
    // text text text 1
    // text text text 1/2
    // text text text 1 of 2

    // playing processor results for the following loop
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // "(fwd) Asian series A_1 - as_01_13.jpg [1/2]"
    // subj     = "Subject: (fwd) Asian series A_1 - as_01_13.jpg [1/2]"
    // afterPtr = "Subject: (fwd) Asian series A_1 - as_01_13.jpg [1/2]"
    // digPtr   = "_""1 - as_01_13.jpg [1/2]"
    // afterPtr = "1"" - as_01_13.jpg [1/2]"
    // while    = 1 && (0 || 1 || 0) = 1 && 1 = 1
    // digPtr   = "_""01_13.jpg [1/2]"
    // afterPtr = "1""_13.jpg [1/2]"
    // while    = 1 && (0 || 1 || 1) = 1 && 1 = 1
    // digPtr   = "_""13.jpg [1/2]"
    // afterPtr = "3"".jpg [1/2]"
    // while    = 1 && (0 || 1 || 1) = 1 && 1 = 1
    // digPtr   = "[""1/2]"
    // afterPtr = "1""/2]"
    // while    = 1 && (0 || 0 || 0) = 1 && 0 = 0

    char * subj = sstrdup(msg.nhdr->subject);

    char * digPtr;
    char * afterPtr = subj;

    do {

        digPtr = strpbrk(afterPtr,"0123456789");
        if (digPtr) afterPtr = digPtr+strspn(digPtr,"0123456789");

    // While we found a digit and (it's at the beginning of the subject or
    // it's not prefixed by " ([{" or it's not suffixed by " o/"), keep looking
    } while ((digPtr != NULL) &&
        ((digPtr == subj) ||
         (strchr(" ([{",digPtr[-1]) == NULL) ||
         (strchr(" o/",*afterPtr) == NULL)));

    if (digPtr)
    {
        int tmpLen = strlen(subj)-strlen(digPtr);

        // ep1 will point to char that stopped strtol of v1
        // ep2 will point to char that stopped strtol of v2
        char * ep1, * ep2 = NULL;

        // Scan the part number info into v1
        long v1 = strtol(digPtr,&ep1,10);

        // Scan for the total number info
        digPtr = strpbrk(ep1,"0123456789");

        // Scan the total number info (if defined) into v2
        long v2 = (digPtr ? strtol(digPtr,&ep2,10) : 0);

        if (v2 <    0) v1 = v2 = 0;
        if (v1 <    0) v1      = 0;
        if (v2 > 1000) v1 = v2 = 0;
        if (v1 > 1000) v1      = 0;

        // if part is defined and total is defined and
        // part is less than or equal to total and
        // total is less than 1000 and
        // the separator is / or "of" or " of "
        if  ((v1 > 0) &&
                ((v2 == 0) ||
                    ((v1 <= v2) &&
                        ((ep1[0] == '/') || (memicmp(ep1,"of",2) == 0) ||
                            (memicmp(ep1," of ",4) == 0)
            )   )   )   )
        {
            rec.subjLen = tmpLen;
            rec.partNum = int(v1);
            rec.totParts = int(v2);
            if ((rec.totParts == 0) && rec.endFound)
                rec.totParts = rec.partNum;
        }
    }

    sstrfre(subj);

    add(rec);
}

void pascal cMSGINFO::get ( sMSGINFO & rec, long num )
{
    sMSGINFO * mip = (sMSGINFO *) recIdxGet(num);

    if (mip)
        memcpy(&rec,mip,sizeof(rec));
    else
        memset(&rec,0,sizeof(rec));
}

void pascal cMSGINFO::put ( sMSGINFO & rec, long num )
{
    sMSGINFO * mip = (sMSGINFO *) recIdxGet(num);

    if (mip)
    {
        memcpy(mip,&rec,sizeof(rec));
        changedRec();
    }
}

void pascal cMSGINFO::process ( char * path, char * line, int lsize )
{
    sort();

    lastArtNum = 0;
    lastSize = 0;

    sMSGINFO msgInfo;
    long records = recCount();

    for (long i = 1; i <= records; ++i)
    {
        get(msgInfo,i);

        if (msgInfo.processed)
        {
            continue;
        }
        else if ((!msgInfo.isEncoded) || (msgInfo.partNum == 0))
        {
            cNETHDR hinfo;
            cPCBMSG minfo(msgInfo.all,msgInfo.stat,&hinfo);

            ++lastArtNum;

            minfo.load(path,msgInfo.fileName,line,lsize);
            minfo.flushMsg();

            msgInfo.processed = TRUE;
            put(msgInfo,i);
        }
        else // must be encoded and multi-part
        {
            cNETHDR hinfo;
            cPCBMSG minfo(msgInfo.all,msgInfo.stat,&hinfo);

            sMSGINFO tmi = msgInfo;
            int maxPart = msgInfo.totParts;
            int j;
            for (j = 0; (j < maxPart) || (maxPart == 0); ++j)
            {
                get(msgInfo,i+j);
                if (memicmp(tmi.subject,msgInfo.subject,msgInfo.subjLen) != 0)
                    break;
                else if ((maxPart == 0) && (msgInfo.totParts != 0))
                    maxPart = msgInfo.totParts;
                if (msgInfo.partNum != (j+1))
                    break;
            }
            if ((j == maxPart) && (maxPart != 0))
            {
                for (j = 0; j < maxPart; ++j)
                {
                    get(msgInfo,i+j);
                    ++lastArtNum;
                    hinfo.kill();
                    minfo.load(path,msgInfo.fileName,line,lsize);
                    msgInfo.processed = TRUE;
                    put(msgInfo,i+j);
                }
                minfo.flushMsg();
            }
        }
    }

    save(path); // Need to find a better way of purging processed records
    load(path); // Than saving (which flags to purge) and loading (which deletes the dataset and reloads)
}

/******************************************************************************/

