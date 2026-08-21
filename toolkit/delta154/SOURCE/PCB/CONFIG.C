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
#include <string.h>
#include "misc.h"
#include "dosfunc.h"
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

typedef struct {
  char Dev[13];
  char TextEdit[36];
  char GrphEdit[36];
} oldConfigType;


void LIBENTRY writeconfig(smConfigType *smConfig, char *ConfigName) {
  int Config;

  unlink(ConfigName);
  if ((Config = doscreatecheck(ConfigName,OPEN_WRIT,OPEN_NORMAL)) != -1) {
    writecheck(Config,smConfig,sizeof(smConfigType));
    dosclose(Config);
  }
}


void LIBENTRY getsmconfig(smConfigType *smConfig, char *ConfigName) {
  int Config;
  int BytesRead;
  oldConfigType Old;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  memset(smConfig,0,sizeof(smConfigType));
  if ((Config = dosopen(ConfigName,OPEN_READ POS2ERROR)) != -1) {
    BytesRead = dosread(Config,smConfig,sizeof(smConfigType) POS2ERROR);
    dosclose(Config);
    switch (BytesRead) {
      case sizeof(smConfigType) : return;  /* no need to write config out */
      case sizeof(oldConfigType): memcpy(&Old,smConfig,sizeof(oldConfigType));
                                  memset(smConfig,0,sizeof(smConfigType));
                                  strcpy(smConfig->Dev     ,Old.Dev);
                                  strcpy(smConfig->TextEdit,Old.TextEdit);
                                  strcpy(smConfig->GrphEdit,Old.GrphEdit);
                                  break;
      default                   : strcpy(smConfig->Dev,"prn");
                                  strcpy(smConfig->TextEdit,"PCBEDIT.EXE");
                                  strcpy(smConfig->GrphEdit,"PCBEDIT.EXE");
                                  break;
    }
  } else {
    strcpy(smConfig->Dev,"prn");
    strcpy(smConfig->TextEdit,"PCBEDIT.EXE");
    strcpy(smConfig->GrphEdit,"PCBEDIT.EXE");
  }
  writeconfig(smConfig,ConfigName);
}


