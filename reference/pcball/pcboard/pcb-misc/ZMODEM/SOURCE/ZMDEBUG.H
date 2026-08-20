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
 ***************************************************************************/

#ifndef	ZDEBUG_H
# define	ZDEBUG_H

# include <stdio.h>

# define	HEXDUMP				0x0001
# define	DISPLAY				0x0002
# define	CAPTURE				0x0004

# define	HEX_DUMP_FILE		"HEXDUMP"

# define	ZDBG_X				0
# define	ZDBG_Y				24
# define	ZDBG_ATTR			0x1F

# ifdef	__cplusplus
extern	"C"	{
# endif

extern	size_t	_Cdecl	zHexDump(void *, size_t);
extern	int		_Cdecl	zHexPrint(const char *, ...);
extern	int		_Cdecl	zDebug(const char *, ...);

# ifdef	__cplusplus
}
# endif

#endif
