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


#include "project.h"
#pragma hdrstop

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <io.h>
  #include <direct.h>
  #include <borland.h>
#else
//#include <dir.h>
#endif

#ifdef __OS2__
  #include <sem.hpp>

  void LIBENTRY createmodemthread(void);
  void LIBENTRY destroymodemthread(void);
#endif

#include <io.h>
#include "account.h"
#include "xmodem.h"
#include "messages.h"

#define SOH     0x01
#define STX     0x02
#define ACK     0x06
#define CAN     0x18
#define NAK     0x15
#define EOT     0x04
#define STARTx  0x15
#define STARTy  'U'
#define STARTg  'G'
#define STARTc  'C'

typedef enum {START,STOP} xferstatustype;
typedef enum {SUM,CRC,PURE} blocktype;
typedef enum {HANDSHAKE,FULLFLOW,ASCII} flowtype;
        enum {NOERROR,TIMEOUT,BADBLOCKNUM,SUMERROR,CRCERROR,SHORTBLOCK};

typedef struct {
  char           Check;
  long           FileSize;
  unsigned short CPS;
  int            NumErrors;
  char           Name[66];
} dszlogtype;

typedef struct {
  char Text[47];
} desctype;


xfervartype XferVars;

static bool PrivateFile;
static char ProtocolLetterToUse;
static int  TotErrors;

static int      BlockSize;
static unsigned BlockNum;
static unsigned TotalFilesSoFar;
static unsigned IgnoreFiles;
static long     TotalBytesSoFar;
static long     TotalSecondsSoFar;
static long     IgnoreBytes;
static long     IgnoreSeconds;
static long     XferStartTime;
static long     BytesSoFar;
static prottype ProtocolToUse;

static unsigned TickMarks[51];
static int  ScaleLoc;
static int  LastMark;
static int  FileNum;
static bool Cancel;
static int  Errors;
static int  LastError;
static int  BytesInShortBlock;
static bool FirstUploadPrivate;   // TRUE if first announced upload should be private
       bool AutoGoodBye;

#ifdef COMM
static char      StartLetter;
static blocktype Type;
static flowtype  Flow;
#endif

static char *BatchMask = "PCBxx";

extern struct ffblk DTA;  /* declared in exist.c */

char static * ErrStrings[SHORTBLOCK+1] = {
/* NOERROR    */ "",
/* TIMEOUT    */ "Timeout",
/* BADBLOCK   */ "Block Num",
/* SUMERROR   */ "SUM Error",
/* CRCERROR   */ "CRC Error",
/* SHORTBLOCK */ "Short Block"
};

typedef struct {
  char Border;
  char Text;
  char Numbers;
  char Bar;
  char Errors;
} xfercolortype;

xfercolortype static XferScreenColors[2] = { {0x07,0x07,0x0F,0x0F,0x07}, {0x12,0x1B,0x1E,0x1E,0x1C} };
xfercolortype static *XferColors;

ScrType static XferScreen[9] = {
  { 4,5,"Errors :",0x1B},
  {13,5,"0",0x1E},
  {37,5,"Total Errors:",0x1B},
  {51,5,"0",0x1E},
  {57,5,"Average CPS:",0x1B},
  { 6,7,"0...10...20...30...40...50...60...70...80...90...100%  Bytes",0x1B},
  { 6,8,"ß",0x1E},
  {61,8,"Done",0x1B},
  {75,8,"0",0x1E} };


int LIBENTRY xferminutesleft(void) {
  return(minutesleft() + (int) ((IgnoreSeconds - TotalSecondsSoFar) / 60));
}


// number of files flagged so far minus those that are FREEFILE
static unsigned _NEAR_ LIBENTRY numfileslessignore(void) {
  return(TotalFilesSoFar - IgnoreFiles);
}


// number of bytes flagged so far minus those that are FREEFILE
long LIBENTRY numbyteslessignore(void) {
  return(TotalBytesSoFar - IgnoreBytes);
}


long LIBENTRY bytesleft(void) {
  return(Status.BytesRemaining - numbyteslessignore());
}


static void _NEAR_ LIBENTRY setxferprotocol(char Letter) {
  char Offset;

  if (Letter >= 'A' && Letter <= 'Z')
    Offset = 'A';
  else
    Offset = '0' - 26;

  ProtocolToUse = Protocols[Letter-Offset];
  ProtocolLetterToUse = Letter;
}


static bool _NEAR_ LIBENTRY insufficientfreespace(void) {
  unsigned long FreeSpace;

  if (! PcbData.DisableDriveCheck) {
    FreeSpace = diskfreespace(Status.CurConf.PrvUpldLoc);
    if (((unsigned long) PcbData.StopFreeSpace << 10) > FreeSpace)
      return(TRUE);
  }
  return(FALSE);
}


/********************************************************************
*
*  Function:  updatedownloadtext()
*
*  Desc    :  This function is used to write information into the DOWNLOAD.TXT
*             file indicating the names of files that are successfully
*             uploaded or downloaded, who transferred the files, and what
*             protocol was used.
*/

static void _NEAR_ LIBENTRY updatedownloadtext(char UpDn, char *FileName, int NumErrors, unsigned CPS) {
  bool    Save;
  char    Temp[9];
  DOSFILE Out;
  char    Date[15];
  char    Time[15];
  char    Str[100];

  if (PcbData.DldFile[0] == 0)
    return;

  if (Asy.Online == LOCAL && PcbData.ExcludeLocals)
    return;

  // create date & time stamp
  timestr2(Time);
  datestr(Date);
  Date[10] = 0;
  Date[9] = Date[7];
  Date[8] = Date[6];
  Date[7] = '9';
  Date[6] = '1';

  if (CPS == 0)
    strcpy(Temp,"External");
  else
    lascii(Temp,CPS);

  // get user name
  Save = Status.UseAlias;
  Status.UseAlias = FALSE;
  getdisplaynames();

  // create line of text for the DOWNLOAD.TXT file showing if the file
  // was uploaded (U) or downloaded (D), the user's name, the date and time
  // of the transfer, the name of the file transferred, the protocol used,
  // the number of errors experienced during transmission, and the CPS rate.
  sprintf(Str,"(%c),%s,%s,%s,%s,%c,%d,%s\r\n",UpDn,Status.DisplayName,Date,Time,FileName,ProtocolLetterToUse,NumErrors,Temp);
  Status.UseAlias = Save;
  getdisplaynames();

  // open the download.txt file in read-write-append mode (to write the text
  // at the end of the file)
  if (dosfopen(PcbData.DldFile,OPEN_RDWR|OPEN_APPEND|OPEN_DENYWRIT,&Out) == -1)
    return;

  // write the text and close the file
  dosfputs(Str,&Out);   //lint !e534
  dosfclose(&Out);
}


#ifdef COMM
static void _NEAR_ LIBENTRY truncatefilesize(char *FileName, long Size) {
  int X;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  if ((X = dosopen(FileName,OPEN_RDWR POS2ERROR)) != -1) {
    dostrunc(X,Size POS2ERROR);          //lint !e534   truncate the file
    dosclose(X);
  }
}


static int _NEAR_ LIBENTRY saveblock(char *Buffer, int Size, DOSFILE *File) {
  #ifdef __OS2__
    return(dosfwrite(Buffer,Size,File));
  #else
    #ifndef MULTIPORT
      int  RetVal;

      if (PcbData.DisableCTS || PcbData.DisableCTSdrop)
        return(dosfwrite(Buffer,Size,File));

      disable();
      if (! RTSoff) {
        enable();
        turnoffrts();
        RetVal = dosfwrite(Buffer,Size,File);
        turnonrts();
        return(RetVal);
      }
      enable();
    #endif
    return(dosfwrite(Buffer,Size,File));
  #endif
}


/********************************************************************
*
*  Function: waitcomm()
*
*  Desc    : waits for the specified number of clock ticks to pass while at
*            the same time eating any data that comes in from the modem and
*            checking for carrier loss while we're at it.
*/

static void _NEAR_ LIBENTRY waitcomm(int Seconds) {
  char Temp[64];

  if (Asy.Online == REMOTE) {
    settimer(4,Seconds);
    while (! timerexpired(4) || cgetbuf(Temp,sizeof(Temp)) != 0) {
      giveup();
      giveup();
      if (cdstillup() == 0) {
        Asy.LostCarrier = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
        break;
      }
    }
  }
}
#endif


static unsigned _NEAR_ LIBENTRY numblocks(long Size) {
  unsigned Blocks;
  long     Temp;

  if (BlockSize == 0)
    return(0);

  Blocks = (unsigned) (Size / BlockSize);
  Temp = (long) Blocks * BlockSize;
  if (Temp != Size)
    Blocks++;
  return(Blocks);
}


static unsigned _NEAR_ LIBENTRY calcdownshift(long Size, unsigned *NumBlocks) {
  unsigned DownShiftBlockNum;
  unsigned Blocks;

  Blocks = numblocks(Size);
  if (BlockSize == 1024 && Size/1024 != Blocks) {
    DownShiftBlockNum = Blocks;
    Blocks--;
    BlockSize = 128;  /* lower it to 128 for the calculation */
    Blocks += numblocks(Size - ((long) Blocks * 1024));
    BlockSize = 1024; /* restore it back to 1024 */
  } else
    DownShiftBlockNum = 0xFFFF;

  *NumBlocks = Blocks;
  return(DownShiftBlockNum);
}


static unsigned _NEAR_ LIBENTRY maxspeed(void) {
  /* We'll allow for up to 25% data compression by the PROTOCOL in the      */
  /* factoring of the maximum possible speed thru the modem - such that a   */
  /* modem opened at 2400 bps might actually achieve a rate as high as 300  */
  /* cps if the PROTOCOL were to perform compression on the file.  However, */
  /* this also means that a modem opened at 19200 could get as high as 2400 */
  /* cps instead of limiting it to 1920 cps.                                */
  return((unsigned) (Asy.ModemSpeed / 8));
}


// static unsigned _NEAR_ LIBENTRY cpsmax(long CPS) {
//   unsigned  Max;
//
//   Max = maxspeed();
//   return(CPS > Max ? Max : (unsigned) CPS);
// }


static unsigned short _NEAR_ LIBENTRY cps(long Bytes, long ElapsedTime) {
  long CPS;

  if (ElapsedTime < 100)
    ElapsedTime = 100;

  if (Bytes < 20000000L)
    CPS = (Bytes * 100) / ElapsedTime;
  else
    CPS = (Bytes / ElapsedTime) * 100;

//return(cpsmax(CPS));
  return((unsigned short) CPS);
}


long LIBENTRY numsecsforxfer(long FileSize) {
  unsigned CPS;
  long     Speed;

  Speed = Asy.CarrierSpeed;
  if (Asy.ModemSpeed > Asy.CarrierSpeed && Asy.ErrorCorrected && ! Status.TimeAdjustedForEvent)
    Speed = (Speed * 6) / 5;   /* 20% boost in speed for MNP (unless an EVENT is coming!) */

  CPS = (unsigned) (Speed / 10);

  /* calculate the number of seconds estimated for the file transfer */
  /* and then add 10 seconds so that at least one TENTH of a minute  */
  /* will be required for the transfer protocol                      */
  return((((FileSize / CPS) * (BlockSize == 128 ? 114 : 107)) / 100) + 10);
}


static void _NEAR_ LIBENTRY downloadtime(long FileSize, char *Str, long *Seconds) {
  int  Minutes;
  int  Tenths;
  long CalcSecs;

  CalcSecs = numsecsforxfer(FileSize);
  Minutes  = (int) (CalcSecs / 60);
  Tenths   = ((int) CalcSecs - (Minutes*60)) / 6;

  if (Str != NULL) {
    if (Tenths == 0)
      sprintf(Str," %d",Minutes);
    else
      sprintf(Str," %d.%d",Minutes,Tenths);
  }

  if (Seconds != NULL)
    *Seconds = CalcSecs;
}




static void _NEAR_ LIBENTRY postupload(int FileSpecNum) {
  bool        Private;
  bool        BadFile;
  bool        UpdateIdx;
  bool        FirstLine;
  long        StartTime;
  long        EndTime;
  long        Offset;
  long        DetectedSize;
  char        *p;
  char        *Buffer;
  char        Date[10];
  DOSFILE     Out;
  DOSFILE     Temp;
  DOSFILE     IdxFile;
  char        Overlay[40];
  char        TmpFile[66];
  char        NewPath[66];
  char        OldPath[66];
  char        IdxPath[66];
  pcbtexttype TxtBuf;
  char        Str[100];
  char        Save[100];
  spectype    File;

  // Don't test a file that is being uploaded into the message editor
  //
  // But DO test the file if it is a file attachment, however, we won't
  // be posting the file into the public upload directory.

  if (Status.MsgUpload)
    return;

  StartTime = gettimer(6);

  buildstr(TmpFile,PcbData.TmpLoc,"UPDESC.",NULL);
  if (PcbData.Network) {
    ascii(Str,PcbData.NodeNum);
    strcat(TmpFile,Str);          //lint !e419
  }

  if ((Buffer = allocatefiledesc()) != NULL) {
    if (getfilespec(FileSpecNum,&File) == -1 || getfiledesc(FileSpecNum,Buffer) == -1)
      return;

    strupr(File.Name);
    if (dosfopen(TmpFile,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&Temp) != -1) {
      sprintf(Str,"%-12.12s%9ld  %s  %s\r\n",File.Name,File.Size,datestr(Date),Buffer);
      dosfputs(Str,&Temp);   //lint !e534

      for (p = Buffer, p = strchr(p,0)+1; strlen(p) > 0; p += strlen(p)+1) {
        sprintf(Str,"%31.31s| %s\r\n","",p);
        if (dosfputs(Str,&Temp) == -1)
          break;
      }
      dosfclose(&Temp);
    }

    BadFile = FALSE;

    if (PcbData.TestUploads) {
      #ifdef COMM
        if (Asy.LostCarrier) {
          initcommwindow(HIDE);
          modemoffhook();
          if (PcbData.Packet)
            turnoffdtr();
        }
      #endif

      if (verifyfile(File.FullPath,File.Name,TmpFile) == -1) {
        BadFile = TRUE;
        File.Failed = TRUE;
        UsersData.Stats.NumVerifyErrors++;
      }

      /* if we still find the file then set the file size in case the */
      /* verification process has changed the size of the file        */
      if (fileexist(File.FullPath) != 255 && File.Size != DTA.ff_fsize)
        sprintf(Overlay,"%-12.12s%9ld",File.Name,DTA.ff_fsize);
    }

    // if the user is attaching a file to a message, then the file should
    // just be moved to where it belongs - do not post it in any DIR file
    // (public or private) - just move it to the attachment directory

    if (Status.FileAttach) {
      // if an attached file fails the test, just delete it
      if (BadFile) {
        unlink(File.FullPath);
        putfilespec(FileSpecNum,&File);  // update "failed" status
      } else {
        strcpy(NewPath,Status.CurConf.AttachLoc);
        strcpy(OldPath,File.FullPath); OldPath[strlen(File.FullPath)-strlen(File.Name)] = 0;

        if (stricmp(NewPath,OldPath) != 0) {
          strcat(NewPath,File.Name);
          if (pcbmovefile(File.FullPath,NewPath) == 0) {
            strcpy(File.FullPath,NewPath);
            putfilespec(FileSpecNum,&File);  // full name/path information
          }
        }
      }
      unlink(TmpFile);  // remove the UPDESC.### file
    } else {
      Private = (bool) (Status.CurConf.PrivUplds || Buffer[0] == '/' || Buffer[0] == '\\' || BadFile);
      p = (Private ? Status.CurConf.PrivDir : Status.CurConf.UpldDir);
      if (fileexist(TmpFile) != 255 && dosfopen(TmpFile,OPEN_READ|OPEN_DENYNONE,&Temp) != -1) {
        if (dosfopen(p,OPEN_RDWR|OPEN_APPEND|OPEN_DENYWRIT,&Out) != -1) {
          Offset = dosfseek(&Out,0,SEEK_CUR);
          strcpy(IdxPath,p);
          strcat(IdxPath,".IDX");
          UpdateIdx = (bool) (dosfopen(IdxPath,OPEN_RDWR|OPEN_DENYWRIT,&IdxFile) != -1);
          if (UpdateIdx && needtoresyncdirindex(p,&IdxFile)) {
            dosfclose(&IdxFile);
            unlink(IdxPath);
            UpdateIdx = FALSE;
          }

          FirstLine = TRUE;
          while (dosfgets(Str,sizeof(Str),&Temp) >= 0) {
            if (FirstLine) {
              FirstLine = FALSE;

              /* if we still find the file then set the file size in case  */
              /* the verification process has changed the size of the file */

              DetectedSize = File.Size;
              if (fileexist(File.FullPath) != 255)
                DetectedSize = DTA.ff_fsize;
              else {
                /* possibly the filename has changed, check for a new name */
                strcpy(NewPath,File.FullPath);
                if ((p = strrchr(NewPath,'\\')) != NULL || (p = strrchr(NewPath,':')) != NULL)
                  p++;
                else
                  p = NewPath;
                maxstrcpy(p,Str,13);
                stripright(p,' ');
                if (fileexist(NewPath) != 255) {
                  DetectedSize = DTA.ff_fsize;
                  strcpy(File.Name,p);
                  strcpy(File.FullPath,NewPath);
                }
              }

              sprintf(Overlay,"%-12.12s%9ld",File.Name,DetectedSize);
              memcpy(Str,Overlay,21);
              strcpy(Save,Str);
            }

            dosfputs(Str,&Out);        //lint !e534
            dosfputs("\r\n",&Out);     //lint !e534
          }

          if (BadFile) {
            getpcbtext(TXT_FILEFAILED,&TxtBuf);
            sprintf(Str,"%31.31s| %s\r\n","",TxtBuf.Str);
            dosfputs(Str,&Out);        //lint !e534
          }

          if (PcbData.UploadBy) {
            strcpy(Status.DisplayText,Status.DisplayName);
            proper(Status.DisplayText);
            getpcbtext(TXT_UPLOADEDBY,&TxtBuf);
            sprintf(Str,"%31.31s| %s\r\n","",TxtBuf.Str);
            dosfputs(Str,&Out);        //lint !e534
          }

          dosfclose(&Out);
          if (UpdateIdx) {
            updatedirindex(p,&IdxFile,File.Name,Save,Offset);
            dosfclose(&IdxFile);
          }
        }
        dosfclose(&Temp);
        unlink(TmpFile);
      }

      if (! Private && (memcmp(Status.CurConf.PubUpldLoc,File.FullPath,strlen(File.FullPath)-strlen(File.Name)) != 0)) {
        buildstr(NewPath,Status.CurConf.PubUpldLoc,File.Name,NULL);
        pcbmovefile(File.FullPath,NewPath);  //lint !e534
      }

      EndTime = gettimer(6);
      #ifdef __OS2__
        addtime((-EndTime+StartTime) / ONESECOND,CREDTIME);
      #else
        addtime(((-EndTime+StartTime)*10) / 182,CREDTIME);
      #endif

      if (BadFile) {
        /* the file was bad so we'll remove the upload credit from the user */
        File.Success = FAILED;
        XferVars.SuccessfulUpldFiles--;
        XferVars.SuccessfulUpldBytes -= File.Size;
        UsersData.NumUploads--;
        UsersData.TotUpldBytes -= File.Size;

        #ifndef PCB_DEMO
          UsersData.DailyDnldBytes += (File.Size * PcbData.ByteCredit) / 10;
          if (Status.BytesRemaining != -1)
            Status.BytesRemaining  -= (File.Size * PcbData.ByteCredit) / 10;
          if (File.CPS > 0)
            subtime(((File.Size / File.CPS) * PcbData.UploadCredit) / 10,CREDTIME);
        #endif
        File.Success = REMOVE;
        putfilespec(FileSpecNum,&File);

        #ifdef PCB152
          if (Status.ActStatus != ACT_DISABLED) {
            recordusage("UPLD FILE FAIL",
                        File.Name,
                        -1.0 * AccountRates.PayBackForUploadFile * File.Multiplier,
                        1,
                        &UsersData.Account.CreditUploadFile);
            recordusage("UPLD BYTES FAIL",
                        File.Name,
                        -1.0 * AccountRates.PayBackForUploadBytes * File.Multiplier,
                        File.Size / 1024,
                        &UsersData.Account.CreditUploadBytes);
          }
        #endif
      }
    }
  }
}


static int _NEAR_ LIBENTRY checkfilefordescription(int FileSpecNum, spectype *File) {
  char *Buffer;

  if (PcbData.TestUploads) {
    if (checkfordizfile(File->FullPath) && (Buffer = allocatefiledesc()) != NULL) {
      File->HasDesc = TRUE;
      if (FirstUploadPrivate)
        strcpy(Buffer,"/Private FILE_ID.DIZ");
      else
        strcpy(Buffer,"Contains FILE_ID.DIZ");
      putfiledesc(FileSpecNum,Buffer);          //lint !e534
      return(0);
    }
  }
  return(-1);
}


static int _NEAR_ LIBENTRY getdescription(int FileSpecNum, spectype *File) {
  bool Save;
  int  X;
  int  Len;
  int  NumBytes;
  int  RetVal;
  char *p;
  char *Buffer;
  desctype *Desc;
  long StartTime;
  long EndTime;
  pcbtexttype Buf;

  // Is the caller entering a message?
  //      No, no problem, get the description
  //      Yes -> is this a file attachment?
  //         Yes, substitute a dummy description
  //         No, simply return

  if (Status.MsgUpload)
    return(0);

  StartTime = gettimer(6);
  NumBytes  = PcbData.NumDescLines * sizeof(desctype);
  RetVal    = 0;
  Desc      = NULL;
  X         = 0;  /* initialize up here because if the caller lost */
                  /*  carrier it won't go thru the input loop   */

  Status.FileXfer = FALSE;         // Allow scrollback and F5 keys to work
  Save = Status.EnteringMessage;   // Save status and restore below
  Status.EnteringMessage = TRUE;   // Don't allow chat at this point

  if ((Buffer = allocatefiledesc()) == NULL) {
    RetVal = -1;
    goto done;
  }

  // we don't really want to ask for a description on an upload that is
  // being attached to a message - the message itself will describe it -
  // so record FILE ATTACHMENT as the description and then get out of here

  if (Status.FileAttach) {
    strcpy(Buffer,"/FILE ATTACHMENT");
    goto store;
  }

  if ((Desc = (desctype *) checkmalloc(NumBytes,"UPLD DESC ENTRY")) == NULL) {
    RetVal = -1;
    goto done;
  }

  memset(Desc,0,NumBytes);

  if (! Asy.LostCarrier && ! Status.KbdTimedOut) {
    maxstrcpy(Status.DisplayText,File->Name,sizeof(Status.DisplayText));
    displaypcbtext(TXT_ENTERDESCRIPTION,NEWLINE|LFBEFORE);
    displaypcbtext(TXT_SLASHFORPRIVATE,NEWLINE);

    displaypcbtext(TXT_MSGENTERTEXT,DEFAULTS);
    ascii(Status.DisplayText,PcbData.NumDescLines);
    displaypcbtext(TXT_45COLUMNS,NEWLINE);

    for (; X < PcbData.NumDescLines;) {
      inputfieldstr(Desc[X].Text,"",Display.DefaultColor,45,NEWLINE|FIELDLEN|WORDWRAP|HIGHASCII|(X == 0 ? GUIDE : DEFAULTS),NOHELP,mask_message);
      removetokens(Desc[X].Text);
      #ifdef COMM
        if (Asy.LostCarrier)
          break;
      #endif
      if (Status.KbdTimedOut)
        break;

      /* on the first line of the description check for three things:     */
      /*  1st:  if the description is blank and the file size is zero it  */
      /*        means the file has not yet been uploaded and the caller   */
      /*        wishes to abort the upload so get out now                 */
      /*  2nd:  if the description is shorter than 5 letters ask for more */
      /*  3rd:  if it starts with "/" the remember that it is private     */

      if (X == 0) {
        if (Desc[0].Text[0] == 0 && File->Size == 0) {
          RetVal = -1;
          goto done;
        }
        if (strlen(Desc[0].Text) < 5) {
          displaypcbtext(TXT_LONGERDESCRIPTION,NEWLINE);
          continue;
        }
        if (Desc[0].Text[0] == '/')
          PrivateFile = TRUE;
      }

      /* any time after the first line we will stop description entry if */
      /* a blank line is entered - it doesn't reach this point on line 1 */

      if (Desc[X].Text[0] == 0)
        break;

      X++;
    }
  }

  if (Asy.LostCarrier || Status.KbdTimedOut) {
    #ifdef COMM
      getsystext(Asy.LostCarrier ? TXT_CARRIERLOST : TXT_KBDTIMEEXPIRED,&Buf);
    #else
      getsystext(TXT_KBDTIMEEXPIRED,&Buf);
    #endif
    if (X > PcbData.NumDescLines-1)
      X = PcbData.NumDescLines-1;
    strcpy(Desc[X].Text,Buf.Str);
  }

  for (X = 0, p = Buffer; X < PcbData.NumDescLines; X++) {
    if ((Len = strlen(Desc[X].Text)) == 0)
      break;
    Len++;                        /* add 1 for NULL terminator */
    memcpy(p,Desc[X].Text,Len);
    p += Len;
  }

store:
  File->HasDesc = TRUE;
  putfiledesc(FileSpecNum,Buffer);               //lint !e534
  if (FileSpecNum == 0 && Buffer[0] == '/')
    FirstUploadPrivate = TRUE;

done:
  if (Desc != NULL)
    bfree(Desc);

  Status.EnteringMessage = Save; // restore the setting saved up above

  EndTime = gettimer(6);
  #ifdef __OS2__
    addtime((-EndTime+StartTime) / ONESECOND,CREDTIME);
  #else
    addtime(((-EndTime+StartTime)*10) / 182,CREDTIME);
  #endif
  return(RetVal);
}


static void _NEAR_ LIBENTRY paintscreen(int RightMargin) {
  char Str[36];

  XferColors = &XferScreenColors[Scrn_Mode];
  XferScreen[0].Color = XferColors->Text;
  XferScreen[1].Color = XferColors->Numbers;
  XferScreen[2].Color = XferColors->Text;
  XferScreen[3].Color = XferColors->Numbers;
  XferScreen[4].Color = XferColors->Text;
  XferScreen[5].Color = XferColors->Text;
  XferScreen[6].Color = XferColors->Bar;
  XferScreen[7].Color = XferColors->Text;
  XferScreen[8].Color = XferColors->Numbers;

  maxstrcpy(Str,ProtocolToUse.Desc,sizeof(Str));
  boxcls(2,3,77,9,XferColors->Border,SINGLE);
  showscreen(XferScreen,9);
  fastprint(RightMargin-strlen(Str),4,Str,XferColors->Text);
  ScaleLoc = xyvalue(6,8);
}


static void _NEAR_ LIBENTRY paintdlscreen(spectype *File) {
  char Str[15];

  paintscreen(61);
  fastprint( 4,4,"Sending:",XferColors->Text);
  fastprint(63,4,"File    of",XferColors->Text);
  ascii(Str,FileNum+1);
  fastprint(70-strlen(Str),4,Str,XferColors->Numbers);
  ascii(Str,NumFiles);
  fastprint(75,4,Str,XferColors->Numbers);
  sprintf(Str,"%8ld",File->Size);
  fastprint(68,7,Str,XferColors->Numbers);
  fastprint(13,4,File->Name,XferColors->Numbers);
  agotoxy(76,8);
}


static void _NEAR_ LIBENTRY updatescreen(xfertype XferType) {
  int  static OldErrors;
  int  Mark;
  long Time;
  char Str[80];

  if (Errors != OldErrors) {
    OldErrors = Errors;
    if (Errors == 0) {
      fastprint(13,5,"0 ",XferColors->Numbers);
      if (XferType == UPLOAD)
        fastprint(15,5,"                   ",XferColors->Numbers);
    } else {
      ascii(Str,Errors);
      fastprint(13,5,Str,XferColors->Errors);
      if (XferType == UPLOAD) {
        if (LastError == SHORTBLOCK) {
          sprintf(Str,"%s (%d)",ErrStrings[LastError],BytesInShortBlock);
          fastprint(15,5,Str,XferColors->Errors);
        } else fastprint(15,5,ErrStrings[LastError],XferColors->Errors);
      }
      ascii(Str,TotErrors);
      fastprint(51,5,Str,XferColors->Errors);
    }
  }

  if (TickMarks[50] != 0) {
    for (Mark = LastMark; Mark <= 50; Mark++) {
      if (BlockNum >= TickMarks[Mark]) {
        ScaleLoc += 2;
        fastputch(ScaleLoc,'ß',XferColors->Bar);
        LastMark = Mark + 1;
      } else break;
    }
  }
  sprintf(Str,"%8ld",BytesSoFar);
  fastprint(68,8,Str,XferColors->Numbers);

  if ((Time = doselapsedtime(XferStartTime)) < 100)
    Time = 100;

  #ifdef COMM
    sprintf(Str,"%6ld",((BytesSoFar-OutBytes) * 100) / Time);
  #else
    sprintf(Str,"%6ld",(BytesSoFar * 100) / Time);
  #endif
  fastprint(70,5,Str,XferColors->Numbers);
}


static void _NEAR_ LIBENTRY setupxferscale(int NumBlocks) {
  int       X;
  unsigned *p;

  LastMark = 0;
  TickMarks[0] = 0;
  for (p = &TickMarks[1], X = 1; X <= 50; X++, p++)
    *p = (int) (((long) NumBlocks * X) / 50);
}


static void _NEAR_ LIBENTRY paintulscreen(char *Name, long Size) {
  char Str[80];

  paintscreen(75);
  fastprint(4,4,"Receiving:",XferColors->Text);
  if (Size != 0) {
    sprintf(Str,"%8ld",Size);
    fastprint(68,7,Str,XferColors->Numbers);
  } else {
    fastprint(71,7,"(N/A)",XferColors->Text);
    fastprint(14,8,"(file transfer status unavailable)",XferColors->Text);
  }
  fastprint(15,4,Name,XferColors->Text);
  agotoxy(76,8);
}


/********************************************************************
*
*  Function:  successful()
*
*  Desc    :  This function is called after a successful upload or download.
*
*             It is used to record the fact that the transfer was successful,
*             both in the caller log as well as the DOWNLOAD.TXT file.
*
*             It is also used to update the user statistics for files and
*             bytes transferred and also to update the accounting information
*             (charging or giving credit for transfers) taking into account
*             files that are "free".
*/

static void _NEAR_ LIBENTRY successful(spectype *File, char XferType) {
  int         FullSpeed;
  char        Str[80];
  pcbtexttype Buf;

  // if uploading a file into the message editor, then don't record the
  // file transfer anywhere...  but if it's a file attachment then go
  // ahead and record the upload

  if (Status.MsgUpload)
    return;

  // If the transfer was at the full rated speed (or higher) and this is
  // a FREETIME transfer - then chances are the caller is trying to cheat
  // and get back time that isn't his from a Zmodem RESUME protocol on a
  // file that had already been downloaded.  If so, we will ignore the
  // file and not add the time back in down below.

  FullSpeed = maxspeed();

  switch (XferType) {
    case 'D': XferVars.SuccessfulDnldFiles++;
              XferVars.SuccessfulDnldBytes += File->Size;
              File->Success = DNOKAY;

              if (! File->FreeFile) {
                if (Status.BytesRemaining != -1)
                  Status.BytesRemaining  -= File->Size;
                UsersData.DailyDnldBytes += File->Size;
                UsersData.TotDnldBytes   += File->Size;
                if (UsersData.NumDownloads < 65535U)
                  UsersData.NumDownloads++;
              }

              if (File->CapType == MSGCAP)
                Status.MsgCapCount++;
              else if (File->CapType == QWKCAP)
                Status.QwkCapCount++;

              #ifdef PCB152
                if (Status.ActStatus != ACT_DISABLED) {
                  recordusage("DNLD FILE",
                              File->Name,
                              AccountRates.ChargeForDownloadFile * File->Multiplier,
                              1,
                              &UsersData.Account.DebitDownloadFile);
                  recordusage("DNLD BYTES",
                              File->Name,
                              AccountRates.ChargeForDownloadBytes * File->Multiplier,
                              File->Size / 1024,
                              &UsersData.Account.DebitDownloadBytes);
                }
              #endif

              if (File->FreeTime && File->CPS < FullSpeed && File->CPS != 0) {
                // take the file SIZE and divide it by the CPS to determine
                // how long the transfer took then add 30 seconds to round
                // it up to the next minute and divide by 60 so that we can
                // add the appropriate number of minutes back in...
                if (File->CPS > 0) {
                  addtime(File->Size / File->CPS,CREDTIME);

                  #ifdef PCB152
                    // add a time credit if TIME is being charged since this
                    // is freetime file
                    if (Status.ActStatus != ACT_DISABLED) {
                      double TimeCharge = (currentlyinpeak() ? AccountRates.ChargeForPeakTime : AccountRates.ChargeForTime);
                      if (TimeCharge != 0) {
                        recordusage("FILETIME CREDIT",
                                    File->Name,
                                    -TimeCharge,
                                    (File->Size / File->CPS) / 60,
                                    &UsersData.Account.DebitTime);
                      }
                    }
                  #endif
                }
              }
              break;

    case 'U': XferVars.SuccessfulUpldFiles++;
              XferVars.SuccessfulUpldBytes += File->Size;
              File->Success = UPOKAY;
              if (! Status.FileAttach) {
                UsersData.TotUpldBytes += File->Size;
                if (UsersData.NumUploads < 65535U)
                  UsersData.NumUploads++;
                #ifndef PCB_DEMO
                  UsersData.DailyDnldBytes -= (File->Size * PcbData.ByteCredit) / 10;
                  if (Status.BytesRemaining != -1)
                    Status.BytesRemaining  += (File->Size * PcbData.ByteCredit) / 10;
                  if (File->CPS > 0)
                    addtime(((File->Size / File->CPS) * PcbData.UploadCredit) / 10,CREDTIME);
                #endif
              }

              #ifdef PCB152
                if (Status.ActStatus != ACT_DISABLED) {
                  recordusage("UPLD FILE",
                              File->Name,
                              AccountRates.PayBackForUploadFile * File->Multiplier,
                              1,
                              &UsersData.Account.CreditUploadFile);
                  recordusage("UPLD BYTES",
                              File->Name,
                              AccountRates.PayBackForUploadBytes * File->Multiplier,
                              File->Size / 1024,
                              &UsersData.Account.CreditUploadBytes);
                }
              #endif
              break;
  }

  // update the DOWNLOAD.TXT file
  updatedownloadtext(XferType,File->Name,TotErrors,File->CPS);

  // write to the caller log indicating the file transfer was successful
  getsystext(TXT_COMPLETEDUSING,&Buf);
  sprintf(Str,"(%c) %s %s%c%s  T/E=%d  CPS=%u",XferType,File->Name,Buf.Str,ProtocolLetterToUse,(Status.Batch ? " (B)" : ""),TotErrors,File->CPS);
  writelog(Str,SPACERIGHT);
}


static void _NEAR_ LIBENTRY cancelled(char *FileName, char XferType) {
  char        Str[80];
  pcbtexttype Buf;

  // if uploading a file into the message editor, then don't record the
  // file transfer anywhere, but if it is a file attachment then it is
  // okay to record it.

  if (Status.FileAttach || ! Status.EnteringMessage) {
    maxstrcpy(Status.DisplayText,FileName,sizeof(Status.DisplayText));
    getsystext(TXT_ABORTEDUSING,&Buf);
    sprintf(Str,"(%c) %s %s%c  T/E=%d",XferType,FileName,Buf.Str,ProtocolLetterToUse,TotErrors);
    writelog(Str,SPACERIGHT);
  }

  #ifdef COMM
    if (Asy.Online == REMOTE) {
      clearoutbuf();
      clearinbuf();
      waitcomm(ONESECOND);
      sendstr("        ",32);
      waitcomm(ONESECOND);
    }
  #endif
}


void LIBENTRY resetxfercounters(bool ForceReset) {
  // Only reset the transfer counters if NumFiles has been reset to 0.  If it
  // is non-zero, that means that there are still files flagged for download.
  // ForceReset is used at the start of the send() function in order to ensure
  // that the numbers are recalculated.

  if (NumFiles == 0 || ForceReset) {
    IgnoreFiles       = 0;
    IgnoreBytes       = 0;
    IgnoreSeconds     = 0;
    TotalBytesSoFar   = 0;
    TotalFilesSoFar   = 0;
    TotalSecondsSoFar = 0;
  }
}


static void _NEAR_ LIBENTRY transferstatus(xferstatustype Stat) {
  switch (Stat) {
    case START: Control.WatchKbdClock = FALSE;
                Control.WatchSessionClock = FALSE;
                Status.FileXfer = TRUE;
                #ifdef COMM
                  Asy.IgnoreCDLoss = TRUE;
                #endif
                LastError = NOERROR;
                writeusernetstatus(TRANSFER,NULL);
                redisplaystatusline();
                break;
    case STOP : turnkbdtimeron();
                Control.WatchSessionClock = TRUE;
                Status.FileXfer = FALSE;
                Status.Batch = FALSE;
                resetxfercounters(FALSE);
                #ifdef PCB152
                  checkaccountbalance();
                #endif
                redisplaystatusline();
                if (Status.KbdTimedOut)
                  loguseroff(ALOGOFF);
                #ifdef COMM
                  Asy.IgnoreCDLoss = FALSE;
                  if (Asy.Online == REMOTE) {
                    if (Asy.LostCarrier || cdstillup() == 0) {
                      Asy.LostCarrier = TRUE;
                      if (PcbData.Packet)
                        turnoffdtr();
                      loguseroff(ALOGOFF);
                    }
                  }
                #endif
                usernetavailable();
                break;
  }
}


static void _NEAR_ LIBENTRY showfileinusernet(char Mode, spectype *File) {
  char Time[30];
  char Size[30];
  char Operation[80];

  if (File == NULL) {
    sprintf(Operation,"(%c) ... (%c) ...",Mode,ProtocolLetterToUse);
  } else {
    downloadtime(File->Size,Time,NULL);
    comma(Size,File->Size);
    sprintf(Operation,"(%c) %s (%c) %s  - %s",Mode,File->Name,ProtocolLetterToUse,Size,Time);
  }
  writeusernetstatus(TRANSFER,Operation);
}


#ifdef COMM
#pragma warn -rvl
static int _NEAR_ LIBENTRY waitforchar(char *List) {
  int  Key;
  char *p;

  while (1) {
    #ifdef __OS2__
      if (InBytes == 0) {
        WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
        WaitKeySem.reset();
      }
    #endif

    if ((Key = commportinkey()) != 0) {
      switch (Key) {
        case  -1: return(-1);          /* carrier was dropped, abort     */
        case CAN: if (Cancel)          /* have we had a CAN before?      */
                    return(-1);        /*   yes, abort the xfer          */
                  Cancel = TRUE;       /*   no, remember this one        */
                  continue;            /*       and wait for more        */
      }
      Cancel = FALSE;                  /* Turn off the Cancel flag       */
      for (p = List; *p != 0; p++)     /* Scan the list of characters to */
        if (*p == Key)                 /* see if Key is in the list      */
          return(Key);                 /* if so, return it now           */
    }                                  /* else loop up and wait for more */
    if (kbdinkey() == 27)              /* ESC character from keyboard?   */
      return(-1);                      /*   yes, abort the xfer          */
    if (timerexpired(4))               /* Has our timer expired?         */
      return(-2);                      /*   yes, signal a timeout        */
    #ifndef __OS2__
      giveup();                        /* give up cycles while we wait   */
    #endif
  }
}
#pragma warn +rvl

#pragma warn -rvl
static int _NEAR_ LIBENTRY watchdlcommport(void) {
  char static List[3] = {ACK,NAK,0};

  while (1) {
    switch (waitforchar(List)) {
      case -1 : return(-1);            /* abort/keyboard abort/lost carrier  */
      case ACK: Errors = 0;            /* clear the errors counter           */
                return(0);             /* return 0 to continue next block    */
      case -2 :                        /* timeout occurred                   */
      case NAK: TotErrors++;           /* Add 1 to the TOTAL # of errors     */
                Errors++;              /* Add 1 to the Current # of errors   */
                updatescreen(DOWNLOAD);/* update the scrn to show the errors */
                if (Errors == 10)      /* if the num of consequtive errors   */
                  return(-1);          /* is equal to 10 then return -1      */
                return(-2);            /* otherwise return -2                */
    }
  }
}
#pragma warn +rvl


static int _NEAR_ LIBENTRY sendblock(char *Block) {
  int  RetVal;
  int  Size;
  int  Key;
  long TimeOut;

/*
   NOTE: for calculating the timeout period - which is the length of time
   it take to send the complete block + 20 seconds - the following formula
   is used:
                                                   Size * 182      Size * 182
   18.2 * (Size/CPS)  =  (182/10) * (Size/CPS)  =  ----------  =  ------------
                                                    CPS * 10      CarrierSpeed

   NOTE:  Under OS/2, multiply it by 10,000 (that's 1000 which represents 1
          second, multiplied by 10 because carrier speed is 10 times the
          cps rate)

*/

  switch (Type) {
    case SUM: Size     = 132;
              Block[0] = SOH;
              #ifdef __OS2__
                Block[131] = xmodemchecksum(&Block[3],BlockSize);
                TimeOut    = 132 * 10000;
              #else
                xmodem(&Block[3],BlockSize);  //lint !e534  return value in DL
                Block[131] = _DL;
                TimeOut    = 132 * 182;
              #endif
              break;
    case CRC: if (BlockSize == 128) {
                Size     = 133;
                Block[0] = SOH;
                Block[131] = Block[132] = 0;
                #ifdef __OS2__
                  unsigned short Crc = xmodemcrc(&Block[3],130);
                  Block[131] = (unsigned char) ((Crc & 0xFF00) >> 8);
                  Block[132] = (unsigned char) (Crc & 0x00FF);
                  TimeOut    = 133 * 10000;
                #else
                  xmodem(&Block[3],130);      //lint !e534  return value in AX
                  Block[131] = _AH;
                  Block[132] = _AL;
                  TimeOut    = 133 * 182;
                #endif
              } else {
                Size     = 1029;
                Block[0] = STX;
                Block[1027] = Block[1028] = 0;
                #ifdef __OS2__
                  unsigned short Crc = xmodemcrc(&Block[3],1026);
                  Block[1027] = (unsigned char) ((Crc & 0xFF00) >> 8);
                  Block[1028] = (unsigned char) (Crc & 0x00FF);
                  TimeOut     = 1029 * 10000;
                #else
                  xmodem(&Block[3],1026);      //lint !e534  return value in AX
                  Block[1027] = _AH;
                  Block[1028] = _AL;
                  TimeOut     = 1029 * 182L;
                #endif
              }
              break;
    default : // do nothing (ASCII doesn't use either CRC or CHECKSUM)
              break;
  }


  switch (Flow) {
    case HANDSHAKE: do {
                      sendstr(Block,Size);  //lint !e644
                      settimer(4,TWENTYSECONDS + (TimeOut/Asy.CarrierSpeed));  //lint !e644
                      #ifdef __OS2__
                        releasekbdblock(TWENTYSECONDS + (TimeOut/Asy.CarrierSpeed));
                      #else
                        giveup();
                      #endif
                      RetVal = watchdlcommport();
                    } while (RetVal == -2);
                    return(RetVal);
    case ASCII    : checkstatus();
                    if (Display.AbortPrintout) {
                      Display.AbortPrintout = FALSE;
                      return(-1);
                    }
                    Size = BlockSize;
                    Block += 3;
                    /* fall thru to send the ASCII block as full flow data */
    case FULLFLOW : sendstr(Block,Size);    //lint !e644
                    /* Don't WAIT for a character and don't call giveup! */
                    if ((Key = commportinkey()) != 0)
                      switch (Key) {
                        case -1 : return(-1);
                        case CAN: if (Cancel)
                                    return(-1);
                                  Cancel = TRUE;
                                  break;
                        default : Cancel = FALSE;
                                  break;
                      }
                    if (kbdinkey() == 27)
                      return(-1);
                    break;
    default : return(-1);
  }
  return(0);
}


#pragma warn -rvl
static int _NEAR_ LIBENTRY sendymodemheaderblock(spectype *File, char *Block) {
  char static List[6] = {NAK,ACK,STARTy,STARTg,STARTc,0};
  int  Temp;
  char *p;

  Block[0] = SOH;
  Block[1] = 0;
  Block[2] = 0xFF;

  Temp = strlen(File->Name);
  p = &Block[3];
  memset(p,0,128);

  if (File->Name[0] != 0) {
    strcpy(p,File->Name);
    strlwr(p);
    p += Temp+1;
    lascii(p,File->Size);
  }

  Block[131] = Block[132] = 0;
  #ifdef __OS2__
    unsigned short Crc = xmodemcrc(&Block[3],130);
    Block[131] = (unsigned char) ((Crc & 0xFF00) >> 8);
    Block[132] = (unsigned char) (Crc & 0x00FF);
  #else
    xmodem(&Block[3],130);      //lint !e534  return value in AX
    Block[131] = _AH;
    Block[132] = _AL;
  #endif

  settimer(4,NINETYSECONDS);
  #ifdef __OS2__
    releasekbdblock(NINETYSECONDS);
  #endif

  while (1) {
    sendstr(Block,133);
    #ifndef __OS2__
      giveup();
    #endif
    if (File->Name[0] == 0)
      return(0);
    switch(waitforchar(List)) {
      case -2    :
      case -1    : return(-1);
      case NAK   : break;         /* loop back and send again */
      case ACK   : goto next;
      case STARTy:
      case STARTg:
      case STARTc: return(0);
    }
  }

next:
  while (1) {
    switch(waitforchar(List)) {
      case -2    :
      case -1    : return(-1);
      case STARTy:
      case STARTg:
      case STARTc: return(0);
    }
    #ifndef __OS2__
      giveup();
    #endif
  }
}
#pragma warn +rvl


static int _NEAR_ LIBENTRY sendall(bool Batch) {
  char static List[4] = {STARTy,STARTg,STARTc,0};
  int      Key;
  int      BytesRead;
  unsigned NumBlocks;
  int      SaveSize;
  unsigned DownShiftBlockNum;
  int      EOTcount;
  long     ElapsedTime;
  DOSFILE  InFile;
  spectype File;
  char     Block[1040];

  SaveSize = BlockSize;

  for (FileNum = 0; FileNum < NumFiles; FileNum++) {
    if (getfilespec(FileNum,&File) == -1)
      goto cancel;

    BlockSize = SaveSize;
    if (dosfopen(File.FullPath,OPEN_READ|OPEN_DENYWRIT,&InFile) == -1)
      goto cancel;

    showfileinusernet('D',&File);
    dossetbuf(&InFile,16384);
    BytesSoFar = 0;
    DownShiftBlockNum = calcdownshift(File.Size,&NumBlocks);
    TotErrors = 0;
    paintdlscreen(&File);
    setupxferscale(NumBlocks);

    if (Batch) {
      if (sendymodemheaderblock(&File,Block) == -1) {
        dosfclose(&InFile);
        goto cancel;
      }
    }

    XferStartTime = dosgetlongtime();
    for (BlockNum = 1; BlockNum <= NumBlocks; BlockNum++) {
      updatescreen(DOWNLOAD);
      Block[1] = (char) BlockNum;
      Block[2] = Block[1] ^ 0xFF;
      if (BlockNum == DownShiftBlockNum)
        BlockSize  = 128;
      if ((BytesRead = dosfread(&Block[3],BlockSize,&InFile)) == -1) {
        dosfclose(&InFile);
        goto cancel;
      }
      if (BytesRead != BlockSize)  /* if the data read in is shorter than a full */
        memset(&Block[3+BytesRead],26,BlockSize-BytesRead);   /* block, then pad it */
      if (sendblock(Block) == -1) {
        dosfclose(&InFile);
        if (BlockNum == NumBlocks)
          goto done;               /* possible LEECH! count it as done */
        else
          goto cancel;
      }
      BytesSoFar += BlockSize;
    }
    dosfclose(&InFile);

    BytesSoFar = File.Size;
    while (1) {          /* wait for the output buffer to empty */
      waitforempty(ONESECOND);
      updatescreen(DOWNLOAD);
      if (OutBytes <= 1)
        break;

      Key = cinkey();
      if (Key == CAN || Key == -1 || Key == 27)
        goto done;
      turnonxmit();
    }

    if (Flow != ASCII) {
      settimer(4,-1);
      EOTcount = 0;
      while (1) {
        if (timerexpired(4)) {
          EOTcount++;
          if (EOTcount > 10)  /* don't send more than 10 EOT's (100 seconds) */
            goto done;        /* done after 10 instead of waiting for ACK    */
          sendbyte(EOT);
          settimer(4,TENSECONDS);
        }
        giveup();
        updatescreen(DOWNLOAD);
        switch(cinkey()) {
          case 27    : goto cancel;
          case -1    :
          case CAN   :
          case STARTy:
          case STARTg:
          case STARTc:
          case ACK   : goto done;
          case NAK   : Errors++;
                       TotErrors++;
                       if (Errors > 10)
                         goto done;
                       settimer(4,-1);
                       break;
        }
      }
    }

done:
    ElapsedTime = doselapsedtime(XferStartTime);
    updatescreen(DOWNLOAD);
    File.CPS = cps(File.Size,ElapsedTime);
    successful(&File,'D');
    if (putfilespec(FileNum,&File) == -1)
      goto cancel;
    if (! Batch)        /* get out now if it's only a SINGLE file */
      return(0);
    settimer(4,TENSECONDS);
    #ifdef __OS2__
      releasekbdblock(TENSECONDS);
    #endif
    if (waitforchar(List) < 0)
      return(FileNum == NumFiles-1 ? 0 : -1);
    if (FileNum == NumFiles-1) {
      File.Name[0] = 0;
      sendymodemheaderblock(&File,Block);  //lint !e534  don't check for "-1" here cuz it's good no matter what
      return(0);
    }
  }

cancel:
  cancelled(File.Name,'D');
  return(-1);
}
#endif


static int _NEAR_ LIBENTRY localtransfer(spectype *File, char *SrcePath, char *DestPath, xfertype XferType) {
  unsigned NumBlocks;
  int      BytesRead;
  int      Ret;
  #ifdef __BORLANDC__
    struct  ftime TimeDate;
  #else
    uint    Time;
    uint    Date;
  #endif
  DOSFILE  InFile;
  DOSFILE  OutFile;
  char     Buf[1024];

  TotErrors  = 0;
  BytesSoFar = 0;
  BlockSize  = 1024;
  XferStartTime = dosgetlongtime();

  if (dosfopen(SrcePath,OPEN_READ|OPEN_DENYNONE,&InFile) == -1)
    return(-1);

  File->Size = dosfseek(&InFile,0,SEEK_END);
  calcdownshift(File->Size,&NumBlocks);     //lint !e534

  if (XferType == DOWNLOAD)
    paintdlscreen(File);
  else
    paintulscreen(File->Name,File->Size);

  setupxferscale(NumBlocks);

  if (stricmp(SrcePath,DestPath) == 0) {
    dosfclose(&InFile);
    goto done;
  }

  if (dosfopen(DestPath,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&OutFile) == -1) {
    dosfclose(&InFile);
    return(-1);
  }

  dosrewind(&InFile);
  showfileinusernet((XferType == DOWNLOAD ? 'D' : 'U'),File);

  dossetbuf(&OutFile,16384);
  dossetbuf(&InFile,16384);

  for (BlockNum = 1; (BytesRead = dosfread(Buf,sizeof(Buf),&InFile)) > 0; BlockNum++) {
    if (dosfwrite(Buf,BytesRead,&OutFile) == -1) {
      dosfclose(&InFile);
      dosfclose(&OutFile);
      unlink(DestPath);
      return(-1);
    }
    BytesSoFar += BytesRead;
    updatescreen(XferType);
  }

  Ret = 0;
  if (XferType == DOWNLOAD) {
    Ret = dosflush(&OutFile);
    #ifdef __BORLANDC__
      getftime(InFile.handle,&TimeDate);
      setftime(OutFile.handle,&TimeDate);
    #elif !defined(__OS2__)
      _dos_getftime(InFile.handle,(unsigned *)&Date,(unsigned *)&Time);
      _dos_setftime(OutFile.handle,Date,Time);
    #endif
  }

  dosfclose(&InFile);
  if (dosfclose(&OutFile) == -1 || Ret == -1)
    return(-1);

done:
  BytesSoFar = File->Size;
  updatescreen(XferType);
  File->CPS = cps(File->Size,doselapsedtime(XferStartTime));
  successful(File,XferType == DOWNLOAD ? 'D' : 'U');
  return(0);
}


static int _NEAR_ LIBENTRY sendlocal(void) {
  char     Path[66];
  char     DestPath[66];
  spectype File;

  if (getfilespec(0,&File) == -1)
    goto cancel;

  Path[0] = 0;
  getlocalinput(Path,TXT_LOCALDNLDPATH,sizeof(Path)-13,mask_filename);
  if (Path[0] == 0)
    goto cancel;

  if (Path[2] == 0 && Path[1] == ':') {  /* just a drive specifier? */
    /* don't add a backslash to the drive specifier - use default directory */
  } else
    addbackslash(Path,sizeof(Path)-13);

  for (FileNum = 0; FileNum < NumFiles; FileNum++) {
    if (getfilespec(FileNum,&File) == -1)
      goto cancel;
    sprintf(DestPath,"%s%s",Path,File.Name);
    if (localtransfer(&File,File.FullPath,DestPath,DOWNLOAD) == -1)
      goto cancel;
    if (putfilespec(FileNum,&File) == -1)
      goto cancel;
  }
  return(0);

cancel:
  cancelled(File.Name,'D');
  return(-1);
}


static bool _NEAR_ LIBENTRY shellout(char *Command, long *Seconds) {
  int  RetCode;
  char Line1;
  char Line2;
  long StartTime;
  long ElapsedTime;
  char Str[80];

  if (DszLog[0] != 0)
    unlink(DszLog);
  unlink("PCBERR.FIL");

  if (DebugLevel > 0) {
    sprintf(Str,"DSZLOG=%s",DszLog);
    writelog(Str,SPACERIGHT);
  }

  #ifdef COMM
    if (ProtocolToUse.PortOpen) {
      #ifdef __OS2__
        // get rid of the modem thread so that it doesn't interfere with the
        // shelled-to process which might try to use the comm handle
        destroymodemthread();
      #endif
    } else {
      closemodem(FALSE);
    }
  #endif

  Line1 = Status.StatusLine1;
  Line2 = Status.StatusLine2;
  Status.StatusLine1 = 0;
  Status.StatusLine2 = 1;
  cls();
  redisplaystatusline();
  fastprint(7,2,"(PCBoard Shell)  * Protocol Transfer - Function Keys Disabled *",0x0F);
  agotoxy(0,3);

  #ifndef __OS2__
    if (ProtocolToUse.LockLines)
      begnoscroll();
  #endif

  StartTime = dosgetlongtime();
  RetCode   = performshell(Command,"",SHELLVIACOMMAND,PcbData.PriorityProtocols,PcbData.MinimizeProtocols & 1,PcbData.MinimizeProtocols & 2,-1);
  if ((ElapsedTime = doselapsedtime(StartTime)) < 100)
    ElapsedTime = 100;

  #ifndef __OS2__
    if (ProtocolToUse.LockLines)
      endnoscroll();
  #endif

  Status.StatusLine1 = Line1;
  Status.StatusLine2 = Line2;

  if (Status.ForceScreenOff) {
    turndisplayon(FALSE);
    cls();
    turndisplayoff(FALSE);
    Status.ForceScreenOff = TRUE;  /* turndisplayon() set it to FALSE */
  }

  #ifdef COMM
    if (ProtocolToUse.PortOpen) {
      #ifdef __OS2__
        // the port is still open so we need to reestablish the modem threads
        createmodemthread();
      #endif
    } else {
      // it was closed up above, so reopen it now
      reopenport();
    }
    if (Asy.Online == REMOTE) {
      online();                   //lint !e534  this just sets the state
      if (cdstillup() == 0) {
        Asy.LostCarrier = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
      }
    }
  #endif

  unlink("PCBDSZ.LST");

  *Seconds = ElapsedTime;
  return((bool) (RetCode != 0 || fileexist("PCBERR.FIL") != 255));
}


static void _NEAR_ LIBENTRY parsedszrecord(char *Buffer, dszlogtype *Log) {
  char *p;
  char *Sep;
  char Str[80];

  if (tokenizestr(Buffer) < 11) {
    memset(Log,0,sizeof(dszlogtype));
    return;
  }

  p = getnexttoken();
  Log->Check = *p;                          /* get STATUS character */
  Log->FileSize = atol(getnexttoken());     /* get file size */
  getnexttoken();                           /* get connect speed */
  getnexttoken();                           /* get 'bps' chars   */
//Log->CPS = cpsmax(atol(getnexttoken()));  /* get CPS speed */
  Log->CPS = (unsigned short)atol(getnexttoken());/* get CPS speed */
  getnexttoken();                           /* get 'cps' chars */
  Log->NumErrors = atoi(getnexttoken());    /* get number of errors */
  getnexttoken();                           /* get 'errors' chars */
  getnexttoken();                           /* get block size */
  getnexttoken();
  maxstrcpy(Str,getnexttoken(),sizeof(Str));  /* get name */

  change(Str,'/','\\');      /* change forward slashes to backslashes */

  /* Filenames inside of DSZLOG may sometimes have names that are longer */
  /* than 8 characters, extensions that are longer than 3 characters or  */
  /* simply names plus extensions longer than DOS's 12 character limit.  */
  /* Below we will attempt to correct whatever name has been uploaded.   */

  if ((Sep = strrchr(Str,'\\')) != NULL)
    Sep++;  /* point to first letter of filename */
  else
    Sep = Str;  /* no backslash found, point to first letter of Str */

  if ((p = strchr(Sep,'.')) != NULL && p - Sep > 9)
    strcpy(Sep + 8,p);
  if (strlen(Sep) > 12)
    Sep[12] = 0;
  maxstrcpy(Log->Name,Str,sizeof(Log->Name));
}


static void _NEAR_ LIBENTRY getdszfullpath(char *Str, char *FullPath, char *FileName) {
  char *p;
  char Buf[80];

  if ((p = strrchr(Str,'\\')) == NULL)
    p = Str;
  else
    p++;

  /* at this point "p" points to just the name */

  strcpy(FullPath,(Status.EnteringMessage ? PcbData.TmpLoc : Status.CurConf.PrvUpldLoc));
  strcat(FullPath,p);
  strcpy(FileName,p);
  if (DebugLevel >= 3) {
    sprintf(Buf,"DSZPATH=%s",FullPath);
    writelog(Buf,SPACERIGHT);
  }
}


static long _NEAR_ LIBENTRY finddszuploadedfile(dszlogtype *DszBuf, char *FullPath, char *FileName) {
  if (DszBuf->Check == 'E' || DszBuf->Check == 'L')
    return(0);

  if (fileexist(FullPath) == 255) {
    if (fileexist(FileName) == 255)
      return(0);
    else if (DTA.ff_fsize == 0)
      return(0);
    pcbmovefile(FileName,FullPath);    //lint !e534
  } else if (DTA.ff_fsize == 0)
    return(0);

  return(DTA.ff_fsize);
}


static void _NEAR_ LIBENTRY uploaded(dszlogtype *DszBuf) {
  int      X;
  long     Size;
  char     FullPath[66];
  char     FileName[66];
  spectype File;

  X = 0;
  getdszfullpath(DszBuf->Name,FullPath,FileName);
  if ((Size = finddszuploadedfile(DszBuf,FullPath,FileName)) == 0) {
    DszBuf->Check = 'E';
    goto post;
  }

  /* if filename was NOT found in the list of files the user specified */
  /* were to be uploaded - then he sent some behind our back so let's  */
  /* add them to the list, check them via UPSEC and check for any      */
  /* duplicate files by the same name already posted...                */
  if ((X = addfiletolist(FileName,Status.CurConf.PrvUpldLoc,Size,TRUE,TRUE)) < 0) {
    DszBuf->Check = 'E';  /* can't add it so cancel it */
    goto post;
  }

  if (getfilespec(X,&File) == -1 || fsecokay(File.FullPath,File.Name,CHECKUPSEC,Status.CurConf.PubUpldLoc) == -1) {
    DszBuf->Check = 'E';
    goto post;
  }

  File.Multiplier = Status.FsecMultiplier;

  /* ignore the file if it is found in the        */
  /* private upload location where we just put it */
  if (scandlpaths(CHECKUPLDS|REPORTONLY|IGNOREPRIVATE,File.Name) > 0) { /* we must have found one! */
    maxstrcpy(Status.DisplayText,File.Name,sizeof(Status.DisplayText));
    displaypcbtext(TXT_DUPLICATEFILE,NEWLINE|LOGIT);
    DszBuf->Check = 'E';  /* already exists so cancel it */
    goto post;
  }

post:
  TotErrors = DszBuf->NumErrors;
  if (DszBuf->Check == 'E') {
    unlink(FullPath);
    cancelled(FileName,'U'); /* don't use FileList cuz it might not be in there */
    if (X > 0) {
      File.Success = REMOVE;
      putfilespec(X,&File);
    }
  } else {
    File.Size = Size;
    File.CPS  = DszBuf->CPS;
    successful(&File,'U');
    putfilespec(X,&File);
  }
}


static int _NEAR_ LIBENTRY externaldownload(spectype *File) {
  bool        Aborted;
  unsigned short AveCPS;
  int         Found;
  int         X;
  char        *p;
  long        Size;
  long        Seconds;
  DOSFILE     OutFile;
  DOSFILE     InFile;
  char        FileName[66];
  char        BatchFile[66];
  dszlogtype  DszBuf;
  char        Command[128];

  BatchMask[3] = 'S';
  BatchMask[4] = ProtocolLetterToUse;
  if (findbat(BatchFile,BatchMask) == -1)
    return(-1);

  if (NumFiles == 1) {
    strcpy(FileName,File->FullPath);
  } else {
    unlink("PCBDSZ.LST");
    if (dosfopen("PCBDSZ.LST",OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&OutFile) == -1)
      return(-1);
    for (X = 0; X < NumFiles; X++) {
      if (getfilespec(X,File) == -1)
        break;
      dosfputs(File->FullPath,&OutFile);    //lint !e534
      dosfputs("\r\n",&OutFile);            //lint !e534
    }
    dosfclose(&OutFile);
    FileName[0] = '@';
    FileName[1] = (char) (dosgetcurdrive()+'A'-1);
    FileName[2] = ':';
    strcpy(&FileName[3],"PCBDSZ.LST");
  }

  showfileinusernet('D',File);

  sprintf(Command,"%s %d %ld %s %ld %ld",BatchFile,Asy.ComPortNumber,Asy.ModemSpeed,FileName,Asy.ConnectSpeed,Asy.CarrierSpeed);

  // Change as of 7/21/95 - only add the 6th parameter (%6), which is the
  // location of the private upload directory, if the transfer protocol is a
  // bidirectional transfer protocol.  This was done because the command line
  // was exceeding 128 characters under the OS/2 version of PCBoard when
  // shelling out to a DOS program.  A suggested fix for the customer is to
  // reduce the length of the path for the private upload directory

  if (ProtocolToUse.BiDirect) {
    addchar(Command,' ');
    strcat(Command,Status.CurConf.PrvUpldLoc);
  }

  Aborted = shellout(Command,&Seconds);

  if (ProtocolToUse.Batch && DszLog[0] != 0 && fileexist(DszLog) != 255 &&
      dosfopen(DszLog,OPEN_READ|OPEN_DENYNONE,&InFile) != -1) {

    Aborted = FALSE;

    while (dosfgets(Command,sizeof(Command),&InFile) != -1) {
      parsedszrecord(Command,&DszBuf);
      if (DszBuf.Name[0] == 0)
        continue;

      if (DszBuf.FileSize == 0)  /* make sure 0-bytes transfer equals abort */
        DszBuf.Check = 'E';

      if ((p = strrchr(DszBuf.Name,'\\')) == NULL)
        p = DszBuf.Name;
      else
        p++;

      /* at this point "p" points to just the name */

      if ((Found = findfilename(p)) == -1) {
        if (ProtocolToUse.BiDirect)
          uploaded(&DszBuf);
      } else {
        if (getfilespec(Found,File) == -1)
          break;

        File->CPS = DszBuf.CPS;
        TotErrors = DszBuf.NumErrors;

        // Apparently if you SKIP a file, using DSZ, without the "-s" slugbait
        // option, all files AFTER the skipped file will show up with an "E" as
        // the status - indicating that the file was ABORTED!
        //
        // Another "signature" of the problem is that the number of errors will
        // be set to 99.  So if the log shows "E" as the status, and 99 as the
        // number of errors, and the number of bytes sent is equal to the
        // actual size of the file .. then let's assume that the file transfer
        // was SUCCESSFUL and override the "E" (abort) status.
        if (TotErrors == 99 && DszBuf.Check == 'E' && DszBuf.FileSize == File->Size)
          DszBuf.Check = 'z';

        switch (DszBuf.Check) {
          case 'E':
          case 'L': cancelled(File->Name,'D');
                    Aborted = TRUE;
                    break;
          default : successful(File,'D');
                    putfilespec(Found,File);
                    break;
        }
      }
    }
    dosfclose(&InFile);
  } else if (! Aborted) {
    for (X = 0, Size = 0; X < NumFiles; X++) {
      if (getfilespec(X,File) == -1)
        break;
      Size += File->Size;
    }
    AveCPS = cps(Size,Seconds);
    for (X = 0; X < NumFiles; X++) {
      if (getfilespec(X,File) == -1)
        break;
      File->CPS = AveCPS;
      successful(File,'D');
      putfilespec(X,File);
    }
  }

  if (Aborted)
    return(-1);
  return(0);
}


static int _NEAR_ LIBENTRY sendfile(spectype *File) {
  #ifdef COMM
  bool         Batch;
  #endif
  char         OldX;
  char         OldY;
  int          Okay;
  int          Saved;

  #ifdef COMM
    if (Asy.Online == REMOTE) {
      waitforempty(TENSECONDS);   // wait for our output bytes to go out
      clearinbuf();               // clear the input buffer
    }
  #endif

  OldX = awherex();
  OldY = awherey();

  if (! ProtocolToUse.Internal) {
    Saved = memsavescreen();
    Okay = externaldownload(File);
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);
    if (Control.GraphicsMode) { /* force it to reset the color! */
      asetcolor(0);
      printdefcolor();
    }
    if (Okay == -1)
      goto cancel;
    return(0);
  }

  FileNum   = 0;
  Cancel    = FALSE;
  Errors    = 0;
  TotErrors = 0;

  if (Asy.Online == LOCAL) {
    Saved = memsavescreen();
    Okay = sendlocal();
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);
    if (Okay != -1)
      return(0);
    goto cancel;
  }

  #ifdef COMM
    settimer(4,NINETYSECONDS);
    #ifdef __OS2__
      releasekbdblock(NINETYSECONDS);
    #endif

    while (1) {
      #ifdef __OS2__
        if (InBytes == 0) {
          WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
          WaitKeySem.reset();
        }
      #endif

      if (timerexpired(4)) {
        cancelled(File->Name,'D');
        goto cancel;
      }

      switch (cinkey()) {
        case 13    : if (ProtocolLetterToUse == 'A') {
                       Batch = FALSE; Type = PURE; Flow = ASCII; BlockSize = 1024; goto start;
                     }
                     break;
        case 27    : if (! Status.Kbd) break;
        case -1    : //lint !e616
        case CAN   : cancelled(File->Name,'D'); goto cancel;
        case STARTx: switch (ProtocolLetterToUse) {
                       case 'X':
                       case 'C': setxferprotocol('X'); Batch = FALSE; Type = SUM; Flow = HANDSHAKE; BlockSize = 128;  goto start;
                     }
                     break;
        case STARTy: switch (ProtocolLetterToUse) {
                       case 'O': setxferprotocol('O'); Batch = FALSE; Type = CRC; Flow = HANDSHAKE; BlockSize = 1024; goto start;
                       case 'Y': setxferprotocol('Y'); Batch = TRUE;  Type = CRC; Flow = HANDSHAKE; BlockSize = 1024; goto start;
                     }
                     break;
        case STARTg: switch (ProtocolLetterToUse) {
                       case 'F': setxferprotocol('F'); Batch = FALSE; Type = CRC; Flow = FULLFLOW;  BlockSize = 1024; goto start;
                       case 'G': setxferprotocol('G'); Batch = TRUE;  Type = CRC; Flow = FULLFLOW;  BlockSize = 1024; goto start;
                     }
                     break;
        case STARTc: switch (ProtocolLetterToUse) {
                       case 'O': setxferprotocol('O'); Batch = FALSE; Type = CRC; Flow = HANDSHAKE; BlockSize = 1024; goto start;
                       case 'X':
                       case 'C': setxferprotocol('C'); Batch = FALSE; Type = CRC; Flow = HANDSHAKE; BlockSize = 128;  goto start;
                       case 'Y': setxferprotocol('Y'); Batch = TRUE;  Type = CRC; Flow = HANDSHAKE; BlockSize = 1024; goto start;
                     }
                     break;
      }
    }

start:
    Saved = memsavescreen();
    Okay = sendall(Batch);
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);
    if (Control.GraphicsMode) { /* force it to reset the color! */
      asetcolor(0);
      printdefcolor();
    }
    if (Okay != -1)
      return(0);
  #endif

cancel:
  if (! Asy.LostCarrier) {
    #ifdef COMM
      waitcomm(THREESECONDS);
    #endif
    checkdisplaystatus();
    displaypcbtext(TXT_TRANSFERABORTED,NEWLINE|LFBEFORE|BELL);
  }
  return(-1);
}


static int _NEAR_ LIBENTRY externalupload(void) {
  bool        Aborted;
  int         Found;
  unsigned short AveCPS;
  int         Len;
  int         X;
  char        *p;
  long        Seconds;
  long        Size;
  long        TotalSize;
  DOSFILE     InFile;
  char        FullPath[66];
  char        FileName[66];
  char        BatchFile[66];
  dszlogtype  DszBuf;
  char        Command[128];
  spectype    File;

  BatchMask[3] = 'R';
  BatchMask[4] = ProtocolLetterToUse;
  if (findbat(BatchFile,BatchMask) == -1)
    return(-1);

  memset(&File,0,sizeof(File));

  if (ProtocolToUse.Batch && ! (PcbData.NoBatchUp || Status.EnteringMessage)) {
    strcpy(FileName,Status.CurConf.PrvUpldLoc);
    stripright(FileName,'\\');
  } else {
    if (getfilespec(0,&File) == -1)
      return(-1);
    strcpy(FileName,File.FullPath);
  }

  showfileinusernet('U',NULL);

  sprintf(Command,"%s %d %ld %s %ld %ld %s",BatchFile,Asy.ComPortNumber,Asy.ModemSpeed,FileName,Asy.ConnectSpeed,Asy.CarrierSpeed,(Status.EnteringMessage ? PcbData.TmpLoc : Status.CurConf.PrvUpldLoc));
  shellout(Command,&Seconds);    //lint !e534
  Aborted = TRUE;  /* default to aborted and change to false if at least 1 file was okay */

  if (ProtocolToUse.Batch && DszLog[0] != 0 && fileexist(DszLog) != 255 &&
      dosfopen(DszLog,OPEN_READ|OPEN_DENYNONE,&InFile) != -1) {
    while (dosfgets(Command,sizeof(Command),&InFile) != -1) {
      parsedszrecord(Command,&DszBuf);
      if (DszBuf.Name[0] == 0)
        continue;

      getdszfullpath(DszBuf.Name,FullPath,FileName);
      p = FileName;

      // HS/Link does not allow you to OVERRIDE the caller-specified-filename
      // on an upload and therefore HS/Link might upload the file under a
      // different name.  So check to see if we are in a NoBatchUp environment
      // or if we are receiving a file into the message editor or a file
      // attachment or a REP packet and if so, compare the received filename
      // with what we were expecting and rename it/move it as needs be.

      if (PcbData.NoBatchUp || Status.EnteringMessage) {
        if (stricmp(FullPath,FileName) != 0 && fileexist(FullPath) != 255) {
          pcbmovefile(FullPath,FileName);  //lint !e534
          p = File.Name;
        }
      }

      /* at this point "p" points to just the name */

      Found = findfilename(p);
      if (Found == -1 && strchr(p,'.') != NULL) {
        for (X = 0; X < NumFiles; X++) {
          if (getfilespec(X,&File) == -1)
            break;
          if (strchr(File.Name,'.') == NULL) {
            Len = strlen(File.Name);
            if (memcmp(File.Name,p,Len) == 0) {
              Found = X;
              strcat(File.Name,p+Len);
              strcat(File.FullPath,p+Len);
              File.Checked = FALSE;  /* we never checked the name w/ an extension */
              putfilespec(X,&File);
              break;
            }
          }
        }
      }

      if (Found != -1) {
        getfilespec(Found,&File);           //lint !e534
        strcpy(FullPath,File.FullPath);
        strcpy(FileName,File.Name);
      } else {
        strcpy(FullPath,Status.CurConf.PrvUpldLoc);
        strcat(FullPath,p);
        strcpy(FileName,p);
      }

      if ((Size = finddszuploadedfile(&DszBuf,FullPath,FileName)) == 0) {
        DszBuf.Check = 'E';
        goto post;
      }

      /* if filename was NOT found in the list of files the user specified */
      /* were to be uploaded - then he sent some behind our back so let's  */
      /* add them to the list, check them via UPSEC and check for any      */
      /* duplicate files by the same name already posted...                */
      if (Found == -1) {
        if ((Found = addfiletolist(FileName,Status.CurConf.PrvUpldLoc,Size,TRUE,TRUE)) < 0) {
          DszBuf.Check = 'E';  /* can't add it so cancel it */
          goto post;
        }
        if (getfilespec(Found,&File) == -1) {
          DszBuf.Check = 'E';  /* can't get it so cancel it */
          goto post;
        }
      }

      // If the file has not been checked, and it is NOT a message upload or
      // file attachment, then check it now to verify that it passes the
      // security check and that it is not a duplicate file

      if (! File.Checked && ! (Status.MsgUpload || Status.QwkUpload || Status.FileAttach)) {
        if (fsecokay(File.FullPath,File.Name,CHECKUPSEC,Status.CurConf.PubUpldLoc) == -1) {
          DszBuf.Check = 'E';  /* not allowed to upload it so cancel it */
          goto post;
        }

        File.Multiplier = Status.FsecMultiplier;

        /* ignore the file if it is found in the        */
        /* private upload location where we just put it */
        if (scandlpaths(CHECKUPLDS|REPORTONLY|IGNOREPRIVATE,File.Name) > 0) { /* we must have found one! */
          maxstrcpy(Status.DisplayText,File.Name,sizeof(Status.DisplayText));
          displaypcbtext(TXT_DUPLICATEFILE,NEWLINE|LOGIT);
          DszBuf.Check = 'E';  /* already exists so cancel it */
          goto post;
        }
      }

post:
      TotErrors = DszBuf.NumErrors;
      if (DszBuf.Check == 'E') {
        unlink(FullPath);
        cancelled(p,'U'); /* don't use FileList cuz it might not be in there */
      } else {
        File.Size = Size;
        File.CPS  = DszBuf.CPS;
        successful(&File,'U');
        putfilespec(Found,&File);
        Aborted = FALSE;
      }
    }
    dosfclose(&InFile);
    unlink(DszLog);
  } else {
    for (X = 0, TotalSize = 0; X < NumFiles; X++) {
      if (getfilespec(X,&File) == -1)
        break;
      if (fileexist(File.FullPath) != 255) {
        File.Size = DTA.ff_fsize;
        TotalSize += DTA.ff_fsize;
      } else
        File.Size = 0;
      putfilespec(X,&File);
    }

    AveCPS = cps(TotalSize,Seconds);

    for (X = 0; X < NumFiles; X++) {
      if (getfilespec(X,&File) == -1)
        break;
      if (File.Size != 0) {
        File.CPS = AveCPS;
        successful(&File,'U');
        putfilespec(X,&File);
        Aborted = FALSE;
      } else {
        unlink(File.Name);
        cancelled(File.Name,'U');
      }
    }
  }

  return(Aborted ? -1 : 0);
}


static int _NEAR_ LIBENTRY uploadpermit(char *Name) {
  int      X;
  char    *q;
  spectype File;

  if ((X = findfilename(Name)) != -1) {  /* did we find the name? */
    if (getfilespec(X,&File) == -1)
      return(-1);
    if (File.Success != FAILED) {        /* was it already sent? */
      cancelled(File.Name,'U');
      return(-1);
    }
    goto checkfsec;                      /* see if okay to send it */
  }

  if ((q = strchr(Name,'.')) != NULL) {  /* maybe the user left off the */
    *q = 0;                              /* extension - try taking it off */
    if ((X = findfilename(Name)) != -1) {/* and scanning for a match */
      *q = '.';                          /* if found, put the extension back on */
      if (getfilespec(X,&File) == -1)
        return(-1);
      strcat(File.FullPath,q);
      strcat(File.Name,q);
      goto checkpath;        /* make sure it's not a dupe */
    }
    *q = '.';   /* put the period back in there */
  }

  /* couldn't find the file so try adding it to the list */
  if ((X = addfiletolist(Name,Status.CurConf.PrvUpldLoc,0,FALSE,TRUE)) < 0) {
    cancelled(Name,'U'); /* couldn't add name to list */
    return(-1);
  }
  if (getfilespec(X,&File) == -1)
    return(-1);

checkpath:
   if (scandlpaths(CHECKUPLDS|REPORTONLY,Name) > 0) { /* we must have found one! */
     strcpy(Status.DisplayText,Name);
     displaypcbtext(TXT_DUPLICATEFILE,NEWLINE|LOGIT|BELL);
     cancelled(File.Name,'U');
     return(-1);
   }

checkfsec:
   Display.AbortPrintout = TRUE;  /* TURN OFF THE MESSAGE DISPLAY! */
   if (fsecokay(File.FullPath,File.Name,CHECKUPSEC,Status.CurConf.PubUpldLoc) == -1) {
     Display.AbortPrintout = FALSE;
     cancelled(File.Name,'U');
     return(-1);
   }
   File.Multiplier = Status.FsecMultiplier;
   Display.AbortPrintout = FALSE;
   return(X);
}


#ifdef COMM
static int _NEAR_ LIBENTRY receiveasciiblock(char *Block) {
  int  Size;
  int  Bytes;
  static bool Done;

  if (Done) {
    Done = FALSE;
    return(3);
  }

  Errors = 0;
  Size = BlockSize;

top:
  if (Errors > 10)
    return(2);

  settimer(4,TENSECONDS);
  while (Size > 0) {
    #ifdef __OS2__
      if (InBytes == 0) {
        WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
        WaitKeySem.reset();
      }
    #endif

    if (kbdinkey() == 27)
      return(2);
    if (InBytes == 0) {
      if (cdstillup() == 0) {
        Asy.LostCarrier = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
        return(2);
      }
      if (timerexpired(4)) {
        LastError = TIMEOUT;
        goto error;
      }
      giveup();
    } else {
      Bytes = cgetbuf(Block,Size);
      if (BytesSoFar == 0 && Size == BlockSize)
        XferStartTime = dosgetlongtime();
      Size -= Bytes;
      if (strnchr(Block,26,Bytes) != NULL) {
        BytesSoFar += Bytes;
        BlockSize -= Size;
        Done = TRUE;
        return(0);
      }
      Block += Bytes;
      settimer(4,TENSECONDS);
    }
  }

  BytesSoFar += BlockSize;
  updatescreen(UPLOAD);
  return(0);

error:
  Errors++;
  TotErrors++;
  updatescreen(UPLOAD);
  goto top;
}


static int _NEAR_ LIBENTRY receivexmodemblock(char *Block) {
  int  Size;
  int  Bytes;
  int  Retries;
  char *p;
  unsigned short Crc;
  unsigned char  CheckSum;

  Retries = 0;
  Errors = 0;

top:
  if (Errors > 10)
    return(2);

  settimer(4,THREESECONDS);
  #ifdef __OS2__
    releasekbdblock(THREESECONDS);
  #endif

  while (1) {
    #ifdef __OS2__
      if (InBytes == 0) {
        WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
        WaitKeySem.reset();
      }
    #endif

    if (Errors > 10)
      return(2);

    if (timerexpired(4)) {
      if (BlockNum > 1) {
        LastError = TIMEOUT;
        goto error;
      }
      if (++Retries >= 20)        /* 20 * THREESECONDS means that they have */
        return(2);                /* one minute to start sending the file   */
      sendbyte(StartLetter);
      goto top;
    }

    switch (commportinkey()) {
      case  -1:
      case CAN: return(2);
      case SOH: Block[0] = SOH; BlockSize = Size = 128;  goto start;
      case STX: Block[0] = STX; BlockSize = Size = 1024; goto start;
      case EOT: sendbyte(ACK); return(3);
    }

    if (kbdinkey() == 27)
      return(2);
  }

start:
  if (BytesSoFar == 0)
    XferStartTime = dosgetlongtime();

  Size += 2 + (Type == SUM ? 1 : 2);
  p = &Block[1];

  settimer(4,THREESECONDS);
  while (Size > 0) {
    #ifdef __OS2__
      if (InBytes == 0) {
        WaitKeySem.waitforevent(SEM_WAIT_NOTIMEOUT);
        WaitKeySem.reset();
      }
    #endif

    if (InBytes == 0) {
      if (cdstillup() == 0) {
        Asy.LostCarrier = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
        return(2);
      }
      if (timerexpired(4)) {
        if (p == &Block[1])
          LastError = TIMEOUT;
        else {
          LastError = SHORTBLOCK;
          BytesInShortBlock = (int) (p-&Block[1]);
        }
        goto error;
      }
      #ifndef __OS2__
        giveup();
      #endif
    } else {
      Bytes = cgetbuf(p,Size);
      Size -= Bytes;
      p    += Bytes;
      settimer(4,THREESECONDS);
    }
  }

  if (Block[2] != (char) ~Block[1]) {  /* bad block number */
    LastError = BADBLOCKNUM;
    goto error;
  }

  if (Block[1] != BlockNum) {
    if (Block[1] == BlockNum-1) {   /* perhaps his ACK got garbled and he */
      sendbyte(ACK);                /* resent the same packet?  if so,    */
      goto top;                     /* ACK it again and wait for more     */
    }
  }

  switch (Type) {
    case SUM:
              #ifdef __OS2__
                CheckSum = xmodemchecksum(&Block[3],BlockSize);
              #else
                xmodem(&Block[3],BlockSize);  //lint !e534  return value in DL
                CheckSum = _DL;
              #endif
              if (CheckSum != Block[131]) {
                LastError = SUMERROR;
                goto error;
              }
              break;
    case CRC: Crc =
              #ifdef __OS2__
                xmodemcrc(&Block[3],BlockSize+2);
              #else
                xmodem(&Block[3],BlockSize+2);     //lint !e534 return val in AX
              #endif
              if (Crc != 0) {
                LastError = CRCERROR;
                goto error;
              }
              break;
  }

  if (BlockNum == 0)    /* skip the ack if we're on block 0 - the calling */
    return(0);          /* routine will send it if the filename is good   */

  if (Flow != FULLFLOW)
    sendbyte(ACK);
  BytesSoFar += BlockSize;
  updatescreen(UPLOAD);
  return(0);

error:
  Errors++;
  TotErrors++;
  updatescreen(UPLOAD);
  if (Errors > 10 || Flow == FULLFLOW)
    return(2);
  while (cgetbuf(Block,128) != 0) {   /* wait for buffer to empty */
    if (cdstillup() == 0) {
      Asy.LostCarrier = TRUE;
      if (PcbData.Packet)
        turnoffdtr();
      return(2);
    }
    giveup();
  }
  sendbyte(StartLetter);
  goto top;
}


#pragma warn -rvl
static int _NEAR_ LIBENTRY receiveall(bool Batch, bool Ascii) {
  bool     Stop;
  int      Found;
  char     Name[13];
  DOSFILE  Out;
  spectype File;
  char     Block[1040];

  if (getfilespec(0,&File) == -1)
    return(-1);

  Found = 0;  // default to the FIRST file in the list (batch or not)

  if (Batch) {
    BlockNum = 0;   /* batches start with block 0 for the header block */
    memset(&Out,0,sizeof(Out));
    if (PcbData.NoBatchUp || Status.EnteringMessage) {
      if (dosfopen(File.FullPath,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&Out) == -1)
        return(-1);
      showfileinusernet('U',NULL);
      dossetbuf(&Out,(PcbData.UploadBufSize >= 2 && PcbData.UploadBufSize <= 16 ? PcbData.UploadBufSize * 1024 : 4096));
    }
  } else {
    BlockNum = 1;
    if (dosfopen(File.FullPath,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&Out) == -1)
      return(-1);
    showfileinusernet('U',NULL);
    dossetbuf(&Out,(PcbData.UploadBufSize >= 2 && PcbData.UploadBufSize <= 16 ? PcbData.UploadBufSize * 1024 : 4096));
  }

  paintulscreen(File.Name,0);
  setupxferscale(0);
  BytesSoFar    = 0;
  XferStartTime = dosgetlongtime();
  BlockSize     = 128;
  Stop          = FALSE;

  while (1) {
    switch (Ascii ? receiveasciiblock(&Block[3]) : receivexmodemblock(Block)) {
      case  0: if (BlockNum == 0) {            // block 0 - get the header info
                 if (Block[3] == 0 || Stop) {  // null header? forced stop?
                   sendbyte(ACK);              // ack the last header
                   return(0);                  // we're done - get outta here
                 }

                 /* in the case of NoBatchUploads we will allow batch */
                 /* protocols to be used but we will ignore the name  */
                 /* passed in block 0 keeping, instead, the original  */
                 /* name specified by the caller.                     */

                 if (Block[1] != 0) {          /* if it's not a HEADER block */
                   cancelled(File.Name,'U');   /* then abort it cuz we want  */
                   return(-1);                 /* to get the filename first! */
                 }

                 maxstrcpy(Name,&Block[3],sizeof(Name));

                 if (PcbData.NoBatchUp || Status.EnteringMessage)
                   Found = 0;  // only one file can be uploaded, point to #1
                 else {
                   if (strlen(&Block[3]) > 12) {  /* if filename is > 12 chars then we don't want it */
                     cancelled(File.Name,'U');
                     return(-1);
                   }
                   strupr(Name);
                   if (invalidfilename(Name,NOWILDSNOINFORM)) {
                     cancelled(File.Name,'U');
                     return(-1);
                   }

                   if ((Found = uploadpermit(Name)) == -1)
                     return(-1);

                   if (getfilespec(Found,&File) == -1)
                     return(-1);

                   if (insufficientfreespace()) {
                     logsystext(TXT_INSUFUPLOADSPACE,SPACERIGHT);
                     return(-1);
                   }

                   if (dosfopen(File.FullPath,OPEN_RDWR|OPEN_CREATE|OPEN_DENYRDWR,&Out) == -1) {
                     cancelled(File.Name,'U');
                     return(-1);
                   }
                 }

                 // some programs want this one
                 sendbyte(StartLetter);
                 // some programs want an ACK - we'll send both (6/22/95)
                 sendbyte(ACK);

                 BlockSize = 1024;
                 File.Size = atol(&Block[strlen(Name)+4]);
                 showfileinusernet('U',&File);
                 dossetbuf(&Out,(PcbData.UploadBufSize >= 2 && PcbData.UploadBufSize <= 16 ? PcbData.UploadBufSize * 1024 : 4096));
                 paintulscreen(File.Name,File.Size);
                 setupxferscale(numblocks(File.Size));
                 BlockNum++;
                 break;       /* break without writing the block out! */
               }
               BlockNum++;                      /* good block */
               if (saveblock(&Block[3],BlockSize,&Out) == -1) {
                 dosfclose(&Out);                 /* abort */
                 unlink(File.FullPath);
                 cancelled(File.Name,'U');
                 return(-1);
               }
               break;
      case  1: break;                           /* bad block */
      case  2: if (Out.handle > 0) {            /* abort */
                 dosfclose(&Out);
                 unlink(File.FullPath);
                 cancelled(File.Name,'U');
               }
               return(-1);
      case  3: dosfclose(&Out);              /* done with this file */
               if (BlockNum == 0)            /* on MULTIPLE EOF get out now! */
                 return(0);
               if (File.Size == 0)           /* was even a FULL BLOCK sent? */
                 File.Size = BytesSoFar;     /* no, so how many BYTES?      */
               if (File.Size == 0) {         /* was anything at all sent?   */
                 unlink(File.FullPath);      /* no, cancel the upload now   */
                 cancelled(File.Name,'U');
                 return(-1);
               }
               File.CPS = cps(BytesSoFar,doselapsedtime(XferStartTime));
               successful(&File,'U');

               if (Found >= 0)
                 putfilespec(Found,&File);       //lint !e534

               if (Batch) {
                 if (File.Size != BytesSoFar)
                   truncatefilesize(File.FullPath,File.Size);
                 if (PcbData.NoBatchUp || Status.EnteringMessage)
                   Stop = TRUE;  // don't let next file come through
               } else
                 return(0);

               BytesSoFar = 0;
               XferStartTime = dosgetlongtime();
               BlockNum = 0; /* reset block number and go looking for more */
               break;
    }
  }
}
#pragma warn +rvl
#endif


static int _NEAR_ LIBENTRY receivelocal(void) {
  int      Count;
  int      Found;
  int      FindMax;
  char    *p;
  ffblk    blk;
  char     Path[56];
  char     SrcePath[66];
  spectype File;
  #ifdef __OS2__
  int      DirHandle = MAKEDIRHANDLE;       //lint !e569
  #endif

  Path[0] = 0;
  getlocalinput(Path,TXT_LOCALUPLDPATH,sizeof(Path)-13,mask_wildname);
  if (Path[0] == 0)
    return(-1);

  if (((Path[1] == ':' && Path[2] == 0) || ((fileexist(Path) & 16) == 16)) && strpbrk(Path,"*?") == NULL) {
    // This section of code assumes that a _PATH_ (not a filename) has just
    // been entered.  This means that PCBoard should use the names of the files
    // ALREADY ENTERED and try to find them in the PATH which has just been
    // entered.
    //
    // If no filenames have been entered (NumFiles == 0) or if none of the
    // files are found to exist, then an error condition should be reported

    if (NumFiles == 0)
      return(-1);

    addbackslash(Path,sizeof(Path)-13);
    for (FileNum = 0; FileNum < NumFiles; FileNum++) {
      if (getfilespec(FileNum,&File) == -1)
        goto cancel;

      if (Status.QwkUpload)
        buildstr(SrcePath,Path,getqwkroot(),".REP",NULL);
      else
        sprintf(SrcePath,"%s%s",Path,File.Name);

      if (insufficientfreespace()) {
        logsystext(TXT_INSUFUPLOADSPACE,SPACERIGHT);
        goto cancel;
      }

      if (localtransfer(&File,SrcePath,File.FullPath,UPLOAD) == -1)
        goto cancel;
      if (putfilespec(FileNum,&File) == -1)
        goto cancel;
    }
  } else {
    // This section of code assumes that the a _FILENAME_ has just been
    // entered...  this can include a path and/or wildcards!
    //
    // PCBoard will try to upload the file (or files) specified.

    FindMax = 1;
    if (dosfindfirst(Path,&blk,FA_NORMAL|FA_ARCH|FA_RDONLY|FA_DIREC,&FindMax PDIRHANDLE) == 0) {

      /* strip down to just the actual path removing file specs */
      /* e.g. C:\SUB\FILENAME.EXT becomes C:\SUB\               */
      /*      A:FILENAME.EXT      becomes A:                    */

      if ((p = strrchr(Path,'\\')) != NULL || (p = strrchr(Path,':')) != NULL)
        *(p+1) = 0;
      else
        Path[0] = 0;

//    if (Status.EnteringMessage && ! Status.FileAttach) {
      if (Status.EnteringMessage) {
        if (getfilespec(0,&File) == -1)
          goto cancel;
        strupr(blk.ff_name);
        sprintf(SrcePath,"%s%s",Path,blk.ff_name);
        if (localtransfer(&File,SrcePath,File.FullPath,UPLOAD) == -1)
          goto cancel;
        if (putfilespec(0,&File) == -1)
          goto cancel;
      } else {
        Count = 0;
        do {
          strupr(blk.ff_name);
          if (blk.ff_attrib != 16) {
            if ((Found = uploadpermit(blk.ff_name)) != -1) {
              if (getfilespec(Found,&File) == -1)
                goto cancel;
              sprintf(SrcePath,"%s%s",Path,File.Name);
              if (localtransfer(&File,SrcePath,File.FullPath,UPLOAD) == -1)
                goto cancel;
              if (putfilespec(Found,&File) == -1)
                goto cancel;
              Count++;
            }
          }
          FindMax = 1;
        } while (dosfindnext(&blk,&FindMax PDIRHANDLE2) == 0);

        #ifdef __OS2__
          dosclosedirhandle(DirHandle);
        #endif

        if (Count == 0)
          return(-1);
      }
      #ifdef __OS2__
        dosclosedirhandle(DirHandle);
      #endif
    }
  }
  return(0);

cancel:
  cancelled(File.Name,'U');
  return(-1);
}


static int _NEAR_ LIBENTRY receivefile(void) {
  #ifdef COMM
  bool         Ascii;
  bool         Batch;
  #endif
  char         OldX;
  char         OldY;
  int          Okay;
  int          Saved;

  #ifdef COMM
    if (Asy.Online == REMOTE)
      waitforempty(TENSECONDS);   // wait for our output bytes to go out
  #endif

  OldX = awherex();
  OldY = awherey();

  if (! ProtocolToUse.Internal) {
    Saved = memsavescreen();
    Okay = externalupload();
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);

    if (Asy.HstMode)
      tickdelay(ONESECOND);

    if (Control.GraphicsMode) { /* force it to reset the color! */
      asetcolor(0);
      printdefcolor();
    }
    if (Okay == -1)
      goto cancel;
    return(0);
  }

  FileNum    = 0;
  Cancel     = FALSE;
  Errors     = 0;
  TotErrors  = 0;

  if (Asy.Online == LOCAL) {
    Saved = memsavescreen();
    Okay = receivelocal();
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);
    if (Okay != -1)
      return(0);
    goto cancel;
  }

  #ifdef COMM
    Ascii = FALSE;
    Batch = FALSE;

    switch (ProtocolLetterToUse) {
      case 'A': Ascii = TRUE; break;
      case 'X': StartLetter=STARTx; Type=SUM; Flow=HANDSHAKE; break;
      case 'C': StartLetter=STARTc; Type=CRC; Flow=HANDSHAKE; break;
      case 'O': StartLetter=STARTc; Type=CRC; Flow=HANDSHAKE; break;
      case 'Y': StartLetter=STARTc; Type=CRC; Flow=HANDSHAKE; Batch = TRUE; break;
      case 'F': StartLetter=STARTg; Type=CRC; Flow=FULLFLOW;  break;
      case 'G': StartLetter=STARTg; Type=CRC; Flow=FULLFLOW;  Batch = TRUE; break;
      default : return(-1);
    }

    Saved = memsavescreen();
    Okay = receiveall(Batch,Ascii);
    memrestorescreen(Saved,FREESCREEN);
    agotoxy(OldX,OldY);

    if (Asy.HstMode)
      tickdelay(ONESECOND);

    if (Control.GraphicsMode) { /* force it to reset the color! */
      asetcolor(0);
      printdefcolor();
    }
    if (Okay != -1)
      return(0);
  #endif

cancel:
  if (! Asy.LostCarrier) {
    #ifdef COMM
      waitcomm(THREESECONDS);
    #endif
    checkdisplaystatus();
    displaypcbtext(TXT_UPLOADABORTED,NEWLINE|LFBEFORE|BELL);
  }
  return(XferVars.SuccessfulUpldFiles == 0 ? -1 : 0);
}


#ifdef FIDO
int LIBENTRY xmodemsendfile(char *FileName) {
  int       RetVal;
  char     *p;
  long      Size;
  char      Name[13];
  char      Path[66];
  spectype  File;

  if (fileexist(FileName) == 255)
    return(-1);

  Size = DTA.ff_fsize;
  deallocatefilelist(THROWAWAY);
  if (allocatefilelist(DOWNLOAD) == -1)
    return(-1);

  maxstrcpy(Path,FileName,sizeof(Path));
  if ((p = strrchr(Path,'\\')) == NULL)
    if ((p = strrchr(Path,':')) == NULL)
      p = Path;

  maxstrcpy(Name,p,sizeof(Name));
  *p = 0;
  addfiletolist(Name,Path,Size,TRUE,FALSE);   //lint !e534
  Status.Batch = FALSE;
  setxferprotocol('C');
  getfilespec(0,&File);                       //lint !e534
  transferstatus(START);
  RetVal = sendfile(&File);
  waitcomm(TWOSECONDS);
  deallocatefilelist(THROWAWAY);
  transferstatus(STOP);
  return(RetVal);
}


int LIBENTRY xmodemreceivefile(char *FileName) {
  int   RetVal;
  char *p;
  char  Name[13];
  char  Path[66];

  if (allocatefilelist(UPLOAD) == -1)
    return(-1);

  maxstrcpy(Path,FileName,sizeof(Path));
  if ((p = strrchr(Path,'\\')) == NULL)
    if ((p = strrchr(Path,':')) == NULL)
      p = Path;

  maxstrcpy(Name,p,sizeof(Name));
  *p = 0;
  addfiletolist(Name,Path,0,TRUE,FALSE);    //lint !e534
  Status.Batch = FALSE;
  setxferprotocol('C');
  transferstatus(START);
  RetVal = receivefile();
  waitcomm(TWOSECONDS);
  deallocatefilelist(THROWAWAY);
  transferstatus(STOP);
  return(RetVal);
}
#endif


void LIBENTRY shownamesizeandtime(int Num, int ProtocolBlockSize) {
  char     Temp[10];
  char     Str[50];
  spectype File;

  if (getfilespec(Num,&File) == -1)
    return;

  printcolor(PCB_GREEN);
  sprintf(Temp,"(%d)",Num+1);
  sprintf(Str,"%-6s%-12s%8ld bytes, ",Temp,File.Name,File.Size);
  print(Str);
  BlockSize = ProtocolBlockSize;
  downloadtime(File.Size,Str,NULL);
  print(Str);
  displaypcbtext(TXT_MINUTES,DEFAULTS);
  #ifndef PCB_DEMO
    if (File.FreeFile)
      displaypcbtext(TXT_FREEDOWNLOAD,DEFAULTS);
    if (File.FreeTime)
      displaypcbtext(TXT_NOTIMECHARGE,DEFAULTS);
    #ifdef PCB152
      if (File.NoCost)
        displaypcbtext(TXT_NOCOST,DEFAULTS);
    #endif /* ifdef PCB152 */
  #endif /* ifdef PCB_DEMO */
  newline();
}


static void _NEAR_ LIBENTRY showdltimeandsize(spectype *File) {
  char Str[100];

  BlockSize = ProtocolToUse.BlockSize;
  displaypcbtext(TXT_DOWNLOADTIME,DEFAULTS);
  printcolor(PCB_GREEN);
  downloadtime(File->Size,Str,NULL);
  print(Str);
  displaypcbtext(TXT_MINUTES,DEFAULTS);
  #ifndef PCB_DEMO
    if (File->FreeFile)
      displaypcbtext(TXT_FREEDOWNLOAD,DEFAULTS);
    if (File->FreeTime)
      displaypcbtext(TXT_NOTIMECHARGE,DEFAULTS);
    #ifdef PCB152
      if (File->NoCost)
        displaypcbtext(TXT_NOCOST,DEFAULTS);
    #endif /* ifdef PCB152 */
  #endif
  newline();

  displaypcbtext(TXT_DOWNLOADSIZE,DEFAULTS);
  printcolor(PCB_GREEN);
  sprintf(Str," %ld bytes (%u blocks)",File->Size,numblocks(File->Size));
  println(Str);

  displaypcbtext(TXT_TOTALWILLBE,DEFAULTS);
  printcolor(PCB_RED);
  sprintf(Str," %ld bytes",UsersData.DailyDnldBytes+numbyteslessignore());
  println(Str);
}


static void _NEAR_ LIBENTRY showdldetails(spectype *File) {
  showdltimeandsize(File);

  displaypcbtext(TXT_PROTOCOLTYPE,DEFAULTS);
  printcolor(PCB_CYAN);
  println(ProtocolToUse.Desc);

  displaypcbtext(TXT_FILESELECTED,DEFAULTS);
  printcolor(PCB_WHITE);
  println(File->Name);
  displaypcbtext((ProtocolLetterToUse == 'A' ? TXT_ENTERSTARTS : TXT_ABORTSTRANSFER),DEFAULTS);
  printdefcolor();
  newline();
}


static void _NEAR_ LIBENTRY showbatchdlfilenames(void) {
  int      X;
  spectype File;

  for (X = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (File.FullPath[0] != 0) {
      displaypcbtext(TXT_FILESELECTED,DEFAULTS);
      shownamesizeandtime(X,ProtocolToUse.BlockSize);
      if (Display.AbortPrintout)
        break;
    }
  }
  checkdisplaystatus();
}


static void _NEAR_ LIBENTRY showmuldldetails(void) {
  int      X;
  long     TotalSize;
  char     Str[100];
  spectype File;

  for (X = 0, TotalSize = 0; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (File.FullPath[0] != 0)
      TotalSize += File.Size;
  }

  BlockSize = ProtocolToUse.BlockSize;
  displaypcbtext(TXT_BATCHDLTIME,LFBEFORE);
  printcolor(PCB_GREEN);
  downloadtime(TotalSize,Str,NULL);
  print(Str);
  displaypcbtext(TXT_MINUTES,NEWLINE);

  displaypcbtext(TXT_BATCHDLSIZE,DEFAULTS);
  printcolor(PCB_GREEN);
  sprintf(Str," %ld bytes (%u blocks)",TotalSize,numblocks(TotalSize));
  println(Str);

  displaypcbtext(TXT_BATCHPROTOCOL,DEFAULTS);
  printcolor(PCB_CYAN);
  println(ProtocolToUse.Desc);
}


static int _NEAR_ LIBENTRY getxferprotocol(bool AskUser) {
  char Str[2];
  spectype File;

  Status.Batch = (bool) (Status.Batch || (NumFiles > 1));

  if (ProtocolToUse.ErrCorrReq && ! Asy.ErrorCorrected) {
reset:
    setxferprotocol('N');
    displaypcbtext(TXT_WRONGMODEMTYPE,NEWLINE);
  }

  if (ProtocolLetterToUse == 'N' ||
      ProtocolToUse.Desc[0] == 0 ||
      (Status.Batch && ! ProtocolToUse.Batch)) {
    setxferprotocol('N');
    AskUser = TRUE;
  }

retry:
  if (AskUser) {
    newline();
    protfile(ProtocolLetterToUse,Status.Batch);
    Str[0] = ProtocolLetterToUse;
    Str[1] = 0;
    inputfield(Str,TXT_PROTOCOLFORXFER,1,FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,ValidProtocols);
    if (Str[0] == 'N' || Str[0] == 0)
      return(0);
    setxferprotocol(Str[0]);
    if (Status.Batch) {
      if (! ProtocolToUse.Batch)
        goto retry;
    } else {
      if (NumFiles > 0 && ProtocolToUse.Internal && Str[0] == 'A') {
        if (getfilespec(0,&File) != -1) {
          if (asciionbinary(File.Name))
            goto retry;
        }
      }
    }
    if (ProtocolToUse.ErrCorrReq && ! Asy.ErrorCorrected)
      goto reset;
  }

  return(1);
}


static bool _NEAR_ LIBENTRY istimeokay(spectype *File, long *Seconds, bool ChargeTime) {
  long SecondsLeft;
  long SecondsToEvent;
  long SuspendTime;
  long Time;

  if (Status.EventMinute != 0) {
    Time = exacttime();
    SuspendTime = Status.SuspendMinute * 60L;
    if (SuspendTime < Time)
      SuspendTime += (24L * 60L * 60L);
    SecondsToEvent = SuspendTime - Time;
  } else
    SecondsToEvent = 0x7FFFFFFFL;

  /* if we are either CHARGING time (i.e. FREETIME was not specified) or if */
  /* the user's time has been adjusted for an event then make sure that     */
  /* the number of seconds required is not longer than the seconds left     */

  /* Also, if an event is coming then SUBTRACT from the seconds that are    */
  /* left those seconds that have already been counted as FREETIME so that  */
  /* multiple NOTIME downloads do not cause the event to be missed          */

  downloadtime(File->Size,NULL,Seconds);
  if (ChargeTime || Status.TimeAdjustedForEvent || *Seconds+TotalSecondsSoFar > SecondsToEvent) {
    SecondsLeft = ((long) xferminutesleft()-1) * 60L;
    if (Status.TimeAdjustedForEvent)
      SecondsLeft -= IgnoreSeconds;
    #ifdef DEBUG
      if (SecondsToEvent < 0) {
        // it is an error for seconds to event to be less than 0, but I can't
        // see how it can happen, I'm leaving this debug code in here in
        // case someday it helps me figure it out.  (DWT-11/08/94)
        char Str[80];
        sprintf(Str,"ERROR: Needed=%ld, Left=%ld",*Seconds,SecondsLeft);
        writelog(Str,SPACERIGHT);
        sprintf(Str,"ERROR: SoFar=%ld, ToEvent=%ld",TotalSecondsSoFar,SecondsToEvent);
        writelog(Str,SPACERIGHT);
      }
    #endif
    if (*Seconds > SecondsLeft || *Seconds+TotalSecondsSoFar > SecondsToEvent) {
      maxstrcpy(Status.DisplayText,File->Name,sizeof(Status.DisplayText));
      displaypcbtext(TXT_NOTIMEFORXFER,NEWLINE|LOGIT|BELL);
      return(FALSE);
    }
  }
  return(TRUE);
}


static bool _NEAR_ LIBENTRY badfileratio(char *Name) {
  return(checkratio(Name,
                    UsersData.NumUploads + Status.FileRatioCredits,
                    UsersData.NumDownloads + numfileslessignore(),
                    1,
                    Status.FileRatio,
                    TXT_FILERATIO,
                    TXT_FILERATIOEXCEEDED));
}


static bool _NEAR_ LIBENTRY badbyteratio(char *Name, long FileSize) {
  return(checkratio2(Name,
                     UsersData.TotUpldBytes + (Status.KByteRatioCredits * 1024L),
                     UsersData.TotDnldBytes + numbyteslessignore(),
                     FileSize,
                     Status.ByteRatio,
                     TXT_BYTERATIO,
                     TXT_BYTERATIOEXCEEDED));
}

static bool _NEAR_ LIBENTRY checklimit(char *Name, long Dn, long New, long CheckLimit, int CurrentTxt, int ExceededText) {
  int  Len;
  char Str[40];

  if (CheckLimit == 0)
    return(FALSE);

  if (Dn+New > CheckLimit) {
    displaypcbtext(CurrentTxt,NEWLINE|LFBEFORE);

    Len = strlen(comma(Str,Dn));
    sprintf(Status.DisplayText,"%*s",Len,comma(Str,CheckLimit));

    displaypcbtext(TXT_DOWNLOADLIMIT,NEWLINE);
    strcpy(Status.DisplayText,Name);
    displaypcbtext(ExceededText,NEWLINE|LFAFTER|LOGIT);
    return(TRUE);
  }
  return(FALSE);
}


static bool _NEAR_ LIBENTRY checklimit2(char *Name, double Dn, double New, long CheckLimit, int CurrentTxt, int ExceededText) {
  int  Len;
  char Str[40];

  if (CheckLimit == 0)
    return(FALSE);

  if (Dn+New > CheckLimit) {
    displaypcbtext(CurrentTxt,NEWLINE|LFBEFORE);

    Len = strlen(comma(Str,Dn));
    sprintf(Status.DisplayText,"%*s",Len,comma(Str,CheckLimit));

    displaypcbtext(TXT_DOWNLOADLIMIT,NEWLINE);
    strcpy(Status.DisplayText,Name);
    displaypcbtext(ExceededText,NEWLINE|LFAFTER|LOGIT);
    return(TRUE);
  }
  return(FALSE);
}


static bool _NEAR_ LIBENTRY badfilelimit(char *Name) {
  return(checklimit(Name,
                    UsersData.NumDownloads,
                    1 + numfileslessignore(),
                    Status.TotalFileLimit,
                    TXT_FILESDOWNLOADED,
                    TXT_FILELIMITEXCEEDED));
}


static bool _NEAR_ LIBENTRY badbytelimit(char *Name, long FileSize) {
  return(checklimit2(Name,
                     UsersData.TotDnldBytes + numbyteslessignore(),
                     FileSize,
                     Status.TotalKByteLimit*1024,
                     TXT_BYTESDOWNLOADED,
                     TXT_BYTELIMITEXCEEDED));
}


void LIBENTRY checkdlfiles(int StartNum) {
  int         Fsec;
  int         X;
  int         Y;
  long        Seconds;
  spectype    File;

  if (StartNum == 0)
    resetxfercounters(TRUE);

  for (X = StartNum; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    maxstrcpy(Status.DisplayText,File.Name,sizeof(Status.DisplayText));
    if (File.FullPath[0] == 0) {
      displaypcbtext(TXT_NOTFOUNDONDISK,NEWLINE|LOGIT|BELL);
      File.FsecOkay = FALSE;
    } else {
      // check to see if the file has already been okayed, and if so,
      // check to see if it is a free file or a free time file
      if (File.FsecOkay) {
        if (File.FreeFile)
          Fsec = (File.FreeTime ? 3 : 1);
        else
          Fsec = (File.FreeTime ? 2 : 0);
      } else {
        // if it hasn't been okayed yet, then check the FSEC file now
        Fsec = fsecokay(File.FullPath,File.Name,CHECKFSEC,NULL);
      }

      if (Fsec != -1) {
        #ifdef PCB152
        {
          double ChargeSoFar;
          double Charge;

          if (Status.ActStatus != ACT_DISABLED) {
            File.Multiplier = Status.FsecMultiplier;
            if (File.Multiplier == 0) {
              // if the multiplier is 0, but we otherwise are set to charge for
              // file or bytes, then record the fact that the file is "NOCOST"
              if (AccountRates.ChargeForDownloadFile != 0 || AccountRates.ChargeForDownloadBytes != 0)
                File.NoCost = TRUE;
            } else {
              // we're taking into account the amount of time required for the
              // transfer when calculating the expected cost, however, we're
              // ignoring the effect of a NOTIME status on that calculation.
              // we have also made no attempt to determine if the time being
              // spent online is during peak hours or not.
              // these things may need to be adjusted
              Charge = AccountRates.ChargeForDownloadFile * File.Multiplier
                     + ((double) AccountRates.ChargeForDownloadBytes / 1024.0) * File.Size * File.Multiplier
                     + (AccountRates.ChargeForTime * numsecsforxfer(File.Size) / 60);
              ChargeSoFar = TotalFilesSoFar * AccountRates.ChargeForDownloadFile * File.Multiplier
                          + ((double) TotalBytesSoFar / 1024.0) * AccountRates.ChargeForDownloadBytes * File.Multiplier
                          + (AccountRates.ChargeForTime * numsecsforxfer(TotalBytesSoFar) / 60);
              if (insufficientcredits(Charge,ChargeSoFar)) {
                File.FsecOkay = FALSE;
                goto put;
              }
            }
          }
        }
        #endif


        /* to check for the 'bytes remaining' we first check to see if we */
        /* should ignore the 'bytesremaining' value which is -1 then we   */
        /* check to see if the file is a free download which is when the  */
        /* Fsec value is equal to 1 or 2 otherwise we don't ignore it and */
        /* finally we should ignore any previously 'free' bytes that have */
        /* been tallied up from previous checks in this routine           */
        if (Status.BytesRemaining != -1 && (Fsec == 0 || Fsec == 2) && (File.Size > bytesleft())) {
          displaypcbtext(TXT_BYTESLEFTARE,NEWLINE|LOGIT|BELL);
          UsersData.Stats.NumReachDnldLim++;
          File.FsecOkay = FALSE;
        } else {
          if ((Fsec == 0 || Fsec == 2) &&
              (badfileratio(File.Name) || badbyteratio(File.Name,File.Size) || badfilelimit(File.Name) || badbytelimit(File.Name,File.Size))
             ) {
            UsersData.Stats.NumReachDnldLim++;
            File.FsecOkay = FALSE;
          } else {
            if (istimeokay(&File,&Seconds,(bool) (Fsec < 2))) {
              if ((Y = findfilename(File.Name)) != -1 && Y < X) {
                maxstrcpy(Status.DisplayText,File.Name,sizeof(Status.DisplayText));
                displaypcbtext(TXT_DUPLICATEBATCHFILE,NEWLINE);
                File.FsecOkay = FALSE;
              } else {
                TotalFilesSoFar++;
                TotalBytesSoFar += File.Size;
                File.Seconds = Seconds;
                TotalSecondsSoFar += Seconds;
                File.FsecOkay = TRUE;
                #ifndef PCB_DEMO
                  switch(Fsec) {
                    case 3: File.FreeFile  = TRUE;
                            File.FreeTime  = TRUE;
                            IgnoreSeconds += Seconds;
                            IgnoreBytes   += File.Size;
                            IgnoreFiles++;
                            break;
                    case 2: File.FreeTime  = TRUE;
                            IgnoreSeconds += Seconds;
                            break;
                    case 1: File.FreeFile  = TRUE;
                            IgnoreBytes   += File.Size;
                            IgnoreFiles++;
                            break;
                  }
                #endif
              }
            } else {
              File.FsecOkay = FALSE;
            }
          }
        }
      }
    }

put:
    if (putfilespec(X,&File) == -1)
      break;
  }

  removefilesfromlist();
}


static char _NEAR_ LIBENTRY overwriteokay(char *Name) {
  static char mask[1] = { 'C' };
  char        Input[2];

  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  Input[0] = NoChar;
  Input[1] = 0;
  inputfield(Input,TXT_OVERWRITE,1,YESNO|NEWLINE|FIELDLEN|UPCASE|FIELDLEN,NOHELP,mask);
  return(Input[0]);
}


static void _NEAR_ LIBENTRY checkulfiles(int StartNum) {
  int         X;
  char        C;
  spectype    File;

  for (X = StartNum; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (File.Found) {
      if (Status.CurSecLevel < PcbData.SysopSec[SEC_OVERWRITE]) {
        maxstrcpy(Status.DisplayText,File.Name,sizeof(Status.DisplayText));
        displaypcbtext(TXT_DUPLICATEFILE,NEWLINE|LOGIT);
        continue;
      } else {
        if (fsecokay(File.FullPath,File.Name,CHECKUPSEC,Status.CurConf.PubUpldLoc) == -1)
          continue;

        File.Multiplier = Status.FsecMultiplier;
        C = overwriteokay(File.Name);
        if (C == YesChar)
          unlink(File.FullPath);
        else if (C != 'C')
          continue;
      }
    }

    if (! File.Duplicate) {
      buildstr(File.FullPath,Status.CurConf.PrvUpldLoc,File.Name,NULL);
      if (fsecokay(File.FullPath,File.Name,CHECKUPSEC,Status.CurConf.PubUpldLoc) != -1) {
        if (getdescription(X,&File) == -1) {
          NumFiles--;
          goto end;
        }
        File.Multiplier = Status.FsecMultiplier;
        File.FsecOkay = TRUE;
        File.Checked = TRUE;
        if (putfilespec(X,&File) == -1)
          break;
      }
    }
  }

end:
  removefilesfromlist();
}


void LIBENTRY showbatchnames(int OldNumFiles, int ProtBlockSize) {
  int X;

  for (X = OldNumFiles; X < NumFiles; X++) {
    shownamesizeandtime(X,ProtBlockSize);
    if (Display.AbortPrintout)
      break;
  }
  checkdisplaystatus();
}


static void _NEAR_ LIBENTRY scanfornames(int NumTokens, xfertype XferType, bool FirstTime, bool LastTime, bool CheckForProtocol) {
  int  static OldNumFiles;
  bool        MsgSent;
  bool        MatchFound;
  int         X;
  char        *p;
  char        *q;
  spectype    File;

  if (FirstTime) {
    OldNumFiles = NumFiles;
    MsgSent = FALSE;
  } else
    MsgSent = TRUE;

  for (; NumTokens; NumTokens--) {
    p = getnexttoken();
    switch (strlen(p)) {
      case 1 : // don't check for a protocol unless it was stacked on the
               // command line ... CheckForProtocol will be TRUE only when
               // the information is passed in from the command line
               if (CheckForProtocol) {
                 if (*p >= 'A' && *p <= 'Z' && Protocols[*p-'A'].Desc[0] != 0) {
                   setxferprotocol(*p);
                   break;
                 } else if (*p >= '0' && *p <= '9' && Protocols[*p-'0'+26].Desc[0] != 0) {
                   setxferprotocol(*p);
                   break;
                 } /* else fall thru */
               }
               /* else fall thru */
      case 2 : if (*p == 'G' && *(p+1) == 'B' && *(p+2) == 0) {
                 AutoGoodBye = TRUE;
                 break;
               } /* else fall thru */
      case 3 : if (*p == 'B' && *(p+1) == 'Y' && *(p+2) == 'E' && *(p+3) == 0) {
                 AutoGoodBye = TRUE;
                 break;
               } /* else fall thru */
      default: if (! MsgSent) {
                 displaypcbtext(TXT_CHECKINGFILEXFER,NEWLINE);
                 MsgSent = TRUE;
               }
               if (XferType == UPLOAD) {
                 if ((q = strchr(p,'.')) != NULL) {
                   if (*(q+1) == 0)   /* on uploads only */
                     *q = 0;          /* change "FILENAME." into "FILENAME" */
                 }
                 if (! invalidfilename(p,INFORM)) {
                   if ((X = scandlpaths(CHECKUPLDS|ADDFILE,p)) == -1)
                     goto done;
                   if (X >= 1) {
                     for (MatchFound = FALSE, X = NumFiles - X; X < NumFiles; X++) {
                       if (getfilespec(X,&File) == -1)
                         break;
                       if (stricmp(File.Name,p) == 0)
                         MatchFound = TRUE;
                       else {
                         File.Duplicate = TRUE;
                         putfilespec(X,&File);
                       }
                     }
                     checkulfiles(OldNumFiles);
                     if (NumFiles == OldNumFiles && ! MatchFound && Status.CurSecLevel >= PcbData.SysopSec[SEC_OVERWRITE]) {
                       if (addfiletolist(p,Status.CurConf.PrvUpldLoc,0,TRUE,TRUE) == -3)
                         goto done;
                     } else OldNumFiles = NumFiles;
                   } else if (X <= 0 && addfiletolist(p,Status.CurConf.PrvUpldLoc,0,TRUE,TRUE) == -3)
                     goto done;
                 }
               } else {
                 if (! invalidfilename(p,ALLOWWILDCARDS|INFORM| (ProtocolLetterToUse == 'A' ? ALLOWTEXTONLY : NOWILDSNOINFORM)))
                   if (scandlpaths(CHECKDNLDS|ADDFILE|SHOWDUPES,p) == -1)
                     goto done;
               }
               break;
    }
  }

done:
  switch (XferType) {
    case DOWNLOAD: checkdlfiles(OldNumFiles);
                   if (NumFiles > 1)
                     Status.Batch = TRUE;

                   if (LastTime) {
                     if (OldNumFiles != NumFiles && Status.Batch)
                       showbatchnames(OldNumFiles,ProtocolToUse.BlockSize);
                   }
                   break;
    case UPLOAD  : checkulfiles(OldNumFiles);
                   break;
  }
}


static void _NEAR_ LIBENTRY getnames(xfertype XferType) {
  bool FirstTime;
  int  TextNum;
  int  HelpNum;
  int  Limit;
  int  NumTokens;
  char *OldPointer;
  char Str[80];

  NumTokens = 0;
  FirstTime = TRUE;

  switch (XferType) {
    case UPLOAD  : TextNum = (Status.Batch ? TXT_FILENAMETOUPLDBTCH : TXT_FILENAMETOUPLOAD);
                   HelpNum = HLP_U;
                   break;
    case DOWNLOAD: TextNum = (Status.Batch ? TXT_FILENAMETODNLDBTCH : TXT_FILENAMETODOWNLOAD);
                   HelpNum = HLP_D;
                   break;
    default      : return;
  }

  Limit = (Status.Batch ? CurBatchLimit : 1);
  while (1) {
    if (NumTokens > 0) {
      maxstrcpy(Str,getnexttoken(),sizeof(Str));
      NumTokens--;
    } else {

      if (Status.KbdTimedOut)
        loguseroff(ALOGOFF);

/*
      newline();
      printcolor(PCB_YELLOW);
      if (Status.Batch) {
        sprintf(Str,"(%d) ",NumFiles+1);
        print(Str);
      }
*/
      FirstTime = TRUE;
      Str[0] = 0;
      inputfield(Str,TextNum,sizeof(Str)-1,NEWLINE|LFBEFORE|UPCASE,HelpNum,mask_wildname);
      removetokens(Str);
      if (Str[0] == 0)
        break;
    }

    savetokenpointer(&OldPointer);
    scanfornames(tokenize(Str),XferType,FirstTime,(bool) (NumTokens == 0),FALSE);
    FirstTime = FALSE;
    restoretokenpointer(&OldPointer);
    Limit = (Status.Batch ? CurBatchLimit : 1);
    if (NumFiles >= Limit)
      break;
  }
}


void LIBENTRY autogoodbye(int Seconds) {
  int Key;

  AutoGoodBye = FALSE;

  Display.NumLinesPrinted = 0;  /* avoid a more prompt here! */
  displaypcbtext(TXT_BYEINTENSECONDS,LFBEFORE);

  print(".");
  while (Seconds > 0) {
    settimer(9,ONESECOND);
    #ifdef __OS2__
      Key = waitforkey(ONESECOND);
    #else
      while (1) {
      Key = cinkey();
    #endif
      if (timerexpired(9)) {
        Seconds--;
        print(".");
        #ifdef __OS2__
          updatelinesnow();
        #else
          break;
        #endif
      }
      switch (Key) {
        case   0: giveup();
                  break;
        case  11:
        case  24: newline();
                  newline();
                  redisplaystatusline();
                  return;
        case 'h':
        case 'H': newline();
                  Status.AutoLogoff = TRUE;
                  loguseroff(NLOGOFF);
                  return;
      }
    #ifndef __OS2__
      }
    #endif
  }
  newline();
  Status.AutoLogoff = TRUE;
  loguseroff(NLOGOFF);
}


static int _NEAR_ LIBENTRY editlist(void) {
  static char mask_viewlist[] = {5, 'A', 'E', 'G', 'L', 'P'};
  int         X;
  int         NumTokens;
  char        Str[20];
  spectype    File;

top:
  showmuldldetails();
  displaypcbtext(TXT_READYTOSENDBATCH,NEWLINE);
  Str[0] = 0;
  inputfield(Str,TXT_GOODBYEAFTERDOWN,1,NEWLINE|LFBEFORE|FIELDLEN|UPCASE,NOHELP,mask_viewlist);
  switch (Str[0]) {
    case 'A': return(-1);
    case 'G': AutoGoodBye = TRUE; return(0);
    case 'L': newline();
              showbatchdlfilenames();
              goto top;
    case 'P': if (NumFiles == 1)
                Status.Batch = FALSE;
              if (getxferprotocol(TRUE) == 0)
                return(-1);
              goto top;
    case 'E': while (1) {
                Str[0] = 0;
                inputfield(Str,TXT_EDITBATCH,1,NEWLINE|LFBEFORE|FIELDLEN|UPCASE,NOHELP,mask_editlist);
                switch (Str[0]) {
                  case 'A': getnames(DOWNLOAD);
                            movefilesinlist();
                            if (NumFiles > 1 && ! isprotbatch(ProtocolLetterToUse)) {
                              if (getxferprotocol(TRUE) == 0)
                                return(-1);
                            }
                            break;
                  case 'R': sprintf(Status.DisplayText,"1-%d",NumFiles);
                            Str[0] = 0;
                            inputfield(Str,TXT_REMOVEFILENUMBER,sizeof(Str)-1,NEWLINE|LFBEFORE|STACKED,NOHELP,mask_numbers);
                            NumTokens = tokenize(Str);
                            for (; NumTokens; NumTokens--) {
                              X = atoi(getnexttoken()) - 1;
                              if (X >= 0 && X < NumFiles) {
                                if (getfilespec(X,&File) != -1) {
                                  maxstrcpy(Status.DisplayText,File.Name,sizeof(Status.DisplayText));
                                  displaypcbtext(TXT_REMOVEDFILE,NEWLINE);
                                  TotalFilesSoFar--;
                                  TotalBytesSoFar -= File.Size;
                                  TotalSecondsSoFar -= File.Seconds;
                                  if (File.FreeFile) {
                                    IgnoreBytes -= File.Size;
                                    IgnoreFiles--;
                                  }
                                  if (File.FreeTime)
                                    IgnoreSeconds -= File.Seconds;
                                  File.FsecOkay = FALSE;
                                  putfilespec(X,&File);
                                }
                              }
                            }
                            removefilesfromlist();
                            break;
                  case 'L': newline();
                            showbatchdlfilenames();
                            break;
                  default : if (NumFiles == 0)
                              return(-1);
                            goto top;
                }
              }
  }
  return(0);  /* keep the compiler happy */
}


#ifndef PCB_DEMO
static void _NEAR_ LIBENTRY uploadcreditminutes(char *Str) {
  int       Minutes;
  unsigned  AveCPS;
  long      Seconds;

  if ((AveCPS = averagecps(XferVars.SuccessfulUpldBytes,UPOKAY)) != 0) {
    Seconds = ((XferVars.SuccessfulUpldBytes / AveCPS) * PcbData.UploadCredit) / 10;
    Minutes = (int) (Seconds / 60);
    Seconds -= Minutes * 60L;
  } else
    Seconds = Minutes = 0;
  sprintf(Str," %d.%d ",Minutes,(int) ((Seconds * 10) / 60));
}


// static long _NEAR_ LIBENTRY uploadcreditbytes(void) {
//  return(XferVars.SuccessfulUpldBytes * PcbData.ByteCredit / 10);
// }

static void _NEAR_ LIBENTRY givecredits(void) {
  char        Str[80];
  pcbtexttype Buf;

  if (Status.EnteringMessage)
    return;

  if (XferVars.SuccessfulUpldFiles != 0) {
    if (PcbData.UploadCredit > 10 || PcbData.ByteCredit > 0) {
      newline();
      getpcbtextshowcolor(TXT_UPLOADCREDITS,&Buf);
      uploadcreditminutes(Str);
      substitute(Buf.Str," 0.0 ",Str,sizeof(Buf.Str));
      sprintf(Str," %ld ",XferVars.SuccessfulUpldBytes * PcbData.ByteCredit / 10);
      substitute(Buf.Str," 0 ",Str,sizeof(Buf.Str));
      println(Buf.Str);

      Display.NumLinesPrinted = 1;  // cause menu to be redisplayed when done

      getsystext(TXT_UPLOADCREDITS,&Buf);
      uploadcreditminutes(Str);
      substitute(Buf.Str," 0.0 ",Str,sizeof(Buf.Str));
      sprintf(Str," %ld ",XferVars.SuccessfulUpldBytes * PcbData.ByteCredit / 10);
      substitute(Buf.Str," 0 ",Str,sizeof(Buf.Str));
      writelog(Buf.Str,SPACERIGHT);
    }
  }
}
#endif


#ifdef COMM
static void _NEAR_ LIBENTRY hangupnow(void) {
   newline();
   displaypcbtext(TXT_HANGINGUPNOW,LFBEFORE|NEWLINE|LOGIT);
   waitforempty(TENSECONDS);
   tickdelay(ONESECOND);
   Asy.IgnoreCDLoss = TRUE;
   turnoffdtr();
   initcommwindow(HIDE);
   tickdelay(TWOSECONDS);
   turnondtr();
   modemoffhook();
}
#endif

static void _NEAR_ LIBENTRY scanforuploads(int StartingPoint) {
  int         X;
  spectype    File;

  if (XferVars.SuccessfulUpldFiles == 0)
    return;

  makepcboardsys();  /* update pcboard.sys in case PCBTEST.BAT needs it */

  /* before getting descriptions tell it to */
  /* watch the clock again to avoid lockups */
  turnkbdtimeron();

  // get descriptions on UN-announced files FIRST

  for (X = StartingPoint; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (File.Success == UPOKAY) {
      if (! File.HasDesc && checkfilefordescription(X,&File) == -1)
        getdescription(X,&File);  //lint !e534
    }
  }

  #ifdef COMM
  {
    int  Key;
    int  Seconds;
    pcbtexttype Buf;

    if (PcbData.TestUploads && Asy.Online == REMOTE && ! Status.EnteringMessage) {
      getpcbtext(TXT_BEGINUPTEST,&Buf);
      if (Buf.Str[0] != 0) {
        Display.NumLinesPrinted = 0;
        displaypcbtext(TXT_BEGINUPTEST,NEWLINE|LFBEFORE);
        displaypcbtext(TXT_HANGUPNOWORWAIT,LFBEFORE);
        for (Seconds = 10; Seconds; Seconds--) {
          print(".");
          settimer(4,ONESECOND);
          while (! timerexpired(4)) {
            Key = cinkey();
            switch (Key) {
              case   0: giveup();
                        break;
              case 'h':
              case 'H': hangupnow();
                        goto done;
              default : goto done;
            }
          }
        }
        // If we got here, then the caller didn't touch the keyboard, so we'll
        // use the AutoGoodBye default.  If AutoGoodBye was requested, then
        // we'll hang up now.
        if (AutoGoodBye)
          hangupnow();
      }
    }
    done:;
    freshline();
    newline();
  }
  #endif

  for (X = StartingPoint; X < NumFiles; X++) {
    if (getfilespec(X,&File) == -1)
      break;
    if (File.Success == UPOKAY)
      postupload(X);
  }

  #ifdef PCBSTATS
    if (XferVars.SuccessfulUpldFiles != 0)
      updatestats(0,XferVars.SuccessfulUpldFiles,0,UPDATE);
  #endif
}


static void _NEAR_ LIBENTRY receiveduploads(void) {
  int   Time;
  int   Time2;

  newline();
  printcolor(PCB_WHITE);

  XferVars.CPSup = averagecps(XferVars.SuccessfulUpldBytes,UPOKAY);
  XferVars.CPSdn = averagecps(XferVars.SuccessfulDnldBytes,DNOKAY);

  Time  = (int) (XferVars.SuccessfulDnldBytes / XferVars.CPSdn);
  Time2 = (int) (XferVars.SuccessfulUpldBytes / XferVars.CPSup);

  if (Time2 > Time)
    Time = Time2;

  XferVars.CPSboth = (int) ((XferVars.SuccessfulDnldBytes + XferVars.SuccessfulUpldBytes) / Time);

  displaypcbtext(TXT_BIRECEIVE,NEWLINE);
  logsystext(TXT_BIRECEIVELOG,SPACERIGHT);
  displaypcbtext(TXT_THANKSFORTHEFILES,NEWLINE|LFBEFORE);
  scanforuploads(NumFiles - XferVars.SuccessfulUpldFiles);

  #ifndef PCB_DEMO
    givecredits();
  #endif
}


void LIBENTRY send(int NumTokens) {
  int      X;
  char     *OldPointer;
  long     Seconds;
  char     CapName[13];
  char     Str[80];
  spectype File;

  #ifdef PCB_DEMO
    if (UsersData.NumDownloads >= 5) {
      newline();
      printcolor(PCB_RED);
      println("You have reached your 5 file download limit and must have");
      println("your user record reset by the sysop in order to download more.");
      return;
    }

    if (UsersData.TotDnldBytes >= 100000L) {
      newline();
      printcolor(PCB_RED);
      println("You have reached your 100,000 byte download limit and must have");
      println("your user record reset by the sysop in order to download more.");
      return;
    }
  #endif

  // if there is nothing to download,
  // and this isn't a transfer capture, then check to see if
  //   the DLPATH location is blank, or the file is missing and, if so
  //      return an error that there are no dirs available
  if (NumFiles == 0 && ! Status.XferCapture) {
    if (Status.CurConf.PthNameLoc[0] == 0 || fileexist(Status.CurConf.PthNameLoc) == 255) {
      displaypcbtext(TXT_NODIRSAVAILABLE,NEWLINE|LFBEFORE|BELL);
      return;
    }
  }

/*
  if promoting to batch is allowed and
  if no filenames have been stacked on the command line and
  if this is not a read-capture download and
  if the default protocol is not set to NONE and
  if the default protocol is a batch protocol and
  if the caller has sufficient security to use the DB command
    then default to a batch download sequence
*/

  if (PcbData.PromoteBatch && NumTokens == 0 && ! Status.XferCapture && UsersData.Protocol != 'N' && isprotbatch(UsersData.Protocol) && Status.CurSecLevel >= PcbData.UserLevels[SEC_BATCH])
    Status.Batch = TRUE;

  setxferprotocol(Status.DefaultProtocol != 0 ? Status.DefaultProtocol : UsersData.Protocol);

  XferVars.SuccessfulDnldFiles = 0;
  XferVars.SuccessfulDnldBytes = 0;
  XferVars.SuccessfulUpldFiles = 0;
  XferVars.SuccessfulUpldBytes = 0;
  AutoGoodBye = FALSE;
  resetxfercounters(TRUE);
  writeusernetstatus(TRANSFER,NULL);

  if (allocatefilelist(DOWNLOAD) == -1) {
    if (Status.XferCapture)
      restorepointers();
    goto exit2;
  }

  if (Status.CapBye) { /* in case of "B # D GB" command */
    AutoGoodBye = TRUE;
    Status.CapBye = FALSE;
  } else if (Status.XferCapture && Status.CapAsk) {
    Str[0] = 0;
    inputfield(Str,TXT_BYEAFTERDOWNLOAD,1,NEWLINE|FIELDLEN|UPCASE,NOHELP,mask_byeabort);
    if (Str[0] == 'A') {
      restorepointers();
      goto exit2;
    }
    AutoGoodBye = (bool) (Str[0] == 'G');
  }

  /* check to see if the user selected (D)ownload after an "F V" command */
  /* if he did then NumTokens will be 0 and LastView will not be blank   */

  if (NumTokens == 0 && Status.LastView[0] != 0) {
    strcpy(Status.DisplayText,Status.LastView);
    inputfield(Status.LastView,TXT_DFLTFILENAMETODNLD,sizeof(Status.LastView)-1,NEWLINE|GUIDE|FIELDLEN|UPCASE,HLP_D,mask_wildname);
    if ((NumTokens = tokenize(Status.LastView)) == 0)
      goto exit2;  /* exit2 instead of exit so as not to deallocate filelist */
  }

  /* if the file list has already been created - signified by the ability */
  /* to get the first record out of the file list - OR - if there are any */
  /* filenames in the taggedfilename list then ask if the caller wants to */
  /* download the flagged files.                                          */

  if (getfilespec(0,&File) != -1 || Status.TaggedFileNames != NULL) {
    /* we want to avoid the 'download flagged files' question if the user */
    /* uses a 'B # D' command UNLESS there are already some other flagged */
    /* files in the list in which case we'll still ask the question       */
    /* unless the caller wants to do an auto-goodbye, then skip it        */
    if (! Status.SendBlts && ! Status.XferCapture && ! AutoGoodBye) {
      Str[0] = YesChar;
      Str[1] = 0;
      inputfield(Str,TXT_DOWNLOADTAGGED,1,YESNO|NEWLINE|UPCASE|FIELDLEN|GUIDE|LFBEFORE,NOHELP,mask_yesno);
      if (Str[0] == NoChar) {
        deallocatefilelist(THROWAWAY);
        if (allocatefilelist(DOWNLOAD) == -1) {
          if (Status.XferCapture)
            restorepointers();
          goto exit2;
        }
        goto skip;
      }
    }
    /* now we need to flag all files that are in the TaggedFileName list */
    if (Status.TaggedFileNames != NULL) {
      savetokenpointer(&OldPointer);
      if ((X = tokenize(Status.TaggedFileNames)) != 0)
        flagfiles(X);  /* also calls checkdlfiles() ! */
      restoretokenpointer(&OldPointer);
    }

    /* and finally, any files that have been placed into the FileList      */
    /* should bow be checked for time, byte or security level restrictions */
    checkdlfiles(0);
  }

  /* if the caller captured messages to a file then let's add   */
  /* it to the list of files that we want to download right now */

  if (Status.XferCapture && NumFiles < CurBatchLimit) {
    memset(&File,0,sizeof(File));
    strcpy(File.FullPath,Status.CapFileName);
    fileexist(Status.CapFileName);     //lint !e534  fill the DTA variable
    strcpy(File.Name,DTA.ff_name);
    strcpy(CapName,DTA.ff_name);
    File.Size = DTA.ff_fsize;
    if (istimeokay(&File,&Seconds,(bool) (! PcbData.StopClockOnCap))) {
      if (PcbData.StopClockOnCap) {
        File.FreeTime = TRUE;
        IgnoreSeconds += Seconds;
      }
      File.FsecOkay = TRUE;
      File.FreeFile = TRUE;
      #ifdef PCB152
        if (Status.ActStatus != ACT_DISABLED) {
          File.Multiplier = 0;
          // if the multiplier is 0, but we otherwise are set to charge for
          // file or bytes, then record the fact that the file is "NOCOST"
          if (AccountRates.ChargeForDownloadFile != 0 || AccountRates.ChargeForDownloadBytes != 0)
              File.NoCost = TRUE;
        }
      #endif
      File.CapType = (strstr(File.Name,".QW") != NULL ? QWKCAP : MSGCAP);
      TotalSecondsSoFar += Seconds;
      TotalBytesSoFar   += DTA.ff_fsize;
      IgnoreBytes       += DTA.ff_fsize;
      TotalFilesSoFar++;
      IgnoreFiles++;
      if (putfilespec(NumFiles,&File) != -1)
        NumFiles++;
    } else {
      if (NumFiles == 0) {
        AutoGoodBye = FALSE;
        goto exit;
      }
    }
  }


skip:
  /* NumTokens will be NON-zero if this routine was called with filenames  */
  /* on the command line or if an "F V" command was followed by a download */
  /* request in which case we need to scan for any new filenames           */
  scanfornames(NumTokens,DOWNLOAD,TRUE,TRUE,TRUE);

  if (NumFiles == 0 || (Status.Batch && ! (AutoGoodBye || Status.SendBlts) && (NumFiles < CurBatchLimit))) {
    getnames(DOWNLOAD);
    if (NumFiles == 0) {
      AutoGoodBye = FALSE;
      goto exit;
    }
  }

  if (getxferprotocol(FALSE) == 0) {
    AutoGoodBye = FALSE;
    goto exit;
  }

  movefilesinlist();
  if (getfilespec(0,&File) == -1)
    goto exit;

  if (! Status.Batch) {
    if (ProtocolLetterToUse == 'A') {
      if (invalidfilename(File.Name,ALLOWTEXTONLY)) {
        AutoGoodBye = FALSE;
        goto exit;
      }
    }
    showdldetails(&File);
    transferstatus(START);
    if (sendfile(&File) != -1) {
      #ifdef COMM
        waitcomm(TWOSECONDS);
      #endif
      checkdisplaystatus();
      displaypcbtext(TXT_TRANSFERSUCCESSFUL,LFBEFORE|BELL);
      getfilespec(0,&File);                      //lint !e534
      XferVars.CPSdn = File.CPS;
      displaypcbtext(TXT_SENDAVECPS,NEWLINE);
      logsystext(TXT_SENDLOG,SPACERIGHT);
    }
  } else {
    if (! AutoGoodBye) {
      if (editlist() == -1) {
        AutoGoodBye = FALSE;
        goto exit;
      }
      getfilespec(0,&File);  //lint !e534 in case they edited/removed first file
    }
    transferstatus(START);
    displaypcbtext(TXT_SENDINGFILES,NEWLINE);

    sendfile(&File);    //lint !e534

    if (XferVars.SuccessfulDnldFiles != 0) {
      #ifdef COMM
        waitcomm(TWOSECONDS);
      #endif
      checkdisplaystatus();
      displaypcbtext(TXT_BATCHXFERENDED,LFBEFORE|BELL);
      XferVars.CPSdn = averagecps(XferVars.SuccessfulDnldBytes,DNOKAY);
      displaypcbtext(TXT_BATCHSEND,NEWLINE);
      logsystext(TXT_SENDLOG,SPACERIGHT);
    }
  }

exit:
  if (Status.XferCapture) {
    X = findfilename(CapName);
    if (X != -1)
      getfilespec(X,&File);  //lint !e534
    if (X == -1 || File.Success == FAILED) {
      restorepointers();
      if (X != -1) {
        File.Success = REMOVE;  /* make removefinishedfilesfromlist() */
        putfilespec(X,&File);   /* remove this file from the list     */
      }
    }
  }

  if (XferVars.SuccessfulUpldFiles != 0)
    receiveduploads();

  deallocatefiledesc();
  removefinishedfilesfromlist();
  deallocatefilelist(NumFiles == 0 ? THROWAWAY : KEEP);

  if (Status.TaggedFileNames != NULL) {
    bfree(Status.TaggedFileNames);
    Status.TaggedFileNames = NULL;
    Status.NumTaggedFiles = 0;
  }

exit2:
  Status.LastView[0] = 0;
  if (Status.XferCapture) {
    unlink(Status.CapFileName);
    Status.CapFileName[0] = 0;
    Status.XferCapture = FALSE;
    Status.ReadCapture = FALSE;
  }

  #ifdef PCBSTATS
    if (XferVars.SuccessfulDnldFiles != 0)
      updatestats(0,0,XferVars.SuccessfulDnldFiles,UPDATE);
  #endif

  transferstatus(STOP);

  if (AutoGoodBye) {
    // If the transfer was successful, give them 10 seconds to abort the auto
    // logoff.  If the transfer was not successful, then extend the timeout to
    // 45 seconds to give them time to abort logoff.
    autogoodbye(XferVars.SuccessfulDnldFiles != 0 || XferVars.SuccessfulUpldFiles != 0 ? 10 : 45);
  }
}


bool LIBENTRY proceedwithupload(void) {
  char Str[2];

  if (filesflagged()) {
    displaypcbtext(TXT_FILESAREFLAGGED,NEWLINE|LFBEFORE|BELL);
    Str[0] = NoChar;
    Str[1] = 0;
    inputfield(Str,TXT_CONTINUEUPLOAD,1,YESNO|NEWLINE|LFBEFORE|UPCASE|FIELDLEN,NOHELP,mask_yesno);
    if (Str[0] != YesChar)
      return(FALSE);
    deallocatefilelist(THROWAWAY);
  }
  return(TRUE);
}



void LIBENTRY receive(int NumTokens) {
  static char mask_agp[] = {3, 'A', 'G', 'P'};
  char     Str[80];
  spectype File;

  if (! proceedwithupload())
    return;

  setxferprotocol(Status.DefaultProtocol != 0 ? Status.DefaultProtocol : UsersData.Protocol);
  XferVars.SuccessfulUpldFiles = 0;
  XferVars.SuccessfulUpldBytes = 0;
  FirstUploadPrivate = FALSE;

  if (Status.MsgUpload) {
    PrivateFile  = TRUE;
    Status.Batch = FALSE;
    AutoGoodBye  = FALSE;
    if (insufficientfreespace()) {
      displaypcbtext(TXT_INSUFUPLOADSPACE,NEWLINE|LOGIT);
      return;
    }
    memset(&File,0,sizeof(File));
    sprintf(File.Name,"MSG%d.$$$",PcbData.NodeNum);
    strcpy(File.FullPath,File.Name);
    File.FsecOkay = TRUE;
    File.Checked  = TRUE;
    unlink(File.FullPath);
    if (allocatefilelist(UPLOAD) == -1 || putfilespec(0,&File) == -1)
      return;
    NumFiles = 1;
  } else {
    if (Status.CurConf.PrivUplds || Status.FileAttach) {
      if (Status.CurConf.PrvUpldLoc[0] == 0) {
        displaypcbtext(TXT_NODIRSAVAILABLE,NEWLINE|LFBEFORE|BELL);
        return;
      }
    } else {
      if (Status.CurConf.PubUpldLoc[0] == 0) {
        displaypcbtext(TXT_NODIRSAVAILABLE,NEWLINE|LFBEFORE|BELL);
        return;
      }
    }

  /*
    if promoting to batch is allowed and
    if no filenames have been stacked on the command line and
    if the default protocol is not set to NONE and
    if the default protocol is a batch protocol and
    if the caller has sufficient security to use the UB command
      then default to a batch download sequence
  */

    if (PcbData.PromoteBatch && NumTokens == 0 && UsersData.Protocol != 'N' && isprotbatch(UsersData.Protocol) && Status.CurSecLevel >= PcbData.UserLevels[SEC_BATCH])
      Status.Batch = TRUE;

    PrivateFile = Status.CurConf.PrivUplds;

    if (insufficientfreespace()) {
      displaypcbtext(TXT_INSUFUPLOADSPACE,NEWLINE|LOGIT);
      return;
    }


    // WARNING!  This code will fail if:
    //
    // The caller has a time limit of 30 minutes,
    // He logs on MORE THAN 30 minutes before the event so that his time
    // has NOT been adjusted for the upcoming event,
    // And yet the minutes prior to event to guard is being infringed upon
    //
    // Example:  Caller has 30 minutes.  Logs on at 11:30.  Event is set for
    // 00:05.  Guard time is set to 59 minutes.  The code below will NOT
    // prevent the caller from uploading because his time was not adjusted
    // and yet he IS within the 59 minute zone.

    if (Status.TimeAdjustedForEvent && PcbData.EventStopUplds) {
      if (PcbData.MinPriorToEvent == 0 || minutesleft() <= PcbData.MinPriorToEvent) {
        displaypcbtext(TXT_UPLOADSDISABLED,NEWLINE);
        return;
      }
    }

    if (allocatefilelist(UPLOAD) == -1)
      return;

    if (Status.FileAttach)
      Status.Batch = FALSE;

    AutoGoodBye = FALSE;
    writeusernetstatus(TRANSFER,NULL);
    scanfornames(NumTokens,UPLOAD,TRUE,TRUE,TRUE);
    if (NumFiles == 0 || (Status.Batch && ! AutoGoodBye)) {
      getnames(UPLOAD);
      if (NumFiles == 0) {
        AutoGoodBye = FALSE;
        goto exit;
      }
    }
  }


  if (getxferprotocol(FALSE) == 0) {
    AutoGoodBye = FALSE;
    goto exit;
  }

prompt:
  if (Status.Batch && ! AutoGoodBye) {
    Str[0] = 0;
    inputfield(Str,TXT_GOODBYEAFTERUP,1,NEWLINE|LFBEFORE|FIELDLEN|UPCASE,NOHELP,mask_agp);
    switch (Str[0]) {
      case 'A': goto exit;
      case 'G': AutoGoodBye = TRUE;
                break;
      case 'P': if (getxferprotocol(TRUE) == 0)
                  goto exit;
                goto prompt;
    }
  }

  newline();
  if (! PcbData.DisableDriveCheck) {
    displaypcbtext(TXT_UPLOADDRIVE,DEFAULTS);
    printcolor(PCB_GREEN);
    print("  ");
    ucomma(Str,diskfreespace(Status.CurConf.PrvUpldLoc));
    print(Str);
    displaypcbtext(TXT_FREEDISKSPACE,NEWLINE);
  }

  if (! Status.EnteringMessage) {
    displaypcbtext(TXT_UPLOADSTATUS,DEFAULTS);
    displaypcbtext((PrivateFile ? TXT_SCREENED : TXT_POSTEDIMMEDIATELY),NEWLINE);
  }

  displaypcbtext(TXT_PROTOCOLTYPE,DEFAULTS);
  printcolor(PCB_CYAN);
  println(ProtocolToUse.Desc);

  if (ProtocolLetterToUse == 'A')
    displaypcbtext(TXT_MUSTENDWITHCTRLZ,NEWLINE);

  if (Status.QwkUpload) {
    displaypcbtext(TXT_FILESELECTED,DEFAULTS);
    printcolor(PCB_WHITE);
    buildstr(Str,getqwkroot(),".REP",NULL);
    println(Str);
  }

  #ifdef COMM
    if (Asy.Online == REMOTE)
      clearinbuf();               // clear the input buffer
  #endif

  displaypcbtext(TXT_ABORTSTRANSFER,NEWLINE);
  printdefcolor();
  transferstatus(START);

  if (receivefile() != -1) {
    #ifdef COMM
      waitcomm(TWOSECONDS);
    #endif
    checkdisplaystatus();
    if (Status.Batch) {
      displaypcbtext(TXT_BATCHXFERENDED,LFBEFORE|BELL);
      XferVars.CPSup = averagecps(XferVars.SuccessfulUpldBytes,UPOKAY);
      displaypcbtext(TXT_BATCHRECEIVE,NEWLINE);
    } else {
      displaypcbtext(TXT_TRANSFERSUCCESSFUL,LFBEFORE|BELL);
      getfilespec(0,&File);       //lint !e534
      XferVars.CPSup = File.CPS;
      displaypcbtext(TXT_RECEIVEAVECPS,NEWLINE);
    }

    if (XferVars.SuccessfulUpldFiles > 1)
      logsystext(TXT_RECEIVELOG,SPACERIGHT);

    if (! Status.EnteringMessage)
      displaypcbtext(TXT_THANKSFORTHEFILES,NEWLINE|LFBEFORE);
  }

exit:
  scanforuploads(0);
  #ifndef PCB_DEMO
    givecredits();
  #endif

  if (! Status.FileAttach) {
    deallocatefilelist(THROWAWAY);
    deallocatefiledesc();
  }

  transferstatus(STOP);

  if (AutoGoodBye) {
    // If the transfer was successful, give them 10 seconds to abort the auto
    // logoff.  If the transfer was not successful, then extend the timeout to
    // 45 seconds to give them time to abort logoff.
    autogoodbye(XferVars.SuccessfulDnldFiles != 0 || XferVars.SuccessfulUpldFiles != 0 ? 10 : 45);
  }
}
