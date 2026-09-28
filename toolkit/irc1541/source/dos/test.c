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


#include <stdio.h>
#include <string.h>
#include <misc.h>
#include "dosfunc.h"


void _cdecl main(void) {
  char Buf[80];
  int File;
  DOSFILE F;
  os2errtype Os2Error;

  Buf[78] = '\r';
  Buf[79] = '\n';

  if (fileexist("testopen.txt") == 255)
    puts("file does not exist");
  else
    puts("file exists");

  if ((File = doscreate("testopen.txt",OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL,&Os2Error)) != -1) {
    memset(Buf,'A',sizeof(Buf)-2);
    doswrite(File,Buf,sizeof(Buf),&Os2Error);
    dosclose(File);
  }

  if ((File = dosopen("testopen.txt",OPEN_RDWR|OPEN_DENYNONE,&Os2Error)) != -1) {
    doslseek(File,sizeof(Buf),SEEK_SET);
    memset(Buf,'B',sizeof(Buf)-2);
    doswrite(File,Buf,sizeof(Buf),&Os2Error);
    doslseek(File,0,SEEK_SET);
    dosread(File,Buf,sizeof(Buf),&Os2Error);
    doslseek(File,sizeof(Buf)*2,SEEK_SET);
    doswrite(File,Buf,sizeof(Buf),&Os2Error);
    dosclose(File);
  }

  if ((File = dosappend("testopen.txt",OPEN_RDWR|OPEN_DENYNONE,&Os2Error)) != -1) {
    memset(Buf,'C',sizeof(Buf)-2);
    doswrite(File,Buf,sizeof(Buf),&Os2Error);
    dosclose(File);
  }

  if (dosfopen("testopen.txt",OPEN_RDWR|OPEN_DENYNONE,&F) != -1) {
    dosfseek(&F,0,SEEK_END);
    memset(Buf,'D',sizeof(Buf)-2);
    dosfwrite(Buf,sizeof(Buf),&F);
    dosfseek(&F,sizeof(Buf),SEEK_SET);
    dosfread(Buf,sizeof(Buf),&F);
    dosfseek(&F,0,SEEK_END);
    dosfwrite(Buf,sizeof(Buf),&F);
    dosfclose(&F);
  }
}

