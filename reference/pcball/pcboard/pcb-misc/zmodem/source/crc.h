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

#ifndef CRC_H
# define        CRC_H

# define        updcrc16(b,c)           (crc16tab[(((int)(c) >> 8) & 0xFF)] ^ ((c) << 8) ^ (b))
# define        updcrc32(b,c)           (crc32tab[((int)(c) ^ (b)) & 0xFF] ^ (((c) >> 8) & 0x00FFFFFFL))

# ifndef        updcrc16
extern  unsigned short  _Cdecl  updcrc16(int, unsigned short);
# endif
# ifndef        updcrc32
extern  unsigned long   _Cdecl  updcrc32(int, unsigned long);
# endif

extern  unsigned short  crc16tab[256];
extern  unsigned long   crc32tab[256];

#endif
