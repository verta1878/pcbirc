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


#include <pcbtools.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

char            Attn[] = { 0 };
char            FileName[MAX_FILENAME] = "";
char            rxHeader[ZMAXHLEN];     /* Receive header */
char            SecBuf[8193];
char            txBuffer[TXBSIZE];
char            txHeader[ZMAXHLEN];     /* Transmit header */
char            zConv;
char            zManag;
char            zTrans;
short           nBytes = 0;         /* number of characters in LineBuffer */
short           rx32crc;
short           tx32crc;
int             lzConversion /** = ZCRESUM **/ ;
int             lzManagement /** = ZMCRC   **/ ;
int             lzTransport;
unsigned        rxBufLength = 4096;
int             rxCount;
int             rxFlags;
int             rxFrameID;
int             rxHeaderLength;
int             rxTimeout = 60;   // 60 second timeout
int             rxType;
int             tx32fcs;
unsigned        txFrameLength = 0;
int             zmAbort = FALSE;
int             zmHeaderType;
int             zmSkipFile = FALSE;
int             BeenHere;
int             BinaryFile;
unsigned        BlockLength = 128;
int             BlockOpt = 0;
int             Command;
int             ControlEncode;
int             EndOfFile;
int             ErrorCount = 0;
int             Errors = 0;
int             Warnings = 0;
int             ExitCode;
int             FileMode;
int             LastSent;
int             Not8Bit;
int             VariableLengthHeaders;
int             Verbose = 0;
int             want32fcs = TRUE;
int             ZNulls;
int             CanSeek = 0;
int             FilesLeft;
int             TotalFiles;
int             ZModem = 0;
unsigned        txWindow;
unsigned        txWSpace;
unsigned        txWCount;
unsigned        EffectiveBaud;
unsigned long   lrxPosition = 0L;
unsigned long   rxPosition = 0L;
unsigned long   txPosition = 0L;
unsigned long   rxBytes;
unsigned long   ByteCount;
unsigned long   LastByteCount = 0xFFFFFFFFL;
long            CPS;
long            ModTime;
unsigned long   FileSize;
long            LastSync = 0;
long            vPos = 0;
long            TotalLeft;
DOSFILE         in;
DOSFILE         out;

char    *FrameTypes[] = {
    "No Response to Error Correction Request",
    "No Carrier Detect",
    "TIMEOUT",
    "ERROR",
    "ZRQINIT",
    "ZRINIT",
    "ZSINIT",
    "ZACK",
    "ZFILE",
    "ZSKIP",
    "ZNAK",
    "ZABORT",
    "ZFIN",
    "ZRPOS",
    "ZDATA",
    "ZEOF",
    "ZFERR",
    "ZCRC",
    "ZCHALLENGE",
    "ZCOMPL",
    "ZCAN",
    "ZFREECNT",
    "ZCOMMAND",
    "ZSTDERR",
    "xxxxxx"
};


# define    ZFTOFFSET   4
# define    FRAMETYPES  22

int     DebugMode = 0;
