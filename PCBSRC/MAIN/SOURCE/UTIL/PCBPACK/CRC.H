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


/***************************************************************************
 *
 * Copyright (c) 1992, 1993
 * Clark Development Company, Inc.  All Rights Reserved.
 *
 * $Revision:   1.1  $
 *
 * $Log:   E:/tc/pcbpack/vcs/crc.h_v  $
 * 
 *    Rev 1.1   20 Sep 1995 18:41:24   DWT
 * Added an extern "C" declaration.
 * 
 *    Rev 1.0   19 Aug 1993 19:32:02   LDZ 
 * Initial revision.
 * 
 ***************************************************************************/

#ifndef	CRC_H
# define	CRC_H

# define    updcrc16(b,c)   (crc16tab[(((c) >> 8) & 0xFF)] ^ ((c) << 8) ^ (b))
# define    updcrc32(b,c)   (crc32tab[((int)(c) ^ (b)) & 0xFF] ^ (((c) >> 8) & 0x00FFFFFFL))

#ifdef __cplusplus
#ifdef __cplusplus
extern "C" {
#endif
#endif
extern  unsigned short  updblkcrc16(char *, int, unsigned short);
extern  unsigned long   updblkcrc32(char *, int, unsigned long);
#ifdef __cplusplus
}
#endif

extern  unsigned short  crc16tab[256];
extern  unsigned long   crc32tab[256];
#endif
