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


#ifndef ZMODEM_H
# define        ZMODEM_H

# define        ZPAD            '*'
# define        ZDLE            0x18
# define        ZDLEE           (ZDLE ^ 0x40)
# define        ZBIN            'A'
# define        ZHEX            'B'
# define        ZBIN32          'C'
# define        ZBINR32         'D'
# define        ZVBIN           'a'
# define        ZVHEX           'b'
# define        ZVBIN32         'c'
# define        ZVBINR32        'd'
# define        ZRESC           0x7E
# define        ZMAXHLEN        16
# define        ZMAXSPLEN       1024

# define        ZRQINIT         0
# define        ZRINIT          1
# define        ZSINIT          2
# define        ZACK            3
# define        ZFILE           4
# define        ZSKIP           5
# define        ZNAK            6
# define        ZABORT          7
# define        ZFIN            8
# define        ZRPOS           9
# define        ZDATA           10
# define        ZEOF            11
# define        ZFERR           12
# define        ZCRC            13
# define        ZCHALLENGE      14
# define        ZCOMPL          15
# define        ZCAN            16
# define        ZFREECNT        17
# define        ZCOMMAND        18
# define        ZSTDERR         19

# define        ZCRCE           'h'
# define        ZCRCG           'i'
# define        ZCRCQ           'j'
# define        ZCRCW           'k'
# define        ZRUB0           'l'
# define        ZRUB1           'm'

# define        GOTOR           0x0100
# define        GOTCRCE         (ZCRCE | GOTOR)
# define        GOTCRCG         (ZCRCG | GOTOR)
# define        GOTCRCQ         (ZCRCQ | GOTOR)
# define        GOTCRCW         (ZCRCW | GOTOR)
# define        GOTCAN          (GOTOR | 0x18)

# define        ZF0                     3
# define        ZF1                     2
# define        ZF2                     1
# define        ZF3                     0
# define        ZP0                     0
# define        ZP1                     1
# define        ZP2                     2
# define        ZP3                     3

# define        CANFDX          0x01
# define        CANOVIO         0x02
# define        CANBRK          0x04
# define        CANRLE          0x08
# define        CANLZW          0x10
# define        CANFC32         0x20
# define        ESCCTL          0x40
# define        ESC8            0x80

# define        CANVHDR         0x01

# define        ZATTNLEN        32
# define        ALTCOFF         ZF1
# define        TESCCTL         0x40
# define        TESC8           0x80

# define        ZCBIN           0x01
# define        ZCNL            0x02
# define        ZCRESUM         0x03
# define        ZMSKNOLOC       0x80
# define        ZMMASK          0x1F
# define        ZMNEWL          0x01
# define        ZMCRC           0x02
# define        ZMAPND          0x03
# define        ZMCLOB          0x04
# define        ZMNEW           0x05
# define        ZMDIFF          0x06
# define        ZMPROT          0x07
# define        ZMCHNG          0x08

# define        ZTLZW           0x01
# define        ZTRLE           0x03

# define        ZXSPARS         0x40
# define        ZCANVHDR        0x01
# define        ZRWOVR          0x04

# define        ZCACK           0x01

# define        OK                      0
# define        FALSE           0
# define        TRUE            1
# define        ERROR           (-1)

# define        ENQ                     0x05
# define        CAN                     ('X' & 0x1F)
# define        XOFF            ('s' & 0x1F)
# define        XON                     ('q' & 0x1F)
# define        SOH                     0x01
# define        STX                     0x02
# define        EOT                     0x04
# define        ACK                     0x06
# define        NAK                     0x15
# define        CPMEOF          0x1A
# define        WANTCRC         0x43
# define        WANTG           0x47
# define        TIMEOUT         (-2)
# define        RCDO            (-3)
# define        GCOUNT          (-4)
# define        CDLOST          (-5)
# define        LABORT          (-6)
# define        RETRYMAX        10

# define        DEFBYTL                         2000000000L
# define        DSZLOG                          "DSZLOG"
# define        HOWMANY                         2
# define        TIMEOUT_COUNTER                 7
# define        CPS_COUNTER                     9
# define        MAX_FILENAME                    65
# define        PATHLEN                         128
# define        UNIXFILE                        0xF000
# define        ZM_FAIL                         0
# define        ZM_SUCCESS                      1
# define        ZM_SEND                         0
# define        ZM_RECV                         1
# define        ZM_ERRORX                       15
# define        ZM_ERRORY                       4
# define        ZM_GRAPHX                       6
# define        ZM_GRAPHY                       7
# define        ZM_GRAPH                        0x1E
# define        ZM_DISPLAY                      0x1E
# define        ZFTOFFSET                       4
# define        FRAMETYPES                      22
# define        TXBSIZE                         16384

#  define       zmXPutByte(Byte)        zmSpoolByte(Byte & 0xFF)
#  define       zmPutByte(Byte)         zmSpoolByte(Byte)

/*** External Variable Definitions *****************************************/
extern  char                    *FrameTypes[];
extern  char                    Attn[];
extern  char                    FileName[MAX_FILENAME];
extern  char                    rxHeader[ZMAXHLEN];             /* Receive header */
extern  char                    SecBuf[8193];
extern  char                    txBuffer[TXBSIZE];
extern  char                    txHeader[ZMAXHLEN];             /* Transmit header */
extern  char                    LineBuffer[HOWMANY];
extern  char                    zConv;
extern  char                    zManag;
extern  char                    zTrans;
extern  short                   nBytes;                                 /* number of characters in LineBuffer */
extern  short                   rx32crc;
extern  short                   tx32crc;
extern  int                     lzConversion;
extern  int                     lzManagement;
extern  int                     lzTransport;
extern  unsigned                rxBufLength;
extern  int                     rxCount;
extern  int                     rxFlags;
extern  int                     rxFrameID;
extern  int                     rxHeaderLength;
extern  int                     rxTimeout;
extern  int                     rxType;
extern  int                     tx32fcs;
extern  unsigned                txFrameLength;
extern  int                     zmHeaderType;
extern  int                     BeenHere;
extern  int                     BinaryFile;
extern  unsigned                BlockLength;
extern  int                     BlockOpt;
extern  int                     Command;
extern  int                     ControlEncode;
extern  int                     EndOfFile;
extern  int                     ErrorCount;
extern  int                     Errors;
extern  int                     Warnings;
extern  int                     ExitCode;
extern  int                     FileMode;
extern  int                     LastSent;
extern  int                     Not8Bit;
extern  int                     VariableLengthHeaders;
extern  int                     Verbose;
extern  int                     want32fcs;
extern  int                     ZNulls;
extern  int                     CanSeek;
extern  int                     FilesLeft;
extern  int                     TotalFiles;
extern  int                     ZModem;
extern  unsigned                txWindow;
extern  unsigned                txWSpace;
extern  unsigned                txWCount;
extern  unsigned                EffectiveBaud;
extern  unsigned long           lrxPosition;
extern  unsigned long           rxPosition;
extern  unsigned long           txPosition;
extern  unsigned long           rxBytes;
extern  unsigned long           ByteCount;
extern  unsigned long           LastByteCount;
extern  unsigned long           FileSize;
extern  long                    LastSync;
extern  long                    ModTime;
extern  long                    vPos;
extern  long                    TotalLeft;
extern  long                    CPS;
extern  DOSFILE                 in;
extern  DOSFILE                 out;
extern  int                     SpoolBytes;
extern  char                    Scrn_Mode;
extern  int                     zmAbort;
extern  bool                    ScanDlPath;

#ifdef DEBUGMODE
extern int  DebugMode;
#endif

/*** External Function Prototypes ******************************************/
// # ifdef __cplusplus
// extern  "C"     {
// # endif

extern  void    _Cdecl  zmCancelTransfer(void);
extern  void    _Cdecl  zmGoodBye(void);
extern  void    _Cdecl  zmGoodByeACK(void);
extern  void    pascal  zmPutHex(register int);
extern  void    pascal  zmPutStr(char *);
extern  void    pascal  zmSendBinaryHeader(int, int, char *);
extern  void    _Cdecl  zmSendBinaryHeader32(int, char *, int, int);
extern  void    pascal  zmSendData(register char *, int, int);
extern  void    _Cdecl  zmSendData16(register char *, int, int);
extern  void    pascal  zmSendData32(register char *, int, int);
extern  void    pascal  zmSendData32RLE(char *, int, int);
extern  void    pascal  zmSendHexHeader(int, int, char *);
extern  void    pascal  zmStoreHeader(long);
extern  void    pascal  zmZPutByte(int);
extern  int     pascal  noXReadByte(void);
extern  int     _Cdecl  zmCountFiles(int, char **);
extern  void    _Cdecl  zmDisplay(int, int, int, char *, ...);
extern  void    _Cdecl  zmError(int, char *, ...);
extern  int     pascal  zmGetByte(int);
extern  int     pascal  zmGetHeader(char *);
extern  int     _Cdecl  zmGetHex(void);
extern  int     pascal  zmGetInSync(int);
extern  int     pascal  zmGetReceiverInit(void);
extern  void    pascal  zmGraph(unsigned long);
extern  int     pascal  zmProcessHeader(char *);
extern  int     pascal  zmPutSector(char *, int);
extern  int     _Cdecl  zmReadByte(void);
extern  int     pascal  zmReceiveBinaryHeader(char *);
extern  int     pascal  zmReceiveBinaryHeader32(char *);
extern  int     pascal  zmReceiveData(char *, int);
extern  int     pascal  zmReceiveData16(char *, int);
extern  int     pascal  zmReceiveData32(char *, int);
extern  int     pascal  zmReceiveData32RLE(char *, int);
extern  int     pascal  zmReceiveFile(void);
extern  int     _Cdecl  zmReceiveFiles(void);
extern  int     pascal  zmReceiveHexHeader(char *);
extern  int     pascal  zmSendFileData(void);
extern  int     pascal  zmSendFileName(char *);
extern  int     _Cdecl  zmSendInit(void);
extern  int     pascal  zmSendStartUp(void);
extern  int     _Cdecl  zmTry(void);
extern  void    pascal  zmWriteLog(int, int);
extern  long    _Cdecl  zmRecoverHeader(register char *);

void pascal zmFlushSpooler(void);
void pascal zmSpoolByte(char Byte);
void pascal zmSpoolStr(char *Str, int StrLen);

extern pcbconftype     CurConf;
extern bool            UseConfInfo;

void pascal showfileinusernet(char Mode, char *Name, long FileSize);
int pascal openusernet(void);
void pascal closeusernet(void);

bool pascal scandlpaths(char *FileName);

//# ifdef __cplusplus
//}
//# endif

#endif
