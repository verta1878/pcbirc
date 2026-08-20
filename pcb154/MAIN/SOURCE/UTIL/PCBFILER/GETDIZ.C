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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <dosfunc.h>
#include <misc.h>
#include <pcb.h>
#include "pcbfiler.h"

#define SOFTSPACE 26 /* we can *never* read in a 26 from the file, so we can */
                     /* safely assume that 26 won't be in the file, so we'll */
                     /* use 26 as a 'soft-space' marker in our word wrapping */

#define SOFTSPACESTR "\x1A"
#define NUMMACROS    77
#define BUFSIZE      1400

typedef enum { INVALIDFILE, ZIPFILE, ARJFILE, EXEFILE, ARCFILE, LZHFILE, SDNFILE } filetype;

int   DizCounter;
char  DizBuf[10][46];


static bool      File_ID;
static bool      Desc_SDI;
// static bool      ShowDates;
static bool      ReservedWord;
static bool      FoundInside;
static char      FileForm[13];
static unsigned  Newest;
static unsigned  Oldest;
// static unsigned  NumFiles;
static bool (near  *checkfilename)(char *Name);


static char Chars[256] = {     /* 1 = format characters, 2 = digits */
/*   0- 31 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*  32- 63 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,
/*  64- 95 */ 0,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*  96-127 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
/* 128-159 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 160-191 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
/* 192-223 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
/* 224-255 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static char *Macros[NUMMACROS] = {
"@AUTOMORE@","@BEEP@","@BICPS@","@BOARDNAME@","@BPS@","@BYTELIMIT@","@BYTERATIO@",
"@BYTESLEFT@","@CARRIER@","@CITY@","@CLREOL@","@CLS@","@CONFNAME@","@CONFNUM@",
"@CURMSGNUM@","@DATAPHONE@","@DAYBYTES@","@DELAY@","@DLBYTES@","@DLFILES@","@EVENT@",
"@EXPDATE@","@EXPDAYS@","@FILERATIO@","@FIRSTU@","@FIRST@","@FREESPACE@",
"@HIGHMSGNUM@","@HOMEPHONE@","@INCONF@","@KBLEFT@","@KBLIMIT@",
"@LASTCALLERSYSTEM@","@LASTCALLERNODE@","@LASTDATEON@","@LASTTIMEON@","@LMR@",
"@LOWMSGNUM@","@MINLEFT@","@MORE@","@MSGLEFT@","@MSGREAD@","@NODE@","@NUMBLT@",
"@NUMCALLS@","@NUMDIR@","@NUMTIMESON@","@OFFHOURS@","@OPTEXT@","@PAUSE@",
"@POFF@","@PON@","@POS@","@PROLTR@","@PRODESC@","@QOFF@","@QON@","@RBYTES@",
"@RCPS@","@RFILES@","@SBYTES@","@SCPS@","@SFILES@","@SECLEVEL@",
"@SYSDATE@", "@SYSOPIN@","@SYSOPOUT@","@SYSTIME@","@TIMELIMIT@","@TIMELEFT@",
"@TIMEUSED@","@TOTALTIME@","@UPBYTES@","@UPFILES@","@USER@","@WAIT@","@WHO@" };

/* return TRUE if we find an expected filename, a name to be ignored when */
/* calculating the Oldest and Newest files                                */

static bool near  checkfordiz(char *Name) {
  strupr(Name);
  if (strstr(Name,"FILE_ID.DIZ") != NULL) {
    File_ID = TRUE;
    return(TRUE);
  } else if (strstr(Name,"DESC.SDI") != NULL) {
    Desc_SDI = TRUE;
    return(TRUE);
  } else if (strcmp(Name,"CON") == 0 ||
             strcmp(Name,"AUX") == 0 ||
             strcmp(Name,"PRN") == 0 ||
             strcmp(Name,"LPT1") == 0 ||
             strcmp(Name,"LPT2") == 0 ||
             strcmp(Name,"LPT3") == 0 ||
             strcmp(Name,"COM1") == 0 ||
             strcmp(Name,"COM2") == 0 ||
             strcmp(Name,"COM3") == 0 ||
             strcmp(Name,"COM4") == 0 ||
             strcmp(Name,"CLOCK$") == 0) {
    ReservedWord = TRUE;
    return(TRUE);
  }
  return(FALSE);
}


static bool near  checkinside(char *Name) {
  strupr(Name);
  formatwild(Name);
  if (equalwilds(FileForm,Name))
    FoundInside = TRUE;

  return(TRUE);
}


static int near  readfile(void *Buf, int Len, DOSFILE *File) {
  if (dosfread(Buf,Len,File) != Len)
    return(-1);
  return(0);
}

/*****************************************************************************/
/* ROUTINES FOR ZIP FILES                                                    */
/*****************************************************************************/

typedef struct {
   long  Signature;
   int   Version;
   int   BitFlag;
   int   CompressionMethod;
   int   FileTime;
   int   FileDate;
   long  CRC32;
   long  CompressedSize;
   long  UnCompressedSize;
   int   FileNameLength;
   int   ExtraFieldLength;
} LOCAL_HEADER;

typedef struct {
   long  Signature;
   int   VersionMadeBy;
   int   VersionNeeded;
   int   BitFlag;
   int   CompressionMethod;
   int   FileTime;
   unsigned FileDate;
   long  CRC32;
   long  CompressedSize;
   long  UnCompressedSize;
   int   FileNameLength;
   int   ExtraFieldLength;
   int   CommentFieldLength;
   int   DiskStartNumber;
   int   InternalAttributes;
   long  ExternalAttributes;
   long  LocalHeaderOffset;
} CENTRAL_RECORD;

#define NUMZIPOFFSETS 6

static int near  readzipfile(DOSFILE *File, bool ExeFile) {
  static long    Offsets[NUMZIPOFFSETS] = {0xBBA, 0x31F0, 0x3CCB, 0x3D99, 0x3D9A, 0x3E71};
  char           FileName[256];
  LOCAL_HEADER   LocalFileHeader;
  CENTRAL_RECORD CentralDirRecord;
  long           FileOffset;
  char           *ReadPtr;
  int            X;

  /* Read the signature from the first local header */

  dosrewind(File);
  if (ExeFile) {
    /* the location where the ZIP file starts in a self-extract EXE varies */
    /* with the version of PKZIP that created it, loop through the known   */
    /* offsets to find a match on the signature, error out if not found    */
    for (X = 0; X < NUMZIPOFFSETS; X++) {
      dosfseek(File,Offsets[X],SEEK_SET);
      if (readfile(&LocalFileHeader,sizeof(long),File) == -1)
        return(-1);
      if (LocalFileHeader.Signature == 0x04034b50L)
        goto found;
    }
    return(-1);
  } else {
    if (readfile(&LocalFileHeader,sizeof(long),File) == -1 ||
        LocalFileHeader.Signature != 0x04034b50L)
      return(-1);
  }

  /* Skip over all of the compressed files and get to the central directory */

found:
  ReadPtr = (char *)&LocalFileHeader + 4;

  while(1) {
    /* Read the remainder of the local file header (we already read the signature. */
    if (readfile(ReadPtr,sizeof(LOCAL_HEADER)-4,File) == -1)
      return(-1);

    FileOffset = LocalFileHeader.FileNameLength +
                 LocalFileHeader.ExtraFieldLength +
                 LocalFileHeader.CompressedSize;

    /* Jump to the next local file header */
    dosfseek(File,FileOffset,SEEK_CUR);

    /* Read the next signature */
    if (readfile(&LocalFileHeader,sizeof(long),File) == -1)
      return(-1);

    /* If we get a match we have found the beginning of the central directory. */
    if (LocalFileHeader.Signature == 0x02014b50L)
      break;
  }


  CentralDirRecord.Signature = LocalFileHeader.Signature;

  /* Read the records in the central directory one at a time */

  ReadPtr = (char *)&CentralDirRecord + 4;

  while(1) {
    /* Read the remainder of the file header record (we already
       have the signature.  ReadPtr points into CentralDirRecord
       4 bytes from the beginning (right behind the signature. */
    if (readfile(ReadPtr,sizeof(CENTRAL_RECORD)-4,File) == -1)
      return(-1);

    /* It is probably not possible for the FileName to have a length of 0 but who knows. */
    if (CentralDirRecord.FileNameLength > 0) {

      /* The file name can be long so allocate space for the name as
         needed rather that write to a statically allocated array */

      /* Read the file name. */
      if (readfile(FileName,CentralDirRecord.FileNameLength,File) == -1)
        return(-1);

      /* Add the trailing \0 byte. */
      FileName[CentralDirRecord.FileNameLength] = '\0';

      /* Check the filename - record stats only if it's not a DIZ file */
      if (! checkfilename(FileName))  {
        if (CentralDirRecord.FileDate > Newest)
          Newest = CentralDirRecord.FileDate;
        if (CentralDirRecord.FileDate < Oldest)
          Oldest = CentralDirRecord.FileDate;
      }
//    NumFiles++;
    }


    /* Skip over the extra field and the comment field */
    FileOffset = CentralDirRecord.ExtraFieldLength +
                 CentralDirRecord.CommentFieldLength;

    if (FileOffset > 0L)
      if (dosfseek(File,FileOffset,SEEK_CUR) == -1)
        return(-1);

    /* Read the signature from the next record. */
    if (readfile(&CentralDirRecord,sizeof(long),File) == -1)
      return(-1);

    /* If we get a match then we found the "End of central dir record"
       and we are done. */
    if (CentralDirRecord.Signature == 0x06054b50L)
      break;
  }

  return(0);
}

/*****************************************************************************/
/* ROUTINES FOR ARJ FILES                                                    */
/*****************************************************************************/

#define ARJHEADER_ID  0xEA60
#define EXTFILE       0x08

typedef struct {
  unsigned Id;
  unsigned BasicSize;
  char     FirstSize;
  char     Version;
  char     VtoExtract;
  char     HostOs;
  char     Flags;
  char     Rsv_001;
  char     Type;
  char     Rsv_002;
  long     TimeDate;
  long     Rsv_003;
  long     Rsv_004;
  long     Rsv_005;
  int      SpecPos;
  int      Rsv_006;
  int      Rsv_007;
} arjmainheader;

typedef struct {
  long    CRC;
  int     ExtLen;
} crcextlen;

typedef struct {
  unsigned Id;
  unsigned BasicSize;
  char     FirstSize;
  char     Version;
  char     VtoExtract;
  char     HostOs;
  char     Flags;
  char     Method;
  char     Type;
  char     Rsv_001;
  long     TimeDate;
  long     CompressedSize;
  long     OriginalSize;
  long     OriginalCRC;
  int      SpecPos;
  int      FileMode;
  int      HostData;
} arjlocalheader;

static int near  readarjfile(DOSFILE *File, bool ExeFile) {
  char           NameAndComment[256];
  arjmainheader  MainHeader;
  arjlocalheader LocalHeader;
  crcextlen      Header2;
  unsigned       Date;

  dosrewind(File);
  if (ExeFile) {
    dosfseek(File,0x14D1,SEEK_SET);
    if (readfile(&MainHeader,sizeof(MainHeader),File) == -1)
      return(-1);
    if (MainHeader.Id != ARJHEADER_ID) {
      dosfseek(File,0x3A0A,SEEK_SET);
      if (readfile(&MainHeader,sizeof(MainHeader),File) == -1)
        return(-1);
      if (MainHeader.Id != ARJHEADER_ID)
        return(-1);
    }
    if (MainHeader.BasicSize == 0)
      return(-1);
  } else {
    dosfseek(File,0,SEEK_SET);
    if (readfile(&MainHeader,sizeof(MainHeader),File) == -1)
      return(-1);
    if (MainHeader.Id != ARJHEADER_ID || MainHeader.BasicSize == 0)
      return(-1);
  }

  /* skip over name & comment fields */
  /* calculate:  BasicSize minus FirstSize */

  dosfseek(File,MainHeader.BasicSize - MainHeader.FirstSize,SEEK_CUR);
  if (readfile(&Header2,sizeof(Header2),File) == -1)
    return(-1);

  /* skip over extended header */
  dosfseek(File,Header2.ExtLen,SEEK_CUR);

  while (1) {
    /* this might not get a full header, don't call it an ERROR if it isn't */
    /* if the header isn't full BasicSize will be 0 for an *normal* return  */
    readfile(&LocalHeader,sizeof(LocalHeader),File);  //lint !e534

    if (LocalHeader.Id != ARJHEADER_ID)
      return(-1);

    if (LocalHeader.BasicSize == 0)  /* that's the end! */
      return(0);

    /* if external file, skip over external file size */

    if (LocalHeader.Flags & EXTFILE)
      dosfseek(File,sizeof(long),SEEK_CUR);

    if (readfile(NameAndComment,LocalHeader.BasicSize - LocalHeader.FirstSize,File) == -1)
      return(-1);

    /* Check the filename - record stats only if it's not a DIZ file */
    if (! checkfilename(NameAndComment))  {
      Date = (int) (LocalHeader.TimeDate >> 16);
      if (Date > Newest)
        Newest = Date;
      if (Date < Oldest)
        Oldest = Date;
    }
//  NumFiles++;

    if (readfile(&Header2,sizeof(Header2),File) == -1)
      return(-1);

    /* skip over extended header + compressed file */
    dosfseek(File,Header2.ExtLen + LocalHeader.CompressedSize,SEEK_CUR);
  }
}


/*****************************************************************************/
/* ROUTINES FOR ARC FILES                                                    */
/*****************************************************************************/

#define ARCHEADER_ID  0x1A

typedef struct {
  char     Id;
  char     Version;
  char     Name[13];
  long     Size;
  unsigned Date;
  int      Time;
  int      CRC;
  long     OriginalLength;
} pakheader;


static int near  readarcfile(DOSFILE *File, bool ExeFile) {
  pakheader Header;

  dosrewind(File);
  if (ExeFile)
    dosfseek(File,0x1A58,SEEK_SET);


  while (1) {
    if (readfile(&Header,sizeof(Header),File) == -1)
      return(0);
    if (Header.Id != ARCHEADER_ID)
      return(-1);
    if (Header.Version == 0)
      return(0);

    if (! checkfilename(Header.Name))  {
      if (Header.Date > Newest)
        Newest = Header.Date;
      if (Header.Date < Oldest)
        Oldest = Header.Date;
    }
//  NumFiles++;

    dosfseek(File,Header.Size,SEEK_CUR);
  }
}


/*****************************************************************************/
/* ROUTINES FOR LZH FILES                                                    */
/*****************************************************************************/

typedef struct {
  unsigned char   Size;
  unsigned char   Sum;
  unsigned char   Method[5];
  unsigned long   Packed;
  unsigned long   Original;
  unsigned long   Time;
  unsigned short  Attr;
  unsigned char   FnameLen;
} lzhheader;


static int near  readlzhfile(DOSFILE *File, bool ExeFile) {
  int        Fudge;
  unsigned   Date;
  long       Pos;
  lzhheader  Header;
  char       Fname[256];

  Fudge = 2;
  if (ExeFile) {
    Pos = 1636;
    dosfseek(File,Pos,SEEK_SET);
    if (readfile(&Header,sizeof(Header),File) == -1)
      return(0);
    if (memcmp(Header.Method,"-lh",3) != 0) {
      Pos = 1945;
/*
      repeat this process as necessary to get to the last known offset
      dosfseek(File,Pos,SEEK_SET);
      if (readfile(&Header,sizeof(Header),File) == -1)
        return(0);

*/
    }
    dosfseek(File,Pos,SEEK_SET);
  }

  while (1) {
    if (readfile(&Header,sizeof(Header),File) == -1)
      return(0);

    if (memcmp(Header.Method,"-lh",3) != 0) {
      /* sometimes Fudge is equal to 5, back up to 3 bytes AFTER the last  */
      /* read, and re-read the header, if we find the signature here, then */
      /* set the Fudge Factor to 5 instead of 2.                           */
      Pos = dosfseek(File,0,SEEK_CUR) - sizeof(Header) + 3;
      dosfseek(File,Pos,SEEK_SET);
      if (readfile(&Header,sizeof(Header),File) == -1)
        return(0);
      if (memcmp(Header.Method,"-lh",3) != 0)
        return(0);
      Fudge = 5;
    }

    if (Header.Size == 0)
      return(0);

    if (readfile(Fname,Header.FnameLen,File) == -1)
      return(-1);

    Fname[Header.FnameLen] = 0;
    if (! checkfilename(Fname))  {
      Date = (int) (Header.Time >> 16);
      if (Date > Newest)
        Newest = Date;
      if (Date < Oldest)
        Oldest = Date;
    }
//  NumFiles++;

    dosfseek(File,Header.Packed+Fudge,SEEK_CUR);
  }
}


/*****************************************************************************/
/* MAIN ROUTINES                                                             */
/*****************************************************************************/

static void near  subst(char *Str, char *Search, char *Replace) {
  char *p;
  int  SrchLen;
  int  RepLen;

  while ((p = strstr(Str,Search)) != NULL) {
    SrchLen = strlen(Search);
    RepLen  = strlen(Replace);

    if (SrchLen >= RepLen) {
      memcpy(p,Replace,RepLen);
      strcpy(p+RepLen,p+SrchLen);
    } else {
      memmove(p+RepLen,p+SrchLen,strlen(p+SrchLen)+1);
      memcpy(p,Replace,RepLen);
    }

    Str = p;
  }
}


static void near  removecolorcodes(char *Buf) {
  char *p;

  while ((p = strstr(Buf,"@X")) != NULL) {
    if (Chars[p[2]] == 2 && Chars[p[3]] == 2) {
      memmove(p,p+4,strlen(p+4)+1);
      Buf = p;
    } else {
      Buf = p+2;  /* wasn't a valid @X code, skip past @X and look for more */
    }
  }
}


static void near  removemacros(char *Buf) {
  int X;

  if (strchr(Buf,'@') != NULL) {
    for (X = 0; X < NUMMACROS; X++)
      subst(Buf,Macros[X],"");
    removecolorcodes(Buf);
  }

  subst(Buf,"\x7" ,"");  /* remove bell */
  subst(Buf,"\x8" ,"");  /* remove backspace */
  subst(Buf,"\x9" ,"");  /* remove tab */
  subst(Buf,"\xA" ,"");  /* remove line feed */
  subst(Buf,"\xC" ,"");  /* remove form feed */
  subst(Buf,"\x13","");  /* remove XOFF */
  subst(Buf,"\x7F","");  /* remove backspace */
}


static char * near  isformatted(char *Line) {
  char *p;

  /* scan line for a SOFTSPACE */
  /* if one is found, check to see if it is bounded on the left and right */
  /* by format characters, if so, return a pointer to the softspace       */

  for (p = Line; *p; p++)
    if (*p == SOFTSPACE)
      if (p == Line || Chars[*(p-1)] == 1)   /* format codes? */
        if (p[1] == 0 || Chars[p[1]] == 1)   /* format codes? */
          return(p);

  return(NULL);
}


static char * near  getnextline(char *p, char *Line) {
  char Temp[46];
  char *q;

  stripleft(p,SOFTSPACE);

  memcpy(Line,p,45);
  Line[45] = 0;

  if ((q = isformatted(Line)) != NULL) {
    *q = 0;
    return((p+ (int) (q-Line)) + 1);
  }

  if (strlen(p) <= 45)
    return(NULL);


  /* Is this a good break point for the line?  If so, just  */
  /* return a pointer to the next character after the space */

  if (p[45] == ' ' || p[45] == SOFTSPACE)
    return(p+46);

  /* Can the line be shortened to LESS THAN 45 characters by finding the  */
  /* last space and then word wrapping it?  If so, terminate the line and */
  /* return a pointer to show where we left off in the string.            */

  memcpy(Temp,Line,46);
  subst(Temp,SOFTSPACESTR," ");

  if ((q = strrchr(Temp,' ')) != NULL) {
    q = (Line + (int) (q-Temp));
    *q = 0;
    return((p+ (int) (q-Line)) + 1);
  }

  /* The string was more than 45 characters long and could not be      */
  /* broken up so we'll just point to the next character in the string */
  return(p+45);
}


/*
static char * near  itostr(int Num) {
  char static Str[20];

  itoa(Num,Str,10);
  return(Str);
}
*/


static void near  copytoline(char *Str, bool IncrementCounter) {
  strcat(DizBuf[DizCounter],Str);
  if (IncrementCounter && DizCounter < 10)
    DizCounter++;
}


static void near  resetbuffer(void) {
  memset(DizBuf,0,sizeof(DizBuf));
  DizCounter = 0;
}


/*
static void near  outputdate(int Date, char *Line) {
  int Month;
  int Day;
  int Year;

  Month = (Date >> 5) & 0x0f;
  Day   = Date & 0x1f;
  Year  = ((Date >> 9) & 0x7f) + 80;

  copytoline(Line,FALSE);
  copytoline(itostr(Month),FALSE);
  copytoline("/",FALSE);
  copytoline(itostr(Day),FALSE);
  copytoline("/",FALSE);
  copytoline(itostr(Year),FALSE);
}



static void near  writedates(void) {

  if (ShowDates) {
    copytoline("Files: ",FALSE);
    copytoline(itostr(NumFiles),FALSE);
    outputdate(Oldest,"  Oldest: ");
    outputdate(Newest,"  Newest: ");
    copytoline("",TRUE);
  }
}
*/


static int near  getdescription(char *InFile, DOSFILE *Out) {
  char    Buf[BUFSIZE];
  char    Line[50];
  DOSFILE File;
  int     NumLines;
  int     NumChars;
  char    *p;

  if (dosfopen(InFile,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(-1);

  for (p = Buf; dosfgets(p,256,&File) != -1 && p < &Buf[1200]; p += strlen(p)) {
    stripright(p,' ');
    stripleft(p,' ');
    addchar(p,SOFTSPACE);      /* add a soft-space */
  }

  stripright(Buf,SOFTSPACE);  /* remove ending soft-space */
  dosfclose(&File);

  removemacros(Buf);

  p = Buf;
  NumLines = 0;
  NumChars = 0;

  while (1) {
    p = getnextline(p,Line);
    subst(Line,SOFTSPACESTR," ");
    stripright(Line,' ');
    copytoline(Line,TRUE);
    if (++NumLines > 10 || (NumChars += strlen(Line)) > 450 || p == NULL)
      break;
  }

  doswrite(Out->handle,"",0);     //lint !e534
  return(0);
}


static filetype near  examinefile(char *FileName) {
  filetype Type;
  DOSFILE  File;

  if (strstr(FileName,".ZIP") != NULL)
    Type = ZIPFILE;
  else if (strstr(FileName,".ARJ") != NULL)
    Type = ARJFILE;
  else if (strstr(FileName,".LZH") != NULL || strstr(FileName,".LHA") != NULL)
    Type = LZHFILE;
  else if (strstr(FileName,".ARC") != NULL || strstr(FileName,".PAK") != NULL)
    Type = ARCFILE;
  else if (strstr(FileName,".EXE") != NULL)
    Type = EXEFILE;
  else if (strstr(FileName,".SDN") != NULL)
    Type = SDNFILE;
  else
    return(INVALIDFILE);  // non-supported format

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(INVALIDFILE);  // can't open file

  if (Type == ZIPFILE) {
    if (readzipfile(&File,FALSE) == -1)
      goto invalid;
  } else if (Type == ARJFILE) {
    if (readarjfile(&File,FALSE) == -1)
      goto invalid;
  } else if (Type == LZHFILE) {
    if (readlzhfile(&File,FALSE) == -1)
      goto invalid;
  } else if (Type == ARCFILE) {
    if (readarcfile(&File,FALSE) == -1)
      goto invalid;
  } else if (Type == EXEFILE) {
    if (readzipfile(&File,TRUE) != -1)
      Type = ZIPFILE;
    else if (readarjfile(&File,TRUE) != -1)
      Type = ARJFILE;
    else if (readarcfile(&File,TRUE) != -1)
      Type = ARCFILE;
    else if (readlzhfile(&File,TRUE) != -1)
      Type = LZHFILE;
    else
      goto invalid;
  } else if (Type == SDNFILE) {
    if (readarjfile(&File,FALSE) != -1)
      Type = ARJFILE;
    else if (readarcfile(&File,FALSE) != -1)
      Type = ARCFILE;
    else
      goto invalid;
  }

  dosfclose(&File);
  return(Type);

invalid:
  dosfclose(&File);
  return(INVALIDFILE);
}



bool  wildcardinsidefile(char *FileName, char *FileSpec) {
  checkfilename = checkinside;

  strcpy(FileForm,FileSpec);
  if (examinefile(FileName) == INVALIDFILE)
    return(FALSE);

  return(FoundInside);
}


int  getdates(char *FileName, unsigned *OldestDate, unsigned *NewestDate) {
  File_ID      = FALSE;
  Desc_SDI     = FALSE;
//ShowDates    = TRUE;
  Oldest       = 0xFFFF;
  Newest       = 0;

  checkfilename = checkfordiz;
  if (examinefile(FileName) == INVALIDFILE)
    return(-1);

  if (Oldest != 0xFFFF)
    *OldestDate = Oldest;

  if (Newest != 0)
    *NewestDate = Newest;

  return(0);
}


int  extractdiz(char *FileName) {
  int           RetVal;
  int           X,Y;
  fonttype      SaveFont;
  filetype      Type;
  char         *DescName;
  char         *UnPacker;
  char         *UnPackerCommand;
  DOSFILE       TempFile;
  char          CommandLine[128];
  savescrntype  ScrnBuf;

  ReservedWord = FALSE;
  File_ID      = FALSE;
  Desc_SDI     = FALSE;
//ShowDates    = FALSE;
  Oldest       = 0xFFFF;
  Newest       = 0;
//NumFiles     = 0;

  savescreen(&ScrnBuf);
  SaveFont = getfont();
  resetbuffer();

  checkfilename = checkfordiz;
  if ((Type = examinefile(FileName)) == INVALIDFILE)
    return(1);

  if (ReservedWord) {
    //  WARNING!!!   FILE CONTAINS A RESERVED WORD!
    return(2);
  }

  if (File_ID || Desc_SDI) {
    if (File_ID)
      DescName = "FILE_ID.DIZ";
    else
      DescName = "DESC.SDI";

    switch (Type) {
      case ZIPFILE:
                    UnPacker = "PKUNZIP";
                    UnPackerCommand = "-o";
                    break;
      case ARJFILE:
                    UnPacker = "ARJ";
                    UnPackerCommand = "e";
                    break;
      case LZHFILE:
                    UnPacker = "LHA";
                    UnPackerCommand = "e";
                    break;
      case ARCFILE:
                    if (strstr(FileName,".ARC") != NULL) {
                      UnPacker = "PKXARC";
                      UnPackerCommand = "-e";
                    } else {
                      UnPacker = "PAK";
                      UnPackerCommand = "e";
                    }
                    break;
      default     : return(1);
    }

    setfont(BIGFONT);
    cls();
    X = wherex();
    Y = wherey();
    gotoxy(0,0);

    sprintf(CommandLine,"%s %s %s",UnPackerCommand,FileName,DescName);
    shelltoprogram(UnPacker,CommandLine);

    setfont(SaveFont);
    restorescreen(&ScrnBuf);
    gotoxy(X,Y);

    if (fileexist(DescName) == 255)
      return(3);
  } else {
//  if (ShowDates)
//    writedates();
    return(5);
  }

  RetVal = getdescription(DescName,&TempFile);
//if (ShowDates)
//  writedates();

  unlink(DescName);
  return(RetVal == -1 ? 4 : 0);
}
