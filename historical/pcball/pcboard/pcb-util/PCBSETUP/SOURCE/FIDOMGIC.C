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

#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <pcb.h>
#include <string.h>
#include <help.h>
#include <mem.h>
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fconfig.h"
#include "fidomgic.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern    NFREQ_MAGIC    *magic_list;
extern    unsigned int  num_magic;

static    char   FIDOMAGIC[]={12,0,'#',')','!',0,'-',':',0,'A','z','?','*'};

class editmagicclass : public editfidoclass {
  public:
    editmagicclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editmagicclass() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editmagicclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  NFREQ_MAGIC *p = (NFREQ_MAGIC *) Rec;
  char       buffer[80];
  char       rnbuf[30];


  maxstrcpy(rnbuf,p->RealName,sizeof(rnbuf));
  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,buffer,Colors[ANSWER]);

  stripright(p->MagicName,' ');
  fastprint(LEFTSIDE+6,LineNum,p->MagicName,Colors[ANSWER]);

  stripright(p->RealName,' ');
  fastprint(LEFTSIDE+28,LineNum,rnbuf,Colors[ANSWER]);

  stripright(p->Password,' ');
  fastprint(LEFTSIDE+59,LineNum,p->Password,Colors[ANSWER]);
}


void pascal editmagicclass::showheaders(void) {
  fastprint(LEFTSIDE+ 6,TOPLINE-2,"Magic Name",Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 6,TOPLINE-1,"様様様様様",Colors[DISPLAY]);

  fastprint(LEFTSIDE+28,TOPLINE-2,"Filename",Colors[DISPLAY]);
  fastprint(LEFTSIDE+28,TOPLINE-1,"様様様様",Colors[DISPLAY]);

  fastprint(LEFTSIDE+59,TOPLINE-2,"Password",Colors[DISPLAY]);
  fastprint(LEFTSIDE+59,TOPLINE-1,"様様様様",Colors[DISPLAY]);
}


void pascal editmagicclass::editrecord(void *Rec) {
  NFREQ_MAGIC *p = (NFREQ_MAGIC *) Rec;

  switch(Column) {
    case 0: //p->MagicName[sizeof(p->MagicName)-1]=NULL;
            //stripright(p->MagicName,' ');
            inputstr(LEFTSIDE+6,LineNum,sizeof(p->MagicName)-1,"",p->MagicName,p->MagicName,ALLFILE,INPUT_CAPS,MAGICEDITOR+0);
            //p->MagicName[sizeof(p->MagicName)-1]=NULL;
            //stripright(p->MagicName,' ');
            break;

    case 1: //p->RealName[sizeof(p->RealName)-1]=NULL;
            //stripright(p->RealName,' ');
            //inputstr(LEFTSIDE+28,LineNum,sizeof(p->RealName)-1,"",p->RealName,p->RealName,FIDOMAGIC,INPUT_CAPS,MAGICEDITOR+1);
            inputextend(LEFTSIDE+28,LineNum,30,MAXDIR,p->RealName,FIDOMAGIC,INPUT_CAPS,MAGICEDITOR+1);
            //p->RealName[sizeof(p->RealName)-1]=NULL;
            //stripright(p->RealName,' ');
            break;

    case 2: //p->Password[sizeof(p->Password)-1]=NULL;
            //stripright(p->Password,' ');
            inputstr(LEFTSIDE+59,LineNum,sizeof(p->Password)-1,"",p->Password,p->Password,FIDOMAGIC,INPUT_CAPS,MAGICEDITOR+1);
            //p->Password[sizeof(p->Password)-1]=NULL;
            //stripright(p->Password,' ');
            break;
  }
}

/****************************************************************************/
/* This is the entry function to editing the translation information.       */

void pascal edit_fido_magic(void)
{
  editmagicclass Magic("Fidonet Magic Names Editor",sizeof(NFREQ_MAGIC),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,2,0);

  if(Magic.load(CONFIG_FILE,(void **) &magic_list,&num_magic,MAGIC_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }
  Magic.edit();
}

#endif
