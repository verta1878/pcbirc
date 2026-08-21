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


#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define EOFCHAR 0x1A

/********************************************************************
*
*  Function:  dosappend()
*
*  Desc    :  opens an existing file for append or creates a new file
*
*  Returns :  File Handle when successful
*             Or, -1 when an error occurs in which case ExtendedErrror and
*             ExtendedAction are set by calling DOS Get Extended Error
*
* NOTES    :  This function REQUIRES read-access to the file to ensure that
*             all EOF markers are stripped from the file.
*/

int LIBENTRY dosappend(char *FileName, int OpenFlags DOS2ERROR) {
  int  MaxRead;
  int  Handle;
  char *p;
  long Len;
  char Buffer[256];

  if (fileexist(FileName) == 255)
    Handle = doscreate(FileName,OpenFlags,OPEN_NORMAL POS2ERROR2);
  else
    Handle = dosopen(FileName,OpenFlags POS2ERROR2);

  if (Handle == -1)
    return(-1);

  Len = doslseek(Handle,0,SEEK_END);

  while (Len > 0) {
    if (Len < sizeof(Buffer))
      MaxRead = (int) Len;
    else
      MaxRead = sizeof(Buffer);

    doslseek(Handle,Len-MaxRead,SEEK_SET);

    if ((MaxRead = dosread(Handle,Buffer,MaxRead POS2ERROR2)) < 1)
      break;
    if ((p = strnchr(Buffer,EOFCHAR,MaxRead)) == NULL)
      break;
    if (p != Buffer) {
      Len -= (MaxRead - (p - Buffer));
      break;
    }
    Len -= MaxRead;
  }

  doslseek(Handle,Len,SEEK_SET);
  dostrunc(Handle,-1 POS2ERROR2);
  return(Handle);
}


/*
void main(void) {
  int File;

  if ((File = dosappend("testfile",OPEN_RDWR|OPEN_DENYNONE)) == -1)
    return;

  doswrite(File,"TEST\r\n",6);
  dosclose(File);
}
*/
