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
#include <mem.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fidoph.h"
#include "fconfig.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern TRANSLATE         *record_list;
extern unsigned int      num_records;

enum {F2=NEXTKEY};

class editphoneclass : public editfidoclass {
  public:
    editphoneclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editphoneclass() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
};


void pascal editphoneclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  TRANSLATE *p = (TRANSLATE *) Rec;
  char       buffer[80];

  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,buffer,Colors[ANSWER]);

  stripright(p->in,' ');
  fastprint(LEFTSIDE+6,LineNum,p->in,Colors[ANSWER]);
  stripright(p->out,' ');
  fastprint(LEFTSIDE+46,LineNum,p->out,Colors[ANSWER]);
}


void pascal editphoneclass::showheaders(void) {
  fastprint(LEFTSIDE+ 9,TOPLINE-2,"Find",Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 6,TOPLINE-1,"อออออออออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+47,TOPLINE-2,"Change To",Colors[DISPLAY]);
  fastprint(LEFTSIDE+46,TOPLINE-1,"อออออออออออ",Colors[DISPLAY]);
}


void pascal editphoneclass::editrecord(void *Rec) {

TRANSLATE *p = (TRANSLATE *) Rec;

  switch(Column) {
    case 0: stripright(p->in,' ');
            inputstr(LEFTSIDE+6,LineNum,sizeof(p->in)-1,"",p->in,p->in,ALLCHAR,INPUT_CAPS,PHONETABLE+0);
            stripright(p->in,' ');
            break;

    case 1: stripright(p->out,' ');
            inputstr(LEFTSIDE+46,LineNum,sizeof(p->out)-1,"",p->out,p->out,ALLCHAR,INPUT_CAPS,PHONETABLE+1);
            stripright(p->out,' ');
            break;
  }
}

void pascal editphoneclass::handlekeys(void *Rec)
{
  TRANSLATE *p  = (TRANSLATE *) Rec;
  char fix[64][20]={"GS",
                    "GP",
                    "IP",
                    "IPX"};
  int i;

  switch (KeyFlags)
  {
    case F2 : if((i=fileselect(20,LineNum-3,4,fix,4,4))!=-1)
                maxstrcpy(p->in,fix[i],sizeof(p->in)-1);
              break;

    default : editclass::handlekeys(Rec); break;
  }
}

/****************************************************************************/
/* This is the entry function to editing the translation information.       */

void pascal edit_fido_phones(void)
{
  editphoneclass Phone("Fidonet Phone Number Translation Table",sizeof(TRANSLATE),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,1);

  if(Phone.load(CONFIG_FILE,(void **) &record_list,&num_records,XLAT_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  Phone.addexitkey(60,F2,"F2=Text");
  Phone.edit();
}

#endif
