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



/******************************************************************************/

#ifndef ___GPROT_HPP___

#define ___GPROT_HPP___

/******************************************************************************/

#include    <commbuf.hpp>

/******************************************************************************/

const int maxWinNodes = 8;

/******************************************************************************/

struct sGPKTHDR
{
    unsigned char dle;
    unsigned char k;
    unsigned char cklo;
    unsigned char ckhi;
    unsigned char ctl;
    unsigned char xor;
};

struct sGPKTNODE
{
    bool       acked;
    sGPKTHDR   hdr;
    char     * data;
};

class cGPDL
{

    public:

                            cGPDL             ( char * _prot );

        virtual            ~cGPDL             ( void );

                int  pascal run               ( void );

    protected:

                int  pascal init              ( void );
                void pascal deinit            ( void );

                bool pascal abortObj          ( void );

                int  pascal sendPacket        ( unsigned char   xf,
                                                unsigned char   yf );

                int  pascal sendPacket        (          char * ds,
                                                         bool   ll = TRUE );

                int  pascal sendPacket        (          int    sz,
                                                         char * ds,
                                                         bool   ll = TRUE );

                int  pascal receivePacket     ( unsigned char & tf,
                                                unsigned char & xf,
                                                unsigned char & yf,
                                                         int  & sz,
                                                         char * ds = NULL );

                int  pascal waitReceivePacket (          long   tk,
                                                unsigned char & tf,
                                                unsigned char & xf,
                                                unsigned char & yf,
                                                         int  & sz,
                                                         char * ds = NULL );

        virtual int  pascal app               ( void ) = 0;

    private:

                int  pascal sendPacket        ( unsigned char   k,
                                                unsigned char   tf,
                                                unsigned char   xf,
                                                unsigned char   yf,
                                                         char * ds );

                int  pascal sendPacket        ( sGPKTHDR      & hdr,
                                                         char * ds );

                void pascal resendWindow      ( int lastAcked );

                void pascal ackPackets        ( int lastAcked );

                bool pascal fullWindow        ( void );

    protected:

        bool initADone;
        bool initBDone;
        bool initCDone;
        bool closeDone;

        unsigned char negWinSize;
        unsigned char negDataBlk;

        unsigned char pktSSeq; // This is the last packet sequence number sent
        unsigned char pktRSeq; // This is the last packet sequence number received
        unsigned char pktRAck; // This is the last ACK sequence number received

        int consecutiveNAKS;
        int consecutiveERRS;

        bool abortFlag;

        sGPKTNODE pktWin [ maxWinNodes ];

        char prot [ 3 + 1 ];

};

class cGPAL : public cGPDL
{

    public:

                 cGPAL ( char * _prot ) : cGPDL(_prot) { }

        virtual ~cGPAL ( void ) { }

        int pascal sendMessage    ( char * msg );
        int pascal sendMessage    ( char * msg, char * file );
        int pascal receiveMessage ( char * msg );

        int pascal sendFile       ( char * file, bool master, char * buf );
        int pascal receiveFile    ( char * file, bool master, char * buf );

    protected:

    private:

};

class cGPPL : public cGPAL
{

    public:

        cGPPL ( char * _spool, char * _sname, char * _prot ) :
            cGPAL(_prot)
        {
            maxstrcpy(spool,_spool,sizeof(spool));
            maxstrcpy(sname,_sname,sizeof(sname));
        }

        virtual ~cGPPL ( void ) { }

        virtual int pascal app       ( void );

                int pascal master    ( void );

                int pascal doCmdFile ( char * name );

        char spool [ 128 + 1 ];
        char sname [ 128 + 1 ];

    protected:

    private:

};

typedef cGPPL cGPROT;

/******************************************************************************/

#endif

