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
 * Copyright (c) 1993   All Rights Reserved.
 * Clark Development Company, Inc.
 *
 * $Revision:   1.3  $
 *
 * $Log:   E:/tc/pcbpack/vcs/addext.c_v  $
 * 
 *    Rev 1.3   20 Sep 1995 18:40:48   DWT
 * Changed to LIBENTRY
 *
 *    Rev 1.2   11 Nov 1993 10:43:42   DWT
 * Changed to LIBENTRY
 *
 *    Rev 1.1   13 Oct 1993 09:24:46   DWT
 * changed the way memcheck is used
 *
 *    Rev 1.0   19 Apr 1993 11:58:14   LDZ
 *  - None
 *
 ***************************************************************************/

#include <string.h>
#include "pcbpack.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif


/***************************************************************************
 *** Add an extension to a filename ****************************************/
int LIBENTRY AddExtension(char *Filename, char *Ext) {
    char    *ext,
            *path;

    path = Filename;
    if ((path = strrchr(Filename, '\\')) != NULL) {
        path++;
        Filename = path;
    }
    if ((ext = strchr(Filename, '.')) != NULL)
        *ext = 0;
    strcat(Filename, ".");
    strcat(Filename, Ext);

    return 1;
}
