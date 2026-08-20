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



#ifndef ___PCBMSGS_HPP___

#define ___PCBMSGS_HPP___

/******************************************************************************/

#include    <mem.h>
#include    <stdlib.h>
#include    <string.h>

#include    <pcbreal.hpp>
#include    <dosclass.hpp>

/******************************************************************************/

#pragma pack(1)
struct sBASEHDR
{
    cBSREAL       high;           //     0 bsreal   4 High Message Number       (0 to 16,700,000)
    cBSREAL       low;            //     4 bsreal   4 Low Message Number        (0 to 16,700,000)
    cBSREAL       active;         //     8 bsreal   4 Number of Active Messages (0 to 32,767)
    cBSREAL       callers;        //    12 bsreal   4 Number of System Callers (Main Message Base Only)
    char          locked [   6 ]; //    16 str      6 The "LOCKED" field for pre-14.2 systems (see note 1)
    char          res    [ 106 ]; //    22 str    106 Reserved for future use
};
#pragma pack()

#pragma pack(1)
struct sMSGHDR
{
    char          status;         //     0 char     1 Message Status Flag (see note 2)
    cBSREAL       number;         //     1 bsreal   4 Message Number   (0 to 16,700,000)
    cBSREAL       refer;          //     5 bsreal   4 Reference Number (0 to 16,700,000)
    unsigned char blocks;         //     9 char     1 Number of 128 Byte Blocks in Message (see note 3)
    char          dmsg   [   8 ]; //    10 str      8 Date of Message Entry (in "mm-dd-yy" format)
    char          tmsg   [   5 ]; //    18 str      5 Time of Message Entry (in "hh:mm" format)
    char          tname  [  25 ]; //    23 str     25 Name of the User to whom the Message is Addressed
    cBSREAL       dread;          //    48 bsreal   4 Date of the Reply Message (in yymmdd format)
    char          tread  [   5 ]; //    52 str      5 Time of the Reply Message (in "hh:mm" format)
    char          reply;          //    57 char     1 The Letter "R" if the Message has a Reply
    char          fname  [  25 ]; //    58 str     25 Name of the User who wrote the Message
    char          subj   [  25 ]; //    83 str     25 Subject of the Message
    char          pwd    [  12 ]; //   108 str     12 Password Need to Read the Message (if any)
    char          active;         //   120 char     1 Active Status (225 = active, 226 = inactive)
    char          echo;           //   121 char     1 The Letter "E" if the Message is to be Echoed
    char          res1   [   4 ]; // | 122 str      4 Reserved for future use
    char          exthdr;         // | 127 bitmap   1 Extended Header Flags
    char          res2;           // | 128 char     1 Reserved for future use
};
#pragma pack()

#pragma pack(1)
struct sMSGEXTHDR
{
    sint          id;             //   int  Ident;        16639 (40FF hex)
    char          funct  [   7 ]; // | char Function[7];  TO,TO2,FROM,FROM2,SUBJECT,ATTACH,LIST,ROUTE,ORIGIN,REQRR,ACKRR,ACKNAME,PACKOUT
    char          colon;          //   char Colon;        always set to ":"
    char          desc   [  60 ]; //   char Descript[60]; user name, subject, filename
    char          stat;           //   char Status;       NONE, READ (if LIST)
    char          cr;             //   char CarReturn;    0xE3 or 0x0D
};
#pragma pack()

#pragma pack(1)
struct sMSGIDX
{
    long          off;            // |   0 long     4 Offset (0 if none, >0 if active, <0 if killed)
    long          num;            // |   4 long     4 Message Number
    char          to     [ 25 ];  // |   8 str     25 TO Name
    char          from   [ 25 ];  // |  33 str     25 FROM Name
    char          status;         // |  58 char     1 Status Character (from Message Header)
    uint          date;           // |  59 int      2 Date of Message (in julian date format)
    char          res    [  3 ];  // |  61 str      3 Reserved for future use
};
#pragma pack()

class cMSG : public cDOSFILE
{

    private:

        cDOS aliasThis;

    public:

        cMSG ( void );

        ~cMSG ( void );

        void         LIBENTRY reallocMsgBuf ( int size );
        void         LIBENTRY freeMsgBuf    ( void );

        int          LIBENTRY open          ( char * name );
        void         LIBENTRY close         ( void );

        long         LIBENTRY lowMsgNum     ( void );
        long         LIBENTRY highMsgNum    ( void );

        int          LIBENTRY readHdr       ( long off );
     // int          LIBENTRY readHdr       ( void );
        int          LIBENTRY readBody      ( unsigned size = 0 );

        int          LIBENTRY writeHdr      ( long off = -1 );

        void         LIBENTRY getExtHdr     ( char * name, void * buf );
        void         LIBENTRY getExtHdrs    ( char * name, char * buf );

        int          LIBENTRY getExtHdrs    ( void ) { return msgExtHdrs; }
        sMSGEXTHDR * LIBENTRY getExtHdr     ( int    num );

        char       * LIBENTRY getLine       ( void );
     // int          LIBENTRY addLine       ( char * line );

        void         LIBENTRY resetLine     ( void );

        int          LIBENTRY year          ( void );
        int          LIBENTRY month         ( void );
        int          LIBENTRY day           ( void );
        int          LIBENTRY hour          ( void );
        int          LIBENTRY minute        ( void );

    protected:

        bool         LIBENTRY validExtHdr   ( int i );
        void         LIBENTRY countExtHdrs  ( void );
        void         LIBENTRY sortExtHdrs   ( void );

        int          LIBENTRY digitsToInt   ( char * s, int o );

    private:

        static int cmpMsgExtHdrs    ( const void * l, const void * r );
        static int chkMsgExtHdrName ( const void * l, const void * r );

    public:

        sMSGHDR      msgHdr;
        char       * msgBody;
        unsigned     msgSize;

    protected:

        long         msgHdrOff;
        char       * msgBuf;
        int          msgBufSize;
        sMSGEXTHDR * msgExtHdr;
        int          msgExtHdrs;
        char       * msgLineStart;
        char       * msgLineEnd;
        int          inGetLine;

};

class cMSGIDX : public cDOSFILE
{

    public:

        void LIBENTRY setStart(long num);

        int LIBENTRY seekIdx(long num);

        long LIBENTRY readIdx(long num = 0);

        sMSGIDX idxRec;

    protected:

        long    idxStart;

    private:

};

class cPCBMSGS : public cMSG, public cMSGIDX
{

    public:

        int LIBENTRY open ( const char * name );

        void LIBENTRY close ( void );

    protected:

    private:

};

/******************************************************************************/

#endif

