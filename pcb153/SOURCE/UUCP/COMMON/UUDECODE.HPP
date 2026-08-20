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



#ifndef ___UUDECODE_HPP___

#define ___UUDECODE_HPP___

#include    <dosclass.hpp>

void pascal decodeLine ( char * i, char * o, int & s );

int  pascal decodeLineCheck ( char * line );

void pascal decodeFile ( char * tpath, char * fname, cDOSFILE & ifile );

int  pascal decodeFileStart ( char * tpath, char * fname );
void pascal decodeFileLine  ( char * line );
void pascal decodeFileStop  ( void );

#endif

