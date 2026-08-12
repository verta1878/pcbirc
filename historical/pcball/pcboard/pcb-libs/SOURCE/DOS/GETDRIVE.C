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


#ifdef __BORLANDC__
  #include <dir.h>
#else
  #include <dos.h>
#endif

#include <types.hpp>
#include <dosfunc.h>

/* returns 1 for A, 2 for B, 3 for C and so on */

unsigned LIBENTRY dosgetcurdrive(void) {
  #ifdef __BORLANDC__
    return(getdisk()+1);
  #else
    unsigned CurDisk;
    _dos_getdrive(&CurDisk);
    return(CurDisk);
  #endif
}
