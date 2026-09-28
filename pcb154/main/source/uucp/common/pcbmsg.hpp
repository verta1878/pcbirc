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



#include    <vmdata.hpp>

/******************************************************************************/

const int msgStartSize = sizeof(msgheadertype)+sizeof(msgextendtype)*MAXEXTHDRS;
const int msgMaxSize   = (255U+1U)*128U;
const int msgBufSize   = msgMaxSize-msgStartSize;

/******************************************************************************/

class cNETHDR
{
    public:

     // Header:                    // Used By:
     // ~~~~~~~~~~~~~~~~~~~~~~~~~~ // ~~~~~~~~~~~~~~~~~~~~~~~
     // Date:                      // mail       & news
     // Distribution:              //              news
     // Followup-To:               //              news
     // From:                      // mail       & news
     // Message-ID:                //              news
     // Message-Id:                // mail
     // Newsgroups:                //              news
     // Organization:              //              news
     // Reply-To:                  //              news
     // Subject:                   // mail       & news
     // To:                        // mail       & news
     // Mime-Version:              // mail       & news
     // Content-Type:              // mail       & news
     // Content-Transfer-Encoding: // mail       & news

        cNETHDR ( void );

        ~cNETHDR ( void );

        void   pascal kill          ( void );
        void   pascal combine       ( char * & _s1, char * _s2 );
        void   pascal getInfo       ( char * _hdr );
        void   pascal setToUser     ( char * _user );
        void   pascal setFileAttach ( char * _file );

        int    lastHdrType;
        int    isMime;

        char * apparentlyto; char * date;
        char * distribution; char * followupto;
        char * from;         char * msgidmail;
        char * msgidnews;    char * newsgroups;
        char * organization; char * replyto;
        char * subject;      char * to;
        char * user;         char * file;

        // MIME processing
        char  MimeVersion[31];
        char  ContentType[256];
        char  EncodeType[81];
        char  Disposition[81];
        char  Boundary[81];
};

/******************************************************************************/

class cPCBMSG
{

    public:

        cPCBMSG ( char _all, char _stat, cNETHDR * _hdr );

        ~cPCBMSG ( void );

        int  pascal flushMsg ( void );

        int  pascal flushMsgPart ( void );

        void pascal addLine ( char * line, int len );

        void pascal load ( char * path, char * name, char * line, int lsize );
        void pascal load ( cVMDATA & msg );

        void pascal readField  ( char * & field, cDOSFILE & msgFile );

    protected:

        static char far   buf     [ msgBufSize ];
               unsigned   size;

               char       decodeName [ 128 + 1 ];

               char       part;

               char       msgAll;
               char       msgStat;

               cNETHDR  * nhdr;

               bool       decodeMode;

        int  pascal flushMsgSub(void);

    private:

};

/******************************************************************************/

class cTMPMSG
{

    public:

        cTMPMSG ( char _all, char _stat, cNETHDR * _hdr, char * _spath );

        ~cTMPMSG ( void );

        int  pascal flushMsg ( void );

        void pascal addLine ( char * line, int len, int hide );

        void pascal writeField ( char *   field );

        void   pascal resetLine ( void );
        char * pascal getLine   ( void );

        char      msgAll;
        char      msgStat;
        cNETHDR * nhdr;
        char      spath   [ 128 + 1 ];
        char      msgName [  12 + 1 ];
        cDOSFILE  msgFile;
        cVMDATA   msgData;
        long      idxBuf  [ 256 ];

        char      isEncoded;
        char      beginFound;
        char      endFound;

        long      lastLine;

    protected:

    private:

};

/******************************************************************************/

struct sMSGINFO
{
    char     processed;
    unsigned date;
    char     all;
    char     stat;
    char     fileName        [  12 + 1 ];
    char     isEncoded;
    char     beginFound;
    char     endFound;
    char     subject         [ 120 + 1 ];
    int      subjLen;
    int      partNum;
    int      totParts;
 // char     nhapparentlyto  [ 120 + 1 ];
 // char     nhdate          [ 120 + 1 ];
 // char     nhdistribution  [ 120 + 1 ];
 // char     nhfollowupto    [ 120 + 1 ];
 // char     nhfrom          [ 120 + 1 ];
 // char     nhmsgidmail     [ 120 + 1 ];
 // char     nhmsgidnews     [ 120 + 1 ];
 // char     nhnewsgroups    [ 120 + 1 ];
 // char     nhorganization  [ 120 + 1 ];
 // char     nhreplyto       [ 120 + 1 ];
 // char     nhto            [ 120 + 1 ];
 // char     nhuser          [ 120 + 1 ];
};

class cMSGINFO : public cVMDATA
{

    public:

        cMSGINFO ( void ) { open(); }

        ~cMSGINFO ( void ) { close(); }

        void pascal open  ( void ) { initRecSet(sizeof(sMSGINFO)); }
        void pascal close ( void ) { doneSet(); }

        void pascal load  ( char * path );
        void pascal save  ( char * path );

        void pascal sort  ( void );

        void pascal add ( sMSGINFO & rec );
        void pascal add ( cNETHDR & hdr );
        void pascal add ( cTMPMSG & msg );

        void pascal get ( sMSGINFO & rec, long num );
        void pascal put ( sMSGINFO & rec, long num );

        void pascal process ( char * path, char * line, int lsize );

    protected:

    private:

};

/******************************************************************************/

