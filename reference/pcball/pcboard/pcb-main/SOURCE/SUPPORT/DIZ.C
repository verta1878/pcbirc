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


static int _NEAR_ LIBENTRY readfile(void *Buf, int Len, DOSFILE *File) {
  if (dosfread(Buf,Len,File) != Len)
    return(-1);
  return(0);
}

static bool _NEAR_ LIBENTRY testfordiz(char *Name) {
  strupr(Name);
  if (strstr(Name,"FILE_ID.DIZ") != NULL || strstr(Name,"DESC.SDI") != NULL)
    return(TRUE);
  return(FALSE);
}


//---------------------------------------------------------------------------
// ROUTINES FOR ZIP FILES
//---------------------------------------------------------------------------

#pragma pack(1)
typedef struct {
   long  Signature;
   sint  Version;
   sint  BitFlag;
   sint  CompressionMethod;
   sint  FileTime;
   sint  FileDate;
   long  CRC32;
   long  CompressedSize;
   long  UnCompressedSize;
   sint  FileNameLength;
   sint  ExtraFieldLength;
} LOCAL_HEADER;
#pragma pack()

#pragma pack(1)
typedef struct {
   long  Signature;
   sint  VersionMadeBy;
   sint  VersionNeeded;
   sint  BitFlag;
   sint  CompressionMethod;
   sint  FileTime;
   sint  FileDate;
   long  CRC32;
   long  CompressedSize;
   long  UnCompressedSize;
   sint  FileNameLength;
   sint  ExtraFieldLength;
   sint  CommentFieldLength;
   sint  DiskStartNumber;
   sint  InternalAttributes;
   long  ExternalAttributes;
   long  LocalHeaderOffset;
} CENTRAL_RECORD;
#pragma pack()


#define NUMZIPOFFSETS 5

static bool _NEAR_ LIBENTRY checkzipfordiz(DOSFILE *File, bool ExeFile) {
  static long    Offsets[NUMZIPOFFSETS] = {0xBBA, 0x31F0, 0x3CCB, 0x3D99, 0x3D9A};
  int            X;
  char           *ReadPtr;
  long           FileOffset;
  LOCAL_HEADER   LocalFileHeader;
  CENTRAL_RECORD CentralDirRecord;
  char           Name[256];

  /* Read the signature from the first local header */

  if (ExeFile) {
    // the location where the ZIP file starts in a self-extract EXE varies
    // with the version of PKZIP that created it, loop through the known
    // offsets to find a match on the signature, error out if not found
    for (X = 0; X < NUMZIPOFFSETS; X++) {
      dosfseek(File,Offsets[X],SEEK_SET);
      if (readfile(&LocalFileHeader,sizeof(long),File) == -1)
        return(FALSE);
      if (LocalFileHeader.Signature == 0x04034b50L)
        goto found;
    }
    return(FALSE);
  } else {
    if (readfile(&LocalFileHeader,sizeof(long),File) == -1 ||
        LocalFileHeader.Signature != 0x04034b50L)
      return(FALSE);
  }

  /* Skip over all of the compressed files and get to the central directory */

found:
  ReadPtr = (char *)&LocalFileHeader + 4;

  while(1) {
    /* Read the remainder of the local file header (we already read the signature. */
    if (readfile(ReadPtr,sizeof(LOCAL_HEADER)-4,File) == -1)
      return(FALSE);

    FileOffset = LocalFileHeader.FileNameLength +
                 LocalFileHeader.ExtraFieldLength +
                 LocalFileHeader.CompressedSize;

    /* Jump to the next local file header */
    dosfseek(File,FileOffset,SEEK_CUR);

    /* Read the next signature */
    if (readfile(&LocalFileHeader,sizeof(long),File) == -1)
      return(FALSE);

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
      return(FALSE);

    /* It is probably not possible for the Name to have a length of 0 but who knows. */
    if (CentralDirRecord.FileNameLength > 0) {

      /* The file name can be long so allocate space for the name as
         needed rather that write to a statically allocated array */

      /* Read the file name. */
      if (readfile(Name,CentralDirRecord.FileNameLength,File) == -1)
        return(FALSE);

      /* Add the trailing \0 byte. */
      Name[CentralDirRecord.FileNameLength] = '\0';

      if (testfordiz(Name))
        return(TRUE);
    }

    /* Skip over the extra field and the comment field */
    FileOffset = CentralDirRecord.ExtraFieldLength +
                 CentralDirRecord.CommentFieldLength;

    if (FileOffset > 0L)
      if (dosfseek(File,FileOffset,SEEK_CUR) == -1)
        return(FALSE);

    /* Read the signature from the next record. */
    if (readfile(&CentralDirRecord,sizeof(long),File) == -1)
      return(FALSE);

    /* If we get a match then we found the "End of central dir record"
       and we are done. */
    if (CentralDirRecord.Signature == 0x06054b50L)
      break;
  }

  return(FALSE);
}


//---------------------------------------------------------------------------
// ROUTINES FOR ARJ FILES
//---------------------------------------------------------------------------

#define ARJHEADER_ID  0xEA60
#define EXTFILE       0x08

#pragma pack(1)
typedef struct {
  uint     Id;
  uint     BasicSize;
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
  sint     SpecPos;
  sint     Rsv_006;
  sint     Rsv_007;
} arjmainheader;
#pragma pack()

#pragma pack(1)
typedef struct {
  long    CRC;
  sint    ExtLen;
} crcextlen;
#pragma pack()

#pragma pack(1)
typedef struct {
  uint     Id;
  uint     BasicSize;
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
  sint     SpecPos;
  sint     FileMode;
  sint     HostData;
} arjlocalheader;
#pragma pack()

static bool _NEAR_ LIBENTRY checkarjfordiz(DOSFILE *File, bool ExeFile) {
  crcextlen      Header2;
  arjmainheader  MainHeader;
  arjlocalheader LocalHeader;
  char           NameAndComment[256];

  if (ExeFile) {
    dosfseek(File,0x14D1,SEEK_SET);
    if (readfile(&MainHeader,sizeof(MainHeader),File) == -1)
      return(FALSE);
    if (MainHeader.Id != ARJHEADER_ID) {
      dosfseek(File,0x3A0A,SEEK_SET);
      if (readfile(&MainHeader,sizeof(MainHeader),File) == -1)
        return(FALSE);
      if (MainHeader.Id != ARJHEADER_ID)
        return(FALSE);
    }
    if (MainHeader.BasicSize == 0)
      return(FALSE);
  } else {
    dosfseek(File,0,SEEK_SET);
    if (readfile(&MainHeader,sizeof(MainHeader),File) == -1)
      return(FALSE);
    if (MainHeader.Id != ARJHEADER_ID || MainHeader.BasicSize == 0)
      return(FALSE);
  }

  /* skip over name & comment fields */
  /* calculate:  BasicSize minus FirstSize */

  dosfseek(File,MainHeader.BasicSize - MainHeader.FirstSize,SEEK_CUR);
  if (readfile(&Header2,sizeof(Header2),File) == -1)
    return(FALSE);

  /* skip over extended header */
  dosfseek(File,Header2.ExtLen,SEEK_CUR);

  while (1) {
    // this might not get a full header, don't call it an ERROR if it isn't
    // if the header isn't full BasicSize will be 0 for an *normal* return
    readfile(&LocalHeader,sizeof(LocalHeader),File);  /*lint !e534 */

    if (LocalHeader.Id != ARJHEADER_ID || LocalHeader.BasicSize == 0)
      return(FALSE);

    /* if external file, skip over external file size */

    if (LocalHeader.Flags & EXTFILE)
      dosfseek(File,sizeof(long),SEEK_CUR);

    if (readfile(NameAndComment,LocalHeader.BasicSize - LocalHeader.FirstSize,File) == -1)
      return(FALSE);

    if (testfordiz(NameAndComment))
      return(TRUE);

    if (readfile(&Header2,sizeof(Header2),File) == -1)
      return(FALSE);

    /* skip over extended header + compressed file */
    dosfseek(File,Header2.ExtLen + LocalHeader.CompressedSize,SEEK_CUR);
  }
}


//---------------------------------------------------------------------------
// ROUTINES FOR ARC & PAK FILES
//---------------------------------------------------------------------------

#define ARCHEADER_ID  0x1A

#pragma pack(1)
typedef struct {
  char     Id;
  char     Version;
  char     Name[13];
  long     Size;
  sint     Date;
  sint     Time;
  sint     CRC;
  long     OriginalLength;
} pakheader;
#pragma pack()

static bool _NEAR_ LIBENTRY checkarcfordiz(DOSFILE *File, bool ExeFile) {
  pakheader Header;

  if (ExeFile)
    dosfseek(File,0x1A58,SEEK_SET);

  while (1) {
    if (readfile(&Header,sizeof(Header),File) == -1 || Header.Id != ARCHEADER_ID)
      return(FALSE);
    if (Header.Version == 0)
      return(FALSE);

    if (testfordiz(Header.Name))
      return(TRUE);

    dosfseek(File,Header.Size,SEEK_CUR);
  }
}


//---------------------------------------------------------------------------
// ROUTINES FOR LZH/LHA FILES
//---------------------------------------------------------------------------

#pragma pack(1)
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
#pragma pack()


static int _NEAR_ LIBENTRY checklzhfordiz(DOSFILE *File, bool ExeFile) {
  int        Fudge;
  long       Pos;
  lzhheader  Header;
  char       Fname[256];

  Fudge = 2;
  if (ExeFile) {
    dosfseek(File,1636,SEEK_SET);
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
      return(FALSE);

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
      return(FALSE);

    Fname[Header.FnameLen] = 0;
    if (testfordiz(Fname))
      return(TRUE);

    dosfseek(File,Header.Packed+Fudge,SEEK_CUR);
  }
}



//---------------------------------------------------------------------------
// MAIN ROUTINE - calls routines above for specific file types
//---------------------------------------------------------------------------

bool LIBENTRY checkfordizfile(char *FileName) {
  DOSFILE File;

  strupr(FileName);
  if (dosfopen(FileName,OPEN_READ|OPEN_DENYWRIT,&File) == -1)
    return(FALSE);

  if (strstr(FileName,".ZIP") != NULL) {
    if (checkzipfordiz(&File,FALSE))
      goto found;
  } else if (strstr(FileName,".ARJ") != NULL) {
    if (checkarjfordiz(&File,FALSE))
      goto found;
  } else if (strstr(FileName,".LZH") != NULL || strstr(FileName,".LHA") != NULL) {
    if (checklzhfordiz(&File,FALSE))
      goto found;
  } else if (strstr(FileName,".ARC") != NULL || strstr(FileName,".PAK") != NULL) {
    if (checkarcfordiz(&File,FALSE))
      goto found;
  } else if (strstr(FileName,".EXE") != NULL) {
    if (checkzipfordiz(&File,TRUE))
      goto found;
    if (checkarjfordiz(&File,TRUE))
      goto found;
    if (checklzhfordiz(&File,TRUE))
      goto found;
    if (checkarcfordiz(&File,TRUE))
      goto found;
  }

  dosfclose(&File);
  return(FALSE);

found:
  dosfclose(&File);
  return(TRUE);
}
