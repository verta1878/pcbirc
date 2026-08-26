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


#ifndef H_WILD
#define H_WILD

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

bool LIBENTRY comparewildcards(char *Criteria, char *FileName);

/* these functions are in CUTIL.ASM for DOS and WILD.C for OS/2 so the */
/* names cannot be mangled except under OS/2 */

#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif

/* prototypes from CUTIL.ASM for DOS or WILD.C for OS/2 */
void LIBENTRY makeidxname(char *FileName, char *IdxStr);
void LIBENTRY criteria(char *FileName, char *Criteria);
bool LIBENTRY wildmatch(char *FileName, char *Criteria);

#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif

#endif  /* ifndef H_WILD */
