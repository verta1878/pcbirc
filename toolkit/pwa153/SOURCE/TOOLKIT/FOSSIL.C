/* ============================================================================
 * fossil.c — Standalone FOSSIL driver object for the PCBoard Toolkit
 *
 * Produces FOSSIL.OBJ when compiled with BC 3.1 large model:
 *   bcc -ml -c -oFOSSIL.OBJ fossil.c
 *
 * Clean-room reconstruction matching the OMF symbol table of Clark's
 * original FOSSIL.OBJ (9,157 bytes, Oct 11 1993) from TOOLKIT2.ZIP.
 *
 * Original OMF source path: y:\modem.c
 * Original build: MODEM.C + MODEMFOS.C + MODEMASY.C compiled as one
 *                 unit with -DCOMM -DMULTIPORT -DLIB -DFOSSIL
 *
 * 79 exports: 30 FOSSIL_* + 11 ASYNC_* + 30 _vtable + 8 helpers
 * 48 imports: 23 ASYNC_* (from ASYNC.ASM) + PCBoard + runtime
 *
 * pcbirc crew (hexadecimal + sysop/0), GPLv3.
 * ==========================================================================*/

/* ---- Minimal type stubs (replaces project.h / model.h / TYPES.HPP) ---- */

#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloc.h>

typedef unsigned char bool;
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef LIBENTRY
#define LIBENTRY pascal
#endif

#ifndef _NEAR_
#define _NEAR_ near
#endif

#define fbmalloc  farmalloc
#define fbfree    farfree
#define bmalloc   malloc
#define bfree     free

typedef int showtype;
#define SHOW 1
#define HIDE 0

/* ---- InBytes/OutBytes macros (from PCBOARD.H under MULTIPORT) ---------- */
#define InBytes   inbytes()
#define OutBytes  outbytes()

/* ---- External: ASYNC.ASM (23 functions, linked separately) ------------- */

extern void LIBENTRY ASYNC_init(int irq, int base, char far *inbuf,
             char far *outbuf, int insize, int outsize, int cts, int share);
extern int  LIBENTRY ASYNC_opencom(int bauddiv, int databits);
extern void LIBENTRY ASYNC_closecom(void);
extern int  LIBENTRY ASYNC_comminkey(void);
extern int  LIBENTRY ASYNC_cgetbuf(char *buf, int len);
extern int  LIBENTRY ASYNC_cgetstr(char *str, int len);
extern int  LIBENTRY ASYNC_checkcomm(void);
extern void LIBENTRY ASYNC_csendbyte(unsigned char b);
extern void LIBENTRY ASYNC_csendstr(char *str, int len);
extern void LIBENTRY ASYNC_turnonfifo(int trigger);
extern void LIBENTRY ASYNC_turnonxmit(void);
extern void LIBENTRY ASYNC_turnondtr(void);
extern void LIBENTRY ASYNC_turnoffdtr(void);
extern void LIBENTRY ASYNC_turnonrts(void);
extern void LIBENTRY ASYNC_turnoffrts(void);
extern void LIBENTRY ASYNC_clearoutbuf(void);
extern void LIBENTRY ASYNC_clearinbuf(void);
extern void LIBENTRY ASYNC_commgo(void);
extern void LIBENTRY ASYNC_commstop(void);
extern void LIBENTRY ASYNC_commpause(void);
extern int  LIBENTRY ASYNC_online(void);
extern int  LIBENTRY ASYNC_cdstillup(void);
extern void LIBENTRY ASYNC_setport(int bauddiv, int databits);

/* ---- External: ASYNC.ASM data exports --------------------------------- */

extern int  _InBytes;
extern int  _OutBytes;
extern int  _FramingErrors;
extern int  _OverrunErrors;
extern int  _ParityErrors;
extern int  _RingDetect;
extern int  _CTSokay;
extern int  _CDokay;

/* ---- External: PCBoard internals (resolved by calling program) --------- */

extern void LIBENTRY errorexittodos(char *msg);
extern void LIBENTRY giveup(void);
extern long LIBENTRY gettimer(void);
extern void LIBENTRY settimer(int id, long ticks);
extern int  LIBENTRY timerexpired(int id);
extern void LIBENTRY tickdelay(long ticks);
extern void LIBENTRY writelog(char *msg, int flags);
extern void LIBENTRY loguseroff(int reason);
extern void LIBENTRY watchsystemfunctions(void);

/* ---- External: PCBoard globals ----------------------------------------- */

extern int  VerifyCDLoss;

/* ---- Timing constants -------------------------------------------------- */

#define TICKSPERSECOND   18L
#define QUARTERSECOND    5L
#define HALFSECOND       9L
#define ONESECOND        18L
#define THREESECONDS     55L
#define TENSECONDS      182L
#define THIRTYSECONDS   546L
#define SIXTYSECONDS   1092L
#define SPACERIGHT        0
#define ALOGOFF           0

/* ---- PCBoard structures (subset used under LIB) ------------------------ */

struct {
    int  ComPortNumber;
    long ModemSpeed;
    int  DataBits;
    int  HstMode;
    int  Online;
    int  LostCarrier;
    int  IgnoreCDLoss;
    long CarrierSpeed;
} Asy;

struct {
    int  DisableCTS;
    int  Packet;
    int  ModemDelay;
    char ModemPort[20];
    char ModemOff[80];
    char ModemInit[80];
    char ModemInit2[80];
    int  ShareIRQs;
    int  IrqNum;
    int  BaseAddress;
    int  OS2Driver;
    int  RequirePwrdToExit;
    int  OffHook;
    int  ResetModem;
    int  ExitToDos;
} PcbData;

#define EXIT_RECYCLE 3

/* ---- Exported globals -------------------------------------------------- */

bool ModemOpened = FALSE;
bool ModemOffHook;
bool ModemFixupsDone = FALSE;

int OutBufSize;

/* ---- Forward declarations (ASYNC stubs + FOSSIL functions) ------------- */

int  LIBENTRY ASYNC_ringdetect(void);
int  LIBENTRY ASYNC_ctsokay(void);
int  LIBENTRY ASYNC_framingerrors(void);
int  LIBENTRY ASYNC_overrunerrors(void);
int  LIBENTRY ASYNC_parityerrors(void);
int  LIBENTRY ASYNC_outbytes(void);
int  LIBENTRY ASYNC_inbytes(void);
int  LIBENTRY ASYNC_bauddivisor(long PortSpeed);
void LIBENTRY ASYNC_disconnectmodem(void);
void LIBENTRY ASYNC_reopenport(void);
void LIBENTRY ASYNC_openmodem(showtype Show);
static void near ASYNC_dofixups(void);

int  LIBENTRY FOSSIL_ringdetect(void);
int  LIBENTRY FOSSIL_ctsokay(void);
int  LIBENTRY FOSSIL_online(void);
int  LIBENTRY FOSSIL_cdstillup(void);
int  LIBENTRY FOSSIL_bauddivisor(long PortSpeed);
void LIBENTRY FOSSIL_setport(int BaudDivisor, int DataBits);
int  LIBENTRY FOSSIL_inbytes(void);
int  LIBENTRY FOSSIL_outbytes(void);
int  LIBENTRY FOSSIL_framingerrors(void);
int  LIBENTRY FOSSIL_overrunerrors(void);
int  LIBENTRY FOSSIL_parityerrors(void);
void LIBENTRY FOSSIL_turnoffdtr(void);
void LIBENTRY FOSSIL_turnondtr(void);
void LIBENTRY FOSSIL_turnoffrts(void);
void LIBENTRY FOSSIL_turnonrts(void);
void LIBENTRY FOSSIL_turnonxmit(void);
void LIBENTRY FOSSIL_clearoutbuf(void);
void LIBENTRY FOSSIL_clearinbuf(void);
void LIBENTRY FOSSIL_commgo(void);
void LIBENTRY FOSSIL_commstop(void);
void LIBENTRY FOSSIL_commpause(void);
int  LIBENTRY FOSSIL_checkcomm(void);
int  LIBENTRY FOSSIL_comminkey(void);
int  LIBENTRY FOSSIL_cgetstr(char *pStr, int StrLen);
int  LIBENTRY FOSSIL_cgetbuf(char *Buf, int BufLen);
void LIBENTRY FOSSIL_csendbyte(unsigned char ByteToSend);
void LIBENTRY FOSSIL_csendstr(char *pStr, int StrLen);
void LIBENTRY FOSSIL_disconnectmodem(void);
void LIBENTRY FOSSIL_openmodem(showtype Show);
void LIBENTRY FOSSIL_reopenport(void);
static void near FOSSIL_dofixups(void);

/* ========================================================================
 * Function pointer vtable (from MODEMASY.C)
 * Exports as _lowercase C-linkage globals.
 * Initialized to ASYNC defaults; FOSSIL_dofixups overwrites them.
 * ======================================================================== */

int  (LIBENTRY *ringdetect)(void)                      = ASYNC_ringdetect;
int  (LIBENTRY *ctsokay)(void)                         = ASYNC_ctsokay;
int  (LIBENTRY *online)(void)                          = ASYNC_online;
int  (LIBENTRY *cdstillup)(void)                       = ASYNC_cdstillup;
int  (LIBENTRY *bauddivisor)(long PortSpeed)           = ASYNC_bauddivisor;
void (LIBENTRY *setport)(int BaudDivisor, int DataBits)= ASYNC_setport;
int  (LIBENTRY *inbytes)(void)                         = ASYNC_inbytes;
int  (LIBENTRY *outbytes)(void)                        = ASYNC_outbytes;
int  (LIBENTRY *framingerrors)(void)                   = ASYNC_framingerrors;
int  (LIBENTRY *overrunerrors)(void)                   = ASYNC_overrunerrors;
int  (LIBENTRY *parityerrors)(void)                    = ASYNC_parityerrors;
void (LIBENTRY *turnoffdtr)(void)                      = ASYNC_turnoffdtr;
void (LIBENTRY *turnondtr)(void)                       = ASYNC_turnondtr;
void (LIBENTRY *turnoffrts)(void)                      = ASYNC_turnoffrts;
void (LIBENTRY *turnonrts)(void)                       = ASYNC_turnonrts;
void (LIBENTRY *turnonxmit)(void)                      = ASYNC_turnonxmit;
void (LIBENTRY *clearoutbuf)(void)                     = ASYNC_clearoutbuf;
void (LIBENTRY *clearinbuf)(void)                      = ASYNC_clearinbuf;
void (LIBENTRY *commgo)(void)                          = ASYNC_commgo;
void (LIBENTRY *commstop)(void)                        = ASYNC_commstop;
void (LIBENTRY *commpause)(void)                       = ASYNC_commpause;
int  (LIBENTRY *checkcomm)(void)                       = ASYNC_checkcomm;
int  (LIBENTRY *comminkey)(void)                       = ASYNC_comminkey;
int  (LIBENTRY *cgetstr)(char *pStr, int StrLen)       = ASYNC_cgetstr;
int  (LIBENTRY *cgetbuf)(char *Buf, int BufLen)        = ASYNC_cgetbuf;
void (LIBENTRY *csendbyte)(unsigned char ByteToSend)   = ASYNC_csendbyte;
void (LIBENTRY *disconnectmodem)(void)                 = ASYNC_disconnectmodem;
void (LIBENTRY *csendstr)(char *pStr, int StrLen)      = ASYNC_csendstr;
void (LIBENTRY *reopenport)(void)                      = ASYNC_reopenport;

/* ========================================================================
 * ASYNC stubs (from MODEMASY.C — read ASYNC.ASM data exports)
 * ======================================================================== */

int  LIBENTRY ASYNC_ringdetect(void)   { return(_RingDetect); }
int  LIBENTRY ASYNC_ctsokay(void)      { return(_CTSokay); }
int  LIBENTRY ASYNC_framingerrors(void) { return(_FramingErrors); }
int  LIBENTRY ASYNC_overrunerrors(void) { return(_OverrunErrors); }
int  LIBENTRY ASYNC_parityerrors(void)  { return(_ParityErrors); }

int  LIBENTRY ASYNC_outbytes(void) {
    int n; disable(); n = _OutBytes; enable(); return(n);
}

int  LIBENTRY ASYNC_inbytes(void) {
    int n; disable(); n = _InBytes; enable(); return(n);
}

int  LIBENTRY ASYNC_bauddivisor(long PortSpeed) {
    int CPS = (int)(PortSpeed / 10);
    switch (CPS) {
        case    30: return(0x0180);  case   120: return(0x0060);
        case   240: return(0x0030);  case   480: return(0x0018);
        case   960: return(0x000C);  case  1920: return(0x0006);
        case  3840: return(0x0003);  case  5760: return(0x0002);
        case 11520: return(0x0001);  default:    return(0x0060);
    }
}

void LIBENTRY ASYNC_disconnectmodem(void) {
    ASYNC_closecom();
    ModemOpened = FALSE;
}

void LIBENTRY ASYNC_reopenport(void) { ASYNC_openmodem(HIDE); }
void LIBENTRY ASYNC_openmodem(showtype Show) { /* stub under LIB */ }

static void near ASYNC_dofixups(void) {
    ringdetect = ASYNC_ringdetect;  ctsokay = ASYNC_ctsokay;
    online = ASYNC_online;          cdstillup = ASYNC_cdstillup;
    bauddivisor = ASYNC_bauddivisor; setport = ASYNC_setport;
    inbytes = ASYNC_inbytes;        outbytes = ASYNC_outbytes;
    framingerrors = ASYNC_framingerrors;
    overrunerrors = ASYNC_overrunerrors;
    parityerrors = ASYNC_parityerrors;
    turnoffdtr = ASYNC_turnoffdtr;  turnondtr = ASYNC_turnondtr;
    turnoffrts = ASYNC_turnoffrts;  turnonrts = ASYNC_turnonrts;
    turnonxmit = ASYNC_turnonxmit;
    clearoutbuf = ASYNC_clearoutbuf; clearinbuf = ASYNC_clearinbuf;
    commgo = ASYNC_commgo;          commstop = ASYNC_commstop;
    commpause = ASYNC_commpause;    checkcomm = ASYNC_checkcomm;
    comminkey = ASYNC_comminkey;    cgetstr = ASYNC_cgetstr;
    cgetbuf = ASYNC_cgetbuf;        csendbyte = ASYNC_csendbyte;
    csendstr = ASYNC_csendstr;
    disconnectmodem = ASYNC_disconnectmodem;
    reopenport = ASYNC_reopenport;
}

/* ========================================================================
 * FOSSIL backend (from MODEMFOS.C — pure INT 14h)
 * ======================================================================== */

#define FOSINBUFSIZE 1024

static char *FosBuffer = NULL;
static int   FosInBytes;
static int   FosHeadPtr;
static int   FosTailPtr;

typedef struct {
    int  StructSize;
    char FossilVersion;
    char DriverLevel;
    char far *ID;
    int  InBufSize;
    int  BufInBytes;
    int  OutBufSize;
    int  BufOutBytes;
    char ScreenW;
    char ScreenH;
    char BaudRateMask;
} fossilstruct;

static int Port;
fossilstruct Fossil;  /* NOT static — exports as _Fossil */

int  LIBENTRY FOSSIL_ringdetect(void) { return(FALSE); }
int  LIBENTRY FOSSIL_ctsokay(void)    { return(TRUE); }

int  LIBENTRY FOSSIL_online(void) {
    _AH = 3; _DX = Port;
    geninterrupt(0x14);
    _AX &= 0x80;
    _CDokay = _AL;
    return(_AX);
}

int  LIBENTRY FOSSIL_cdstillup(void) {
    int X;
    if (VerifyCDLoss) {
        for (X = 0; X < 15; X++) {
            if (online()) return(TRUE);
            settimer(4, 3);
            while (!timerexpired(4)) giveup();
        }
        goto cdoff;
    }
    if (online()) return(TRUE);
cdoff:
    clearoutbuf();
    return(FALSE);
}

int  LIBENTRY FOSSIL_bauddivisor(long PortSpeed) {
    int CPS = (int)(PortSpeed / 10);
    switch (CPS) {
        case    30: return(0x02);  case   120: return(0x04);
        case   240: return(0x05);  case   480: return(0x06);
        case   960: return(0x07);  case  1920: return(0x00);
        case  3840: return(0x01);  default:    return(0x01);
    }
}

void LIBENTRY FOSSIL_setport(int BaudDivisor, int DataBits) {
    bool N81 = (DataBits == 8);
    BaudDivisor <<= 5;
    BaudDivisor += (N81 ? 3 : 2);
    BaudDivisor += (N81 ? 0 : 0x18);
    _AX = BaudDivisor; _AH = 0; _DX = Port;
    geninterrupt(0x14);
}

static int near LIBENTRY FOSSIL_bytesinbuffer(void) {
    _CX = sizeof(Fossil);
    _ES = FP_SEG((void far *)&Fossil);
    _DI = FP_OFF((void far *)&Fossil);
    _DX = Port; _AH = 0x1b;
    geninterrupt(0x14);
    return(Fossil.InBufSize - Fossil.BufInBytes);
}

static int near LIBENTRY FOSSIL_readin(char *Buf, int BufLen) {
    int NumBytes = FOSSIL_bytesinbuffer();
    if (NumBytes > BufLen) NumBytes = BufLen;
    if (NumBytes != 0) {
        if (NumBytes == 1) {
            _AH = 2; _DX = Port;
            geninterrupt(0x14);
            Buf[0] = _AL;
        } else {
            _ES = FP_SEG((void far *)Buf);
            _DI = FP_OFF((void far *)Buf);
            _AH = 0x18; _CX = NumBytes; _DX = Port;
            geninterrupt(0x14);
        }
    }
    return(NumBytes);
}

static void near LIBENTRY FOSSIL_readport(void) {
    int FreeBytes, BytesRead, BytesToBufEnd;
    char *pTempBuf, TempBuf[FOSINBUFSIZE];

    if ((FreeBytes = FOSINBUFSIZE - FosInBytes) > 0) {
        pTempBuf = TempBuf;
        if ((BytesRead = FOSSIL_readin(pTempBuf, FreeBytes)) > 0) {
            if (FosInBytes == 0) FosHeadPtr = FosTailPtr = 0;
            BytesToBufEnd = FOSINBUFSIZE - FosHeadPtr;
            if (BytesRead > BytesToBufEnd) {
                memcpy(FosBuffer + FosHeadPtr, pTempBuf, BytesToBufEnd);
                FosHeadPtr = 0;
                BytesRead -= BytesToBufEnd;
                pTempBuf += BytesToBufEnd;
                FosInBytes += BytesToBufEnd;
            }
            memcpy(FosBuffer + FosHeadPtr, pTempBuf, BytesRead);
            FosHeadPtr += BytesRead;
            FosHeadPtr &= (FOSINBUFSIZE - 1);
            FosInBytes += BytesRead;
        }
    }
}

int  LIBENTRY FOSSIL_inbytes(void) {
    if (FosInBytes != 0) return(FosInBytes);
    FOSSIL_readport();
    return(FosInBytes);
}

int  LIBENTRY FOSSIL_outbytes(void) {
    _CX = sizeof(Fossil);
    _ES = FP_SEG((void far *)&Fossil);
    _DI = FP_OFF((void far *)&Fossil);
    _DX = Port; _AH = 0x1b;
    geninterrupt(0x14);
    return(Fossil.OutBufSize - Fossil.BufOutBytes);
}

int  LIBENTRY FOSSIL_framingerrors(void)  { return(0); }
int  LIBENTRY FOSSIL_overrunerrors(void)  { return(0); }
int  LIBENTRY FOSSIL_parityerrors(void)   { return(0); }

void LIBENTRY FOSSIL_turnoffdtr(void) { _AX = 0x0600; _DX = Port; geninterrupt(0x14); }
void LIBENTRY FOSSIL_turnondtr(void)  { _AX = 0x0601; _DX = Port; geninterrupt(0x14); }
void LIBENTRY FOSSIL_turnoffrts(void) { }
void LIBENTRY FOSSIL_turnonrts(void)  { }
void LIBENTRY FOSSIL_turnonxmit(void) { }

void LIBENTRY FOSSIL_clearoutbuf(void) { _AH = 9; _DX = Port; geninterrupt(0x14); }

void LIBENTRY FOSSIL_clearinbuf(void) {
    _AH = 0x0a; _DX = Port; geninterrupt(0x14);
    FosInBytes = FosHeadPtr = FosTailPtr = 0;
}

void LIBENTRY FOSSIL_commgo(void)   { _AX = 0x1000; _DX = Port; geninterrupt(0x14); }
void LIBENTRY FOSSIL_commstop(void) { _AX = 0x1002; _DX = Port; geninterrupt(0x14); }
void LIBENTRY FOSSIL_commpause(void) { }

int  LIBENTRY FOSSIL_checkcomm(void) {
    int NumBytes = InBytes;
    if (NumBytes == 0) return(0);
    _AH = 0x0c; _DX = Port;
    geninterrupt(0x14);
    switch (_AL) { case 11: case 19: case 24: return(_AX); }
    return(0);
}

int  LIBENTRY FOSSIL_comminkey(void) {
    char Byte;
    if (InBytes == 0) return(-1);
    FosInBytes--;
    Byte = (char)FosBuffer[FosTailPtr++];
    FosTailPtr &= (FOSINBUFSIZE - 1);
    return(Byte);
}

int  LIBENTRY FOSSIL_cgetstr(char *pStr, int StrLen) {
    int NumBytesFound, NumBytesToCopy, NumBytesToBufEnd;
    StrLen--;
    NumBytesFound = InBytes;
    if (NumBytesFound > StrLen) NumBytesFound = StrLen;
    if (NumBytesFound != 0) {
        NumBytesToCopy = NumBytesFound;
        NumBytesToBufEnd = FOSINBUFSIZE - FosTailPtr;
        if (NumBytesToCopy > NumBytesToBufEnd) {
            memcpy(pStr, FosBuffer + FosTailPtr, NumBytesToBufEnd);
            pStr += NumBytesToBufEnd;
            FosTailPtr = 0;
            NumBytesToCopy -= NumBytesToBufEnd;
        }
        memcpy(pStr, FosBuffer + FosTailPtr, NumBytesToCopy);
        pStr[NumBytesToCopy] = 0;
        FosTailPtr += NumBytesToCopy;
        FosTailPtr &= (FOSINBUFSIZE - 1);
        FosInBytes -= NumBytesFound;
    }
    return(NumBytesFound);
}

int  LIBENTRY FOSSIL_cgetbuf(char *Buf, int BufLen) {
    int NumBytesFound, NumBytesToCopy, NumBytesToBufEnd;
    NumBytesFound = InBytes;
    if (NumBytesFound > BufLen) NumBytesFound = BufLen;
    if (NumBytesFound != 0) {
        NumBytesToCopy = NumBytesFound;
        NumBytesToBufEnd = FOSINBUFSIZE - FosTailPtr;
        if (NumBytesToCopy > NumBytesToBufEnd) {
            memcpy(Buf, FosBuffer + FosTailPtr, NumBytesToBufEnd);
            Buf += NumBytesToBufEnd;
            FosTailPtr = 0;
            NumBytesToCopy -= NumBytesToBufEnd;
        }
        memcpy(Buf, FosBuffer + FosTailPtr, NumBytesToCopy);
        FosTailPtr += NumBytesToCopy;
        FosTailPtr &= (FOSINBUFSIZE - 1);
        FosInBytes -= NumBytesFound;
    }
    return(NumBytesFound);
}

void LIBENTRY FOSSIL_csendbyte(unsigned char ByteToSend) {
    _AH = 1; _AL = ByteToSend; _DX = Port; geninterrupt(0x14);
}

void LIBENTRY FOSSIL_csendstr(char *pStr, int StrLen) {
    _CX = StrLen;
    _ES = FP_SEG((void far *)pStr);
    _DI = FP_OFF((void far *)pStr);
    _DX = Port; _AH = 0x19;
    geninterrupt(0x14);
}

static int near LIBENTRY FOSSIL_initializedriver(int PortNum) {
    _AH = 4; _DX = PortNum; _BX = 0;
    geninterrupt(0x14);
    _AX -= 0x1954;
    return(_AX);
}

static int near LIBENTRY FOSSIL_getdriverinfo(void) {
    _CX = sizeof(Fossil);
    _ES = FP_SEG((void far *)&Fossil);
    _DI = FP_OFF((void far *)&Fossil);
    _DX = Port; _AH = 0x1b;
    geninterrupt(0x14);
    OutBufSize = Fossil.OutBufSize;
    return(0);
}

void LIBENTRY FOSSIL_disconnectmodem(void) {
    if (FosBuffer != NULL) { bfree(FosBuffer); FosBuffer = NULL; }
    ModemOpened = FALSE;
    FosInBytes = FosHeadPtr = FosTailPtr = 0;
}

void LIBENTRY FOSSIL_openmodem(showtype Show) {
    if (Asy.ComPortNumber == 0 || !ModemFixupsDone) return;

    if ((FosBuffer = (char *)bmalloc(FOSINBUFSIZE)) == NULL) {
        char Str[80];
        sprintf(Str, "insufficient memory for comm buffers: %u : %ld",
                FOSINBUFSIZE, farcoreleft());
        errorexittodos(Str);
        return;
    }

    FosInBytes = FosHeadPtr = FosTailPtr = 0;
    Port = Asy.ComPortNumber - 1;

    if (FOSSIL_initializedriver(Port) != 0) {
        errorexittodos("Invalid comm port - FOSSIL driver not found");
        return;
    }
    if (FOSSIL_getdriverinfo() != 0) {
        errorexittodos("Error obtaining FOSSIL information");
        return;
    }

    if (!PcbData.DisableCTS) {
        _AX = 0x0f02; _DX = Port; geninterrupt(0x14);
    }
    _AX = 0x1000; _DX = Port; geninterrupt(0x14);

    setport(bauddivisor(Asy.ModemSpeed), Asy.DataBits);
    OutBufSize -= 128;

    tickdelay((PcbData.ModemDelay * HALFSECOND) + QUARTERSECOND);
}

void LIBENTRY FOSSIL_reopenport(void) { FOSSIL_openmodem(HIDE); }

static void near FOSSIL_dofixups(void) {
    ringdetect = FOSSIL_ringdetect;  ctsokay = FOSSIL_ctsokay;
    online = FOSSIL_online;          cdstillup = FOSSIL_cdstillup;
    bauddivisor = FOSSIL_bauddivisor; setport = FOSSIL_setport;
    inbytes = FOSSIL_inbytes;        outbytes = FOSSIL_outbytes;
    framingerrors = FOSSIL_framingerrors;
    overrunerrors = FOSSIL_overrunerrors;
    parityerrors = FOSSIL_parityerrors;
    turnoffdtr = FOSSIL_turnoffdtr;  turnondtr = FOSSIL_turnondtr;
    turnoffrts = FOSSIL_turnoffrts;  turnonrts = FOSSIL_turnonrts;
    turnonxmit = FOSSIL_turnonxmit;
    clearoutbuf = FOSSIL_clearoutbuf; clearinbuf = FOSSIL_clearinbuf;
    commgo = FOSSIL_commgo;          commstop = FOSSIL_commstop;
    commpause = FOSSIL_commpause;    checkcomm = FOSSIL_checkcomm;
    comminkey = FOSSIL_comminkey;    cgetstr = FOSSIL_cgetstr;
    cgetbuf = FOSSIL_cgetbuf;        csendbyte = FOSSIL_csendbyte;
    csendstr = FOSSIL_csendstr;
    disconnectmodem = FOSSIL_disconnectmodem;
    reopenport = FOSSIL_reopenport;
}

/* ========================================================================
 * MODEM.C helpers (LIB-filtered — no resetmodem, no sendmodemwaitokay,
 * no slowsendtomodem, no modemoffhook, no watchkbddropout, etc)
 * ======================================================================== */

#pragma warn -par
static bool near initializemodem(showtype Show) {
    ModemOpened = TRUE;
    ModemOffHook = FALSE;
    tickdelay((PcbData.ModemDelay * HALFSECOND) + QUARTERSECOND);
    turnondtr();
    turnonrts();
    return(TRUE);
}
#pragma warn +par

static void _NEAR_ LIBENTRY waitforroominbuffer(int Len) {
    settimer(0, SIXTYSECONDS);
    while (1) {
        if (Asy.Online == REMOTE) {
            if (Asy.LostCarrier || cdstillup() == 0) {
                Asy.LostCarrier = TRUE;
                if (PcbData.Packet) turnoffdtr();
                if (Asy.IgnoreCDLoss) return;
                loguseroff(ALOGOFF);
                return;
            }
            if (timerexpired(0)) {
                clearoutbuf();
                writelog("FLOW TIMEOUT", SPACERIGHT);
                return;
            }
        }
        if (OutBytes + Len < OutBufSize) return;
        turnonxmit();
        giveup();
        watchsystemfunctions();
    }
}

void LIBENTRY waitforempty(int Seconds) {
    if (Asy.Online == REMOTE) {
        settimer(4, Seconds);
        do {
            turnonxmit(); giveup();
            if (OutBytes <= 1) break;
            if (!cdstillup()) {
                if (PcbData.Packet) turnoffdtr();
                break;
            }
        } while (!timerexpired(4));
    }
}

void LIBENTRY clearandwaitformodemempty(void) {
    int CPS, DelayBytes;
    long Delay, BytesBuffered;

    if (Asy.Online != REMOTE) return;
    BytesBuffered = OutBytes;
    clearoutbuf();

    if (BytesBuffered != 0 &&
        (Asy.ModemSpeed > Asy.CarrierSpeed || Asy.ModemSpeed > 2400))
        DelayBytes = (BytesBuffered > 128 ?
                     (BytesBuffered > 1500 ? 3076 : 2048) : 1024);
    else
        DelayBytes = 128;

    CPS = (int)(Asy.CarrierSpeed / 10);
    Delay = (CPS <= 0) ? HALFSECOND :
            ((long)DelayBytes * TICKSPERSECOND) / CPS;

    settimer(3, Delay);
    while (!timerexpired(3) && cdstillup()) { giveup(); giveup(); }
}

void LIBENTRY sendstr(char *Str, int StrLen) {
    int HalfBuf = OutBufSize / 2;
    while (StrLen > HalfBuf) {
        waitforroominbuffer(HalfBuf);
        csendstr(Str, HalfBuf);
        Str += HalfBuf;
        StrLen -= HalfBuf;
    }
    if (StrLen > 0) {
        waitforroominbuffer(StrLen);
        csendstr(Str, StrLen);
    }
}

void LIBENTRY sendbyte(char Byte) {
    waitforroominbuffer(1);
    csendbyte(Byte);
}

void LIBENTRY closemodem(bool TurnOffDTR) {
    long BytesBuffered, Delay;
    int CPS;

    if (!ModemOpened) return;
    BytesBuffered = OutBytes;

    if (online()) {
        waitforempty(THIRTYSECONDS);
        if (TurnOffDTR) {
            Delay = HALFSECOND;
            if (Asy.CarrierSpeed > 2400 || Asy.CarrierSpeed != Asy.ModemSpeed) {
                CPS = (int)(Asy.CarrierSpeed / 10);
                if (CPS > 0 && BytesBuffered > 1000)
                    Delay = ((BytesBuffered * TICKSPERSECOND) / CPS) + HALFSECOND;
            }
            settimer(3, Delay);
            while (!timerexpired(3) && cdstillup()) { turnonxmit(); giveup(); }
        } else
            tickdelay(HALFSECOND);
    }

    if (PcbData.ModemPort[0] == 'C') turnoffrts();

    if (TurnOffDTR) {
        if (online()) {
            tickdelay(PcbData.ModemDelay * (ONESECOND + HALFSECOND));
            turnoffdtr();
            settimer(3, (PcbData.ModemDelay * HALFSECOND) + ONESECOND);
            while (!timerexpired(3) && cdstillup()) { giveup(); giveup(); }
        } else
            turnoffdtr();
    }
    disconnectmodem();
    ModemOpened = FALSE;
}

void LIBENTRY openmodem(showtype Show) {
    if (Asy.ComPortNumber == 0) return;

    if (PcbData.ModemPort[0] == 'C') {
        ASYNC_dofixups();
        ModemFixupsDone = 'A';
        ASYNC_openmodem(Show);
        return;
    }

    FOSSIL_dofixups();
    ModemFixupsDone = 'F';
    FOSSIL_openmodem(Show);
}
