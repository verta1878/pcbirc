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

#include <stdlib.h>
#include <dir.h>

#include <misc.h>
#include <pcb.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <string.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fidothis.h"
#include "fidocfg.h"
#include "fconfig.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       24

extern far FREQ_INFO   freq_info;

char  mask_listed[]={8,'A','U','N','L','a','u','n','l'};

/*****************************************************************************/
/* Initialize the screen fields.                                             */

static void pascal near initfreqfields(FldType *P) {
  addquest(P, 0,vUNSIGNED,FREQRESTRICT+0,ALLNUM     ,3, 7,5 ,Questions[FLDFIDOFREQ+ 0],&freq_info.stime ,CLEAR,NULL);
  addquest(P, 1,vUNLONG  ,FREQRESTRICT+1,ALLNUM     ,3, 8,10,Questions[FLDFIDOFREQ+ 1],&freq_info.sbytes,CLEAR,NULL);
  addquest(P, 2,vUNSIGNED,FREQRESTRICT+2,ALLNUM     ,3, 9,5 ,Questions[FLDFIDOFREQ+ 2],&freq_info.dtime ,CLEAR,NULL);
  addquest(P, 3,vUNLONG  ,FREQRESTRICT+3,ALLNUM     ,3,10,10,Questions[FLDFIDOFREQ+ 3],&freq_info.dbytes,CLEAR,NULL);
  addquest(P, 4,vUPSTR   ,FREQRESTRICT+4,mask_listed,3,11,1 ,Questions[FLDFIDOFREQ+ 4],&freq_info.listed,CLEAR,NULL);
  addquest(P, 5,vUNSIGNED,FREQRESTRICT+5,ALLNUM     ,3,12,8 ,Questions[FLDFIDOFREQ+ 5],&freq_info.baud  ,CLEAR,NULL);
}

/*****************************************************************************/
/* This is the entry point to this module.  It activates editing the Fido    */
/* directory and archive information.                                        */

void pascal edit_fido_freq(void)
{
  FldType         *InfoFields;
  savescrntype     screenbuff;

  //if(fileexist(CONFIG_FILE)==255)
  //  make_fido_config();

  if((InfoFields=(FldType *)mallochk(NUMFIDOFREQ*sizeof(FldType)))==NULL)
    return;

  KeyFlags = 0;
  savescreen(&screenbuff);
  if(read_fido_config(0))
    {
      clscolor(Colors[OUTBOX]);
      box(0,0,79,24,Colors[OUTBOX],DOUBLE);
      setcursor(CUR_BLANK);
      initfreqfields(InfoFields);
      clsbox(1,BOTTOM-2,78,BOTTOM-1,Colors[DESC]);
      fastcenter(BOTTOM-2,"Valid values for allowed nodes.",Colors[DESC]);
      fastcenter(BOTTOM-1,"A=All, U=Userlist, N=Nodelist, L=Listed (Userlist or Nodelist)",Colors[DESC]);
      readscrn(InfoFields,NUMFIDOFREQ-1,0,"Fido FREQ restrictions","",1,NOCLEARFLD);
      write_fido_config();
      restorescreen(&screenbuff);
      setcursor(CUR_NORMAL);
    }
  freescrn(InfoFields,NUMFIDOFREQ-1);
  KeyFlags = ESC;
}


#endif
