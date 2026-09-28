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


/***************************************************************************
 *
 * Copyright (c) 1992, 1993.  All Rights Reserved.
 * Clark Development Company, Inc.
 *
 * The following global "#define" statements may be used to enable various
 * features of PCBPack.  Most of these features have a command line switch
 * associated with them so they will be optional for the user.
 *
 *   ALLOCMODE    - Enable packing of messages in "message by message" mode
 *                  where the entire body of a message is read into memory
 *                  and then checked against a duplicate list (if requested)
 *                  and checked for file attachments or carbon copy lists.
 *                  If memory is not available for the text to be loaded,
 *                  then PCBPack falls back to "block by block" mode.
 *
 *   ARCHIVE      - Enable the ability to archive certain messages found in
 *                  the message base.  The criteria
 *
 *   BLOCKMODE    - Enable packing of messages in "block by block" mode where
 *                  PCBPack will read and write each block of a message
 *                  individually instead of on a "message by message" basis.
 *
 *   CHECKDUPS    - Enable checking of duplicate messages.  A 32-bit CRC is
 *                  calculated for each message and if any CRC values match
 *                  the latter is assumed to be a duplicate and is removed.
 *
 *   CLEANUP      - Enable the ability for PCBPack to "clean up" after a
 *                  crash by removing any of the "temporary" files that might
 *                  be left over.
 *
 *   DEBUGMODE    - Enable the use of the /DEBUG command line switch which
 *                  allows display of extra output to aid in debugging.
 *
 *   MAILWAITING  - Enables the ability for PCBPack to update a users mail
 *                  waiting flags so they will be notified of new mail upon
 *                  logging into the system.
 *
 *   DEBUG        - Enable the use of the MemCheck libraries.
 *
 *   NEWINDEX     - Enable PCBPack to use the version 15.0 compatible index
 *                  files.  These files are much different than the old
 *                  index files so to make PCBPack backward/forward
 *                  compatible this option exists.
 *                  (this option isn't implemented at this time)
 *
 *   ONLINE       - Enable PCBPack to be used in an "online" mode.  It will
 *                  interactively ask for criteria to use while packing.
 *                  This option will most likely NOT be available in the
 *
 *   PACKATTACH   - Enable removal of any file attachments that a message may
 *                  contain.
 *
 *   PACKGAPS     - Enable renumbering of messages where some program (either
 *                  a third party utility or otherwise) has created a large
 *                  "gap" of missing message numbers.  Such a "gap" would
 *                  cause an extremely large IDX file to be created and this
 *                  would solve such a problem.
 *
 *   REFNUM       - Enable checking and processing of reference numbers.
 *
 *   RENUMBER     - Enable renumbering of messages.  It is recommended that
 *                  this option NOT be used due to problems it causes with
 *                  Last Message Read pointers.
 *
 *   REPORT       - Enable PCBPack reporting features.  With this option
 *                  enabled, the operator can specify what type of report
 *                  should be generated (if any) when the MSGS files are
 *                  packed.
 *
 *   USECONFIG    - Enable PCBPack to read a configuration file containing
 *                  parameters to be used while packing MSGS files.
 *
 ***************************************************************************/

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
#else
  #include <alloc.h>
#endif

#include <search.h>
#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <screen.h>
#include <string.h>
#include <dosfunc.h>
#include <misc.h>
#ifdef __OS2__
  #include <stdlib.h>
#else
  #include <zsort.h>
#endif
#include "pcb.h"
#include "pcbpack.h"
#include "crc.h"


#ifdef __WATCOMC__
  #undef CHECKDUPS                //  need to finish this!
#endif

#if defined(__BORLANDC__) || defined(__TURBOC__)
  extern struct ffblk DTA;
#elif defined(__WATCOMC__)
  extern struct find_t DTA;
  #define ff_name   name
  #define ff_ftime  wr_time
  #define ff_fdate  wr_date
  #define ff_fsize  size
  #define ff_attrib attrib
#endif

#ifdef __OS2__
  #include <semafore.hpp>
  extern CTimeoutSemaphore TimerSemaphore;
#endif

#define max(a,b)    (((a) > (b)) ? (a) : (b))

// compdate.h defines a macro called COMPILE_DATE which is the date on which
// this module was compiled.  This information is used simply to determine if
// the system clock is a semi-reasonable date.  If the system date appears to
// be valid, then packconf() will use the system date to validate the dates of
// the messages that are found.  If the messages are found to have been left
// in the future by more than MAXFUTUREDAYS then the date of the message is
// changed to TODAY's date and the message is then saved with the new header.
// This way the message won't just "disappear" without ever having been seen.
// Instead, it will stick around until the message becomes "old" by the
// standards set by the sysop using the /DAYS: switch.

#include "compdate.h"
#define  VALIDDATEMIN   (COMPILE_DATE - 1)           // don't forget GW dateline
#define  VALIDDATEMAX   (COMPILE_DATE + (365 * 4))   // allow max of 4 years, after that, the feature will be disabled
#define  MAXFUTUREDAYS  30   // max days into future before message is re-dated

#ifdef DEBUG
#include <memcheck.h>
#endif


#ifdef PACKGAPS
  bool PackedGap = FALSE;  // set to true if any gaps are ever packed
#endif

void LIBENTRY updatestatusline(char *Str, char Color) {
  clsbox(0,24,47,24,Color);
  fastprint(0,24,Str,Color);
}


char * LIBENTRY getfilenamewithoutpath(char *FileName) {
  char *p;

  if ((p = strrchr(FileName,'\\')) != NULL || (p = strrchr(FileName,':')) != NULL)
    return(p+1);
  return(FileName);
}


int LIBENTRY removefile(char *FileName) {
  char Str[80];
  char *p;

  p = getfilenamewithoutpath(FileName);

  setpacktimer(SIXTYSECONDS);
  if (fileexist(FileName) != 255) {
    sprintf(Str,"Removing: %s",p);
    updatestatusline(Str,0x3C);
    while (remove(FileName) == -1) {
      if (packtimerexpired(ONESECOND) || Escape()) {
        DisplayLine(TRUE,ScreenColor->Intense,"%s Removal Aborted",FileName);
        return(-1);
      }
      giveup();
    }
  }
  return(0);
}


int LIBENTRY renamefile(char *OldName,char *NewName) {
  char Str[80];
  char *op;
  char *np;

  op = getfilenamewithoutpath(OldName);
  np = getfilenamewithoutpath(NewName);

  setpacktimer(SIXTYSECONDS);
  if (fileexist(OldName) != 255) {
    sprintf(Str,"Renaming: %s to %s",op,np);
    updatestatusline(Str,0x3C);
    while (rename(OldName,NewName) == -1) {
      if (packtimerexpired(ONESECOND) || Escape()) {
        DisplayLine(TRUE,ScreenColor->Intense,"%s Rename Aborted",OldName);
        return(-1);
      }
      giveup();
    }
  }
  return(0);
}


int LIBENTRY resynch(DOSFILE *mfp) {
  unsigned       MsgDate;
  long           Pos;
  long           MsgNum;
  msgheadertype  Header;

  // move the read pointer to 1 byte past the start of the last attempted
  // header block
  Pos = dosfseek(mfp,0,SEEK_CUR) - sizeof(msgheadertype) + 1;
  dosfseek(mfp,Pos,SEEK_SET);

  while ((dosfread(&Header,sizeof(msgheadertype),mfp) == sizeof(msgheadertype))) {
    // check for a valid header
    if (ValidHeader(&Header,&MsgNum,&MsgDate)) {
      dosfseek(mfp,Pos,SEEK_SET);
      return(0);
    }

    Pos++;

    // don't check for escape if we're compressing!
    if (! Compress) {
      // otherwise, check for an escape only once every 255 bytes
      if ((Pos & 0xFF) == 0xFF && Escape())
        return(-1);
    }

    // move forward one byte and check for another valid header
    dosfseek(mfp,Pos,SEEK_SET);
  }

  return(-1);
}


#pragma warn -bei
static exthdrtype _NEAR_ LIBENTRY getextheadertype(char *Function) {
  exthdrtype TestType;

  switch(Function[0]) {
    case 'A': switch(Function[3]) {
                case 'A': TestType = EXTHDR_ATTACH;  break;
                case 'R': TestType = EXTHDR_ACKRR;   break;
                case 'N': TestType = EXTHDR_ACKNAME; break;
                default : return((exthdrtype) 0xFFFF);
              }
              break;
    case 'F': switch (Function[4]) {
                case ' ': TestType = EXTHDR_FROM;   break;
                case '2': TestType = EXTHDR_FROM2;  break;
                default : return((exthdrtype) 0xFFFF);
              }
              break;
    case 'L': TestType = EXTHDR_LIST;
              break;
    case 'O': TestType = EXTHDR_ORIGIN;
              break;
    case 'P': if (IgnorePackoutDate)
                return((exthdrtype) 0xFFFF);
              TestType = EXTHDR_PACKOUT;
              break;
    case 'R': switch(Function[1]) {
                case 'O': TestType = EXTHDR_ROUTE;  break;
                case 'E': TestType = EXTHDR_REQRR;  break;
                default : return((exthdrtype) 0xFFFF);
              }
              break;
    case 'S': TestType = EXTHDR_SUBJECT;
              break;
    case 'T': switch (Function[2]) {
                case ' ': TestType = EXTHDR_TO;  break;
                case '2': TestType = EXTHDR_TO2; break;
                default : return((exthdrtype) 0xFFFF);
              }
              break;
    default : return((exthdrtype) 0xFFFF);
  }

  if (memcmp(Function,ExtHdrFunctions[TestType],EXTFUNCLEN) == 0)
    return(TestType);
  return((exthdrtype) 0xFFFF);
}


char * LIBENTRY findendofmsg(char *Text, unsigned Len) {
   char     *p;
   unsigned  Count;
   bool      HasKludgeLines = FALSE;

  // first try to find a 'tear line' inside the message body
  // scan for the first occurance of either \0xE3---\0xE3 or \0xE3-!-\0xE3
  // NOTE:  Use LineSeparator instead of \0xE3
  for (p = Text, Count = Len; Count > 0; p++, Count--) {
    if (*p == LineSeparator) {
      if (p[1] == '-') {
        if (p[2] == '-' || p[2] == '!') {
          if (p[3] == '-') {
            if (p[4] == LineSeparator)
              return(p);
          }
        }
      } else {
        // check for lines that start with a CTRL-A in the first position
        if (p[1] == 1)
          HasKludgeLines = TRUE;
      }
    }
  }

  // not having found a tear line, let's scan backwards looking for the
  // very last LineSeparator
  for (p = Text+Len-1, Count = Len; Count > 0; p--, Count--) {
    if (*p == LineSeparator) {
      // if we found any kludge lines (lines that start with CTRL-A) then
      // we'll scan backwards further to find out if this is a kludge line
      // we'll return a pointer to the end of the first non-kludge line
      if (HasKludgeLines) {
        char *q;
        char *r;
        // continue scanning backwards
        for (q = p-1, r = p, Count--; Count > 0; q--) {
          // check for the end of the 'next previous' line and see if the
          // start of the line is a CTRL-A and if not, return a pointer to it
          // NOTE:  The very last line written may end with two \xE3's, so
          // skip that line, if found
          if (*q == LineSeparator) {
            if (q[1] != 1 && q[1] != LineSeparator) {
              // return a pointer to the last non-kludge-line end of line
              return(r);
            }
            // remember the last non-kludge-line end of line found
            r = q;
          }
        }

        // If the kludge lines were not at the end of the message, then we'll
        // end up landing here, so return the pointer we were going to return
        // had there been no kludge lines found
        return(p);
      } else {
        // no kludgelines found, so return pointer to end of last line found
        return(p);
      }
    }
  }

  // as a default, return a pointer to the start of the message -- however,
  // this should *never* happen
  return(Text);
}


/***************************************************************************
 *** Pack unwanted messages from MSGS file *********************************/
int LIBENTRY PackConference(unsigned Conf) {
    char static         Zero[4] = { 0x00, 0x02, 0x00, 0x00 };
    bool                MakeOldIndex = OldIndex;
    bool                WriteToDisk;
//  char                Drive;
    unsigned            MsgDate;
    unsigned            MaxFutureDate;
    unsigned            Today;
    int                 i;
    int                 MessageStatus = MSG_KEPT;
    int                 NumBlocks;
    int                 OpenFlags;
    int                 MaxBlocks;
    #ifdef  CHECKDUPS
    int                 RemoveDups = RemoveDuplicates;
    #endif
    int                 Seconds;
    int                 BadBlocks = 0;
    unsigned int        MaxCRCsInArray;
/*  char                *pth; */
    char                *Text = NULL;
    long                CRCsArraySize;
    long                MsgNum  = 0L;
    long                LastNum;
    #ifdef  RENUMBER
    long                LRenumber = Renumber;
    #endif
    long                NumKept = 0L;
    long                NumProcessed = 0L;
    long                NumRemoved = 0L;
    long                NumActive = 0L;
    long                NumActiveStart = 0L;
    unsigned long       FreeSpaceNeeded;
    unsigned long       FreeSpaceAvailable;
    long                ReadOffset;
    long                WriteOffset  = 128L;
    #ifdef  REFNUM
    long                HighMark;
    long                LowMark;
    long                RefNum;
    #endif
    #ifdef  CHECKDUPS
    long                FileSize;
    #endif
    #ifdef CHECKDUPS
    crctype _HUGE_     *crcList = NULL;
    crchdrtype          crcHeader;
    #endif
    #ifdef  PACKATTACH
    char                *FileName;
    char                *EndName;
    #endif
    #ifdef  REFNUM
    char                *RefList = NULL;
    #endif
    DOSFILE             imfp;
    DOSFILE             omfp;
    DOSFILE             onfp;
    DOSFILE             oifp;
    #ifdef CHECKDUPS
    DOSFILE             cfp;
    #endif
    #ifdef  CHECKDUPS
    char                crcFile[MAX_FILENAME];
    #endif
    char                idxTemp[MAX_FILENAME];
    char                idxFile[MAX_FILENAME];
    char                idxBack[MAX_FILENAME];
    char                msgTemp[MAX_FILENAME];
    char                msgFile[MAX_FILENAME];
    char                msgBack[MAX_FILENAME];
    char                ndxTemp[MAX_FILENAME];
    char                ndxFile[MAX_FILENAME];
    char                ndxBack[MAX_FILENAME];
    #ifdef PACKATTACH
    char                AttachFile[MAX_FILENAME];
    #endif
    char                str[100];

    pcbconftype         ConfRec;
    msgbasediskstattype MsgDiskStats;
    msgbasestattype     MsgStats;
    msgheadertype       Header;
    msgextendtype       *ExtInfo;
    newindextype        Index;
//  struct dfree        df;

#ifdef PACKEDGAPS
    PackedGap = FALSE;   // set to TRUE if a GAP is ever packed
#endif

    AttachFile[0] = 0;

    if (Escape()) {
      AbortPack = TRUE;
      return(FALSE);
    }

    // if we are compressing, then default to NOT writing to disk,
    // otherwise if not compressing then we ALWAYS write to disk.
    WriteToDisk = (bool) (! Compress);

    getconfrecord(Conf, &ConfRec);

    if (OldIndex == FALSE)
      MakeOldIndex = ConfRec.OldIndex;

#ifdef  DEBUGMODE
    if (DebugLevel >= 25) {
      DebugDisplayLine(ScreenColor->Intense, "*** Conference : %-15s : %-5u ***", ConfRec.Name, Conf);
      DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    if (ConfRec.MsgFile[0]== 0 || ConfRec.Name[0] == 0) {
#ifdef  DEBUGMODE
      if (DebugLevel >= 25) {
        DebugDisplayLine(ScreenColor->Intense, "No MSGS file to pack.");
        DebugDisplayLine(ScreenColor->Intense, "");
      }
#endif
      return FALSE;
    }

    if (fileexist(ConfRec.MsgFile) == 255)
      return FALSE;

    FreeSpaceNeeded = DTA.ff_fsize;  // record the free space needed

#ifdef  DEBUGMODE
    if (DebugLevel >= 100) {
      DebugDisplayLine(ScreenColor->Intense, "Using ALLOC mode");
      DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    /*** MSGS file *********************************************************/
    strcpy(msgFile, ConfRec.MsgFile);
    strupr(msgFile);

    strcpy(msgBack, msgFile);
    AddExtension(msgBack, EXT_BACKUP);

    strcpy(msgTemp, msgFile);
    AddExtension(msgTemp, EXT_TEMP);

    /*** IDX file **********************************************************/
    strcpy(idxFile, msgFile);
    AddExtension(idxFile, EXT_IDX);

    strcpy(idxBack, idxFile);
    AddExtension(idxBack, EXT_IDXBAK);

    strcpy(idxTemp, idxFile);
    AddExtension(idxTemp, EXT_IDXTEMP);

    /*** NDX file **********************************************************/
    if (MakeOldIndex) {
        strcpy(ndxFile, msgFile);
        AddExtension(ndxFile, EXT_NDX);

        strcpy(ndxBack, ndxFile);
        AddExtension(ndxBack, EXT_NDXBAK);

        strcpy(ndxTemp, ndxFile);
        AddExtension(ndxTemp, EXT_NDXTEMP);
    }

#ifdef  DEBUGMODE
    if (DebugLevel) {
      DebugDisplayLine(ScreenColor->Intense, "MSGS      = [%s]", msgFile);
      DebugDisplayLine(ScreenColor->Intense, "MSGS.BAK  = [%s]", msgBack);
      DebugDisplayLine(ScreenColor->Intense, "MSGS Temp = [%s]", msgTemp);

      DebugDisplayLine(ScreenColor->Intense, "MSGS.IDX  = [%s]", idxFile);
      DebugDisplayLine(ScreenColor->Intense, "MSGS.IBK  = [%s]", idxBack);
      DebugDisplayLine(ScreenColor->Intense, "IDX Temp  = [%s]", idxTemp);

      if (MakeOldIndex) {
        DebugDisplayLine(ScreenColor->Intense, "MSGS.NDX  = [%s]", ndxFile);
        DebugDisplayLine(ScreenColor->Intense, "MSGS.NBK  = [%s]", ndxBack);
        DebugDisplayLine(ScreenColor->Intense, "NDX Temp  = [%s]", ndxTemp);
      }

      DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    DisplayConference(Conf, &ConfRec);

    // if we're compressing the file, then we don't care how much disk space
    // is available because we'll either use the same amount -- or LESS.
    if (! Compress) {

      FreeSpaceAvailable = diskfreespace(msgFile);

      if (FreeSpaceNeeded > FreeSpaceAvailable) {
          /*** possibly not enough space... **********************************/
          DisplayLine(TRUE, ScreenColor->Intense, "Not enough space to pack.");
          DisplayLine(TRUE, ScreenColor->Intense, "%s bytes available.",comma(str,FreeSpaceAvailable));
          DisplayLine(TRUE, ScreenColor->Intense, "%s bytes needed.", comma(str,FreeSpaceNeeded));
          DisplayLine(TRUE, ScreenColor->Intense, "");
      } else {
          /*** should be plenty of space... **********************************/
          DisplayLine(TRUE, ScreenColor->Intense, "%s bytes available.",comma(str,FreeSpaceAvailable));
          DisplayLine(TRUE, ScreenColor->Intense, "");
      }
    }

    // if Compress, then we'll open the same file twice, once for reading and once for writing,
    // otherwise, we'll open a temporary file and lockout any reading or writing
    OpenFlags = OPEN_READ|OPEN_NOERRCHK|(Compress ? OPEN_DENYREAD : OPEN_DENYRDWR);

    updatestatusline("Attempting to open MSGS file...                                Seconds:", ScreenColor->Status);
    for (Seconds = Timeout; dosfopen(msgFile, OpenFlags, &imfp) == -1; Seconds--) {
        char    tmpStr[30];

        if (Seconds <= 0) {
          clsbox(0,24,79,24,ScreenColor->Status);
          return FALSE;
        }
        sprintf(tmpStr, "%4d", Seconds);
        fastprint(74, 24, tmpStr, ScreenColor->StatusHighlight);
        if (waitonesecond()) {
          clsbox(0,24,79,24,ScreenColor->Status);
          return FALSE;
        }
    }

    dossetbuf(&imfp,16384);

    dosfread(&MsgDiskStats, sizeof(msgbasediskstattype), &imfp);
    dosfseek(&imfp, sizeof(msgheadertype) - sizeof(msgbasediskstattype), SEEK_CUR);
    ReadOffset = sizeof(msgheadertype);

    MsgStats.LowMsgNum                  = bassngltolong(MsgDiskStats.LowMsgNum);
    MsgStats.HighMsgNum                 = bassngltolong(MsgDiskStats.HighMsgNum);
    NumActive = MsgStats.NumActiveMsgs  = bassngltolong(MsgDiskStats.NumActiveMsgs);
    NumActiveStart = NumActive;

    if ((MsgStats.LowMsgNum > MAX_MSGNUM)     || (MsgStats.LowMsgNum < 0)  ||
        (MsgStats.HighMsgNum > MAX_MSGNUM)    || (MsgStats.HighMsgNum < 0) ||
        (MsgStats.NumActiveMsgs > MAX_MSGNUM) || (MsgStats.NumActiveMsgs < 0)) {
            MsgStats.LowMsgNum = 0L;
            MsgStats.HighMsgNum = 0L;
            MsgStats.NumActiveMsgs = 0L;
    }

    if (MsgStats.NumActiveMsgs < MinimumPackThreshold) {
      dosfclose(&imfp);
      clsbox(0,24,79,24,ScreenColor->Status);
      DisplayLine(TRUE, ScreenColor->Warning, "Conference (%u) Below Threshold (Active=%ld,Threshold=%u)", Conf,MsgStats.NumActiveMsgs,MinimumPackThreshold);
      return (FALSE);
    }

    clsbox(0,24,79,24,ScreenColor->Status);
    sprintf(str, "Conference (%u) -- %s", Conf, ConfRec.Name);
    updatestatusline(str,ScreenColor->Status);


    // if we are compressing the file, then we'll open the same file again, but in write-only mode
    // otherwise, we'll create a new temp file and open it with exclusive access
    OpenFlags = OPEN_WRIT|(Compress ? OPEN_DENYWRIT : OPEN_DENYRDWR|OPEN_CREATE);

    if (dosfopen((Compress ? msgFile : msgTemp), OpenFlags, &omfp) == -1) {
        dosfclose(&imfp);
        return FALSE;
    }

    dossetbuf(&omfp,32768U);

    // if we're compressing then there's no need to write the stats to disk
    // because it's already there, otherwise we've just created a new file
    // and need to write the information out now and update it later.
    if (! Compress)
      dosfwrite(&MsgDiskStats, sizeof(msgbasediskstattype), &omfp);

    // if compressing, open the existing file (creating will delete it).
    // otherwise, open a temporary file
    if (dosfopen((Compress ? idxFile : idxTemp), OPEN_CREATE|OPEN_WRIT|OPEN_DENYRDWR, &oifp) == -1) {
      dosfclose(&imfp);
      dosfclose(&omfp);
      return FALSE;
    }

    dossetbuf(&oifp,16384);

    if (MakeOldIndex) {
      // if compressing, open the existing file (creating will delete it).
      // otherwise, open a temporary file

      if (dosfopen((Compress ? ndxFile : ndxTemp), OPEN_CREATE|OPEN_WRIT|OPEN_DENYRDWR, &onfp) == -1) {
        MakeOldIndex = FALSE;
      } else {
        unsigned  i;

        updatestatusline("Creating Empty Old Index File...                               Seconds:", ScreenColor->Status);
        dossetbuf(&onfp, 8192);
        for (i = 0; i < (ConfRec.MsgBlocks * 1024); i++)
          dosfwrite(Zero, sizeof(bassngl), &onfp);

        dosrewind(&onfp);
        // redisplay the status line with the conference name and number
        updatestatusline(str,ScreenColor->Status);
      }
    }


    if (Compress) {
      // if compressing, the file already exists, including the header, so
      // just seek to the end of the header
      dosfseek(&omfp,sizeof(msgheadertype),SEEK_SET);
    } else {
      // otherwise, we have to create a temporary header to save some space
      // while we write the messages to the file.
      for (i = 0; i < sizeof(msgheadertype) - sizeof(msgbasediskstattype); i++) {
        if (dosfwrite(" ", sizeof(char), &omfp) == -1) {
          AbortPack = TRUE;
          goto Abort;
        }
      }
    }

    memset(&Index, 0, sizeof(Index));


#ifdef  REFNUM
    LowMark     = MsgStats.LowMsgNum;
    HighMark    = MsgStats.HighMsgNum;
    if ((RefList = (char *) calloc((((unsigned)(HighMark - LowMark) + 7) / 8) + 1, sizeof(char))) == NULL) {
      DisplayLine(TRUE, ScreenColor->Warning, "Insufficient memory to check for reference numbers");
    }
#endif

#ifdef  DEBUGMODE
    if (DebugLevel >= 5) {
      DebugDisplayLine(ScreenColor->Intense, "Low Message Number        : %ld", MsgStats.LowMsgNum);
      DebugDisplayLine(ScreenColor->Intense, "High Message Number       : %ld", MsgStats.HighMsgNum);
      DebugDisplayLine(ScreenColor->Intense, "Number of Active Messages : %ld", NumActive);
      DebugDisplayLine(ScreenColor->Intense, "");
    }
#endif

    WriteOffset  = sizeof(msgheadertype);
    LastNum = MsgStats.LowMsgNum - 1;
    MsgStats.LowMsgNum      = 0L;
#ifdef  RENUMBER
    if (LRenumber)
        MsgStats.LowMsgNum  = LRenumber;
#endif
    MsgStats.NumActiveMsgs  = 0L;

#ifdef  CHECKDUPS
/*-------------------------------------------------------------------------*/
/***************************************************************************
 ***************************************************************************
 At this point we check the message base for any duplicates that may have
 been placed in the message base.  Any messages that are found to duplicate
 other messages are flagged for removal from the message base.
 ***************************************************************************
 ***************************************************************************/
    if (RemoveDups) {

/***************************************************************************
 Now if we're storing a CRC database, let's open the sucker up and find out
 how many CRC values it contains...
 ***************************************************************************/
        if (CRCDays) {
            /*** load CRCs from disk ***************************************/
            DisplayLine(TRUE, ScreenColor->Intense, "Loading Message CRCs...");

            strcpy(crcFile, msgFile);
            AddExtension(crcFile, EXT_CRC);

            if (dosfopen(crcFile, OPEN_RDWR|OPEN_DENYNONE, &cfp) == -1) {
                /*** open call failed!!! ***********************************/
                crcHeader.Num = 0;
                crcHeader.LastMsgNum = 0L;
            } else {
                dossetbuf(&cfp, 8192);
                FileSize = dosfseek(&cfp, 0L, SEEK_END);
                dosrewind(&cfp);
                if (dosfread(&crcHeader, sizeof(crchdrtype), &cfp) < 1) {
                    crcHeader.Num = 0;
                    crcHeader.LastMsgNum = 0L;
                } else {
                    if ((FileSize - (long)sizeof(crchdrtype)) != (crcHeader.Num * (long)sizeof(crctype))) {
                        DisplayLine(TRUE, ScreenColor->Intense, "CRC File is corrupted : Size (%ld) CRCs (%u)", FileSize, crcHeader.Num);
                        DisplayLine(TRUE, ScreenColor->Intense, "");
                        crcHeader.Num = 0;
                        crcHeader.LastMsgNum = 0L;
                    }
                }
            }
        } else {
            /*** No CRC file specified for use *****************************/
            crcHeader.Num = 0;
            crcHeader.LastMsgNum = 0L;
        }

/***************************************************************************
 Now let's allocate memory for all of our CRC values so we can check them...
 ***************************************************************************/
        if (NumActive == 0) {
            if (CRCDays) {
                dosfclose(&cfp);
            }
            RemoveDups = FALSE;
            goto EndDupCheck;
        }
        MaxCRCsInArray = crcHeader.Num + (unsigned int) NumActive + 100;  /* 100 is a fudge factor */
        CRCsArraySize  = (long) MaxCRCsInArray * sizeof(crctype);
        if ((crcList = (crctype *) farmalloc(CRCsArraySize)) == NULL) {
            DisplayLine(TRUE, ScreenColor->Warning, "Insufficient memory to check for duplicates");
            if (CRCDays) {
                dosfclose(&cfp);
                CRCDays = 0;
            }
            RemoveDups = FALSE;
            goto EndDupCheck;
        }

/***************************************************************************
 We've allocated our memory, now let's load the CRC values from the CRC
 database if it exists...
 ***************************************************************************/
        if (CRCDays) {
            unsigned    j;              /*** local loop variable ***********/

            /* skip over header to position at first CRC value */
            dosfseek(&cfp, sizeof(crchdrtype), SEEK_SET);

            for (j = 0; j < crcHeader.Num; j++) {
                if (dosfread((void *) (crcList + j), sizeof(crctype), &cfp) == -1)
                  break;
            }
            dosfclose(&cfp);
        }

    }
/*-------------------------------------------------------------------------*/
EndDupCheck:
#endif

    dosrewind(&imfp);
    dosfseek(&imfp, sizeof(msgheadertype), SEEK_CUR);
    sprintf(str, "Kept:        Removed:      ");
    fastprint(48, 24, str, ScreenColor->Status);

    // if the /KILLALL switch was used TOGETHER WITH the /FAST switch, then
    // just skip the read pointer to the end of the file and let the normal
    // processing finish everything else up (updating indexes, etc)

    if (RemoveAll && QuietPack) {
      dosfseek(&imfp,0,SEEK_END);
      WriteToDisk = TRUE;
    }

    AbortPack = FALSE;

    Today = getjuliandate();

    // check the validity of the date on the system clock
    if (Today >= VALIDDATEMIN && Today <= VALIDDATEMAX) {
      // Set the maximum future date
      MaxFutureDate = Today + MAXFUTUREDAYS;
    } else {
      // Disable the max future date checking
      MaxFutureDate = 0;
    }


    #ifdef __OS2__
      // Under OS/2, we have plenty of memory, so we'll just allocate the
      // Text pointer right up right then set MaxBlocks to a value that is
      // higher than can ever be reached so that the Text pointer will never
      // have to be reallocated.  (NOTE:  If this allocation fails, and it
      // shouldn't, but if it does, then we'll work the same way as we do
      // under DOS, letting the first message encountered do the allocation.
      MaxBlocks = 256;
      if ((Text = (char *) malloc((MaxBlocks - 1) * sizeof(msgheadertype))) == NULL)
        MaxBlocks = 0;
    #else
      // Initialize MaxBlocks to 0.  The Text pointer will be allocated based
      // on the largest block encountered, we'll seed it with 0 and let the
      // first message encountered determine its initial size.
      MaxBlocks = 0;
    #endif


/***************************************************************************
 ***************************************************************************
 This is where we begin the process of reading each message and determining
 if it is to removed or not.  Messages that are to be kept are written to a
 temporary file and then if the entire packing process suceeds the temporary
 file replaces the actual message base.
 ***************************************************************************
 ***************************************************************************/
    while (! AbortPack) {
        if (Compress && ! WriteToDisk)
          ReadOffset = dosfseek(&imfp,0,SEEK_CUR);

        if ((dosfread(&Header, sizeof(msgheadertype), &imfp) != sizeof(msgheadertype)))
          break;

#ifdef  DEBUGMODE
        if (DebugLevel >= 25) {
          DebugDisplayLine(ScreenColor->Warning , "NumActive: %ld", NumActive);
        }
#endif

        // check for a valid header, plus get the current message number & date
        if (ValidHeader(&Header,&MsgNum,&MsgDate)) {

            NumProcessed++;

            if (!LRenumber)
              if (MsgNum <= LastNum)
                continue;

            /*** Press ESC to abort ****************************************/
            if (! Compress) {
              // we can't check for ESC if we're compressing, instead we
              // have to finish the conference.
              if (Escape()) {
                AbortPack = TRUE;
                goto Abort;
              }
            }

            MessageStatus = MSG_KEPT;

#ifdef  PACKGAPS
            /* NOTE:  don't use PACKGAPS if renumbering is possible */

            if (MsgNum > LastNum + MsgGap && LastNum) {
                MsgNum = LastNum + 1;
                PackedGap = TRUE;
            }
#endif

            if (!LRenumber) {
                if (MsgStats.LowMsgNum) {
                    if (MsgNum > LastNum + 1) {
                        memset(&Index, 0, sizeof(newindextype));
                        for (; LastNum < MsgNum - 1; LastNum++) {
                            if (dosfwrite(&Index, sizeof(newindextype), &oifp) == -1) {
                                AbortPack = TRUE;
                                goto Abort;
                            }
                            if (MakeOldIndex) {
                                if (dosfwrite(Zero, sizeof(Zero), &onfp) == -1) {
                                    dosfclose(&onfp);
                                    MakeOldIndex = FALSE;
                                }
                            }
                            if (! Compress) {
                              // we can't check for ESC if we're compressing, instead we
                              // have to finish the conference.
                              if (Escape()) {
                                AbortPack = TRUE;
                                goto Abort;
                              }
                            }
                        }
                    }
                }
            }

            NumBlocks = Header.NumBlocks;

            if (NumBlocks > MaxBlocks) {
              if (Text != NULL) {
                free(Text);
                Text = NULL;
              }

              MaxBlocks = NumBlocks;
              if ((Text = (char *) malloc((NumBlocks - 1) * sizeof(msgheadertype))) == NULL) {
                DisplayLine(TRUE, ScreenColor->Warning, "Message too large to fit in memory : %ld", MsgNum);
                MaxBlocks = 0;
              }
            }

            if (Text != NULL) {
              if (dosfread(Text, (NumBlocks - 1) * sizeof(msgheadertype), &imfp) == -1) {
                AbortPack = TRUE;
                goto Abort;
              }
            }
noBlock:

            if (Days) {
              if (MsgDate < Today - Days)
                MessageStatus = MSG_REMOVED;
              else if (MaxFutureDate && MsgDate > MaxFutureDate) {
                // the system date has been validated, a MaxFutureDate has
                // been determined, and the date found on this message has
                // been found to EXCEED that date (i.e. the date on this
                // message is an unreasonable futuristic date) so let's change
                // the date on the message to TODAY's date and that way the
                // message will be able to be packed out after "Days" number of
                // days have passed.
                memcpy(Header.Date,juliantodate((short) Today),8);
                MsgDate = Today;
              }
            }


            #ifdef  CHECKDUPS
            if (RemoveDups && MsgNum > crcHeader.LastMsgNum && Header.ActiveFlag == MSG_ACTIVE) {
                if (Text != NULL) {
                   /*********************************************************
                    Get the CRC value of the current message in memory...
                    *********************************************************/
                    char    *p;
                    char     Temp[76];
                    crctype  NewMsg;

                    p = findendofmsg(Text,(NumBlocks - 1) * sizeof(msgheadertype));

                    memcpy(Temp   ,Header.ToField  ,25);
                    memcmp(Temp+25,Header.FromField,25);
                    memcmp(Temp+50,Header.SubjField,25);
                    Temp[75] = 0;

                    // force the header to all upper case so that the CRC's
                    // will be comparable!
                    strupr(Temp);

                    NewMsg.CRC  = updblkcrc32(Temp,75,0xFFFFFFFFL);
                    NewMsg.CRC  = updblkcrc32(Text,(int)(p - Text),NewMsg.CRC);
                    NewMsg.Date = (short) MsgDate;
                    NewMsg.MsgNumber = MsgNum;

                    if (crcHeader.Num) {
                        crctype     *Dup;
                        unsigned int Num;

                        Num = crcHeader.Num;
                        if ((Dup = (crctype *) lfind(&NewMsg, (void *) crcList, &Num, sizeof(crctype), CompareCRC)) == NULL) {
                            /*** add it to the database ********************/
                            *(crcList + crcHeader.Num)  = NewMsg;
                        } else {
                            /*** KILL IT!!! ********************************/
                            MessageStatus = MSG_REMOVED;
                            Header.ActiveFlag = MSG_INACTIVE;
                            DisplayLine(TRUE, ScreenColor->Duplicate, "Message %ld duplicates %ld", NewMsg.MsgNumber, Dup->MsgNumber);
                            goto PackItNow;
                        }
                    } else {
                        *crcList = NewMsg;
                    }

                    crcHeader.LastMsgNum = MsgNum;
                    if (crcHeader.Num < MaxCRCsInArray)
                      crcHeader.Num++;
                }
            }
            #endif

            #ifdef  DEBUGMODE
            if (DebugLevel >= 50)
              DebugDisplayLine(ScreenColor->Intense, "Checking Headers: 0x%08X", Text);
            #endif

            if (Text != NULL) {
              for (ExtInfo = (msgextendtype *)Text; ExtInfo->Ident == EXTHDRID; ExtInfo++) {
                switch (getextheadertype(ExtInfo->Function)) {
                  case EXTHDR_LIST: {
                       msgcarbontype   *CarbonInfo;

                       CarbonInfo = (msgcarbontype *)ExtInfo;
                       if (CarbonInfo->Status == EXTSTAT_NONE) {
                         if (Header.ActiveFlag == MSG_INACTIVE)
                             MessageStatus = MSG_REMOVED;
                       }
                       break;
                  #ifdef PACKATTACH
                  case EXTHDR_ATTACH: {
                       msgextendtype   ExtAttachInfo;

                       memcpy(&ExtAttachInfo, ExtInfo, sizeof(msgextendtype));

                       /*** store filename to purge (if necessary) ***/
                       if ((FileName = strchr(ExtAttachInfo.Descript, ' ')) != NULL) {
                         if ((FileName = strchr(FileName+1, ' ')) != NULL) {
                           FileName++;
                           if ((EndName = strchr(FileName, ' ')) != NULL)
                             *EndName = 0;
                             sprintf(AttachFile, "%s%s", ConfRec.AttachLoc, FileName);
                           }
                         }
                       }

                       #ifdef DEBUGMODE
                       if (DebugLevel >= 25)
                         DebugDisplayLine(ScreenColor->Warning, "File Attachment: %s", FileName);
                       #endif
                       break;
                  #endif
                  case EXTHDR_PACKOUT:
                       #ifdef  DEBUGMODE
                       if (DebugLevel >= 75) {
                         DebugDisplayLine(ScreenColor->Warning, "Packout date found.");
                       }
                       #endif
                       MessageStatus = MSG_KEPT;
                       if (Today >= datetojulian(ExtInfo->Descript))
                         MessageStatus = MSG_REMOVED;
                       else if (Header.ActiveFlag != MSG_ACTIVE)
                         MessageStatus = MSG_REMOVED;
                       break;
                  case EXTHDR_ACKRR:
                       if (Header.Status == MSG_RCVR_READ)
                         MessageStatus = MSG_REMOVED;
                       break;
                  case EXTHDR_SUBJECT:
                       if (ToUpperCase) {
                         char Temp = ExtInfo->Status;

                         ExtInfo->Status = 0;
                         strupr(ExtInfo->Descript);
                         ExtInfo->Status = Temp;
                       }
                       break;
                    }
                }

                if (! Compress) {
                  // we can't check for ESC if we're compressing,
                  // instead we have to finish the pack
                  if (Escape()) {
                    AbortPack = TRUE;
                    goto Abort;
                  }
                }
              }
            } else {
                DisplayLine(TRUE, ScreenColor->Normal, "Could not check extended headers.");
                Text = NULL;
            }

            if (ToUpperCase) {
                char    Name[26];

                memset(Name, 0, 26);

                memcpy(Name, Header.ToField, 25);       /*** TO: field *****/
                strupr(Name);
                memcpy(Header.ToField, Name, 25);

                memcpy(Name, Header.FromField, 25);     /*** FROM: field ***/
                strupr(Name);
                memcpy(Header.FromField, Name, 25);

                memcpy(Name, Header.SubjField, 25);     /*** SUBJ: field ***/
                strupr(Name);
                memcpy(Header.SubjField, Name, 25);
            }

            if (PackRange && MsgNum >= RangeLowMsg && MsgNum <= RangeHighMsg) {
              DisplayLine(FALSE, ScreenColor->Intense, "RANGE message purged: #%ld", MsgNum);
              MessageStatus = MSG_REMOVED;
//            Header.ActiveFlag = MSG_INACTIVE;
            }

            if (PurgeReceived) {
              if (Header.Status == MSG_RCVR_READ ||
                  Header.Status == MSG_CMNT_READ ||
                  Header.Status == MSG_SPWD_READ ||
                  Header.Status == MSG_GPWD_READ) {
                if (Header.ToField[0] != '@') {
                  if (MsgDate <= (Today - PurgeReceivedAfterDays)) {
                    DisplayLine(FALSE, ScreenColor->Intense, "PRIV+RCVD message purged: #%ld (MsgDate=%u PurgeDate=%u)", MsgNum, MsgDate, Today - PurgeReceivedAfterDays);
                    MessageStatus = MSG_REMOVED;
//                  Header.ActiveFlag = MSG_INACTIVE;
                  }
                }
              }
            }

            if (MessageStatus == MSG_REMOVED && KeepUnreceived != 0) {
              if (Header.Status == MSG_RCVR ||
                  Header.Status == MSG_CMNT ||
                  Header.Status == MSG_SPWD ||
                  Header.Status == MSG_GPWD) {

                // KeepUnreceived == -1 is a special value indicating that
                // private+unread mail should never be removed.  If not -1,
                // then follow the rule set forth below.
                //
                // if the date on the message is GREATER THAN today's date
                // minus the number of days to keep unreceived mail, then we
                // want to KEEP this one.  Otherwise, if the date is LESS THAN
                // today's date minus the number of days to keep unreceived
                // mail then it is TOO OLD and should be removed.

                if (KeepUnreceived == -1 || (MsgDate > Today - (KeepUnreceived - 1))) {
                  DisplayLine(FALSE, ScreenColor->Intense, "Kept due to Private+Unread: #%ld", MsgNum);
                  MessageStatus = MSG_KEPT;
                }
              }
            }

            if (MinMessages) {
              if (NumActive <= MinMessages)
                MessageStatus = MSG_KEPT;
            }

            if (MaxMessages) {
#ifdef  DEBUGMODE
              if (DebugLevel >= 25) {
                DebugDisplayLine(ScreenColor->Warning, ">> %ld", NumActive);
              }
#endif
              if (NumActive > MaxMessages)
                MessageStatus = MSG_REMOVED;
            }

            if (Header.ActiveFlag != MSG_ACTIVE)
              MessageStatus = MSG_REMOVED;

PackItNow:
/****
            if (!QuietPack)
                DisplayMessage(&Header);
 ****/
            if (!QuietPack)
              DisplayStatus(MessageStatus, &Header);

#ifdef  REFNUM
            /*** check any reference numbers now... ************************/
            /*** if not renumbering, and if RefList is allocated, and if ***/
            /*** the RefNum is currently != 0, then check to see if the  ***/
            /*** RefNum is out of range (less than LowMark or greater    ***/
            /*** than HighMark) or if the referenced message does not    ***/
            /*** exist.  If out of range or does not exist then change   ***/
            /*** the reference number to zero.                           ***/
            if (!LRenumber && RefList != NULL && (RefNum = bassngltolong(Header.RefNumber)) > 0)
              if (RefNum<LowMark || RefNum>HighMark || !isset(RefList,(unsigned)(RefNum - LowMark)))
                longtobassngl(Header.RefNumber, 0L);
#endif

            if (RemoveAll)
              MessageStatus = MSG_REMOVED;

            /*** now decided if the message is to be kept or removed... ****/
            if (MessageStatus == MSG_KEPT) {

                NumKept++;

#ifdef  REFNUM
                /*** set a bit indicating this message exists **************/
                if (!LRenumber && RefList != NULL && MsgNum <= HighMark) {
                    setbit(RefList, (unsigned)(MsgNum - LowMark));
                }
#endif

#ifdef  RENUMBER
                if (LRenumber) {
                    DisplayLine(FALSE, ScreenColor->Intense, "Message %ld renumbered to %ld", MsgNum, LRenumber);
                    MsgNum = LRenumber;
                    LRenumber++;
                    MsgStats.HighMsgNum = MsgNum;
                    if (MsgStats.LowMsgNum > MsgNum)
                        MsgStats.LowMsgNum = MsgNum;
                    longtobassngl(Header.RefNumber, 0L);
                }
#endif

                if (!MsgStats.LowMsgNum)
                    MsgStats.LowMsgNum = MsgNum;

#ifdef  PACKGAPS
                if (PackedGap)
                  longtobassngl(Header.MsgNumber, MsgNum);
#endif

                LastNum                 = MsgNum;
                Index.Num               = MsgNum;
                Index.Offset            = (Header.ActiveFlag == MSG_ACTIVE) ? WriteOffset : -WriteOffset;
                Index.Status            = Header.Status;
                Index.Date              = (short) MsgDate;

// don't force to uppercase because the SYSTEM's table for uppercase may not
// be the same table that was used by the caller while the caller was online
//              {
//                char ToName[26];
//                memcpy(ToName,Header.ToField,25); ToName[25] = 0;
//                strupr(ToName);
//                memcpy(Index.To,ToName,25);
//              }
                memcpy(Index.To,Header.ToField,25);

                memcpy(Index.From,      Header.FromField,       25);

                MsgStats.NumActiveMsgs++;

                // We save time by not writing the MSGS file to disk if
                // we're compressing and nothing has changed.  As soon as
                // something changes, then we set the WriteToDisk variable
                // and begin writing to disk at the offset that WriteOffset
                // is pointing to.
                if (Compress && ! WriteToDisk) {
                  if (ReadOffset != WriteOffset) {
                    WriteToDisk = TRUE;
                    dosfseek(&omfp,WriteOffset,SEEK_SET);
                  }
                }

                if (WriteToDisk && dosfwrite(&Header, sizeof(msgheadertype), &omfp) == -1) {
                    AbortPack = TRUE;
                    goto Abort;
                }

                if (dosfwrite(&Index, sizeof(newindextype), &oifp) == -1) {
                    AbortPack = TRUE;
                    goto Abort;
                }

                if (MakeOldIndex) {
                    bassngl     OldOffset;
                    long        CalcOffset;

                    CalcOffset = (WriteOffset >> 7) + 1;
                    if (Header.ActiveFlag == MSG_INACTIVE)
                      CalcOffset = -CalcOffset;

                    longtobassngl(OldOffset,CalcOffset);
                    if (dosfwrite(OldOffset, sizeof(bassngl), &onfp) == -1) {
                        dosfclose(&onfp);
                        MakeOldIndex = FALSE;
                    }
                }

                WriteOffset += sizeof(msgheadertype) * (Header.NumBlocks);

                #ifdef  DEBUGMODE
                  if (DebugLevel >= 50) {
                    DebugDisplayLine(ScreenColor->Intense, "Writing Message : 0x%08X", Text);
                  }
                #endif

                if (Text != NULL) {
                    if (WriteToDisk && dosfwrite(Text, (NumBlocks - 1) * sizeof(msgheadertype), &omfp) == -1) {
                        AbortPack = TRUE;
                        goto Abort;
                    }
                    // don't free it here, we want to keep it allocated for speed
                    // free(Text);
                    // Text = NULL;
                } else {
                    for (i = 1; i < NumBlocks; i++) {
                        if (dosfread(&Header, sizeof(msgheadertype), &imfp) == -1) {
                            AbortPack = TRUE;
                            goto Abort;
                        }
                        if (WriteToDisk && dosfwrite(&Header, sizeof(msgheadertype), &omfp) == -1) {
                            AbortPack = TRUE;
                            goto Abort;
                        }
                    }
                }

                AttachFile[0] = 0;

            } else {

                NumRemoved++;
                NumActive--;

                LastNum = MsgNum;

                DisplayLine(FALSE, ScreenColor->Warning, "Message removed : %ld", MsgNum);

                if (AttachFile[0] != 0) {
                  if (fileexist(AttachFile) != 255) {
                    if (removefile(AttachFile) != -1)
                      DisplayLine(TRUE, ScreenColor->Warning, "Removed Attached File: %s", AttachFile);
                  } else {
                    DisplayLine(TRUE, ScreenColor->Intense, "File not found : %s", AttachFile);
                  }
                  AttachFile[0] = 0;
                }

#ifndef ALLOCMODE
                dosfseek(&imfp, (Header.NumBlocks - 1) * sizeof(msgheadertype), SEEK_CUR);
#endif

                if (MsgStats.LowMsgNum) {

                    memset(&Index, 0, sizeof(newindextype));
                    if (dosfwrite(&Index, sizeof(newindextype), &oifp) == -1) {
                        AbortPack = TRUE;
                        goto Abort;
                    }

                    if (MakeOldIndex) {
                        if (dosfwrite(Zero, sizeof(bassngl), &onfp) == -1) {
                            dosfclose(&onfp);
                            MakeOldIndex = FALSE;
                        }
                    }
                }

                // don't free it here, we want to keep it allocated for speed
                // if (Text != NULL) {
                //   free(Text);
                //   Text = NULL;
                // }
            }

            sprintf(str, "%5ld", NumKept);
            fastprint(54, 24, str, ScreenColor->StatusHighlight);
            sprintf(str, "%5ld", NumRemoved);
            fastprint(70, 24, str, ScreenColor->StatusHighlight);

        } else {
            // invalid header block - try to resynch the header
            DisplayLine(FALSE, ScreenColor->Intense, "Invalid message header block - attempting to resynchronize");
            if (resynch(&imfp) == -1) {
              MsgNum = 0;
              break;
            }
        }

        if (!QuietPack) {
            DisplayGraph(NumProcessed, NumActiveStart);
        }

    }

/*  DisplayLine(FALSE, ScreenColor->Intense, ""); */


#ifdef  PACKGAPS
    if (PackedGap)
      MsgStats.HighMsgNum = MsgNum;
#endif

#ifdef  RENUMBER
    if (LRenumber)
        MsgStats.HighMsgNum = MsgStats.LowMsgNum + NumActive - 1;
#endif

    if (MsgStats.LowMsgNum) {

        memset(&Index, 0, sizeof(Index));

        while (LastNum < max(MsgStats.HighMsgNum, MsgNum)) {

            if (dosfwrite(&Index, sizeof(Index), &oifp) == -1) {
                AbortPack = TRUE;
                goto Abort;
            }

            if (MakeOldIndex) {
                if (dosfwrite(Zero, sizeof(bassngl), &onfp) == -1) {
                    dosfclose(&onfp);
                    MakeOldIndex = FALSE;
                }
            }
            LastNum++;
        }
    } else {
        MsgStats.LowMsgNum = MsgStats.HighMsgNum;
        memset(&Index, 0, sizeof(Index));
        if (dosfwrite(&Index, sizeof(Index), &oifp) == -1) {
            AbortPack = TRUE;
            goto Abort;
        }
        if (MakeOldIndex) {
            if (dosfwrite(Zero, sizeof(bassngl), &onfp) == -1) {
                dosfclose(&onfp);
                MakeOldIndex = FALSE;
            }
        }
    }

    // if we are compressing, then we may need to truncate the messages file
    // - if WriteToDisk is set, it means that we removed some messages from
    //   the middle of the file so we need to truncate the rest
    // - if NumKept is 0, then we killed all of the messages and the message
    //   base therefore needs to be truncated
    if (Compress && (! AbortPack) && (WriteToDisk || NumKept == 0))
      dosftrunc(&omfp,-1);

    longtobassngl(MsgDiskStats.LowMsgNum, MsgStats.LowMsgNum);
    longtobassngl(MsgDiskStats.HighMsgNum, max(MsgStats.HighMsgNum, MsgNum));
    longtobassngl(MsgDiskStats.NumActiveMsgs, MsgStats.NumActiveMsgs);

    if (WriteToDisk) {
      if (dosfseek(&omfp, 0L, SEEK_SET) == -1) {
          AbortPack = TRUE;
          goto Abort;
      }
      if (dosfwrite(&MsgDiskStats, sizeof(msgbasediskstattype), &omfp) == -1) {
          AbortPack = TRUE;
          goto Abort;
      }
      for (i = 0; i < sizeof(msgheadertype) - sizeof(msgbasediskstattype); i++) {
          if (dosfwrite(" ", sizeof(char), &omfp) == -1) {
              AbortPack = TRUE;
              goto Abort;
          }
      }
    }

    if (!QuietPack) {
        DisplayGraph(NumProcessed, NumActiveStart);
    }

    DisplayLine(TRUE, ScreenColor->Intense, "Number of messages kept      = %ld", NumKept);
    DisplayLine(TRUE, ScreenColor->Intense, "Number of messages removed   = %ld", NumRemoved);
    DisplayLine(TRUE, ScreenColor->Intense, "Number of messages processed = %ld", NumProcessed);
    DisplayLine(TRUE, ScreenColor->Intense, "Number of extraneous blocks  = %d", BadBlocks);
    DisplayLine(TRUE, ScreenColor->Intense, "");

    #ifdef __OS2__
      updatelinesnow();
    #endif

#ifdef  CHECKDUPS
    if (RemoveDups) {
/***************************************************************************
 We need to keep n number of days worth of the CRC database, so let's sort
 the thing and then we'll purge those records that are old and then store
 the rest in the CRC database.
 ***************************************************************************/

      if (CRCDays) {
        unsigned    i;

        #ifdef __OS2__
          qsort(crcList,crcHeader.Num,sizeof(crctype),CompareDate);
        #else

          // WARNING:  See warning about zsort() in PCBPACK.C

          zsort((char *) crcList, crcHeader.Num, sizeof(crctype), CompareDate, zswapstr);
        #endif

        for (i = 0; i < crcHeader.Num; i++) {
          if ((crcList + i)->Date < (Today - CRCDays)) {
            crcHeader.Num = (short) i;
            break;
          }
        }
        if (dosfopen(crcFile, OPEN_CREATE|OPEN_WRIT|OPEN_DENYNONE, &cfp) == -1)
          goto Abort;
        if (dosfwrite(&crcHeader, sizeof(crchdrtype), &cfp) != -1) {
          for (i = 0; i < crcHeader.Num; i++)
            if (dosfwrite((void*) (crcList + i), sizeof(crctype), &cfp) == -1)
              break;
        }
        dosfclose(&cfp);
      }
    }
#endif

Abort:
    if (Text != NULL) {
      free(Text);
      Text = NULL;
    }

#ifdef  REFNUM
    if (RefList != NULL) {
      free(RefList);
      RefList = NULL;
    }
#endif

#ifdef  CHECKDUPS
/***************************************************************************
 Let's shut everything down and start cleaning up...
 ***************************************************************************/
    if (RemoveDups) {
      if (crcList != NULL) {
        farfree((void _FAR_ *) crcList);
        crcList = NULL;
      }
    }
#endif

    if (! Compress)
      DisplayLine(FALSE, ScreenColor->Warning, "Renaming temporary files...  Please wait...");

    if (MakeOldIndex && dosfclose(&onfp) == -1)
      AbortPack = TRUE;
    if (dosfclose(&oifp) == -1)
      AbortPack = TRUE;
    if (dosfclose(&omfp) == -1)
      AbortPack = TRUE;
    if (dosfclose(&imfp) == -1)
      AbortPack = TRUE;

    // if the process aborted, then don't remove the backups.
    // if we're compressing the file, then there are no backups to remove

    if (! AbortPack && ! Compress) {
      if (removefile(msgBack) == -1)
        AbortPack = TRUE;
      if (removefile(idxBack) == -1)
        AbortPack = TRUE;
      if (MakeOldIndex && removefile(ndxBack) == -1)
        AbortPack = TRUE;
    }

    if (AbortPack) {
      // if we're compressing, then there are no temporary files to kill
      // so just return FALSE now
      if (Compress)
        return(FALSE);
      // otherwise, remove the temp files
      if ((removefile(msgTemp) == -1) ||
          (removefile(idxTemp) == -1) ||
          (MakeOldIndex && removefile(ndxTemp) == -1))
        return(FALSE);
    } else {
      // if we're compressing, then there's nothing to rename or remove,
      // otherwise, remove the backups first, then rename the old file
      // to the backup filename, then rename the temp file to the real name.
      if (! Compress) {
        if (RemoveBackup) {
          if ((removefile(msgFile) == -1) ||
              (removefile(idxFile) == -1) ||
              (MakeOldIndex && removefile(ndxFile) == -1))
            return(FALSE);
        } else {
          if ((renamefile(msgFile,msgBack) == -1) ||
              (renamefile(idxFile,idxBack) == -1) ||
              (MakeOldIndex && renamefile(ndxFile,ndxBack) == -1))
            return(FALSE);
        }

        if ((renamefile(msgTemp,msgFile) == -1) ||
            (renamefile(idxTemp,idxFile) == -1) ||
            (MakeOldIndex && renamefile(ndxTemp,ndxFile) == -1))
          return(FALSE);
      }

      DisplayLine(FALSE, 0x0F, "Proceeding.");
      DisplayLine(TRUE, 0x0F, "");
      return TRUE;
    }

    return FALSE;
}

