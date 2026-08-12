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
#include "fidothis.h"
#include "fidocfg.h"
#include "fconfig.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LEFTSIDE     1
#define RIGHTSIDE   78
#define TOPLINE      5
#define BOTTOM       2

extern NFREQ_PATH            *reqlist;
extern unsigned int         num_reqs;


class editfidopathclass : public editfidoclass {
  public:
    editfidopathclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editfidoclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editfidopathclass() {}
    void pascal showheaders(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
};


void pascal editfidopathclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  NFREQ_PATH *p = (NFREQ_PATH *) Rec;
  char    buffer[10],path[50];

  maxstrcpy(path,p->Path,sizeof(path));
  sprintf(buffer,"%3ld)",RecNum);
  fastprint(LEFTSIDE+ 1,LineNum,buffer,Colors[ANSWER]);
  fastprint(LEFTSIDE+ 6,LineNum,path,Colors[ANSWER]);
  fastprint(RIGHTSIDE-14,LineNum,p->Password,Colors[ANSWER]);
}


void pascal editfidopathclass::showheaders(void) {
  fastprint(LEFTSIDE+6,TOPLINE-2,  "Path",Colors[DISPLAY]);
  fastprint(LEFTSIDE+6,TOPLINE-1,"อออออออออ",Colors[DISPLAY]);

  fastprint(RIGHTSIDE-14,TOPLINE-2,"Password",Colors[DISPLAY]);
  fastprint(RIGHTSIDE-14,TOPLINE-1,"อออออออออ",Colors[DISPLAY]);
}


void pascal editfidopathclass::editrecord(void *Rec) {
  NFREQ_PATH *p = (NFREQ_PATH *) Rec;
  char tp[MAXDIR];
  switch(Column)
  {
    case 0:
       maxstrcpy(tp,p->Path,sizeof(tp));
       inputextend(LEFTSIDE+6,LineNum,50,sizeof(p->Path)-2,p->Path,ALLFILE,INPUT_CAPS,FREQCONFIG+0);
       if(strcmp(tp,p->Path)!=0)
         addbackslash(p->Path,sizeof(p->Path)-1);
       break;
    case 1:
       inputstr(RIGHTSIDE-14,LineNum,sizeof(p->Password)-1,"",p->Password,p->Password,ALLCHAR,INPUT_CAPS,FREQCONFIG+0);
       break;
  }
}


/****************************************************************************/
/* This is the entry function to editing the freq path information.         */

void pascal Configure_This_Path(void)
{
  editfidopathclass This_Path("FREQ Path Configuration",sizeof(NFREQ_PATH),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,1,0);

  if (This_Path.load(CONFIG_FILE,(void **)&reqlist,&num_reqs,FREQ_RECS) == -1)
    {
      KeyFlags=ESC;
      return;
    }

  This_Path.edit();
}

#endif
