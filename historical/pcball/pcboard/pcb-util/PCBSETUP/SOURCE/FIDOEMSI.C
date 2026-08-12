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

#include <string.h>
#include <dir.h>

#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
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

#ifdef DEBUG
#include <memcheck.h>
#endif

extern far EMSI_DATA  emsi_data;

char  Phone[5]={4,0,'0','9','-'};

/*****************************************************************************/
/* Initialize the screen fields.                                             */

static void near pascal init_emsi_fields(FldType *P) {
  addquest(P, 0,vSTR  ,EMSICFG+0,ALLCHAR, 3, 7,sizeof(emsi_data.BBS_Name)-1-14,Questions[FLDFIDOEMSI+ 0],emsi_data.BBS_Name,CLEAR,NULL);
  addquest(P, 1,vSTR  ,EMSICFG+1,ALLCHAR, 3, 8,sizeof(emsi_data.Sysop)-1      ,Questions[FLDFIDOEMSI+ 1],emsi_data.Sysop   ,CLEAR,NULL);
  addquest(P, 2,vSTR  ,EMSICFG+2,ALLCHAR, 3, 9,sizeof(emsi_data.City)-1       ,Questions[FLDFIDOEMSI+ 2],emsi_data.City    ,CLEAR,NULL);
  addquest(P, 3,vSTR  ,EMSICFG+3,Phone  , 3,10,sizeof(emsi_data.Phone)-1-20   ,Questions[FLDFIDOEMSI+ 3],emsi_data.Phone   ,CLEAR,NULL);
  addquest(P, 4,vSTR  ,EMSICFG+4,ALLNUM , 3,11,sizeof(emsi_data.Baud)-1       ,Questions[FLDFIDOEMSI+ 4],emsi_data.Baud    ,CLEAR,NULL);
  addquest(P, 5,vUPSTR,EMSICFG+5,ALLCHAR, 3,12,sizeof(emsi_data.Flags)-1-4    ,Questions[FLDFIDOEMSI+ 5],emsi_data.Flags   ,CLEAR,NULL);
}

/*****************************************************************************/
/* This is the entry point to this module.                                   */

void pascal edit_fido_emsi(void)
{
  FldType         *InfoFields;
  savescrntype     screenbuff;

  //if(fileexist(CONFIG_FILE)==255)
  //  make_fido_config();

  if((InfoFields=(FldType *)mallochk(NUMFIDOEMSI*sizeof(FldType)))==NULL)
    return;

  KeyFlags = 0;
  savescreen(&screenbuff);
  if(read_fido_config(0))
    {
      clscolor(Colors[OUTBOX]);
      box(0,0,79,24,Colors[OUTBOX],DOUBLE);
      setcursor(CUR_BLANK);
      init_emsi_fields(InfoFields);
      readscrn(InfoFields,NUMFIDOEMSI-1,0,"Fido EMSI Profile Configuration","",1,NOCLEARFLD);
      write_fido_config();
      restorescreen(&screenbuff);
      setcursor(CUR_NORMAL);
    }
  freescrn(InfoFields,NUMFIDOEMSI-1);
  KeyFlags = ESC;
}

#endif
