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


#ifdef FIDO

#include <stdio.h>
#include <string.h>
#include <dir.h>

#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <data.hpp>
#include <misc.h>
#include <pcb.h>
#include <help.h>

#include "setup.h"
#include "setup.ext"
#include "defines.h"
#include "structs.h"
#include "fidocfg.h"
#include "fidoinfo.h"
#include "fconfig.h"

#define     FIDO_INDEX_FILE         PcbData.FidoIndex

#ifdef DEBUG
#include <memcheck.h>
#endif

extern   bool            setfido;
extern   char            ALLTEXT[];
extern   char            ALLFILE[];
extern   int             NumExitKeys;
extern   char            ExitKeyNum[];
extern   int             ExitKeyFlag[];
extern   pcbconftype     Conf;
extern   pcbdattype      PcbData;
extern   unsigned        ConfSel;
extern   int             num_areas;

extern      unsigned int    ConfigFileVer;

char                        FIDOALLNUM[]= { 0, '0', '9' };
char                        FIDOADDR[]= {6, 0,'0','9', ':','/','.'};
extern      char            ALLCHAR[];

static  _FAR_    NAREA_STRUCT    Fido_Conference;
static  _FAR_    bool            NewRecord=FALSE;
static           unsigned int    oldrecnum = 0;
static  _FAR_    NAREA_STRUCT    oldrec;

/*****************************************************************************/
/* Initialize the screen fields.                                             */

static void pascal near initcfgfields(FldType *P) {
  addquest(P, 0,vUPSTR,FIDOAREACFG+0,ALLCHAR      , 2, 7,54,"Fido Area Name      ", Fido_Conference.AreaTag,        NOCLEARFLD,NULL);
  addquest(P, 1,vCHAR, FIDOAREACFG+2,YESNO        , 2, 8, 1,"Allow Private mail  ",&Fido_Conference.AllowPrivate,   NOCLEARFLD,NULL);
  addquest(P, 2,vCHAR, FIDOAREACFG+3,YESNO        , 2, 9, 1,"Allow File Attach   ",&Fido_Conference.AllowFileAttach,NOCLEARFLD,NULL);
  addquest(P, 3,vCHAR, FIDOAREACFG+4,HighAsciiMask, 2,10, 1,"High Ascii (S/R/C/N)",&Fido_Conference.HighAscii,      NOCLEARFLD,NULL);
}

/*****************************************************************************/
/* This is the entry point to this module.  It activates editing the Fido    */
/* directory and archive information.                                        */

void pascal Configure_Fido(void) {
  FldType         *CfgFields;
  savescrntype     screenbuff;
  char             workbuff[80];
  cAKAS            akas;
  cORIGINS         origins;
  NADDRESS         addr;
  ORIGIN           origin;


  if((CfgFields=(FldType *)mallochk(NUMFIDOAREA*sizeof(FldType)))==NULL)
    return;

  KeyFlags = 0;
  savescreen(&screenbuff);
  if(read_fido_config(0))
    {
      get_fido_record();
      addr   = akas.getRec(Fido_Conference.AkaIndex);
      origin = origins.getRec(Fido_Conference.OriginIndex);
      clscolor(Colors[OUTBOX]);
      box(0,0,79,24,Colors[OUTBOX],DOUBLE);
      setcursor(CUR_BLANK);

      Fido_Conference.ConfNum=ConfSel;
      fastprint(2,3,"PCBoard Conference   :",Colors[QUESTION]);

      sprintf(workbuff,"%d",Fido_Conference.ConfNum);
      fastprint(25,3,workbuff,Colors[ANSWER]);

      fastprint(2,4,"Default AKA          :",Colors[QUESTION]);

      if(addr.point == 0) sprintf(workbuff,"%u:%u/%u",addr.zone,
                                                         addr.net,
                                                         addr.node);
      else  sprintf(workbuff,"Default AKA          : %u:%u/%u.%u",addr.zone,
                                                         addr.net,
                                                         addr.node,addr.point);
      fastprint(25,4,workbuff,Colors[ANSWER]);


      fastprint(2,5,"Default Origin       :",Colors[QUESTION]);
      fastprint(25,5,origin.origin,Colors[ANSWER]);

      initcfgfields(CfgFields);
      readscrn(CfgFields,NUMFIDOAREA-1,0,"Fidonet Area Configuration","",1,NOCLEARFLD);

      write_fido_conference();
      free_fido_memory();
      restorescreen(&screenbuff);
      setcursor(CUR_NORMAL);
    }
  freescrn(CfgFields,NUMFIDOAREA-1);
}

/*****************************************************************************/
/* Get the correct Area record.                                              */

void get_fido_record(void)
{

cAREAS       areas;
cAKAS        akas;
cORIGINS     origins;
NAREA_STRUCT mtrec;

  memset(&Fido_Conference,0,sizeof(AREA_STRUCT));
  memset(&mtrec,0,sizeof(mtrec));
  Fido_Conference = areas.getAreaRec(ConfSel);
  oldrecnum = areas.findRec(Fido_Conference.AreaTag);
  if(memcmp(&Fido_Conference,&mtrec,sizeof(NAREA_STRUCT)) == 0)
    NewRecord = TRUE;
  else
  {
    NewRecord = FALSE;

    if(Fido_Conference.AkaIndex == 0 || Fido_Conference.AkaIndex > akas.totRecs()) Fido_Conference.AkaIndex = 1;

    if(Fido_Conference.OriginIndex == 0 || Fido_Conference.OriginIndex > origins.totRecs()) Fido_Conference.OriginIndex = 1;
  }
  memcpy(&oldrec,&Fido_Conference,sizeof(oldrec));
}

/****************************************************************************/
/* Write the fido conference record out to disk.                            */

void write_fido_conference(void)
{

cAREAS        areas;
NAREA_STRUCT  rec,tmprec;
cAKAS         akas;
cORIGINS      origins;
unsigned int  recnum = 0;


    memcpy(&rec,&Fido_Conference,sizeof(rec));

    if(memicmp(&rec,&oldrec,sizeof(rec)) == 0) return;

    if( (recnum = areas.findRec(rec.AreaTag)) == 0)
    {
        if(oldrecnum != 0) areas.removeRec(oldrecnum);
        areas.addRec(rec);
    }
    else
        areas.modifyRec(rec,recnum);

    areas.sortFile();
}

#endif
