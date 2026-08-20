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
#include "fidonofr.h"
#include "fidocfg.h"
#include "fconfig.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern THIS_ADDRESS         *deny_list;
extern unsigned int         num_deny;

class editdenyclass : public editfidoclass {
  public:
    editdenyclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editdenyclass() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editdenyclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  THIS_ADDRESS *p = (THIS_ADDRESS *) Rec;
  char          buffer[80];

  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,buffer,Colors[ANSWER]);


  if (p->this_point == 0)
    sprintf(buffer,"%u:%u/%u",p->this_zone,p->this_net,p->this_node);
  else
    sprintf(buffer,"%u:%u/%u.%u",p->this_zone,p->this_net,p->this_node,p->this_point);

  if(p->this_zone==0 || p->this_net==0)
    memset(buffer,0,sizeof(buffer));

  fastprint(7,LineNum,buffer,Colors[ANSWER]);
}


void pascal editdenyclass::showheaders(void) {
  fastprint(LEFTSIDE+14,TOPLINE-2,  "Node",Colors[DISPLAY]);
  fastprint(LEFTSIDE+12,TOPLINE-1,"อออออออออ",Colors[DISPLAY]);
}


void pascal editdenyclass::editrecord(void *Rec) {
  THIS_ADDRESS *p = (THIS_ADDRESS *) Rec;
  char      mask_address[7]={6,0,'0','9',':','/','.'};
  char      buffer[30];

  if(p->nodestr[0] != '\x0' && p->this_zone != 0)
    maxstrcpy(buffer,p->nodestr,sizeof(buffer));
  else
    memset(buffer,0,sizeof(buffer));

  inputstr(LEFTSIDE+6,LineNum,sizeof(p->nodestr)-1,"",buffer,p->nodestr,mask_address,INPUT_CAPS,DENYCONFIG+0);
  //maxstrcpy(p->nodestr,buffer,sizeof(p->nodestr));
  fido_nodestr_to_int(p->nodestr,&p->this_zone,&p->this_net,&p->this_node,&p->this_point);
}


/****************************************************************************/
/* This is the entry function to editing the fido address information.      */

void pascal edit_fido_deny(void)
{
  editdenyclass This("FREQ Deny Nodelist Configuration",sizeof(THIS_ADDRESS),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,0,0);

  if (This.load(CONFIG_FILE,(void **)&deny_list,&num_deny,DENY_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  This.edit();
}
#endif
