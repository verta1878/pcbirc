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

#include <dir.h>
#include <stdio.h>
#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <pcb.h>
#include <help.h>
#include "setup.h"
#include "defines.h"
#include "structs.h"
#include "fidocfg.h"
#include "fidoinfo.h"
#include "fconfig.h"

#define     NUM_FIDO_FIELDS1             16
#define     FIDO_INDEX_FILE         PcbData.FidoIndex

/* If we want to have MemCheck included, we need to have the environment    */
/* variable DEBUGset.  Otherwise, MemCheck won't be compiled in.            */

#ifdef DEBUG
#include <memcheck.h>
#endif

extern pcbdattype      PcbData;
extern char            ALLTEXT[];
extern char            ALLFILE[];

extern ARCHIVERS        archiver_info;          /* archiver information     */
extern DIRECTORIES      directory_info;         /* directory information    */

/*****************************************************************************/
/* Initialize the screen fields.                                             */

void pascal near initarcfields(FldType *P)
{
  addquest(P, 0,vUPSTR,FIDOARCCFG+0 ,ALLTEXT,2,ZIP_LINE,30,        "ZIP                    ",archiver_info.archivers[0]          ,CLEAR,NULL);
  addquest(P, 1,vSTR  ,FIDOARCCFG+1 ,ALLTEXT,2,ZIP_SW_LINE,30,     "Switches for ZIP       ",archiver_info.archiver_switches[0]  ,CLEAR,NULL);
  addquest(P, 2,vUPSTR,FIDOARCCFG+2 ,ALLTEXT,2,UNZIP_LINE,30,      "UNZIP                  ",archiver_info.unarchivers[0]        ,CLEAR,NULL);
  addquest(P, 3,vSTR  ,FIDOARCCFG+3 ,ALLTEXT,2,UNZIP_SW_LINE,30,   "Switches for UNZIP     ",archiver_info.unarchiver_switches[0],CLEAR,NULL);

  addquest(P, 4,vUPSTR,FIDOARCCFG+4 ,ALLTEXT,2,ARJ_LINE,30,        "ARJ                    ",archiver_info.archivers[1]          ,CLEAR,NULL);
  addquest(P, 5,vSTR  ,FIDOARCCFG+5 ,ALLTEXT,2,ARJ_SW_LINE,30,     "Switches for ARJ       ",archiver_info.archiver_switches[1]  ,CLEAR,NULL);
  addquest(P, 6,vUPSTR,FIDOARCCFG+6 ,ALLTEXT,2,UNARJ_LINE,30,      "UNARJ                  ",archiver_info.unarchivers[1]        ,CLEAR,NULL);
  addquest(P, 7,vSTR  ,FIDOARCCFG+7 ,ALLTEXT,2,UNARJ_SW_LINE,30,   "Switches for UNARJ     ",archiver_info.unarchiver_switches[1],CLEAR,NULL);

  addquest(P, 8,vUPSTR,FIDOARCCFG+8 ,ALLTEXT,2,ARC_LINE,30,        "ARC                    ",archiver_info.archivers[2]          ,CLEAR,NULL);
  addquest(P, 9,vSTR  ,FIDOARCCFG+9 ,ALLTEXT,2,ARC_SW_LINE,30,     "Switches for ARC       ",archiver_info.archiver_switches[2]  ,CLEAR,NULL);
  addquest(P,10,vUPSTR,FIDOARCCFG+10,ALLTEXT,2,UNARC_LINE,30,      "UNARC                  ",archiver_info.unarchivers[2]        ,CLEAR,NULL);
  addquest(P,11,vSTR  ,FIDOARCCFG+11,ALLTEXT,2,UNARC_SW_LINE,30,   "Switches for UNARC     ",archiver_info.unarchiver_switches[2],CLEAR,NULL);

  addquest(P,12,vUPSTR,FIDOARCCFG+12,ALLTEXT,2,LZH_LINE,30,        "LZH                    ",archiver_info.archivers[3]          ,CLEAR,NULL);
  addquest(P,13,vSTR  ,FIDOARCCFG+13,ALLTEXT,2,LZH_SW_LINE,30,     "Switches for LZH       ",archiver_info.archiver_switches[3]  ,CLEAR,NULL);
  addquest(P,14,vUPSTR,FIDOARCCFG+14,ALLTEXT,2,UNLZH_LINE,30,      "UNLZH                  ",archiver_info.unarchivers[3]        ,CLEAR,NULL);
  addquest(P,15,vSTR  ,FIDOARCCFG+15,ALLTEXT,2,UNLZH_SW_LINE,30,   "Switches for UNLZH     ",archiver_info.unarchiver_switches[3],CLEAR,NULL);
}

/*****************************************************************************/
/* This is the entry point to this module.  It activates editing the Fido    */
/* directory and archive information.                                        */

void pascal edit_fido_arcs(void)
{
FldType         *InfoFields;
savescrntype     screenbuff;

  //if(fileexist(CONFIG_FILE)==255)
  //  make_fido_config();

  if((InfoFields=(FldType *)mallochk(NUM_FIDO_FIELDS1*sizeof(FldType)))==NULL)
    return;

  KeyFlags = 0;
  savescreen(&screenbuff);
  if(read_fido_config(0))
    {
      clscolor(Colors[OUTBOX]);
      box(0,0,79,24,0x01,DOUBLE);
      setcursor(CUR_BLANK);
      initarcfields(InfoFields);
      readscrn(InfoFields,NUM_FIDO_FIELDS1-1,0,"Fido Archiver Configuration","",1,NOCLEARFLD);
      write_fido_config();
      restorescreen(&screenbuff);
      setcursor(CUR_NORMAL);
    }
  freescrn(InfoFields,NUM_FIDO_FIELDS1-1);
}

#endif
