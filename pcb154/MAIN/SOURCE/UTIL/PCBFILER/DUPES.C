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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <newdata.h>
#include <pcb.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <dosfunc.h>
#include <country.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static int near  keepwhich(parenttype *First, parenttype *Second) {
  char Frst[80];
  char Scnd[80];
  char Temp[10];
  int  DescLen;

  DescLen = 50 - strlen(First->Fields.NName);

  dtoc(First->Fields.NDate,Temp);
  if (First->Fields.NDate != 0xFFFF && First->Fields.NDate != 0xFFFE)
    countrydate(Temp);

  sprintf(Frst,"1) %s%9ld %s %-*.*s",First->Fields.NName ,First->Fields.FSize,Temp,DescLen,DescLen,First->Fields.FDesc);

  dtoc(Second->Fields.NDate,Temp);
  if (Second->Fields.NDate != 0xFFFF && Second->Fields.NDate != 0xFFFE)
    countrydate(Temp);

  sprintf(Scnd,"2) %s%9ld %s %-*.*s",Second->Fields.NName,Second->Fields.FSize,Temp,DescLen,DescLen,Second->Fields.FDesc);

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.Save      = TRUE;
  MsgData.X1        = 0;
  MsgData.Y1        = 13;
  MsgData.X2        = 79;
  MsgData.Y2        = 22;
  MsgData.Msg1      = "Found Duplicate File Names!";
  MsgData.Line1     = 15;
  MsgData.Color1    = Colors[HEADING];
  MsgData.Msg2      = Frst;
  MsgData.Line2     = 17;
  MsgData.Color2    = Colors[ANSWER];
  MsgData.Msg3      = Scnd;
  MsgData.Line3     = 18;
  MsgData.Color3    = Colors[ANSWER];
  MsgData.Quest     = "Keep which one?  (type 1 or 2, ESC to keep both)";
  MsgData.QuestLine = 20;
  MsgData.Answer[0] = '1';
  MsgData.Answer[1] = 0;
  MsgData.Mask      = OTT;
  showmessage();

  if (KeyFlags == ESC)
    MsgData.Answer[0] = '3';

  switch (MsgData.Answer[0]) {
    case '1': return(1);
    case '2': return(2);
    default : return(3);
  }
}


void dupeschanged(void) {
  bool              ChangesMade;
  int               Len;
  dupestype        *d1;
  dupestype        *d2;
  parenttype        p1;
  parenttype        p2;
  long              Pos;
  VMAVLWalkContext  InOrderContext;
  VMAVLWalkContext  SearchContext;
  char              Temp[80];
  char              Buffer[100];

  if (openchangeread() == -1)
    return;

  while (dosfgets(Buffer,100,&DosChangedList) != -1) {
    if (Buffer[0] == 0 || Buffer[0] == '-' || Buffer[0] == '=')
      continue;

    Len = strlen(Buffer);
    Buffer[Len-2] = 0;
    sprintf(Temp,"Check Dupe: %s",Buffer);
    printscroll(Temp,Colors[QUESTION]);

    VMInitRec(&Parents,NULL,0,sizeof(parenttype));
    VMInitRec(&Children,NULL,0,sizeof(childtype));
    VMInitRec(&ParentsOnly,NULL,0,sizeof(parentsonlytype));
    VMInitRec(&Dupes,NULL,0,sizeof(dupestype));

    VMAccessAttrSet(&ParentsOnly,VM_SEQUENTIAL);
    VMAccessAttrSet(&Children,VM_SEQUENTIAL);
    VMAccessAttrSet(&Parents,VM_SEQUENTIAL);

    VMAVLControlInit(&Dupes,&DupesControl,comparedupes,0);
    VMAVLTreeInit(&DupesTree,1);
    ChangesMade = FALSE;

    fastprintmove(2,19,"Reading",Colors[ANSWER]);
    startaction('.',FALSE,32);

    if (readdirfile(Buffer,0,0,(loadflagstype)(SHOWACTION|UPDATEDUPES)) != -1) {
      // only check for dupes if there are two or more parent names
      if (VMRecordCount(&Dupes) > 1) {
        clsbox(2,19,45,19,Colors[DISPLAY]);
        fastprintmove(2,19,"Scanning",Colors[ANSWER]);
        startaction('.',FALSE,256);
        ChangesMade = FALSE;

        if ((Pos = VMAVLFirstGet(&DupesTree,&DupesControl)) != VM_INVALID_POS) {
          VMAVLWalkContextInit(&DupesTree,&DupesControl,&InOrderContext,Pos);

          do {
            d1 = (dupestype *) VMRecordGetByPos(&Dupes,Pos);
            p1 = *(parenttype *) VMRecordGetByPos(&Parents,d1->ParentPos);
            if (p1.Fields.Keep == DELETE)
              continue;

            SearchContext = InOrderContext;
            while ((Pos = VMAVLNextGet(&DupesTree,&DupesControl,&SearchContext)) != VM_INVALID_POS) {
              d2 = (dupestype *) VMRecordGetByPos(&Dupes,Pos);

              // check to see if second filename matches the first, if not break out
              if (strcmp(d1->FName,d2->FName) != 0)
                break;

              p2 = *(parenttype *) VMRecordGetByPos(&Parents,d2->ParentPos);
              if (p2.Fields.Keep == DELETE)
                continue;

               switch (keepwhich(&p1,&p2)) {
                 case 1: p2.Fields.Keep = DELETE;
                         VMWrite(&Parents,&p2,d2->ParentPos,sizeof(parenttype));
                         ChangesMade = TRUE;
                         break;
                 case 2: p1.Fields.Keep = DELETE;
                         VMWrite(&Parents,&p1,d1->ParentPos,sizeof(parenttype));
                         ChangesMade = TRUE;
                         break;
                 case 3: break;
               }

            }
            showaction();
          } while ((Pos = VMAVLNextGet(&DupesTree,&DupesControl,&InOrderContext)) != VM_INVALID_POS);
        }
      }

      clsbox(2,19,45,19,Colors[DISPLAY]);
      if (ChangesMade) {
        fastprintmove(2,19,"Writing",Colors[ANSWER]);
        startaction('.',FALSE,32);
        writedirfile(Buffer);
      } else {
        fastprintmove(2,19,"No changes made",Colors[ANSWER]);
      }
    }

    VMDone(&Parents);
    VMDone(&Children);
    VMDone(&ParentsOnly);
    VMDone(&Dupes);
  }
  dosfclose(&DosChangedList);
}
