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


#ifndef H_MD5
#define H_MD5

#if defined(MULTIPORT) && ! defined(LIB)

#ifdef __OS2__
  extern "C" void pascal md5String32(char *Str, long Bytes, char *Buf);
  #define md5String(Str,Bytes,Buf) md5String32(Str,Bytes,Buf)
#else
  extern "C" void pascal md5String(char *Str, int Bytes, char *Buf);
#endif

extern char ReceivedMd5String[35];

#else   /* MULTIPORT */

#define md5String(Str,Bytes,Buf)

#endif  /* MULTIPORT */
#endif  /* H_MD5 */
