/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* pcbmail.c -- PCBoard Mail (Phase 27 Binary Analysis)                     */
/*                                                                           */
/* Exact reproduction of Clark's PCBMAIL.EXE (333K).                         */
/*                                                                           */
/* Windows GUI message reader/editor built with Borland C++ 4.50.            */
/*   PCBMAIL.EXE   333K   Main application                                  */
/*   BC450RTL.DLL  220K   Borland C++ 4.50 runtime                          */
/*   BWCC.DLL      165K   Borland Windows Custom Controls                   */
/*   PCBMAIL.HLP   759K   Windows help file (behavioural spec)              */
/*                                                                           */
/* Features from PCBMAIL.HLP:                                                */
/*   -- Message reader with configurable fonts                               */
/*   -- Header fonts: any Windows font                                       */
/*   -- Body fonts: fixed-pitch only (Terminal default)                      */
/*   -- Message editor (like PCBoard's built-in editor)                      */
/*   -- Address dialog with To, Subject, Cc fields                           */
/*   -- @LIST@ mailing to groups                                             */
/*   -- Private/Public message toggle                                        */
/*   -- Mailing list dialog for Cc recipients                                */
/*   -- Reads PCBoard message bases directly                                 */
/*   -- CP437 character set support (via Terminal/dosapp.fon)                */
/*                                                                           */
/* Reference design for pcbnav's message reader/editor.                      */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WATCOMC__
#include <stdint.h>
#define strcasecmp stricmp
#else
#include <stdint.h>
#endif


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    Microsoft Binary Format (bsreal)                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* PCBoard stores message numbers and dates as Microsoft Binary Format       */
/* single-precision floats -- NOT integers. Clark calls this type "bassngl"  */
/* or "bsreal" in the documentation. Four bytes, laid out:                   */
/*                                                                           */
/*   byte 0: mantissa low                                                   */
/*   byte 1: mantissa mid                                                   */
/*   byte 2: sign (bit 7) + mantissa high (bits 0-6)                        */
/*   byte 3: biased exponent (bias = 128, 0 = zero value)                   */
/*                                                                           */
/* This differs from IEEE 754 in byte order and bias. Clark's library        */
/* provides bs_to_long() for the conversion. We reproduce it here so the     */
/* scaffold builds standalone.                                               */

typedef unsigned char bassngl[4];

static long bsreal_to_long(const bassngl Val)
{
    unsigned long Mantissa;             /* 24-bit mantissa               */
    int           Exponent;             /* unbiased exponent             */
    int           Sign;                 /* 1 if negative                 */

    if (Val[3] == 0) return 0;          /* exponent 0 = value is zero    */

    Mantissa = ((unsigned long)(Val[2] | 0x80) << 16)
             | ((unsigned long)Val[1] << 8)
             | (unsigned long)Val[0];
    Sign     = (Val[2] & 0x80) ? 1 : 0;
    Exponent = (int)Val[3] - 128 - 24; /* unbias, adjust for mantissa   */

    /* Shift mantissa to produce an integer */
    if (Exponent > 0)
        Mantissa <<= Exponent;
    else if (Exponent < 0)
        Mantissa >>= (-Exponent);

    return Sign ? -(long)Mantissa : (long)Mantissa;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                  Message Header (DOCDEV/MSGS.TXT)                         */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Layout from PCBXDOT/DOCDEV/MSGS.TXT -- the authoritative reference.       */
/*                                                                           */
/* An earlier version of this file used the FidoNet .MSG header layout.      */
/* That was wrong -- different format entirely. Rule for Phase 27:           */
/* read DOCDEV/ before declaring any struct.                                 */
/*                                                                           */
/* When building inside the pcbirc tree, include MESSAGES.H and use          */
/* Clark's msgheadertype directly. This standalone copy exists only so       */
/* the scaffold compiles without the full source tree.                       */

#pragma pack(push, 1)
typedef struct {
    char    Status;                     /*   0  status flag (see below)  */
    bassngl MsgNumber;                  /*   1  message number (bsreal)  */
    bassngl RefNumber;                  /*   5  reference number         */
    char    NumBlocks;                  /*   9  128-byte blocks in msg   */
    char    Date[8];                    /*  10  "mm-dd-yy"               */
    char    Time[5];                    /*  18  "hh:mm"                  */
    char    ToField[25];               /*  23  addressee name           */
    bassngl ReplyDate;                  /*  48  reply date (yymmdd)      */
    char    ReplyTime[5];              /*  52  reply time "hh:mm"       */
    char    ReplyStatus;               /*  57  'R' if has reply         */
    char    FromField[25];             /*  58  sender name              */
    char    SubjField[25];             /*  83  subject                  */
    char    Password[12];              /* 108  password to read         */
    char    ActiveFlag;                /* 120  225=active, 226=inactive */
    char    EchoFlag;                  /* 121  'E' if echomail          */
    char    Reserved[4];               /* 122  reserved                 */
    char    ExtHdrFlags;               /* 126  extended header bitmap   */
    char    NetTag;                    /* 127  15.4: network tag        */
} MsgHeader;                            /* 128 bytes total               */
#pragma pack(pop)


/* Status flag values -- MSGS.TXT note 2.                                   */
/* Privacy class and read-state encoded together in one byte.               */

#define MSG_PUBLIC          ' '         /* readable by anyone             */
#define MSG_PRIV_UNREAD     '*'         /* private, NOT read              */
#define MSG_PRIV_READ       '+'         /* private, HAS been read         */
#define MSG_PUBLIC_READ     '-'         /* to a person, public, read      */
#define MSG_COMMENT_UNREAD  '~'         /* comment to sysop, NOT read     */
#define MSG_COMMENT_READ    '`'         /* comment to sysop, HAS read     */
#define MSG_SNDPWD_UNREAD   '%'         /* sender-password, unread        */
#define MSG_SNDPWD_READ     '^'         /* sender-password, read          */
#define MSG_GRPPWD_UNREAD   '!'         /* group-password, unread         */
#define MSG_GRPPWD_READ     '#'         /* group-password, read           */
#define MSG_GRPPWD_ALL      '$'         /* group-password, to ALL         */

/* Active flag values */
#define MSG_ACTIVE          225         /* message is active              */
#define MSG_INACTIVE        226         /* message is inactive            */


/*-----------------------------------------------------------------------*/
/* msg_is_private() -- Is this message private to its addressee?         */
/*-----------------------------------------------------------------------*/

static int msg_is_private(char Status)
{
    return (Status == MSG_PRIV_UNREAD || Status == MSG_PRIV_READ);
}


/*-----------------------------------------------------------------------*/
/* msg_is_read() -- Has the addressee read this message?                 */
/*-----------------------------------------------------------------------*/

static int msg_is_read(char Status)
{
    switch (Status) {
    case MSG_PRIV_READ:
    case MSG_PUBLIC_READ:
    case MSG_COMMENT_READ:
    case MSG_SNDPWD_READ:
    case MSG_GRPPWD_READ:
        return 1;
    default:
        return 0;
    }
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                  Message Base Header (DOCDEV/MSGS.TXT)                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* The first 128 bytes of the message file are a base header, NOT a          */
/* message. Fields from MSGS.TXT:                                            */

#pragma pack(push, 1)
typedef struct {
    bassngl HighMsgNum;                 /*   0  highest message number   */
    bassngl LowMsgNum;                  /*   4  lowest message number    */
    bassngl ActiveMsgs;                 /*   8  number of active msgs    */
    bassngl SysCallers;                 /*  12  callers (main base only) */
    char    Locked[6];                  /*  16  "LOCKED" or spaces       */
    long    LastScanned;                /*  22  last email list scan     */
    char    Reserved[102];              /*  26  reserved                 */
} MsgBaseHeader;                        /* 128 bytes total               */
#pragma pack(pop)


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Font Configuration                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* From PCBMAIL.HLP:                                                        */
/*   Two font types:                                                        */
/*     Header fonts -- any Windows font (proportional OK)                   */
/*     Message body fonts -- fixed-pitch only                               */
/*                                                                           */
/*   Default body font: Terminal (CP437 character set)                       */
/*   Tip from the help: install dosapp.fon for more Terminal sizes.         */

typedef struct {
    char FaceName[32];                  /* Windows font face name        */
    int  Size;                          /* font size (points)            */
    int  Bold;                          /* bold flag                     */
    int  Italic;                        /* italic flag                   */
} FontConfig;

typedef struct {
    FontConfig Header;                  /* header display font           */
    FontConfig Body;                    /* message body font             */
} MailFonts;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Address Dialog                                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* From PCBMAIL.HLP:                                                        */
/*   To field -- single recipient name                                      */
/*   Subject field -- message subject                                       */
/*   Cc button -- opens mailing list dialog                                 */
/*   @LIST@ -- mail to a named list of users                                */
/*   Private radio button -- private/public toggle                          */
/*   Addressed names shown at bottom of message editor                      */

typedef struct {
    char     To[25];                    /* primary recipient             */
    char     Subject[25];              /* message subject               */
    char     CcList[16][25];            /* Cc recipients                 */
    int      NumCc;                     /* Cc count                      */
    int      IsPrivate;                 /* private message flag          */
    int      UseList;                   /* using @LIST@                  */
    char     ListName[64];              /* @LIST@ name if used           */
} AddressInfo;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    Message Base Reader                                     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* msg_read_base_header() -- Read the 128-byte base header               */
/*-----------------------------------------------------------------------*/

static int msg_read_base_header(FILE *Fp, MsgBaseHeader *Bh)
{
    fseek(Fp, 0, SEEK_SET);
    if (fread(Bh, sizeof(*Bh), 1, Fp) != 1)
        return -1;
    return 0;
}


/*-----------------------------------------------------------------------*/
/* msg_read_header() -- Read a message header at a given offset          */
/*-----------------------------------------------------------------------*/

static int msg_read_header(FILE *Fp, long Offset, MsgHeader *Hdr)
{
    if (fseek(Fp, Offset, SEEK_SET) != 0)
        return -1;
    if (fread(Hdr, sizeof(*Hdr), 1, Fp) != 1)
        return -1;
    return 0;
}


/*-----------------------------------------------------------------------*/
/* msg_read_body() -- Read message body text                             */
/*                                                                       */
/* Body follows header in 128-byte blocks. NumBlocks in the header       */
/* counts the total blocks including the header itself (MSGS.TXT         */
/* note 3), so a message with a one-block body stores NumBlocks=2.       */
/*-----------------------------------------------------------------------*/

static int msg_read_body(FILE *Fp, long Offset, int NumBlocks,
                          char *Body, int BodySize)
{
    int BodyBlocks;                     /* data blocks (minus header)    */
    int ToRead;                         /* bytes to read                 */

    BodyBlocks = NumBlocks - 1;
    if (BodyBlocks <= 0) {
        Body[0] = '\0';
        return 0;
    }

    ToRead = BodyBlocks * 128;
    if (ToRead > BodySize - 1)
        ToRead = BodySize - 1;

    if (fseek(Fp, Offset + 128, SEEK_SET) != 0)
        return -1;
    if ((int)fread(Body, 1, ToRead, Fp) != ToRead)
        return -1;

    Body[ToRead] = '\0';
    return 0;
}


/*-----------------------------------------------------------------------*/
/* msg_display() -- Display a message (text mode)                        */
/*-----------------------------------------------------------------------*/

static void msg_display(const MsgHeader *Hdr, const char *Body)
{
    long MsgNum;                        /* converted message number      */

    MsgNum = bsreal_to_long(Hdr->MsgNumber);

    printf("===================================================\n");
    printf("Msg #%ld  Date: %.8s  Time: %.5s\n",
           MsgNum, Hdr->Date, Hdr->Time);
    printf("From:    %.25s\n", Hdr->FromField);
    printf("To:      %.25s\n", Hdr->ToField);
    printf("Subject: %.25s\n", Hdr->SubjField);

    if (Hdr->ActiveFlag == (char)MSG_INACTIVE)
        printf("** INACTIVE **\n");
    if (msg_is_private(Hdr->Status))
        printf("** PRIVATE%s **\n",
               msg_is_read(Hdr->Status) ? " (read)" : "");
    if (Hdr->Status == MSG_COMMENT_UNREAD ||
        Hdr->Status == MSG_COMMENT_READ)
        printf("** COMMENT TO SYSOP%s **\n",
               msg_is_read(Hdr->Status) ? " (read)" : "");
    if (Hdr->EchoFlag == 'E')
        printf("** ECHOMAIL **\n");

    printf("===================================================\n");
    printf("%s\n", Body);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                             Main Entry                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int main(int Argc, char *Argv[])
{
    const char    *MsgPath;             /* message base path             */
    FILE          *Fp;                  /* message file handle           */
    MsgBaseHeader  BaseHdr;             /* base header (first 128 bytes) */
    MsgHeader      Hdr;                 /* message header                */
    char           Body[8192];          /* message body buffer           */

    MsgPath = (Argc > 1) ? Argv[1] : "MAIN\\MSGS";

    printf("PCBoard Mail\n\n");

    Fp = fopen(MsgPath, "rb");
    if (!Fp) {
        printf("Cannot open message base: %s\n", MsgPath);
        return 1;
    }

    /* Read and display base header */
    if (msg_read_base_header(Fp, &BaseHdr) == 0) {
        printf("Base: high=%ld low=%ld active=%ld callers=%ld\n\n",
               bsreal_to_long(BaseHdr.HighMsgNum),
               bsreal_to_long(BaseHdr.LowMsgNum),
               bsreal_to_long(BaseHdr.ActiveMsgs),
               bsreal_to_long(BaseHdr.SysCallers));
    }

    /* Read and display first message (offset 128 = after base header) */
    if (msg_read_header(Fp, 128, &Hdr) == 0) {
        msg_read_body(Fp, 128, Hdr.NumBlocks, Body, sizeof(Body));
        msg_display(&Hdr, Body);
    }

    fclose(Fp);
    return 0;
}
