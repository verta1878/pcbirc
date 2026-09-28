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
#include <dosfunc.h>
#include <pcb.h>
#include <misc.h>

#ifdef DEBUG
#include <memcheck.h>
#endif

static DOSFILE ConfFile;
static DOSFILE AddConfFile;
static bool    AddConfInfo;
static bool    ConfFileOpen = FALSE;


static void _NEAR_ LIBENTRY converttopcbconftype(oldconftype * Old,
                        addconftype * Add, pcbconftype * Conf) {

  strcpy(Conf->Name,Old->Name);
  memcpy(&Conf->PublicConf,&Old->PublicConf,sizeof(*Old)-sizeof(Old->Name));

  if (AddConfInfo) {
    strcat(Conf->Name,Add->Name2);
    memcpy(&Conf->ForceEcho,Add,sizeof(*Add)-ADDCONFRESERVE);
  } else
    memset(&Conf->ForceEcho,0,sizeof(*Add)-ADDCONFRESERVE);
}


static void _NEAR_ LIBENTRY convertfrompcbconftype(oldconftype * Old,
                        addconftype * Add, pcbconftype * Conf) {

  memset(Old->Name, 0,sizeof(Old->Name));
  memset(Add->Name2,0,sizeof(Add->Name2));

  if (strlen(Conf->Name) <= 13)
    strcpy(Old->Name,Conf->Name);
  else
    memcpy(Old->Name,Conf->Name,13), strcpy(Add->Name2,Conf->Name+13);

  memcpy(&Old->PublicConf,&Conf->PublicConf,sizeof(*Old)-sizeof(Old->Name));
  memcpy( Add,            &Conf->ForceEcho, sizeof(*Add)-ADDCONFRESERVE);

  memset(Add->Reserved,0,sizeof(Add->Reserved));
}


/********************************************************************
*
*  Function:  getconfrecord()
*
*  Desc    :  Fills Conf with the contents of the CNAMES.@@@ information based
*             on the conference number passed as ConfNum.
*
*  Returns :  1 if the ConfNum is out of range.
*             0 if everything is okay
*            -1 if an error occured reading the file
*/

int LIBENTRY getconfrecord(unsigned ConfNum, pcbconftype *Conf) {
  oldconftype Old;
  addconftype Add;

  if (! ConfFileOpen)
    return(-1);

  memset(Conf,0,sizeof(pcbconftype));

  if (ConfNum > PcbData.NumConf)
    return(1);

  dosfseek(&ConfFile,((long) ConfNum*sizeof(oldconftype))+sizeof(short),SEEK_SET);
  if (dosfread(&Old,sizeof(oldconftype),&ConfFile) != sizeof(oldconftype))
    return(-1);

  /* If the CNAMES.ADD file is in use then go ahead and read it.  Otherwise */
  /* don't call it an error, the above memset() will fill it in with zeroes */
  if (AddConfInfo) {
    dosfseek(&AddConfFile,(long) ConfNum*sizeof(addconftype),SEEK_SET);
    if (dosfread(&Add,sizeof(addconftype),&AddConfFile) != sizeof(addconftype))
      return(-1);
  }
  converttopcbconftype(&Old,&Add,Conf);
  return(0);
}


int LIBENTRY putconfrecord(unsigned ConfNum, pcbconftype *Conf) {
  oldconftype Old;
  addconftype Add;

  if (! ConfFileOpen)
    return(-1);

  convertfrompcbconftype(&Old,&Add,Conf);

  dosfseek(&ConfFile,((long) ConfNum*sizeof(oldconftype))+sizeof(short),SEEK_SET);
  if (dosfwrite(&Old,sizeof(oldconftype),&ConfFile) != 0)
    return(-1);

  /* If the CNAMES.ADD file is in use then go ahead and read it.  Otherwise */
  /* don't call it an error, the above memset() will fill it in with zeroes */
  if (AddConfInfo) {
    dosfseek(&AddConfFile,(long) ConfNum*sizeof(addconftype),SEEK_SET);
    if (dosfwrite(&Add,sizeof(addconftype),&AddConfFile) != 0)
      return(-1);
  }
  return(0);
}


int LIBENTRY opencnames(void) {
  char FileName[66];

  AddConfInfo = FALSE;

  strcpy(FileName,PcbData.CnfFile);
  strcat(FileName,".@@@");
  if (dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE,&ConfFile) == -1)
    return(-1);

  ConfFileOpen = TRUE;
  if (strstr(PcbData.Version,"15.0") == NULL)
    return(0);

  /* Version 15.0 has a new CNAMES.ADD file.  Open it, if it is available */
  /* but if not, don't call it an error because it might be v14.5 instead */
  strcpy(FileName,PcbData.CnfFile);
  strcat(FileName,".ADD");
  if (fileexist(FileName) != 255 && dosfopen(FileName,OPEN_RDWR|OPEN_DENYNONE,&AddConfFile) != -1)
    AddConfInfo = TRUE;

  return(0);
}


void LIBENTRY closecnames(void) {
  dosfclose(&ConfFile);
  if (AddConfInfo)
    dosfclose(&AddConfFile);
  ConfFileOpen = FALSE;
}
