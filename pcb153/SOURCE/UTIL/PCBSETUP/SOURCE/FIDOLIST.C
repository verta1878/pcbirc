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
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <string.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "editfido.hpp"
#include "defines.h"
#include "structs.h"
#include "fidolist.h"
#include "fconfig.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern NODELIST                    *nodelist_list;
extern unsigned long               list_offset;
extern unsigned int                num_lists;

class editlistclass : public editfidoclass {
  public:
    editlistclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editlistclass() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editlistclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  NODELIST *p = (NODELIST *) Rec;
  char      buffer[80];

  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+1,LineNum,buffer,Colors[ANSWER]);

  stripright(p->basename,' ');
  fastprint(LEFTSIDE+6,LineNum,p->basename,Colors[ANSWER]);
  stripright(p->diffname,' ');
  fastprint(LEFTSIDE+63,LineNum,p->diffname,Colors[ANSWER]);
}


void pascal editlistclass::showheaders(void) {
  fastprint(LEFTSIDE+ 6,TOPLINE-2,"Nodelist Path/Filename (No Extension)",Colors[DISPLAY]);
  fastprint(LEFTSIDE+ 6,TOPLINE-1,"อออออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
  fastprint(LEFTSIDE+61,TOPLINE-2,"Diff Filename",Colors[DISPLAY]);
  fastprint(LEFTSIDE+61,TOPLINE-1,"อออออออออออออ",Colors[DISPLAY]);
}


void pascal editlistclass::editrecord(void *Rec) {
  NODELIST *p = (NODELIST *) Rec;

  switch(Column) {
    case 0: //stripright(p->basename,' ');
            inputstr(LEFTSIDE+6,LineNum,50,"",p->basename,p->basename,ALLFILE,INPUT_CAPS,NODELISTCFG+0);
            //stripright(p->basename,' ');
            break;

    case 1: //stripright(p->diffname,' ');
            inputstr(LEFTSIDE+63,LineNum,sizeof(p->diffname)-1,"",p->diffname,p->diffname,ALLFILE,INPUT_CAPS,NODELISTCFG+1);
            //stripright(p->diffname,' ');
            break;
   }
}


/****************************************************************************/
/* Entry point to module.                                                   */

void pascal configure_nodelist(void)
{
  editlistclass List("Fidonet Nodelist Configuration",sizeof(NODELIST),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,0);

  if (List.load(CONFIG_FILE,(void **)&nodelist_list,&num_lists,LIST_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  List.edit();
}

#endif FIDO
