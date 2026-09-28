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
/*                                                                                                                                                        */
/*                                                                 DBASE.HPP                                                              */
/*                                                                                                                                                        */
/*----------------------------------------------------------------------------*/
/*                                                                                                                                                        */
/*                      Data and functions for accessing dBASE compatible files.                  */
/*                                                                                                                                                        */
/*============================================================================*/
/*                                                                                                                                                        */
/*                                               Written by Scott Dale Robison                                            */
/*                                                                                                                                                        */
/*----------------------------------------------------------------------------*/
/*                                                                                                                                                        */
/*                                Copyright (C) 1994 Clark Development Company                            */
/*                                                                                                                                                        */
/******************************************************************************/

#ifndef ___DBASE_HPP___

#define ___DBASE_HPP___

/******************************************************************************/

// Included Files

#ifdef __OS2__
  #pragma pack(1)
#endif

#include        <d4all.h>

#ifdef __OS2__
  #pragma pack(4)
#endif

#ifdef ___USE_VAR___
#include        <var.hpp>
#endif

/******************************************************************************/

// Types

template <class T, class U>
class cINFSTRUCT
{

        public:

                                 cINFSTRUCT ( void );

                void LIBENTRY clrInfo ( void );

        void LIBENTRY bldInfo ( U      info [], int infs = -1 );

           #ifdef ___USE_VAR___
        void LIBENTRY bldInfo ( cVAR & info,    int infs = -1 );
           #endif

                T *  LIBENTRY getInfo ( void );

        protected:

        virtual void LIBENTRY clrNode ( T       & info ) = 0;

                void LIBENTRY addInfo ( T       & info );

        virtual void LIBENTRY cvtInfo ( U         info ) = 0;

           #ifdef ___USE_VAR___
                void LIBENTRY cvtInfo ( cVARVAL & info );
           #endif

        private:

                T       * infPtr;
                int   infCnt;
                T         emptyINF;

};

        /*--------------------------------------------------------------------*/

class cFLDSTRUCT : public cINFSTRUCT<FIELD4INFO,char *>
{

        public:

                virtual ~cFLDSTRUCT ( void )
                {
                        clrInfo();
                }

        protected:

                void LIBENTRY clrNode ( FIELD4INFO & info );

        void LIBENTRY cvtInfo ( char       * info );

        private:

};

        /*--------------------------------------------------------------------*/

#if 0

class cTAGSTRUCT : public cINFSTRUCT<TAG4INFO,char *>
{

        public:

                virtual ~cTAGSTRUCT ( void )
                {
                        clrInfo();
                }

        protected:

        void LIBENTRY clrNode ( TAG4INFO   & info );

        void LIBENTRY cvtInfo ( char       * info );

        private:

};

#endif

        /*--------------------------------------------------------------------*/

#ifdef ___USE_VAR___

class cGRPSTRUCT : public cINFSTRUCT<long,long>
{

        public:

        virtual ~cGRPSTRUCT ( void )
                {
                        clrInfo();
                }

        protected:

        void LIBENTRY clrNode ( long       & info );

        void LIBENTRY cvtInfo ( long         info );

        private:

};

#endif

        /*--------------------------------------------------------------------*/

class cCODEBASE
{

        public:

                cCODEBASE ( void ) { if (++cnt == 1) d4init(&c4); }

                virtual ~cCODEBASE ( void )
                {
                        if (cnt-- == 1)
                        {
                                d4init_undo(&c4);
                                mem4reset();
                        }
                }

        protected:

                static CODE4 c4;

                static int       cnt;

        private:

};

        /*--------------------------------------------------------------------*/

#define dbfCreatePrototype(t)                                                                                            \
dbfCreate ( char * name, int exclusive, t finfo )

        /*--------------------------------------------------------------------*/

#if 0

#define ndxCreatePrototype(t)                                                                                            \
ndxCreate ( char * name, t tinfo )

#endif

        /*--------------------------------------------------------------------*/

class cDBF : public cCODEBASE
{

    friend void e4hook(CODE4 * code_base, int err_code, char * desc1, char * desc2,
    char * desc3);

        public:

                 cDBF ( void );
                ~cDBF ( void );

        char *   LIBENTRY errText(int code);
                void     LIBENTRY dbfCreatePrototype ( char * * );
           #ifdef ___USE_VAR___
                void     LIBENTRY dbfCreatePrototype ( cVAR     & );
           #endif
        void     LIBENTRY dbfOpen            ( char   * name, int      exclusive );
                void     LIBENTRY dbfClose                 ( void );
                char   * LIBENTRY dbfAlias                 ( char       * alias = NULL );

        void     LIBENTRY pack               ( void );

        void     LIBENTRY lock               ( void );
        void     LIBENTRY lock               ( long     recNo );
        void     LIBENTRY lock               ( long   * recs, int      count );
#ifdef ___USE_VAR___
        void     LIBENTRY lock               ( cVAR   & recs, int      count );
#endif
        void     LIBENTRY unlock             ( void );

#if 0

                void     LIBENTRY ndxCreatePrototype ( char * * );
           #ifdef ___USE_VAR___
                void     LIBENTRY ndxCreatePrototype ( cVAR     & );
           #endif

#endif

        void     LIBENTRY ndxCreate          ( char   * name, char   * expr );
                void     LIBENTRY ndxOpen                  ( char       * name );
                void     LIBENTRY ndxClose                 ( char       * name );
        void     LIBENTRY ndxCloseAll        ( void );

        void     LIBENTRY start              ( void );
        void     LIBENTRY add                ( void );
        void     LIBENTRY append             ( void );

                void     LIBENTRY top                      ( void );
                void     LIBENTRY go                               ( long         recNo );
                void     LIBENTRY bottom                           ( void );

        long     LIBENTRY reccount           ( void );
        long     LIBENTRY recno              ( void );

        void     LIBENTRY skip               ( long     recs );

                int      LIBENTRY bof                      ( void );
                int      LIBENTRY eof                      ( void );

                void     LIBENTRY blank                    ( void );

                void     LIBENTRY kill                     ( void );
                void     LIBENTRY resurrect                ( void );
                int      LIBENTRY killed                           ( void );

                int      LIBENTRY changed                  ( int          flag = -1 );

                void     LIBENTRY tag                      ( char       * name );

                void     LIBENTRY seek                     ( char       * key );
                void     LIBENTRY seek                     ( double   key );

                int      LIBENTRY fields                           ( void );
                char   * LIBENTRY name                     ( int          n );
                char     LIBENTRY type                     ( char       * name );
                unsigned LIBENTRY length                           ( char       * name );
                int      LIBENTRY decimals                 ( char       * name );

                void     LIBENTRY blank                    ( char       * name );

                void     LIBENTRY get                      ( char       * name, char   * val );
                void     LIBENTRY get                      ( char       * name, char   & val );
                void     LIBENTRY get                      ( char       * name, int    & val );
                void     LIBENTRY get                      ( char       * name, long   & val );
                void     LIBENTRY get                      ( char       * name, double & val );

                void     LIBENTRY put                      ( char       * name, char   * val );
                void     LIBENTRY put                      ( char       * name, char     val );
                void     LIBENTRY put                      ( char       * name, int      val );
                void     LIBENTRY put                      ( char       * name, long     val );
                void     LIBENTRY put                      ( char       * name, double   val );

        void     LIBENTRY copy               ( char   * dest, cDBF   & dbf,
                                             char   * src );
        char *   LIBENTRY dbfGetAlias        ( void );
        int      LIBENTRY notInUseErr        ( void );
        int      LIBENTRY inUseErr           ( void );

                int      LIBENTRY error                    ( void );
        int      LIBENTRY returnStatus       ( void );

        int     inUse;
        int     myChannel;
        protected:

        private:
        int     statusFlag;
                DATA4 * d4;

                int     err;


};

/******************************************************************************/

// cINFSTRUCT Template Class Member Functions

template <class T, class U>
cINFSTRUCT<T,U>::cINFSTRUCT(void) :
        infPtr(NULL),
        infCnt(0)
{
        memset(&emptyINF,0,sizeof(T));
}

        /*--------------------------------------------------------------------*/

template <class T, class U>
void LIBENTRY cINFSTRUCT<T,U>::clrInfo(void)
{
        for (int i = 0; i < infCnt; ++i) clrNode(infPtr[i]);
        if (infPtr) delete [] infPtr;
        infPtr = NULL;
        infCnt = 0;
}

        /*--------------------------------------------------------------------*/

template <class T, class U>
void LIBENTRY cINFSTRUCT<T,U>::bldInfo(U info [], int infs)
{
        clrInfo();

        if (info == NULL) return;

        if (infs == -1) infs = maxInt;

        for (int i = 0; (i < infs) && (info[i] != NULL); ++i)
        cvtInfo(info[i]);

        addInfo(emptyINF);
}

        /*--------------------------------------------------------------------*/

#ifdef ___USE_VAR___

template <class T, class U>
void LIBENTRY cINFSTRUCT<T,U>::bldInfo(cVAR & info, int infs)
{
        clrInfo();

        if ((infs == -1) || (infs > info.info.vs)) infs = info.info.vs;

    for (int i = 0; i <= infs; ++i)
        cvtInfo(*info.getVal(i));

        addInfo(emptyINF);
}

#endif

        /*--------------------------------------------------------------------*/

template <class T, class U>
T * LIBENTRY cINFSTRUCT<T,U>::getInfo(void)
{
        return (infPtr ? infPtr : &emptyINF);
}

        /*--------------------------------------------------------------------*/

template <class T, class U>
void LIBENTRY cINFSTRUCT<T,U>::addInfo(T & info)
{
        T * tmpPtr = new T [ infCnt + 1 ];
        if (tmpPtr == NULL) return;

        if ((infPtr != NULL) && (infCnt > 0))
        {
                memcpy(tmpPtr,infPtr,sizeof(T)*infCnt);
                delete [] infPtr;
        }
        infPtr = tmpPtr;

        infPtr[infCnt++] = info;
}

        /*--------------------------------------------------------------------*/

#ifdef ___USE_VAR___

template <class T, class U>
void LIBENTRY cINFSTRUCT<T,U>::cvtInfo(cVARVAL & info)
{
    U value = U(info);
    cvtInfo(value);
}

#endif

/******************************************************************************/

#endif

