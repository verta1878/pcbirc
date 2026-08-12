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
#include <bios.h>
#include <dos.h>
#include <dosfunc.h>
#include <scrnio.h>
#include <newdata.h>
#include <pcb.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMCOMPTYPES      8
#define NUMDEFLATETYPES   6


/* Structure of the local file header */

struct LOCAL_HEADER {
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
};


/* Structure of the central directory record */
struct CENTRAL_RECORD {
   long  Signature;
   int   VersionMadeBy;
   int   VersionNeeded;
   int   BitFlag;
   int   CompressionMethod;
   int   FileTime;
   int   FileDate;
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
};



/* Global variables */

static int   TotalFiles;
static long  TotalBytes;
static long  TotalUnCompressedBytes;


static char *CompressionType[NUMCOMPTYPES+1] = {
  " Stored ",
  " Shrunk ",
  "Reduced1",
  "Reduced2",
  "Reduced3",
  "Reduced4",
  "Imploded",
  "Unknown ",
  "DeflateX",
};

static char *DeflateTypes = "NNXXFFSS";


/* Function prototypes */

static void near pascal DisplayHeaderRecord(struct CENTRAL_RECORD *CentralDirRecord,char *FileName);
static void near pascal DisplayTotals(void);
extern "C" int  pascal _PRINTARCLINE(char *str);



int pascal ListZipFile(char *ZipFileName) {
  struct  LOCAL_HEADER   LocalFileHeader;
  struct  CENTRAL_RECORD CentralDirRecord;
  DOSFILE ZipFile;
  int     Status;
  long    FileOffset;
  char    *FileName;
  char    *ReadPtr;


  if (dosfopen(ZipFileName,OPEN_READ|OPEN_DENYWRIT,&ZipFile) == -1) {
    _PRINTARCLINE("unable to open file");  //lint !e534
    return(1);
  }

  dossetbuf(&ZipFile,8192);

  TotalFiles = 0;
  TotalBytes = 0L;
  TotalUnCompressedBytes = 0L;


  /* Read the signature from the first local header */

  Status = dosfread((char *)&LocalFileHeader,sizeof(long),&ZipFile);

  if (Status != sizeof(long) || LocalFileHeader.Signature != 0x04034b50L) {
    _PRINTARCLINE("Not a ZIP file");   //lint !e534
    dosfclose(&ZipFile);
    return(1);
  }

  /* Skip over all of the compressed files and get to the central directory */

  ReadPtr = (char *)&LocalFileHeader;
  ReadPtr += 4;


  while(1) {
    /* Read the remainder of the local file header (we already read the signature. */
    if (dosfread(ReadPtr,sizeof(struct LOCAL_HEADER)-4,&ZipFile) != sizeof(struct LOCAL_HEADER)-4) {
      _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
      dosfclose(&ZipFile);
      return(1);
    }

    FileOffset = LocalFileHeader.FileNameLength +
                 LocalFileHeader.ExtraFieldLength +
                 LocalFileHeader.CompressedSize;

    /* Jump to the next local file header */
    if (dosfseek(&ZipFile,FileOffset,SEEK_CUR) == -1) {
      _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
      dosfclose(&ZipFile);
      return(1);
    }

    /* Read the next signature */
    if (dosfread((char *)&LocalFileHeader,sizeof(long),&ZipFile) != sizeof(long)) {
      _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
      dosfclose(&ZipFile);
      return(1);
    }

    /* If we get a match we have found the beginning of the central directory. */
    if (LocalFileHeader.Signature == 0x02014b50L)
      break;
  }


  CentralDirRecord.Signature = LocalFileHeader.Signature;

  /* Read the records in the central directory one at a time */

  ReadPtr = (char *)&CentralDirRecord;
  ReadPtr += 4;

  while(1) {
    /* Read the remainder of the file header record (we already
       have the signature.  ReadPtr points into CentralDirRecord
       4 bytes from the beginning (right behind the signature. */
    if (dosfread(ReadPtr,sizeof(struct CENTRAL_RECORD)-4,&ZipFile) != sizeof(struct CENTRAL_RECORD)-4) {
      _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
      dosfclose(&ZipFile);
      return(1);
    }

    /* It is probably not possible for the FileName to have a length of 0 but who knows. */
    if (CentralDirRecord.FileNameLength > 0) {

      /* The file name can be long so allocate space for the name as
         needed rather that write to a statically allocated array */

      if ((FileName = (char *) mallochk(CentralDirRecord.FileNameLength + 1)) == NULL)
        return(1);

      /* Read the file name. */
      if (dosfread(FileName,CentralDirRecord.FileNameLength,&ZipFile) != CentralDirRecord.FileNameLength) {
        _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
        dosfclose(&ZipFile);
        return(1);
      }

      /* Add the trailing \0 byte. */
      FileName[CentralDirRecord.FileNameLength] = '\0';

      /* Display the record. */
      DisplayHeaderRecord(&CentralDirRecord,FileName);

      /* The file name is not needed any more. */
      free(FileName);
    }


    /* Skip over the extra field and the comment field */
    FileOffset = CentralDirRecord.ExtraFieldLength +
                 CentralDirRecord.CommentFieldLength;

    if (FileOffset > 0L) {
      if (dosfseek(&ZipFile,FileOffset,SEEK_CUR) == -1) {
        _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
        dosfclose(&ZipFile);
        return(1);
      }
    }


    /* Read the signature from the next record. */
    if (dosfread((char *)&CentralDirRecord,sizeof(long),&ZipFile) != sizeof(long)) {
      _PRINTARCLINE("Invalid ZIP file format");  //lint !e534
      dosfclose(&ZipFile);
      return(1);
    }

    /* If we get a match then we found the "End of central dir record"
       and we are done. */
    if (CentralDirRecord.Signature == 0x06054b50L)
      break;

  }

  dosfclose(&ZipFile);
  DisplayTotals();
  return(0);
}



static void near pascal DisplayHeaderRecord(struct CENTRAL_RECORD *CentralDirRecord, char *FileName) {
  char  Str[80];
  char  Temp[20];
  long  SizeReduction;
  int   Month;
  int   Day;
  int   Year;
  int   Hour;
  int   Minutes;
  int   StowageFactor;
  char  *NamePtr;

  if (CentralDirRecord->UnCompressedSize == 0) {
    StowageFactor = 0;
  } else {
    if (CentralDirRecord->CompressedSize <= CentralDirRecord->UnCompressedSize) {
      SizeReduction = CentralDirRecord->UnCompressedSize - CentralDirRecord->CompressedSize;
      if (SizeReduction == 0L) {
        StowageFactor = 0;
      } else {
        SizeReduction = SizeReduction * 100L + 50;
        StowageFactor = (int)(SizeReduction/CentralDirRecord->UnCompressedSize);
      }
    } else {
      SizeReduction = CentralDirRecord->CompressedSize - CentralDirRecord->UnCompressedSize;
      SizeReduction = SizeReduction * 100L + 50;
      StowageFactor = (int)(SizeReduction/CentralDirRecord->UnCompressedSize);
      StowageFactor *= -1;
    }
  }

  if (StowageFactor >= 100)
    StowageFactor = 0;

  /* Convert the DOS internal date and time format to something we can
     use for output. */

  if (CentralDirRecord->FileDate != 0) {
    Month = (CentralDirRecord->FileDate >> 5) & 0x0f;
    Day   = CentralDirRecord->FileDate & 0x1f;
    Year  = ((CentralDirRecord->FileDate >> 9) & 0x7f) + 80;
    if (Month < 1 || Month > 12)
      Month = 1;
    if (Year > 100)
      Year -= 100;
  } else {
    Month = 1;
    Day   = 1;
    Year  = 80;
  }

  if (CentralDirRecord->FileTime != 0) {
    Hour = (CentralDirRecord->FileTime >> 11) & 0x1f;
    Minutes = (CentralDirRecord->FileTime >> 5) & 0x3f;
  } else {
    Hour = 0;
    Minutes = 0;
  }

  /* The ZIP documentation says that path names are stored with '/'s
     rather than '\'s.  Look for a '/' and if so point to the file
     name rather than the whole path name. */

  if ((NamePtr = strrchr(FileName,'/')) == NULL) {
    NamePtr = FileName;
  }

  if (CentralDirRecord->CompressionMethod > NUMCOMPTYPES)
    strcpy(Temp,"Unknown ");
  else {
    strcpy(Temp,CompressionType[CentralDirRecord->CompressionMethod]);
    if (CentralDirRecord->CompressionMethod == 8) {
      if (CentralDirRecord->BitFlag <= NUMDEFLATETYPES) {
        Temp[7] = DeflateTypes[CentralDirRecord->BitFlag];
      }
    }
  }

  sprintf(Str,"%-17s%8ld  %s   %2d%%    %8ld  %02d %s %2d  %02d:%02d",
          NamePtr,
          CentralDirRecord->UnCompressedSize,
          Temp,
          StowageFactor,
          CentralDirRecord->CompressedSize,
          Day,
          MonthName[Month-1],
          Year,
          Hour,
          Minutes);
  _PRINTARCLINE(Str);  //lint !e534

  TotalFiles += 1;
  TotalBytes += CentralDirRecord->CompressedSize;
  TotalUnCompressedBytes += CentralDirRecord->UnCompressedSize;
}


static void near pascal DisplayTotals(void) {
  char  Str[80];
  long  SizeReduction;
  int   StowageFactor;

  if (TotalUnCompressedBytes == 0) {
    StowageFactor = 0;
  } else {
    if (TotalBytes <= TotalUnCompressedBytes) {
      SizeReduction = TotalUnCompressedBytes - TotalBytes;
      if (SizeReduction == 0L) {
        StowageFactor = 0;
      } else {
        SizeReduction = SizeReduction * 100L + 50;
        StowageFactor = (int)(SizeReduction/TotalUnCompressedBytes);
      }
    } else {
      SizeReduction = TotalBytes - TotalUnCompressedBytes;
      SizeReduction = SizeReduction * 100L + 50;
      StowageFactor = (int)(SizeReduction/TotalUnCompressedBytes);
      StowageFactor *= -1;
    }
  }

  if (StowageFactor >= 100)
    StowageFactor = 0;

  _PRINTARCLINE("------------   ----------            ----  ----------");  //lint !e534
  sprintf(Str,"Total %6d     %8ld             %2d%%    %8ld",
          TotalFiles,
          TotalUnCompressedBytes,
          StowageFactor,
          TotalBytes);
  _PRINTARCLINE(Str);  //lint !e534
}
