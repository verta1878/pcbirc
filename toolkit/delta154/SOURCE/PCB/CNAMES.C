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
  #include <alloc.h>
#else
  #include <malloc.h>
#endif

#ifndef __OS2__
  #include "model.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "dosfunc.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#if defined(__OS2__)
  #define farmalloc(x) malloc(x)
  #define farfree(x)   free(x)
#endif


#define CACHESIZE 16    /* MUST be a power of 2! */


typedef enum {MEMORY,CACHE,DIRECT} accesstype;

typedef struct {
  unsigned ConfNum;
  unsigned Offset;
} cachetype;

accesstype  static Access;
unsigned    static NumRecs;
bool        static PcbSetup;
unsigned    static LastOffset;
cachetype   static Cache[CACHESIZE];
DOSFILE     static ConfFile;
DOSFILE     static AddConfFile;
pcbconftype static _FAR_ *ConfList = NULL;


static void _NEAR_ LIBENTRY converttopcbconftype(oldconftype * Old, addconftype * Add, pcbconftype * Conf) {
  strcpy(Conf->Name,Old->Name);
  strcat(Conf->Name,Add->Name2);

  memcpy(&Conf->PublicConf,&Old->PublicConf,sizeof(*Old)-sizeof(Old->Name));
  memcpy(&Conf->ForceEcho,  Add,            sizeof(*Add)-ADDCONFRESERVE);

  /* override..  if they have force echo turned on, then make sure */
  /* that the main echo mail switch is also enabled                */
  if (Conf->ForceEcho)
    Conf->EchoMail = TRUE;
}


static void _NEAR_ LIBENTRY convertfrompcbconftype(oldconftype * Old, addconftype * Add, pcbconftype * Conf) {
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


int LIBENTRY putconfrecord(unsigned ConfNum, pcbconftype *Conf) {
  oldconftype Old;
  addconftype Add;

  #ifdef DEBUG
    mc_register(&Old,sizeof(Old));
    mc_register(&Add,sizeof(Add));
  #endif
/*
  memcpy(Old.Name,Conf,sizeof(oldconftype));
  memcpy(&Add,&Conf->ForceEcho,ADDCONFSIZE);
  memset(Add.Reserved,0,sizeof(Add.Reserved));
*/
  convertfrompcbconftype(&Old,&Add,Conf);

  dosflush(&ConfFile);  /* clear junk out of the buffer */
  dosfseek(&ConfFile,((long) ConfNum*sizeof(oldconftype))+sizeof(short),SEEK_SET);
  dosfwrite(&Old,sizeof(oldconftype),&ConfFile);

  dosflush(&AddConfFile);
  dosfseek(&AddConfFile,(long) ConfNum * sizeof(addconftype),SEEK_SET);
  dosfwrite(&Add,sizeof(addconftype),&AddConfFile);

  if (ConfNum >= NumRecs) {
    if (ConfNum > NumRecs) {
      memset(&Old,0,sizeof(oldconftype));
      memset(&Add,0,sizeof(addconftype));
      dosfseek(&ConfFile,((long) NumRecs*sizeof(oldconftype))+sizeof(short),SEEK_SET);
      dosfseek(&AddConfFile,(long) NumRecs*sizeof(addconftype),SEEK_SET);
      for (; NumRecs < ConfNum; NumRecs++) {
        dosfwrite(&Old,sizeof(oldconftype),&ConfFile);
        dosfwrite(&Add,sizeof(addconftype),&AddConfFile);
      }
    }
    NumRecs = ConfNum + 1;
  }
  dosflush(&ConfFile);  /* write it to disk now */
  dosflush(&AddConfFile);  /* write it to disk now */

  #ifdef DEBUG
    mc_unregister(&Old);
    mc_unregister(&Add);
  #endif
  return 0;
}


int LIBENTRY getconfrecord(unsigned ConfNum, pcbconftype *Conf) {
  oldconftype Old;
  addconftype Add;
  cachetype *p;

  if (ConfNum >= NumRecs) {
    memset(Conf,0,sizeof(pcbconftype));
    if (PcbSetup)
      putconfrecord(ConfNum,Conf);
    return 0;
  }

  switch(Access) {
    case MEMORY: farmemcpy(Conf,&ConfList[ConfNum],sizeof(pcbconftype));
                 return 0;
    case DIRECT: dosfseek(&ConfFile,((long) ConfNum*sizeof(oldconftype))+sizeof(short),SEEK_SET);
                 dosfread(&Old,sizeof(oldconftype),&ConfFile);
                 dosfseek(&AddConfFile,(long) ConfNum*sizeof(addconftype),SEEK_SET);
                 if (dosfread(&Add,sizeof(addconftype),&AddConfFile) != sizeof(addconftype))
                   memset(&Add,0,sizeof(addconftype));
                 converttopcbconftype(&Old,&Add,Conf);
                 return 0;
    case CACHE : for (p = Cache; p < &Cache[CACHESIZE]; p++) {
                   if (p->ConfNum == ConfNum) {
                     farmemcpy(Conf,&ConfList[p->Offset],sizeof(pcbconftype));
                     return 0;
                   }
                 }
                 memmove(&Cache[1],Cache,(CACHESIZE-1)*sizeof(cachetype));
                 dosfseek(&ConfFile,((long) ConfNum*sizeof(oldconftype))+sizeof(short),SEEK_SET);
                 dosfread(&Old,sizeof(oldconftype),&ConfFile);
                 dosfseek(&AddConfFile,(long) ConfNum*sizeof(addconftype),SEEK_SET);
                 if (dosfread(&Add,sizeof(addconftype),&AddConfFile) != sizeof(addconftype))
                   memset(&Add,0,sizeof(addconftype));
                 converttopcbconftype(&Old,&Add,Conf);
                 farmemcpy(&ConfList[LastOffset],Conf,sizeof(pcbconftype));
                 Cache[0].ConfNum = ConfNum;
                 Cache[0].Offset  = LastOffset;
                 LastOffset++;
                 LastOffset &= (CACHESIZE-1);  /* wrap around to zero */
                 return 0;
  }
  return 0;
}


void LIBENTRY resetconffile(void) {
  dosflush(&ConfFile);
  dosflush(&AddConfFile);
  NumRecs = (unsigned) ((dosfseek(&ConfFile,0,SEEK_END) - sizeof(short)) / sizeof(oldconftype));

  if (NumRecs > PcbData.NumAreas) {
    NumRecs = PcbData.NumAreas;
    /* truncate the file */
    dosftrunc(&ConfFile,sizeof(short)+ ((long)PcbData.NumAreas * sizeof(oldconftype)));
    dosrewind(&ConfFile);
    /* truncate the file */
    dosftrunc(&AddConfFile,((long)PcbData.NumAreas * sizeof(addconftype)));
    dosrewind(&AddConfFile);
  }
  dosfseek(&ConfFile,sizeof(short),SEEK_SET);
  dosrewind(&AddConfFile);
}



/*
     IS THIS FUNCTION NEEDED ANY MORE?
     SDR NOTE : This function not modified for new conference info on 06/03/94

void LIBENTRY readconfrecord(pcbconftype *Conf) {
  addconftype Temp;

  if (dosfread(Conf,sizeof(oldconftype),&ConfFile) != sizeof(oldconftype))
    memset(Conf,0,sizeof(oldconftype));

  if (dosfread(&Temp,sizeof(addconftype),&AddConfFile) != sizeof(addconftype))
    memset(&Temp,0,sizeof(addconftype));

  memcpy(&Conf->ForceEcho,&Temp.ForceEcho,ADDCONFSIZE);
}
*/



void LIBENTRY writeconfrecord(pcbconftype *Conf) {
  oldconftype Old;
  addconftype Add;

  convertfrompcbconftype(&Old,&Add,Conf);

/*
  // SDR NOTE : This function not modified for new conference info on 06/03/94
  addconftype Temp;
  memcpy(&Temp.ForceEcho,&Conf->ForceEcho,ADDCONFSIZE);
  memset(Temp.Reserved,0,sizeof(Temp.Reserved));
  dosfwrite(&Temp,sizeof(addconftype),&AddConfFile);
*/

  /* this is all that is needed since DATAFILE.C is the only module that  */
  /* calls this function and it is only using it to update the CNAMES.@@@ */
  /* file because CNAMES only has the OLD information in it.              */
  dosfwrite(&Old,sizeof(oldconftype),&ConfFile);
}


void closecnames(void) {
  dosfclose(&ConfFile);
  dosfclose(&AddConfFile);
  if (ConfList != NULL) {
    farfree(ConfList);
    ConfList = NULL;
  }
}


static void _NEAR_ LIBENTRY createcnamesadd(void) {
  unsigned    X;
  char        *p;
  addconftype Add;
  pcbconftype Conf;

  X = (unsigned) (dosfseek(&AddConfFile,0,SEEK_END) / sizeof(addconftype));

  dosfseek(&ConfFile,((long) X*sizeof(oldconftype))+sizeof(short),SEEK_SET);

  for (; X < NumRecs; X++) {
    dosfread(&Conf,sizeof(oldconftype),&ConfFile);
    memset(&Add,0,sizeof(addconftype));

    if ((p = strrchr(Conf.MsgFile,'\\')) != NULL)
      memcpy(Add.AttachLoc,Conf.MsgFile,(int) (p-Conf.MsgFile) + 1);

    if (X == 0 || (Conf.PublicConf && Conf.ReqSecLevel <= 0))
      memcpy(Add.RegFlags,"RXS",3);

    dosfwrite(&Add,sizeof(addconftype),&AddConfFile);
  }
  dosflush(&AddConfFile);
}


static void cdecl_closecnames(void) { closecnames(); }
void LIBENTRY loadcnames(bool Setup) {
  int         Mode;
  int         BufSize;
  unsigned short X;
  pcbconftype _FAR_ *p;
  char        FileName1[66];
  char        FileName2[66];
  oldconftype Old;
  addconftype Add;
  pcbconftype Conf;

  closecnames();

  strcpy(FileName1,PcbData.CnfFile);
  strcpy(FileName2,PcbData.CnfFile);
  strcat(FileName1,".@@@");
  strcat(FileName2,".ADD");

  PcbSetup = Setup;   /* setup can call putconfrecord() from getconfrecord() */

  if (Setup) {
    Mode   = OPEN_RDWR|OPEN_DENYNONE;
    Access = DIRECT;
  } else {
    if (PcbData.FastCnames && ! PcbData.EnableFido) {
      Mode   = OPEN_READ|OPEN_DENYNONE;
      Access = ((long) PcbData.NumAreas*sizeof(pcbconftype) < 64000L ? MEMORY : CACHE);
    } else {
      Mode   = OPEN_RDWR|OPEN_DENYNONE;
      Access = DIRECT;
    }
  }

  if (Setup || Access == MEMORY)
    BufSize = 8192;
  else
    BufSize = 2048;

  if (Setup && fileexist(FileName1) == 255) {
top:
    if (dosfopen(FileName1,Mode+OPEN_CREATE,&ConfFile) == -1)
      errorexittodos("Unable to open CNAMES.@@@ file");
    if (dosfopen(FileName2,Mode+OPEN_CREATE,&AddConfFile) == -1)
      errorexittodos("Unable to open CNAMES.ADD file");
    dossetbuf(&ConfFile,BufSize);
    dossetbuf(&AddConfFile,BufSize);
    X = sizeof(oldconftype);
    dosfwrite(&X,sizeof(short),&ConfFile);
    NumRecs = 0;
  } else {
    if (dosfopen(FileName1,Mode,&ConfFile) == -1)
      errorexittodos("Unable to open CNAMES.@@@ file");
    if (dosfopen(FileName2,Mode,&AddConfFile) == -1)
      errorexittodos("Unable to open CNAMES.ADD file");
    dossetbuf(&ConfFile,BufSize);
    dossetbuf(&AddConfFile,BufSize);
    NumRecs = (unsigned) ((dosfseek(&ConfFile,0,SEEK_END) - sizeof(short)) / sizeof(oldconftype));
    dosrewind(&ConfFile);
    dosfread(&X,sizeof(short),&ConfFile);
    if (X != sizeof(oldconftype)) {
      dosfclose(&ConfFile);
      dosfclose(&AddConfFile);
      if (Setup)
        goto top;
      errorexittodos("CNAMES.@@@ file is formatted wrong - run PCBSETUP!");
    }
  }

  if (Setup && (dosfseek(&AddConfFile,0,SEEK_END) / sizeof(addconftype)) != NumRecs) {
    createcnamesadd();
    dosrewind(&AddConfFile);
  }

  atexit(cdecl_closecnames);

  switch(Access) {
    case MEMORY : if ((ConfList = (pcbconftype _FAR_ *) farmalloc((long) PcbData.NumAreas*sizeof(pcbconftype))) != NULL) {
                    for (X = 0, p = ConfList; X < PcbData.NumAreas; X++, p++) {
                      dosfread(&Old,sizeof(oldconftype),&ConfFile);
                      dosfread(&Add,sizeof(addconftype),&AddConfFile);
                      converttopcbconftype(&Old,&Add,&Conf);
                      *p = Conf;
                    }
                    dosfclose(&ConfFile);
                    dosfclose(&AddConfFile);
                    return;
                  }
                  Access = CACHE;
                  /* drop thru */
    case CACHE  : if ((ConfList = (pcbconftype _FAR_ *) farmalloc(CACHESIZE*sizeof(pcbconftype))) != NULL) {
                    fmemset(ConfList,0,CACHESIZE*sizeof(pcbconftype));
                    memset(Cache,0xFFFF,sizeof(Cache));
                    LastOffset = 0;
                    return;
                  }
                  Access = DIRECT;
                  return;
  }
}
