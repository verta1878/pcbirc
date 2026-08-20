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
/* Filename: FIDOAKA.C                                                      */
/*                                                                          */
/* This module goes into PCBSetup to help configure the Fido tosser         */
/* programs.  It configures the Conference specific akas of the system.     */
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
#include "fidoaka.h"
#include "fidocfg.h"
#include "fconfig.h"
#include <fidonofr.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern NADDRESS             *address;
extern unsigned int         num_akas;



class editcakas : public editfidoclass {
  public:
    editcakas(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editcakas() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editcakas::displayrecord(long RecNum, int LineNum, void *Rec) {
  NADDRESS *    addr,mtrec;
  char          addrstr[25];

  if(!Rec)
  {
    memset(&mtrec,0,sizeof(mtrec));
    addr = &mtrec;
  }
  else
  {
    addr = (NADDRESS *)Rec;
    if(addr->point == 0) sprintf(addrstr,"%u:%u/%u",addr->zone,addr->net,addr->node);
    else                 sprintf(addrstr,"%u:%u/%u.u",addr->zone,addr->net,addr->node,addr->point);
  }


  fastprint(LEFTSIDE+ 6,LineNum,addrstr,Colors[ANSWER]);
  fastprint(LEFTSIDE+40,LineNum,addr->range,Colors[ANSWER]);

}


void pascal editcakas::showheaders(void) {
  fastprint(LEFTSIDE+6,TOPLINE-2, "AKA",Colors[DISPLAY]);
  fastprint(LEFTSIDE+6,TOPLINE-1, "อออออออออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+32,TOPLINE-2,"Conferences",Colors[DISPLAY]);
  fastprint(LEFTSIDE+32,TOPLINE-1,"อออออออออออ",Colors[DISPLAY]);
}


void pascal editcakas::editrecord(void *Rec) {
  NADDRESS   *    addr;
  char            buffer[70];
  char            addrbuf[25];


  memset(buffer,0,sizeof(buffer));
  memset(addrbuf,0,sizeof(addrbuf));

  if(Rec)
  {
     addr = (NADDRESS *) Rec;
     if(addr->point == 0) sprintf(addrbuf,"%u:%u/%u",addr->zone,addr->net,addr->node);
     else                 sprintf(addrbuf,"%u:%u/%u.%u",addr->zone,addr->net,addr->node,addr->point);
  }

  switch(Column)
  {
    case 0:
       memcpy(buffer,addrbuf,sizeof(buffer));
       inputstr(LEFTSIDE+6,LineNum,25,"",buffer,buffer,ALLTEXT,INPUT_CAPS,ADDRESSCFG+0);
       { unsigned _z,_n,_d,_p; fido_nodestr_to_int(addrbuf,&_z,&_n,&_d,&_p); addr->zone=(unsigned short)_z; addr->net=(unsigned short)_n; addr->node=(unsigned short)_d; addr->point=(unsigned short)_p; }
       break;
    case 1:
       memcpy(buffer,addr->range,sizeof(buffer));
       inputstr(LEFTSIDE+32,LineNum,30,"",buffer,buffer,ALLTEXT,INPUT_CAPS,ADDRESSCFG+0);
       break;
  }
}


/****************************************************************************/
/* This is the entry function to editing the fido address information.      */

void pascal Configure_AKA(void)
{
  editcakas This("Fidonet Conference AKA Configuration",sizeof(ORIGIN),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,0);

  if (This.load(CONFIG_FILE,(void **)&address,&num_akas,ADDR_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  This.edit();
}
#endif
