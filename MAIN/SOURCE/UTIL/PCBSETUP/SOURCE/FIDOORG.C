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


/****************************************************************************/
/* Filename: FIDOorg.C                                                     */
/*                                                                          */
/* This module goes into PCBSetup to help configure the Fido tosser         */
/* programs.  It configures the origins of the system.                      */
/****************************************************************************/

#ifdef FIDO

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>

#include <misc.h>
#include <pcb.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <string.h>
#include <help.h>
#include <data.hpp>
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fidoorg.h"
#include "fidocfg.h"
#include "fconfig.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern NADDRESS             *address;
extern unsigned int         num_akas;



class editorigins : public editfidoclass {
  public:
    editorigins(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editorigins() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editorigins::displayrecord(long RecNum, int LineNum, void *Rec) {
  ORIGIN *org = (ORIGIN *) Rec;
  char          orgin[35],range[20];

  maxstrcpy(orgin,org->origin,sizeof(orgin));
  maxstrcpy(range,org->ranges,sizeof(range));

  fastprint(LEFTSIDE+ 6,LineNum,orgin,Colors[ANSWER]);
  fastprint(LEFTSIDE+45,LineNum,range,Colors[ANSWER]);

}


void pascal editorigins::showheaders(void) {
  fastprint(LEFTSIDE+6,TOPLINE-2, "Origin",Colors[DISPLAY]);
  fastprint(LEFTSIDE+6,TOPLINE-1, "อออออออออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+45,TOPLINE-2,"Conferences",Colors[DISPLAY]);
  fastprint(LEFTSIDE+45,TOPLINE-1,"อออออออออออ",Colors[DISPLAY]);
}


void pascal editorigins::editrecord(void *Rec) {
  ORIGIN   *    origin = (ORIGIN *) Rec;


  switch(Column)
  {
    case 0:
       //inputstr(LEFTSIDE+6,LineNum,35,sizeof(orgbuf)-1,orgbuf,ALLTEXT,INPUT_DEFAULT,FIDOORIGIN+0);
       inputextend(LEFTSIDE+6,LineNum,35,sizeof(origin->origin)-1,origin->origin,ALLCHAR,INPUT_DEFAULT,FIDOORIGIN+0);
       break;
    case 1:
       //inputstr(LEFTSIDE+45,LineNum,30,"",rangebuf,rangebuf,ALLTEXT,INPUT_DEFAULT,FIDOORIGIN+1);
       inputextend(LEFTSIDE+45,LineNum,30,sizeof(origin->ranges)-1,origin->ranges,ALLTEXT,INPUT_DEFAULT,FIDOORIGIN+1);
       break;
  }
  origin->origin[sizeof(origin->origin)-1] = '\x0';
}


/****************************************************************************/
/* This is the entry function to editing the fido address information.      */

void pascal Configure_Origin(void)
{
ORIGIN * orgs,tmporg;
unsigned int numrecs=0;
bool         changed = FALSE;

  {
  cORIGINS origins;
   numrecs = origins.totRecs();
  }

  if(numrecs == 0) numrecs = 1;
  orgs = new ORIGIN[numrecs];

  if(!orgs) return;
  memset(orgs,0,numrecs*sizeof(ORIGIN));
  {
  cORIGINS origins;
   for(unsigned int i=1;i<=numrecs;i++)
   {
     memset(&tmporg,0,sizeof(tmporg));
     tmporg = origins.getRec(i);
     memcpy(&orgs[i-1],&tmporg,sizeof(tmporg));
   }
  }

  editorigins This("Fidonet Origin Configuration",sizeof(ORIGIN),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,0);


  if (This.load(CONFIG_FILE,(void**)&orgs,&numrecs,ADDR_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  changed = This.edit();

  {
  cORIGINS origins;
    origins.removeFile();
    for(uint i=0;i<numrecs;i++) origins.addRec(orgs[i]);
  }

  delete(orgs);

  if(changed)
  {
  cAREAS areas;
     areas.updateOriginsIndex();
  }
}
#endif
