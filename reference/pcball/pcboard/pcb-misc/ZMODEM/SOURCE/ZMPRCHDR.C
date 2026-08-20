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

#include <dos.h>
#include <io.h>
#include <pcbtools.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <misc.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

extern  bool    BatchTransfer;
extern  char    UploadPath[MAX_FILENAME];

typedef union {
    struct ftime    fTime;
    long            uTime;
} TimeJunk;
TimeJunk        tj;

static int ThisFile = 0;

/********************************************************************
*
*  Function:  dosreservedword()
*
*  Desc    :  Checks to see if the parameter passed has one of the DOS reserved
*             words such as CLOCK$ or COM1 or PRN in the filename.  Use of
*             these words can cause a machine to lock up.
*
*  Returns :  TRUE if a reserved word is found, otherwise FALSE.
*/

bool pascal dosreservedword(char *Name) {
   return(strstr(Name,"CLOCK$") != NULL ||
          (memcmp(Name,"COM",3) == 0 && (Name[3] >= '1' && Name[3] <= '4') && (Name[4] == ':' || Name[4] == 0)) ||
          (memcmp(Name,"LPT",3) == 0 && (Name[3] >= '1' && Name[3] <= '3') && (Name[4] == ':' || Name[4] == 0)) ||
          strcmp(Name,"AUX") == 0 ||
          strcmp(Name,"PRN") == 0 ||
          strcmp(Name,"CON") == 0);
}


/***************************************************************************
 ***************************************************************************/
int pascal zmProcessHeader(char *Name) {
    int             OpenMode = OPEN_DENYWRIT;
    int             DummyShort;
    long            DummyLong;
    char           *p;
    struct stat     f;
    struct date     d;
    struct time     t;
    char            Str[80];

    OpenMode |= OPEN_WRIT;
    if (zConv == ZCBIN && lzConversion != ZCRESUM)
        lzConversion = zConv;
    if (lzConversion)
        zConv = lzConversion;
    if (lzManagement)
        zManag = lzManagement;
    if (zConv == ZCNL)
        BinaryFile = TRUE;
    else if (zManag == ZMAPND)
        OpenMode |= OPEN_APPEND;

    FileSize = DEFBYTL;
    FileMode = 0;
    ModTime = 0L;

    p = Name + 1 + strlen(Name);
    if (BatchTransfer == TRUE) {
        sprintf(FileName, "%s\\%s", UploadPath, Name);
    } else {
        strcpy(FileName, UploadPath);
    }
    strupr(FileName);

    if (*p) {
        sscanf(p, "%ld%lo%o%lo%d%ld%d%d",
                    &ByteCount, &ModTime, &FileMode, &DummyLong, &FilesLeft,
                    &TotalLeft, &DummyShort, &DummyShort);
        if (FilesLeft)
            if (TotalFiles == 0)
                TotalFiles = FilesLeft;
        if (FileMode & UNIXFILE)
            ++BinaryFile;
        FileSize = ByteCount;
        ModTime += 18000;
        unixtodos(ModTime, &d, &t);
        tj.fTime.ft_year = d.da_year - 60;
        tj.fTime.ft_month = d.da_mon;
        tj.fTime.ft_day = d.da_day;
        tj.fTime.ft_hour = t.ti_hour;
        tj.fTime.ft_min = t.ti_min;
        tj.fTime.ft_tsec = t.ti_sec / 2;
/****/
        zmDisplay(15, 3, ZM_DISPLAY, "%-12s", strupr(Name));
        if (FilesLeft)
            zmDisplay(68, 3, ZM_DISPLAY, "%2d of %2d", (int)++ThisFile, (int)TotalFiles);
        else
            zmDisplay(68, 3, ZM_DISPLAY, "  (N/A) ");
        zmDisplay(69, 6, ZM_DISPLAY, "%7ld", ByteCount);
        zmDisplay(69, 7, ZM_DISPLAY, "      0");
/****
        zmDisplay(0, 24, ZM_DISPLAY, "%02d-%02d-%02d %02d:%02d:%02d",
                    tj.fTime.ft_month, tj.fTime.ft_day, tj.fTime.ft_year,
                    tj.fTime.ft_hour, tj.fTime.ft_min, tj.fTime.ft_tsec);
 ****/
/****
        zmDisplay(20, 9, ZM_DISPLAY, "Incoming: %s %ld %lo %o", Name, ByteCount, ModTime,
                        FileMode);
 ****/
    } else {
        for (p = Name; *p; ++p)
            if (*p == '/')
                *p = '_';
        if (*--p == '.')
            *p = 0;
    }

/****
    strcpy(PathName, Name);
    zmCheckPath(Name);
 ****/

    if (*FileName) {
      // remove invalid characters from the filename
      stripall(FileName,' ');
      stripall(FileName,'<');
      stripall(FileName,'>');
      stripall(FileName,'[');
      stripall(FileName,']');
      stripall(FileName,'|');
      stripall(FileName,'+');
      stripall(FileName,'=');
      stripall(FileName,'?');
      stripall(FileName,';');
      stripall(FileName,',');
      stripall(FileName,'"');
    }


    if (*FileName && ! dosreservedword(FileName) && stat(FileName, &f) != -1) {
        zManag &= ZMMASK;
        if (BinaryFile && zConv == ZCRESUM) {
            rxBytes = f.st_size & ~0x01FF;
            if (ByteCount < rxBytes) {
                rxBytes = 0;
                goto OpenIt;
            } else {
                if (fileexist(FileName) != 255) {
                    zmWriteLog(ZM_RECV, ZM_FAIL);
                    return ZFERR;
                }
                if ((dosfopen(FileName, OpenMode, &in)) == -1) {
                    zmWriteLog(ZM_RECV, ZM_FAIL);
                    return ZFERR;
                }
                setftime(in.handle, &tj.fTime);
                dossetbuf(&in,(PcbData.UploadBufSize >= 2 && PcbData.UploadBufSize <= 16 ? PcbData.UploadBufSize * 1024 : 4096));
                dosfseek(&in, rxBytes, SEEK_SET);
            }
            return 0;
        } else if ((zManag == ZMNEW) ||
                ((zManag == ZMNEWL) && ByteCount <= f.st_size)) {  //lint !e574 ByteCount vs f.st_size comparison is okay
            if ((f.st_mtime + 1) >= ModTime)
                goto SkipFile;
            goto OpenIt;
        }
    } else if (zManag & ZMSKNOLOC) {
SkipFile:
        return ZSKIP;
    }
OpenIt:

    // Don't bother scanning the d/l paths if it is NOT a batch transfer
    // because, on forced-single-file transfers, PCBoard will have already
    // scanned the paths to see if the file already exists.  We really only
    // need to worry about un-announced uploads.

    if (BatchTransfer) {
      /* does the file exist in the upload path specified on the command line? */
      if (fileexist(FileName) != 255) {
          sprintf(Str,"(ZM) %s is a duplicate",FileName);
          writelog(Str,SPACERIGHT);
          zmError(0,"Duplicate - Skipping File");
// NOTE:  Do *not* write the filename to the log, otherwise it will cause
//        PCBoard to detect the failed upload and it will then DELETE the
//        *existing* file.  In other words, the very reason for skipping this
//        file is to avoid losing it because it already exists.  But if we
//        write an error to the log, the PCBoard will DELETE it! Don't do that!
//        zmWriteLog(ZM_RECV, ZM_FAIL);
          return ZSKIP;
      }

      /* does the file exist anywhere in the download paths? */
      if (ScanDlPath && scandlpaths(FileName)) {
          sprintf(Str,"(ZM) %s is a duplicate",FileName);
          writelog(Str,SPACERIGHT);
          zmError(0,"Duplicate - Skipping File");
// NOTE:  See reason given above for not writing to the log
//        zmWriteLog(ZM_RECV, ZM_FAIL);
          return ZSKIP;
      }
    }

    if ((dosfopen(FileName, OpenMode, &in)) == -1) {
        sprintf(Str,"(ZM) Cannot open %s",FileName);
        writelog(Str,SPACERIGHT);
        zmError(0,"Cannot Open - Skipping File");
        zmWriteLog(ZM_RECV, ZM_FAIL);
        return ZFERR;
    }
    setftime(in.handle, &tj.fTime);
    dossetbuf(&in,(PcbData.UploadBufSize >= 2 && PcbData.UploadBufSize <= 16 ? PcbData.UploadBufSize * 1024 : 4096));
    return 0;
}
