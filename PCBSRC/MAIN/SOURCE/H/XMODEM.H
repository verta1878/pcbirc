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


#ifndef H_XMODEM
#define H_XMODEM

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

#ifdef __OS2__
  unsigned short LIBENTRY xmodemcrc(char *Str, int Len);
  unsigned char LIBENTRY xmodemchecksum(char *Str, int Len);
#else
  #ifdef __cplusplus
    extern "C" {
  #endif
  int LIBENTRY xmodem(char *String, int Len);
  #ifdef __cplusplus
    }
  #endif  
#endif  /* ifdef __OS2__ */
#endif  /* ifndef H_XMODEM */
