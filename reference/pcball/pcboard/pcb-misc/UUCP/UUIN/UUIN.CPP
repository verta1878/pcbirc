
/******************************************************************************/

// Temporarily change default signed character type for standard libraries
// #pragma option -K-

#include    <ctype.h>
#include    <dir.h>
#include    <dos.h>
#include    <string.h>
#include    <time.h>

#include    <constrea.h>
#include    <alloc.h>

#include    <cnameidx.h>
#include    <umwf.hpp>

#include    "decode.h"
#include    "uuin.hpp"

// This needs to come last because it includes memcheck
#include    <uucp.hpp>

/******************************************************************************/

// extern "C" {

void pascal reinstallhandlers(void);
void        uninstallhandlers(void);

// }

/******************************************************************************/

/*  These are all declared in PCBTOOLS.H */
//extern int      ExtendedError;
//extern char     ExtendedAction;
//extern char     ExtendedClass;
//extern char     ExtendedLocus;

/******************************************************************************/

static   constream conscr;
// constream conout;
static   constream conout0;
static   constream conout1;
static   constream conout2;
         constream conout3;
         constream conerr;

   cDOSFILE  logFile;
   cDOSFILE  junkGroups;

   int       lastArtNum;
   long      lastSize;

   char    * messageTypeLabel = "Message";

static  cMSGINFO  msgInfo;

//   int       DebugLevel = 0; // 99;

static   int       lineCount = 0;

/******************************************************************************/

static bool killFiles = TRUE;
static bool abortFlag = FALSE;

static char _FAR_ buf [ 16384 ]; // Miscellaneous buffer for screen management & file IO

/******************************************************************************/

/*
  char uucpPath[33];       /* location for UUCP files                        */
  char uucpSpoolPath[33];  /* location for UUCP Spool files                  */
  char uucpLogPath[33];    /* location for UUCP Log files                    */
  char CompBatFile[33];    /* name/loc of compress batch file for UUCP       */
  char DeCompBatFile[33];  /* name/loc of decompress batch file for UUCP     */
  char uucpName[17];       /* UUCP Name                                      */
  char uucpDomainName[33]; /* UUCP Domain Name                               */
  char uucpEmailHost[17];  /* UUCP Email Host Name                           */
  char uucpNewsHost[17];   /* UUCP News Host Name                            */
  char uucpDefDist[17];    /* UUCP Default News Distribution                 */
  char uucpTimeZone[6];    /* Time Zone - offset from GMT                    */
  char uucpSeparator[2];   /* UUCP Name Separator                            */
  char Organization[65];   /* Organization Name                              */
  char uucpHighAscii;      /* S = Strip, R = Replace, C = Convert            */
  unsigned uucpEmailConf;  /* number of UUCP Email Conference                */
  unsigned uucpJunkConf;   /* number of UUCP Junk Conference                 */
  bool uucpBangDomain;     /* Bang Domain                                    */
  bool uucpSubDomain;      /* Sub Domain                                     */
*/

/******************************************************************************/

const int ESC = 27;

static inline int pascal getkey(void)
{
    int key = getch();
    if (key == 0) key = getch()*0x0100;
    return key;
}

/******************************************************************************/

static int near pascal doSystem(char * cmd)
{
    text_info ti;
    gettextinfo(&ti);

    gettext(1,1,ti.screenwidth,ti.screenheight,buf);

    conscr << setattr(0x07);
    conscr.clrscr();
    conscr << "Swapping: " << cmd << endl;

    uninstallhandlers();
    VMEMSStateSave();
    system(cmd);
    VMEMSStateRestore();
    reinstallhandlers();

    conout3 << setxy(ti.curx,ti.cury);
    puttext(1,1,ti.screenwidth,ti.screenheight,buf);

    return 0;
}

static void near pascal copyDOSFILEtoDOS(cDOSFILE & src, cDOS & dst)
{
    const unsigned msize = sizeof(buf);

    unsigned rsize;

    do {

        rsize = src.read(buf,msize);

    } while ((rsize > 0) && (dst.writechk(buf,rsize) == 0));
}

static char * near pascal findFileExt(char * pathname)
{
    char * ext = strrchr(pathname,'\\');
    if (ext == NULL) ext = pathname;

    ext = strchr(ext,'.');
    if (ext == NULL) ext = ext+strlen(ext);

    return ext;
}

static inline void replaceFileExt(char * pathname, char * newext)
{
    strcpy(findFileExt(pathname),newext);
}

static inline void stripFileExt(char * pathname)
{
    findFileExt(pathname)[0] = NUL;
}

/******************************************************************************/

const int maxHdrs = 32;
static int  keyInit = 0;
static int  keyCnt  = 0;
static char _FAR_ hdrKeys [ maxHdrs ] [ 32 + 1 ];

static int near pascal hdrMatch(char * line)
{
    if (line[0] == NUL) return TRUE;

    if (keyInit == 0)
    {
        keyInit = 1;

        char keyName [ 128 + 1 ];
        buildstr(keyName,PcbData.uucpPath,"HDRKEYS.LST",NULL);

        if (fileexist(keyName) != 255)
        {
            cDOSFILE keyFile;
            if (keyFile.open(keyName,OPEN_READ|OPEN_DENYNONE) == 0)
            {
                int i = 0;
                while ((keyFile.getln(hdrKeys[i],sizeof(hdrKeys[i])) == 0) &&
                    (keyCnt < maxHdrs))
                {
                    if (hdrKeys[i][0] != NUL)
                    {
                        ++i;
                        ++keyCnt;
                    }
                }
                keyFile.close();
            }
        }
        else
        {
            keyInit = 2;
        }
    }

    if (keyInit == 2) return TRUE;

    for (int i = 0; i < keyCnt; ++i)
        if (memicmp(line,hdrKeys[i],strlen(hdrKeys[i])) == 0)
            return TRUE;

    return FALSE;
}

/******************************************************************************/
// Reads in a line of text.
//
// maxlen:    the actual number of bytes read
// returns:   actual size of string after stripping trailing \r
//
int pascal readline( cDOSFILE &file, char *buf, unsigned maxlen, unsigned &actual ) {
    int llen = -1;

    memset(buf,0,maxlen);
    if ((llen = file.getuln(buf,maxlen)) == 0) {
       actual = strlen(buf) + 1;
       stripright(buf,'\r');
       llen = strlen(buf);
    }
    return (llen);
}


static long near pascal rdNetHdr(cNETHDR & hdr, cDOSFILE & txt, cTMPMSG & msg,
    char * line, unsigned lsize)
{
    int lastMatch = 0;
    long read = 0;
    starting = 1L;
    int llen;

/*
     while (txt.getuln(line,lsize) == 0)
    {
        stripright(line,'\r');
        int llen = strlen(line);
        read += llen+1;

        if ((line[0] != ' ') && (line[0] != '\t'))
            lastMatch = hdrMatch(line);
        msg.addLine(line,llen,((line[0] != NUL) && !lastMatch));

        if (line[0] == NUL) break;

        hdr.getInfo(line);
    }
*/
    unsigned actual;
    while (1) {
       llen = readline(txt,line,lsize,actual);

       if (llen == -1) break;
       read += (long) actual;

       if ((line[0] != ' ') && (line[0] != '\t'))
          lastMatch = hdrMatch(line);
       msg.addLine(line,llen,((line[0] != NUL) && !lastMatch));

       if (! llen) break;

       hdr.getInfo(line);
    }

    return read;
}

    /*--------------------------------------------------------------------*/
long pascal addMsgLine( cTMPMSG & minfo, cDOSFILE & msg, long read, long fsize, char *Boundary ) {
  unsigned  lsize;
  char      line[4096];

  lsize = sizeof(line) - 1;
//  memset(line,0,sizeof(line));

  unsigned actual;
  while ((fsize == -1) || (read < fsize)) {
      int llen = readline(msg,line,lsize,actual);

      if (llen == -1) break;
      read += (long) actual;

      if (kbhit() && (getkey() == ESC))
          abortFlag = TRUE;

      // check to see if we are processing on a boundary/multipart message
      // for MIME.  If we are and found one, then just exit without reading
      // any more of the file.
      if (line[0] != 0 && Boundary != NULL && Boundary[0] != 0) {
        if (memicmp(Boundary,line+2,strlen(Boundary)) == 0)
          break;
      }

      minfo.addLine(line,llen,0);

      ++lineCount;

      if (uucpGiveup && ((lineCount & 0x3F) == 0)) tickdelay(1);
  }

  return read;
}

    /*--------------------------------------------------------------------*/
#pragma argsused
static long near pascal importMail(cDOSFILE & msg, long fsize,
    char * line, unsigned lsize, char * user, char * spath)
{
    long startoff = msg.seek(0,SEEK_CUR);

    char * puser = user;
    char * nuser = strchr(puser,' ');

    long read = 0;
    messageTypeLabel = "Email  ";

    while (puser != NULL)
    {
        if (nuser != NULL) *(nuser++) = NUL;

        char tmpUser [ 120 + 1 ];
        maxstrcpy(tmpUser,puser,sizeof(tmpUser));

        strlwr(tmpUser);

        // Is there a domain/site separator?
        int domainInfo = ((strchr(tmpUser,'@') != NULL) ||
                          (strchr(tmpUser,'!') != NULL) ||
                          (strchr(tmpUser,'%') != NULL));

        // If so, is the systems domain or site name specified?
        if (domainInfo &&
            (strstr(tmpUser,PcbData.uucpName)       == NULL) &&
            (strstr(tmpUser,PcbData.uucpDomainName) == NULL) &&
            (strstr(tmpUser,uucpName)               == NULL))
            tmpUser[0] = NUL; // No, so invalid user

        if (tmpUser[0] != NUL)
        {
            msg.seek(startoff,SEEK_SET);

            cNETHDR hinfo;
            cTMPMSG minfo(FALSE,MSG_RCVR,&hinfo,spath);

            read = rdNetHdr(hinfo,msg,minfo,line,lsize);
            hinfo.setToUser(tmpUser);
            checkmime(hinfo);

            if (hinfo.MimeVersion[0] != 0 || hinfo.ContentType[0] != 0 || hinfo.EncodeType[0] != 0) {
               if (memicmp(hinfo.ContentType,"multi",5) == 0) {
                 // this is a multipart message.
                 read = decodeMultiMsg(hinfo,minfo,msg,read,fsize,line,lsize);
               } else if (memicmp(hinfo.EncodeType,"base64",6) == 0) {
                 // only base64 decoding is required
                 if (Base64FileName[0] == 0) strcpy(Base64FileName,DEFAULT_FILENAME);
                 hinfo.file = Base64FileName;
                 read = decode64Msg(Base64FileName,msg,read,fsize,NULL);
               } else if (memicmp(hinfo.EncodeType,"quoted",6) == 0) {
                 // only Quoted-Printable decoding is required.
                 read = decodeQuotedMsg(msg,"DECODE.TMP",NULL,read,fsize);
                 msg.close();
                 msg.open("DECODE.TMP",OPEN_READ|OPEN_DENYRDWR);
                 addMsgLine(minfo,msg,0,msg.size(),NULL);   //lint  !e534
                 goto SKIPBODY;
               }
            }

            read = addMsgLine(minfo,msg,read,fsize,NULL);

         SKIPBODY:
            char STo [ 64 + 1 ]; char LTo [ 120 + 1 ];
            parseName(hinfo.to ? hinfo.to : hinfo.apparentlyto,
                STo, sizeof(STo), LTo, sizeof(LTo), TRUE);
            if (hinfo.user)
            {
                char tmpLTo [ 120 + 1 ];
                parseName(hinfo.user,
                    STo, sizeof(STo), tmpLTo, sizeof(tmpLTo), TRUE);
            }

            if (isalnum(STo[0]))
            {
                if ((strchr(STo,'@') == NULL) &&
                    (strchr(STo,'%') == NULL) &&
                    (strchr(STo,'!') == NULL))
                {
                    if (minfo.flushMsg() != 0) read = -1;
                    if (minfo.isEncoded) msgInfo.add(minfo);
                }
                else
                {
                    /*char msgSTo [ 64 + 1 ];*/  char msgLTo [ 120 + 1 ];
                    //msgSTo[0] = NUL;
                    strcpy(msgLTo,STo);

                    char msgSFrom [  64 + 1 ]; char msgLFrom [ 120 + 1 ];
                    char replyto  [ 120 + 1 ];
                    char sender   [ 120 + 1 ];
                    parseName(hinfo.from,   msgSFrom,sizeof(msgSFrom),
                                            msgLFrom,sizeof(msgLFrom),FALSE);
                    parseName(hinfo.replyto,msgSFrom,sizeof(msgSFrom),
                                            replyto, sizeof(replyto), FALSE);
                    msgSFrom[0] = NUL;
                    strcpy(sender,makeaddress("postmaster"));

                 // char msgSFrom [  64 + 1 ]; char msgLFrom [ 120 + 1 ];
                 // char replyto  [ 120 + 1 ];
                 // parseName(hinfo.from, msgSFrom, sizeof(msgSFrom),
                 //                       replyto,  sizeof(replyto), FALSE);
                 // msgSFrom[0] = NUL;
                 // strcpy(msgLFrom,makeaddress("postmaster"));

                    char mfrom [ 120 + 1 ];
                    strcpy(mfrom,makemfrom(msgSFrom));

                    char sDate [ 60 + 1 ];
                    strcpy(sDate,makemdate());

                    // Build the message id line
                    char msgid [ 60 + 1 ];
                    strcpy(msgid,makemsgid("forward"));

                    char subject [ 240 + 1 ];
                    buildstr(subject,"(fwd) ",hinfo.subject,NULL);

                    char filename [ _MAX_PATH ];
                    buildstr(filename,PcbData.uucpPath,"FORWARD.TXT",NULL);

                    cDOSFILE message;
                    if (message.open(filename,OPEN_READ|OPEN_DENYNONE) == 0)
                        exportMessage(msgTypeEmail,mailPath,mfrom,msgLTo,
                            msgLFrom,subject,NULL,
                            NULL,msgid,sDate,NULL,PcbData.Organization,
                            replyto,NULL,&message,NULL,"",NULL,0,&minfo,
                            sender);
                }
            }
            else
            {
                switch (STo[0])
                {
                    case '%': // to infobot processor
                    {
                        char msgSTo [ 64 + 1 ]; char msgLTo [ 120 + 1 ];
                        parseName(hinfo.from, msgSTo, sizeof(msgSTo),
                                              msgLTo, sizeof(msgLTo), FALSE);

                        char msgSFrom [ 64 + 1 ]; char msgLFrom [ 120 + 1 ];
                        parseName(hinfo.to, msgSFrom, sizeof(msgSFrom),
                                            msgLFrom, sizeof(msgLFrom), FALSE);

                        logFile.printf("infobot %s to %s\r\n",msgSFrom,msgLTo);

                        char mfrom [ 120 + 1 ];
                        strcpy(mfrom,makemfrom(msgSFrom));

                        char sDate [ 60 + 1 ];
                        strcpy(sDate,makemdate());

                        // Setup replyto
                        char replyto [ 120 + 1 ];
                        strcpy(replyto,makeaddress("postmaster"));

                        // Build the message id line
                        char msgid [ 60 + 1 ];
                        strcpy(msgid,makemsgid("text"));

                        char subject [ 240 + 1 ];
                        strcpy(subject,msgLFrom);
                        strcat(subject," Auto Reply Daemon");

                        cDOSFILE message;
                        if (message.open(STo+1,OPEN_READ|OPEN_DENYNONE) == 0)
                            exportMessage(msgTypeEmail,mailPath,mfrom,msgLTo,
                                msgLFrom,subject,NULL,
                                NULL,msgid,sDate,NULL,PcbData.Organization,
                                replyto,NULL,&message,NULL,"",NULL,0,&minfo,
                                NULL);

                        break; // bounce text file
                    }

                    // ***FEATURE***
                    case '!': break; // give to external program
                    // ***FEATURE***
                    case '&': break; // give to listserv manager
                    // ***FEATURE***
                    case '$': break; // give to listserv mailer
                    // ***FEATURE***
                    case '#': break; // give to FTP daemon
                }
            }
        }

        if (read == -1) break;

        puser = nuser;
        if (nuser != NULL) nuser = strchr(puser,' ');
    }

    return read;
}

    /*--------------------------------------------------------------------*/

static int near pascal rmail(char * spath, char * sname, char * fname,
    char * user)
{
    if (abortFlag) return 0;

    char _spath [ 128 + 1 ];
    buildstr(_spath,spath,sname,"\\",NULL);

    conout2 << endl << "    Executing rmail " << maxstr(user,40) << ' ' <<
        fname;

    char mname [ 128 + 1 ];
    buildstr(mname,spath,sname,"\\",fname,NULL);

    long rv = -1;

    cDOSFILE mfile;
    if (mfile.open(mname,OPEN_READ|OPEN_DENYRDWR) == 0)
    {
        char line [ 1024 + 1 ];
        lastArtNum = 0;
        lastSize   = mfile.size();
        rv = importMail(mfile,lastSize,line,sizeof(line),user,_spath);
        mfile.close();
        if (fileexist("DECODE.TMP") != 255) unlink("DECODE.TMP");
        if (killFiles && (rv != -1)) unlink(mname);
        msgInfo.process(_spath,line,sizeof(line));
    }
    else
    {
        conerr << endl << "Error Opening Email File (" << mname << ")!";
    }

    return ((rv == -1) ? -1 : 0);
}

    /*--------------------------------------------------------------------*/

static long near pascal importArticle(cDOSFILE & art, long fsize,
    char * line, unsigned lsize, char * spath)
{
    cNETHDR   hinfo;
    cTMPMSG   minfo(TRUE,MSG_PBLC,&hinfo,spath);
    cDOSFILE  tmpFile;

    messageTypeLabel = "Article  ";
    long read = rdNetHdr(hinfo,art,minfo,line,lsize);
    checkmime(hinfo);

    if (hinfo.MimeVersion[0] != 0 || hinfo.ContentType[0] != 0 || hinfo.EncodeType[0] != 0) {
       if (memicmp(hinfo.ContentType,"multi",5) == 0) {
         // this is a multipart message.
         read = decodeMultiMsg(hinfo,minfo,art,read,fsize,line,lsize);
       } else if (memicmp(hinfo.EncodeType,"base64",6) == 0) {
         // only base64 decoding is required
         if (Base64FileName[0] == 0) strcpy(Base64FileName,DEFAULT_FILENAME);
         hinfo.file = Base64FileName;
         read = decode64Msg(Base64FileName,art,read,fsize,NULL);
       } else if (memicmp(hinfo.EncodeType,"quoted",6) == 0) {
         // only Quoted-Printable decoding is required.
         read = decodeQuotedMsg(art,"DECODE.TMP",NULL,read,fsize);
         tmpFile.open("DECODE.TMP",OPEN_READ|OPEN_DENYRDWR);
         addMsgLine(minfo,tmpFile,0,tmpFile.size(),NULL);      //lint  !e534
         tmpFile.close();
         unlink("DECODE.TMP");
         minfo.flushMsg();
         if (minfo.isEncoded) msgInfo.add(minfo);
         return read;
       }
    }

    read = addMsgLine(minfo,art,read,fsize,NULL);
    minfo.flushMsg();

    if (minfo.isEncoded) msgInfo.add(minfo);

    return read;
}

    /*--------------------------------------------------------------------*/
static int near pascal checksize( int artNum, char *nname, long fsize, long read ) {

  if (read != fsize) {
    conerr << endl << "Size Mismatch Importing " << messageTypeLabel << (artNum+1) << " in " << nname << "!";
    logFile.printf("Mismatch size:  Number: %d  Name: %s\r\n",artNum+1,nname);

    conerr << endl << " Expected " << fsize << ", Received " << read;
    logFile.printf("Expected: %ld    Received:  %ld",fsize,read);
    return -1;
  }

  return 0;
}

    /*--------------------------------------------------------------------*/

static int near pascal rnews(char * spath, char * sname, char * fname)
{
    if (abortFlag) return -1;

    char _spath [ 128 + 1 ];
    buildstr(_spath,spath,sname,"\\",NULL);

    int rv = 0;

    bool lclKillFiles = killFiles;

    logFile.printf("    rnews %s%s\\%s\r\n",spath,sname,fname);

    conout2 << endl << "    Executing rnews " << fname;

    char nname [ 128 + 1 ];
    buildstr(nname,spath,sname,"\\",fname,NULL);

    cDOSFILE nfile;
    if (nfile.open(nname,OPEN_READ|OPEN_DENYRDWR) != 0) return -1;

    int artNum = 0;

    char line [ 2048 + 1 ];

    bool firstLine = TRUE;
    unsigned llen;

    while (readline(nfile,line,sizeof(line),llen) != -1)
    {
        messageTypeLabel = "Message";
        if (memcmp(line,"#! rnews ",9) == 0)
        {
            long fsize = atol(line+9);
         /*
            logFile.printf("      Article %4d Size %6ld ",++artNum,fsize);

            conout3 << endl << "      Article " << setw(4) <<
                artNum << " Size " << setw(6) << fsize << ' ';
         */
            lastArtNum = artNum++;
            lastSize   = fsize;

            long read = importArticle(nfile,fsize,line,sizeof(line),_spath);

            if ((rv = checksize(artNum,nname,fsize,read)) == -1) {
//              lclKillFiles = FALSE;
//              break;
            }
        }
        else if (memcmp(line,"#! rmail ",9) == 0)
        {
            long fsize = atol(line+9);

            char * ptr = strchr(line+9,' ');
            if (ptr) ++ptr;

//            char users [ 1024 + 1 ];
            char users [ 256 + 1 ];
            maxstrcpy(users,ptr?ptr:"",sizeof(users));

         /*
            logFile.printf("      Email   %4d Size %6ld ",
                ++artNum,fsize);

            conout3 << "      Email   " << setw(4) <<
                artNum << " Size " << setw(6) << fsize << ' ';
         */
            lastArtNum = ++artNum;
            lastSize   = fsize;

            long read = importMail(nfile,fsize,line,sizeof(line),users,_spath);

            if ((rv = checksize(artNum,nname,fsize,read)) == -1) {
//              lclKillFiles = FALSE;
//              break;
            }
        }
        else if (firstLine) // (strcmp(line,"#! cunbatch") == 0)
        {
            if ((uucpDebug == 0) || (uucpDebug >= 10))
                logFile.putln("      Decompressing Batched Messages");

            conout3 << endl << "      Decompressing Batched Messages";

            nfile.rewind();

            if ((strcmp(line,"#! cunbatch") == 0) ||
                (strcmp(line,"#! gunbatch") == 0))
                nfile.seek(12,SEEK_SET);

            char zname [ 128 + 1 ];
            strcpy(zname,nname);
            replaceFileExt(zname,".Z");

            cDOS zfile;
            zfile.create(zname,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);

            copyDOSFILEtoDOS(nfile,zfile);

            zfile.close();
            nfile.close();

            char cmd [ 128 + 1 ];
            buildstr(cmd,PcbData.DeCompBatFile," ",zname,NULL);

            if (fileexist(PcbData.DeCompBatFile) == 255) {
              char Str[81];
              sprintf(Str,"Unable to locate: %s",PcbData.DeCompBatFile);
              logFile.putln(Str);
            }

            doSystem(cmd);

            if (fileexist(zname) == 255)
            {
                unlink(nname);

                strcpy(zname,nname);
                stripFileExt(zname);

                rename(zname,nname);
            }
            else
            {
                char  Str[81];
                sprintf(Str,"Error Decompressing: %s", zname);
                logFile.putln(Str);

                unlink(zname);

                conerr << endl << "Error Decompressing Batched Messages (" <<
                    nname << ")!";

                lclKillFiles = FALSE;
                rv = -1;

                break;
            }

            if (nfile.open(nname,OPEN_READ|OPEN_DENYRDWR) != 0)
              return -1;
        }

        firstLine = FALSE;
    }

    nfile.close();

    if (lclKillFiles)
        unlink(nname);
    else
    {
        strcpy(line,nname);
        strcpy(strrchr(line,'.'),".BAD");
        rename(nname,line);
    }

    msgInfo.process(_spath,line,sizeof(line));

    return rv;
}

    /*--------------------------------------------------------------------*/

#define ifChkCmdSet(c,v) if (memcmp(lineCMD,c,sizeof(c)-1) == 0) cmdType = v

static void near pascal doXFiles(char * spath, char * sname)
{
    conout0 << endl << "Processing spool " << sname;

    char xname [ 128 + 1 ];
    buildstr(xname,spath,sname,"\\*.X",NULL);

    ffblk xFFB;
    int done = findfirst(xname,&xFFB,FA_ARCH);
    while (!done)
    {
        if (kbhit() && (getkey() == ESC))
            abortFlag = TRUE;

        if (!abortFlag)
        {
            if ((uucpDebug == 0) || (uucpDebug >= 10))
                logFile.printf("  *.X = %s\r\n",xFFB.ff_name);

            conout1 << endl << "  Executing uuxqt " << xFFB.ff_name;

            buildstr(xname,spath,sname,"\\",xFFB.ff_name,NULL);

            cDOSFILE xfile;
            xfile.open(xname,OPEN_READ|OPEN_DENYRDWR);

            char line [ 256 + 1 ];

            int  retStat = FALSE;

            defInitStrBuf(lineUNAME,   64);
            defInitStrBuf(lineUSYS,    64);
            defInitStrBuf(lineRETADDR,120);
            defInitStrBuf(lineJOB,     32);
            defInitStrBuf(lineFILE,    12);
            defInitStrBuf(lineINPUT,   12);
            defInitStrBuf(lineCMD,    256);

            int llen;
            unsigned actual;
            while ((llen = readline(xfile,line,sizeof(line),actual)) != -1)
            {
               if (llen > 0) {
                  switch (line[0]) {
                    case 'U': strcpy(lineUNAME,strtok(line+2," "));
                              strcpy(lineUSYS, strtok(NULL,  " ")); break;

                    case 'N': retStat = FALSE;                      break;

                    case 'Z': retStat = TRUE;                       break;

                    case 'R': strcpy(lineRETADDR,line+2);           break;

                    case 'J': strcpy(lineJOB,line+2);               break;

                    case 'F': munge(lineFILE,line+2,sname);         break;

                    case 'I': munge(lineINPUT,line+2,sname);        break;

                    case 'C': strcpy(lineCMD,line+2);               break;

                    case '#':
                    case 'e':
                    default:                                        break;
                  }
               }
            }

            xfile.close();

            stripleft(lineCMD,'!');

            const int cmdNONE  = 0;
            const int cmdRMAIL = 1;
            const int cmdRNEWS = 2;

            int cmdType = cmdNONE;

                 ifChkCmdSet("rmail ",cmdRMAIL);
            else ifChkCmdSet("rnews", cmdRNEWS);

            int retValue = 0;

            switch (cmdType)
            {
                case cmdRMAIL:
                    retValue = rmail(spath,sname,lineINPUT,lineCMD+6);
                    break;

                case cmdRNEWS:
                    retValue = rnews(spath,sname,lineINPUT);
                    break;

                case cmdNONE:
                    conerr << endl << "No Command (" << lineCMD <<
                        ") Found for " << xname;
                    break;

                default:
                    conerr << endl << "Command (" << cmdType <<
                        ") Not Processed for " << xname;
                    break;
            }

            retStat = ((retValue != 0) && retStat);

            if (retStat)
                switch (cmdType)
                {
                    case cmdRMAIL:
                        break;

                    case cmdRNEWS:
                        break;
                }

            if (killFiles && (retValue == 0))
                unlink(xname);
            else if ((cmdType == cmdRNEWS) && (retValue == -1))
            {
                char sLine [ 128*2 + 1 ];
                strcpy(sLine,xname);
                strcpy(strrchr(sLine,'.'),".BAX");
                rename(xname,sLine);
            }
        }

        done = findnext(&xFFB);
    }
}

    /*--------------------------------------------------------------------*/

static void near pascal doBags(char * spath, char * sname)
{
    char bname [ 128 + 1 ];
    buildstr(bname,spath,sname,"\\*.BAG",NULL);

    ffblk bFFB;
    int done = findfirst(bname,&bFFB,0);
    while (!done)
    {
        if (kbhit() && (getkey() == ESC))
            abortFlag = TRUE;

        if (!abortFlag)
        {
            if ((uucpDebug == 0) || (uucpDebug >= 10))
                logFile.printf("  *.BAG = %s\r\n",bFFB.ff_name);
            rnews(spath,sname,bFFB.ff_name);
        }

        done = findnext(&bFFB);
    }
}

    /*--------------------------------------------------------------------*/

static void near pascal doProcess ( char * spool )
{
    char line [ 1024 + 1 ];
    msgInfo.process(spool,line,sizeof(line));
}

static void near pascal doSpools(void)
{
    char spool [ 128 + 1 ];
    buildstr(spool,PcbData.uucpSpoolPath,"*.*",NULL);

    ffblk sFFB;
    int done = findfirst(spool,&sFFB,FA_DIREC);
    while (!done)
    {
        if (kbhit() && (getkey() == ESC))
            abortFlag = TRUE;

        if ((sFFB.ff_attrib & FA_DIREC) && (sFFB.ff_name[0] != '.') &&
            !abortFlag)
        {
            if ((uucpSystem[0] == NUL)                  ||
                (stricmp(uucpSystem,sFFB.ff_name) == 0) ||
                (stricmp(uucpSystem,"all")        == 0) ||
                (stricmp(uucpSystem,"any")        == 0))
            {
                logFile.printf("Spool = %s\r\n",sFFB.ff_name);
                buildstr(spool,PcbData.uucpSpoolPath,sFFB.ff_name,"\\",NULL);
                msgInfo.load(spool);
                doXFiles(PcbData.uucpSpoolPath,sFFB.ff_name);
                doBags(PcbData.uucpSpoolPath,sFFB.ff_name);
                doProcess(spool);
            }
        }

        done = findnext(&sFFB);
    }
}

    /*--------------------------------------------------------------------*/

char mailPath [ _MAX_PATH ];

static void initout ( void )
{
    buildstr(mailPath,PcbData.uucpSpoolPath,PcbData.uucpEmailHost,
        "\\",NULL);
}

const char * makemfrom ( const char user [] )
{
    // Build the news path (must occur after initial build of lfrom)
    char npath [ 60 + 1 ];
    sprintf(npath,"%s!%s",PcbData.uucpName,user);

    // Get and break down the system time
    time_t now = time(NULL);
    tm msgTime = *localtime(&now);

    // Build the alternate date information
    char altdate [ 60 + 1 ];
    strftime(altdate, sizeof(altdate), "%a %b %d %H:%M:%S %Y", &msgTime);

    // Build the mail "From site!user date" line
    static char mfrom [ 120 + 1 ];
    buildstr(mfrom,npath," ",altdate,NULL);

    return mfrom;
}
const char * makemdate ( void )
{
    // Get and break down the system time
    time_t now = time(NULL);
    tm msgTime = *localtime(&now);

    // Build the alternate date information
    static char mdate [ 60 + 1 ];
    strftime(mdate, sizeof(mdate), "%a, %d %b %y %H:%M:%S ", &msgTime);
    strcat(mdate, PcbData.uucpTimeZone);

    return mdate;
}

const char * makeaddress ( const char user [] )
{
    // Setup address
    static char address [ 120 + 1 ];
    if (PcbData.uucpBangDomain)
    {
        // host!user@domain
        buildstr(address,PcbData.uucpName,"!",user,"@",
            PcbData.uucpDomainName,NULL);
    }
    else if (PcbData.uucpSubDomain)
    {
        // user%host@domain
        buildstr(address,user,"%",PcbData.uucpName,"@",
            PcbData.uucpDomainName,NULL);
    }
    else
    {
        // user@domain
        buildstr(address,user,"@",PcbData.uucpDomainName,NULL);
    }
    return address;
}

const char * makemsgid ( const char service [] )
{
    static char msgid [ 60 + 1 ];

    // Get and break down the system time
    time_t now = time(NULL);
    tm msgTime = *localtime(&now);

    sprintf(msgid,"<%s.%04X%04X.%04X%08lX.uuout@%s>",
        service,
        unsigned((msgTime.tm_year*12U*31U)+((msgTime.tm_mon-1U)*31U)+
            msgTime.tm_mday-1),
        unsigned((msgTime.tm_hour*60+msgTime.tm_min)*30+msgTime.tm_sec/2),
        random(0x7FFF),long(nextSeq+1),PcbData.uucpDomainName);

    return msgid;
}

    /*--------------------------------------------------------------------*/

int main(void)
{
    #ifdef MEMCHECK
      mc_startcheck(erf_standard);
    #endif

    heapfillfree(0x55);

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
        buf[i*2+0] = ' ';
        buf[i*2+1] = outAttr;
    }

    for (i = 0; i < ti.screenheight; ++i)
    {
        buf[(i*ti.screenwidth+(             1-1))*2] =
        buf[(i*ti.screenwidth+(ti.screenwidth-1))*2] = '³';
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
        buf[((outTop-1-1)*ti.screenwidth+i)*2] =
        buf[((outTop+1-1)*ti.screenwidth+i)*2] =
        buf[((outTop+3-1)*ti.screenwidth+i)*2] =
        buf[((outTop+5-1)*ti.screenwidth+i)*2] =
        buf[((outBot+1-1)*ti.screenwidth+i)*2] =
        buf[((errBot+1-1)*ti.screenwidth+i)*2] = ch;
    }

    buf[((              1-1)*ti.screenwidth+(             1-1))*2] = 'Ú';
    buf[((              1-1)*ti.screenwidth+(ti.screenwidth-1))*2] = '¿';
    buf[((ti.screenheight-1)*ti.screenwidth+(             1-1))*2] = 'À';
    buf[((ti.screenheight-1)*ti.screenwidth+(ti.screenwidth-1))*2] = 'Ù';

    puttext(1,1,ti.screenwidth,ti.screenheight,buf);

    conscr << setxy(14,1)               <<
        " UUIN Version 1.30 -- UUCP Import Utility for PCBoard ";
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

    strlwr(PcbData.uucpName);
    strlwr(PcbData.uucpDomainName);

    opencnamesidx(READ_IDX);
    initMsgBaseList();

    initUMWF();

    getSeqNum();

    char logName [ 128 + 1 ];
    buildstr(logName,PcbData.uucpLogPath,"UUIN.LOG",NULL);
    logFile.open(logName,OPEN_WRIT|OPEN_DENYWRIT|OPEN_APPEND);
    logFile.putln("*******************************************************************************");
    strcpy(logName,"START = DDDDDDDD TTTTTTTT");
    datestr (logName+ 8);
    timestr1(logName+17);
    logName[16] = ' ';
    logFile.putln(logName);

    if (uucpJunkOut)
    {
        buildstr(logName,PcbData.uucpPath,"JNKGROUP.LST",NULL);
        junkGroups.open(logName,OPEN_RDWR|OPEN_DENYWRIT|OPEN_APPEND);
    }

    initout();

    doSpools();

    if (uucpJunkOut) junkGroups.close();

    strcpy(logName,"END   = DDDDDDDD TTTTTTTT");
    datestr (logName+ 8);
    timestr1(logName+17);
    logName[16] = ' ';
    logFile.putln(logName);

    logFile.close();

    conout0 << endl << "Processing complete, exiting ...";

    putSeqNum();

    doUMWF();

    resetUMWF();

    deinitMsgBaseList();
    closecnamesidx();
    deinitialize();

    conscr << setattr(0x07);
    conscr.clrscr();
    conscr << "Exiting UUIN ..." << endl;

    #ifdef MEMCHECK
      mc_endcheck();
    #endif

    return 0;
}

/******************************************************************************/

