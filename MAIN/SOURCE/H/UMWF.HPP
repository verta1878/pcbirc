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



#ifndef ___UMWF_HPP___

#define ___UMWF_HPP___

/******************************************************************************/

#include    "types.hpp"

/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void LIBENTRY initUMWF  ( void );
void LIBENTRY resetUMWF ( void );

void LIBENTRY doUMWF    ( void );
void LIBENTRY addUMWF   ( char * name, unsigned conf );

#ifdef __cplusplus
}
#endif

/******************************************************************************/

#endif

