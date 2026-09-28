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

/*****************************************************************************/
/* Filename:  FIDONODE.C                                                     */
/*                                                                           */
/* This module is to be included in PCBSetup to configure different nodes    */
/* to send to and what archivers to use.                                     */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>

#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <pcb.h>
#include <string.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fidonode.h"
#include "fidocfg.h"
#include "fconfig.h"
#include <fidonofr.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       4

extern NODE_T           *node_list;             /* the list of nodes        */
extern unsigned int     node_count;             /* number of nodes          */

class editnodeclass : public editfidoclass {
  public:
    editnodeclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editnodeclass() {}
    void pascal showheaders(void);
    void pascal showfooters(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


/****************************************************************************/
/* This translates the node list                                            */

void near pascal translate_nodelist(void)
{
int             i;
char            *p,buffer[30];

  for(i=0;i<node_count;i++)
    {
      strcpy(buffer,node_list[i].nodestr);
      p=strstr(buffer,".");  node_list[i].point=atoi(p+1);
      p=strtok(buffer,":");  node_list[i].zone=atoi(p);
      p=strtok(NULL,"/");    node_list[i].net=atoi(p);
      p=strtok(NULL,".");    node_list[i].node=atoi(p);
      stripright(node_list[i].nodestr,' ');
    }
}


void pascal editnodeclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  NODE_T *p = (NODE_T *) Rec;
  char    buffer[80];

  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,buffer,Colors[ANSWER]);


  if (p->point == 0)
    sprintf(buffer,"%u:%u/%u",p->zone,p->net,p->node);
  else
    sprintf(buffer,"%u:%u/%u.%u",p->zone,p->net,p->node,p->point);

  if(p->zone==0 || p->net==0)
    memset(buffer,0,sizeof(buffer));

  fastprint(LEFTSIDE+6,LineNum,buffer,Colors[ANSWER]);

  sprintf(buffer,"%1u",p->archiver_index);
  fastprint(LEFTSIDE+37,LineNum,buffer,Colors[ANSWER]);
  sprintf(buffer,"%1u",p->pkt_type);
  fastprint(LEFTSIDE+45,LineNum,buffer,Colors[ANSWER]);
}


void pascal editnodeclass::showheaders(void) {
  fastprint(LEFTSIDE+ 8,TOPLINE-2,"Node",Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 6,TOPLINE-1,"อออออออออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+36,TOPLINE-2,"Arc",Colors[DISPLAY]);
  fastprint(LEFTSIDE+36,TOPLINE-1,"อออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+44,TOPLINE-2,"Pkt",Colors[DISPLAY]);
  fastprint(LEFTSIDE+44,TOPLINE-1,"อออ",Colors[DISPLAY]);
}


void pascal editnodeclass::showfooters(void) {
  clsbox(1,Scrn_BottomRow-3,78,Scrn_BottomRow-2,Colors[DESC]);
  fastcenter(Scrn_BottomRow-3,"Archiver Indexes: 0=.ZIP, 1=.ARJ, 2=.ARC, 3=.LZH",Colors[DESC]);
  fastcenter(Scrn_BottomRow-2,"Packet Types: 1=Stone Age, 2=Type 2+",Colors[DESC]);
  editclass::showfooters();
}


void pascal editnodeclass::editrecord(void *Rec) {
  NODE_T *p = (NODE_T *) Rec;
  char      buffer[25];
  char      mask_address[7]={6,0,'0','9',':','/','.'};


  if (p->point == 0)
    sprintf(buffer,"%u:%u/%u",p->zone,p->net,p->node);
  else
    sprintf(buffer,"%u:%u/%u.%u",p->zone,p->net,p->node,p->point);

  if(p->zone == 0 || p->net == 0)
    memset(buffer,0,sizeof(buffer));

  switch(Column) {
    case 0:  inputstr(LEFTSIDE+6,LineNum,sizeof(buffer)-1,"",buffer,buffer,mask_address,INPUT_CAPS,NODEARCCFG+0);
             { unsigned _z,_n,_d,_p; fido_nodestr_to_int(buffer,&_z,&_n,&_d,&_p); p->zone=(unsigned short)_z; p->net=(unsigned short)_n; p->node=(unsigned short)_d; p->point=(unsigned short)_p; }
             break;
    case 1:  inputnum(LEFTSIDE+37,LineNum,1,"",&p->archiver_index,vINT,NODEARCCFG+1);
             break;
    case 2:  inputnum(LEFTSIDE+45,LineNum,1,"",&p->pkt_type,vINT,NODEARCCFG+2);
             break;
  }
}



/****************************************************************************/
/* This is the entry function to editing the fido node information.         */

void pascal edit_fido_nodes(void)
{
  editnodeclass Node("Fidonet Node Archiver Configuration",sizeof(NODE_T),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,2,0);

  if(Node.load(CONFIG_FILE,(void **) &node_list,&node_count,NODE_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  Node.edit();

//  translate_nodelist();
}

#endif
