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
/* Filename: FIDOTHIS.C                                                     */
/*                                                                          */
/* This module goes into PCBSetup to help configure the Fido tosser         */
/* programs.  It configures the address of the system.                      */
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
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fidothis.h"
#include "fidocfg.h"
#include "fconfig.h"
#include <data.hpp>
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern NADDRESS             *address;
extern unsigned int         num_akas;


char UD[] = {2,'U','D'};

/****************************************************************************/
/* Convert the address string to intgers.                                   */

void fido_nodestr_to_int(char *nodestr, unsigned int *zone, unsigned int *net, unsigned int *node, unsigned int *point)
{
char            str[20];
char            *tptr;
int             i,j;
unsigned int    tzone;
unsigned int    tnet;
unsigned int    tnode;
unsigned int    tpoint;
bool            nozone,nonet,nopoint;

  nozone=nonet=nopoint=FALSE;

  if(strchr(nodestr,':')==NULL)
    nozone=TRUE;
  if(strchr(nodestr,'/')==NULL)
    nonet=TRUE;
  if(strchr(nodestr,'.')==NULL)
    nopoint=TRUE;

  if(zone!=NULL)
    *zone=0;
  if(net!=NULL)
    *net=0;
  if(point!=NULL)
    *point=0;
  *node=0;

  tzone=0;
  tnet=0;
  tnode=0;
  tpoint=0;

  if((tptr=strchr(nodestr,'@'))!=NULL)
    *tptr=NULL;

  for(i=0,j=0;j<sizeof(str);i++,j++)
    {
      switch(nodestr[i])
        {
          case  ':' : if(nozone!=TRUE)
                        {
                          str[j]=NULL;
                          tzone=0;
                          tzone=atoi(str);
                        }
                      j=-1;
                      break;

          case  '/' : if(nonet!=TRUE)
                        {
                          str[j]=NULL;
                          tnet=0;
                          tnet=atoi(str);
                        }
                      j=-1;
                      break;

          case  '.' :
                      str[j]=NULL;
                      tnode=0;
                      tnode=atoi(str);
                      j=-1;
                      break;

          case  NULL: if(nopoint==TRUE)
                        {
                          str[j]=NULL;
                          tnode=0;
                          tnode=atoi(str);
                        }
                      else
                        {

                          str[j]=NULL;
                          tpoint=0;
                          tpoint=atoi(str);
                        }
                      if(tptr!=NULL)
                        *tptr='@';
                      if(zone!=NULL)
                        *zone=tzone;
                      if(net!=NULL)
                        *net=tnet;
                      if(point!=NULL)
                        *point=tpoint;
                      *node=tnode;
                      return;

          default   :
                      str[j]=nodestr[i];
                      break;
        }
    }
  if(tptr!=NULL)
    *tptr='@';
  return;
}


class editthisclass : public editfidoclass {
  public:
    editthisclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editthisclass() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editthisclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  NADDRESS *p = (NADDRESS *) Rec;
  char          buffer[80],pri[2],inseen[2],pres[2],range[21];

  memset(pri,0,sizeof(pri));
  memset(inseen,0,sizeof(inseen));
  memset(pres,0,sizeof(pres));

  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,buffer,Colors[ANSWER]);

  maxstrcpy(range,p->range,sizeof(range));
  if (p->point == 0)
    sprintf(buffer,"%u:%u/%u",p->zone,p->net,p->node);
  else
    sprintf(buffer,"%u:%u/%u.%u",p->zone,p->net,p->node,p->point);

  if(p->zone==0 || p->net==0)
    memset(buffer,0,sizeof(buffer));

  fastprint(LEFTSIDE+6,LineNum,buffer,Colors[ANSWER]);

  if(p->primary == 0 && p->zone != 0)  pri[0] = 'N';
  else pri[0] = 'Y';

  if(p->inseenby == 0&& p->zone != 0)  inseen[0] = 'N';
  else inseen[0] = 'Y';

  if(p->present == 0&& p->zone != 0)  pres[0] = 'N';
  else pres[0] = 'Y';


  fastprint(LEFTSIDE+32,LineNum,pri,Colors[ANSWER]);
  fastprint(LEFTSIDE+40,LineNum,inseen,Colors[ANSWER]);
  fastprint(LEFTSIDE+48,LineNum,pres,Colors[ANSWER]);
  fastprint(LEFTSIDE+57,LineNum,range,Colors[ANSWER]);
}


void pascal editthisclass::showheaders(void) {
  fastprint(LEFTSIDE+15,TOPLINE-2,          "Node             Primary Seen-By Present  Range",Colors[DISPLAY]);
  fastprint(LEFTSIDE+6 ,TOPLINE-1, "ออออออออออออออออออออออออ  อออออออ อออออออ อออออออ  อออออ",Colors[DISPLAY]);
}


void pascal editthisclass::editrecord(void *Rec) {
  NADDRESS *p = (NADDRESS *) Rec;
  char          buffer[25],oldbuf[25],pri[2],inseen[2],pres[2],range[70];
  char      mask_address[7]={6,0,'0','9',':','/','.'};

  memset(pri,0,sizeof(pri));
  memset(inseen,0,sizeof(inseen));
  memset(pres,0,sizeof(pres));
  memset(range,0,sizeof(range));

  if (p->point == 0)
    sprintf(buffer,"%u:%u/%u",p->zone,p->net,p->node);
  else
    sprintf(buffer,"%u:%u/%u.%u",p->zone,p->net,p->node,p->point);

  if(p->zone==0 || p->net==0)
  {
    memset(buffer,0,sizeof(buffer));
    memset(oldbuf,0,sizeof(buffer));
  }

  maxstrcpy(oldbuf,buffer,sizeof(oldbuf));

  if(p->primary == 0 && p->zone != 0)  pri[0] = 'N';
  else pri[0] = 'Y';

  if(p->inseenby == FALSE && p->zone != 0)  inseen[0] = 'N';
  else inseen[0] = 'Y';

  if(p->present == FALSE && p->zone != 0)  pres[0] = 'N';
  else pres[0] = 'Y';

  maxstrcpy(range,p->range,sizeof(range));

  switch(Column)
  {
    case 0:
       inputstr(LEFTSIDE+6,LineNum,sizeof(buffer)-1,"",buffer,buffer,mask_address,INPUT_CAPS,ADDRESSCFG+0);
       fido_nodestr_to_int(buffer,&p->zone,&p->net,&p->node,&p->point);
       break;
    case 1:
       inputstr(LEFTSIDE+32,LineNum,1,"",pri,pri,YESNO,INPUT_CAPS,ADDRESSCFG+1);
       break;
    case 2:
       inputstr(LEFTSIDE+40,LineNum,1,"",inseen,inseen,YESNO,INPUT_CAPS,ADDRESSCFG+2);
       break;
    case 3:
       inputstr(LEFTSIDE+48,LineNum,1,"",pres,pres,YESNO,INPUT_CAPS,ADDRESSCFG+3);
       break;
    case 4:
       //inputstr(LEFTSIDE+57,LineNum,20,"",range,range,ALLCHAR,INPUT_CAPS,ADDRESSCFG+0);
//     inputextend(LEFTSIDE+57,LineNum,69,sizeof(range),range,ALLCHAR,INPUT_CAPS,ADDRESSCFG+4);
       inputextend(LEFTSIDE+57,LineNum,20,sizeof(range),range,ALLCHAR,INPUT_CAPS,ADDRESSCFG+4);
       break;
  }
    p->primary      = (pri[0]    == 'Y' ?TRUE :FALSE);
    p->inseenby     = (inseen[0] == 'Y' ?TRUE:FALSE);
    p->present      = (pres[0]   == 'Y' ?TRUE:FALSE);
    maxstrcpy(p->range,range,sizeof(p->range));
}


/****************************************************************************/
/* This is the entry function to editing the fido address information.      */

void pascal Configure_This_Address(void)
{
  editthisclass This("Fidonet Address Configuration",sizeof(NADDRESS),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,4,0);

  if (This.load(CONFIG_FILE,(void **)&address,&num_akas,ADDR_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  if(This.edit())
  {
  cAREAS areas;
    areas.updateAkaIndex();
  }
}
#endif

