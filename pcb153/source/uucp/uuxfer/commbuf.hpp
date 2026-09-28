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



#ifndef ___COMMBUF_HPP___

#define ___COMMBUF_HPP___

#include    <uucp.hpp>

//int  pascal getCommBuf   ( void * buf, int size );
//
//int  pascal ungetCommBuf ( void * buf, int size, int off );

void * pascal getCommPtr   ( int size );
void   pascal ungetCommPtr ( int off );
void   pascal syncCommPtr  ( void );

void pascal putCommBuf   ( void * buf, int size );

inline void pascal putCommBuf ( void * buf, int size )
{
    sendstr((char*)buf,size);
}

#endif

