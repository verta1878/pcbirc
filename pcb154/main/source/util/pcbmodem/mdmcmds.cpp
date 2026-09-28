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


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
// Borland C/C++ includes
#define gotoxy bor_gotoxy
#define wherex bor_wherex
#define wherey bor_wherey
#include <conio.h>
#undef gotoxy
#undef wherex
#undef wherey

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

// CDC includes
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <misc.h>

// Local includes
#include "modems.h"
#include "common.h"

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
#define NUMMDMCMDS 25
#define NUMDEFCMDS 21

// Used with the 'Options' information
typedef struct
{
  char  Modems   [ 40 ],
        PCBDat   [ 40 ],
        History  [ 40 ],
        Name     [ 15 ],
        Version  [ 10 ];
} cfgtype;

extern  char  YN [];
extern  char  mask_alphanum [];
extern  int  Update;
extern  char * Questions[];
extern  cfgtype   Config;

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int chkstdcfg(int Before)
{
  if (Before)
    return(0);

  getmdmdata(FALSE);
  return(2);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void pascal initmdmfields(FldType *P)
{
   addquest(P,0,vSTR,0,mask_alphanum,2,3,14,Questions[0],Manufdata[CurrentModem].Manufs,CLEAR,NULL);
   addquest(P,1,vSTR,0,mask_alphanum,39,3,24,Questions[1],Manufdata[CurrentModem].Name,CLEAR,NULL);
   addquest(P,2,vUNSIGNED,0,ALLNUM,2,4,4,Questions[2],&Manufdata[CurrentModem].Number,CLEAR,NULL);
   addquest(P,3,vUNSIGNED,0,ALLNUM,39,4,2,Questions[4],&Manufdata[CurrentModem].StdCfg,CLEAR,chkstdcfg);

   addquest(P,4,vBOOL,0,YN,2,6,1,Questions[5],&Modemdata.Fax,CLEAR,NULL);
   addquest(P,5,vBOOL,0,YN,25,6,1,Questions[6],&Modemdata.Eprom,CLEAR,NULL);
   addquest(P,6,vBOOL,0,YN,58,6,1,Questions[7],&Modemdata.Lock,CLEAR,NULL);
   addquest(P,7,vBYTE,0,ALLNUM,2,7,2,Questions[8],&Modemdata.Seconds,CLEAR,NULL);
   addquest(P,8,vUNLONG,0,ALLNUM,25,7,6,Questions[9],&Modemdata.Baud,CLEAR,NULL);
   addquest(P,9,vBYTE,0,ALLNUM,58,7,2,Questions[3],&Modemdata.Factory,CLEAR,NULL);
   addquest(P,10,vBOOL,0,YN,2,8,1,Questions[28],&Modemdata.V42,CLEAR,NULL);
   addquest(P,11,vBOOL,0,YN,25,8,1,Questions[29],&Modemdata.CallID,CLEAR,NULL);

   addquest(P,12,vDATE,0,mask_alphanum,2,9,8,Questions[21],&Modemdata.Date,CLEAR,NULL);
   addquest(P,13,vSTR,0,mask_alphanum,49,9,14,Questions[22],Modemdata.Who,CLEAR,NULL);
   addquest(P,14,vUPSTR,0,mask_alphanum,6,10,14,Questions[10],Modemdata.Lspd,CLEAR,NULL);
   addquest(P,15,vUPSTR,0,mask_alphanum,41,10,14,Questions[11],Modemdata.Hspd,CLEAR,NULL);

   addquest(P,16,vUPSTR,0,mask_alphanum,2,12,39,Questions[12],Modemdata.Send1,CLEAR,NULL);
   addquest(P,17,vUPSTR,0,mask_alphanum,2,13,39,Questions[13],Modemdata.Send2,CLEAR,NULL);
   addquest(P,18,vUPSTR,0,mask_alphanum,2,14,39,Questions[14],Modemdata.Send3,CLEAR,NULL);

   addquest(P,19,vSTR,0,mask_alphanum,2,16,49,Questions[15],Modemdata.Cmnt1,CLEAR,NULL);
   addquest(P,20,vSTR,0,mask_alphanum,2,17,49,Questions[16],Modemdata.Cmnt2,CLEAR,NULL);
   addquest(P,21,vSTR,0,mask_alphanum,2,18,49,Questions[17],Modemdata.Cmnt3,CLEAR,NULL);

   addquest(P,22,vUPSTR,0,mask_alphanum,2,20,39,Questions[18],Modemdata.Init1,CLEAR,NULL);
   addquest(P,23,vUPSTR,0,mask_alphanum,2,21,39,Questions[19],Modemdata.Init2,CLEAR,NULL);
   addquest(P,24,vUPSTR,0,mask_alphanum,2,22,39,Questions[20],Modemdata.Offhook,CLEAR,NULL);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void pascal initdeffields(FldType *P)
{
   addquest(P,0,vBOOL,0,YN,2,6,1,Questions[5],&Modemdata.Fax,CLEAR,NULL);
   addquest(P,1,vBOOL,0,YN,25,6,1,Questions[6],&Modemdata.Eprom,CLEAR,NULL);
   addquest(P,2,vBOOL,0,YN,58,6,1,Questions[7],&Modemdata.Lock,CLEAR,NULL);
   addquest(P,3,vBYTE,0,ALLNUM,2,7,2,Questions[8],&Modemdata.Seconds,CLEAR,NULL);
   addquest(P,4,vUNLONG,0,ALLNUM,25,7,6,Questions[9],&Modemdata.Baud,CLEAR,NULL);
   addquest(P,5,vBYTE,0,ALLNUM,58,7,2,Questions[3],&Modemdata.Factory,CLEAR,NULL);
   addquest(P,6,vBOOL,0,YN,2,8,1,Questions[28],&Modemdata.V42,CLEAR,NULL);
   addquest(P,7,vBOOL,0,YN,25,8,1,Questions[29],&Modemdata.CallID,CLEAR,NULL);

   addquest(P,8,vDATE,0,mask_alphanum,2,9,8,Questions[21],&Modemdata.Date,CLEAR,NULL);
   addquest(P,9,vSTR,0,mask_alphanum,49,9,14,Questions[22],Modemdata.Who,CLEAR,NULL);
   addquest(P,10,vUPSTR,0,mask_alphanum,6,10,14,Questions[10],Modemdata.Lspd,CLEAR,NULL);
   addquest(P,11,vUPSTR,0,mask_alphanum,41,10,14,Questions[11],Modemdata.Hspd,CLEAR,NULL);

   addquest(P,12,vUPSTR,0,mask_alphanum,2,12,39,Questions[12],Modemdata.Send1,CLEAR,NULL);
   addquest(P,13,vUPSTR,0,mask_alphanum,2,13,39,Questions[13],Modemdata.Send2,CLEAR,NULL);
   addquest(P,14,vUPSTR,0,mask_alphanum,2,14,39,Questions[14],Modemdata.Send3,CLEAR,NULL);

   addquest(P,15,vSTR,0,mask_alphanum,2,16,49,Questions[15],Modemdata.Cmnt1,CLEAR,NULL);
   addquest(P,16,vSTR,0,mask_alphanum,2,17,49,Questions[16],Modemdata.Cmnt2,CLEAR,NULL);
   addquest(P,17,vSTR,0,mask_alphanum,2,18,49,Questions[17],Modemdata.Cmnt3,CLEAR,NULL);

   addquest(P,18,vUPSTR,0,mask_alphanum,2,20,39,Questions[18],Modemdata.Init1,CLEAR,NULL);
   addquest(P,19,vUPSTR,0,mask_alphanum,2,21,39,Questions[19],Modemdata.Init2,CLEAR,NULL);
   addquest(P,20,vUPSTR,0,mask_alphanum,2,22,39,Questions[20],Modemdata.Offhook,CLEAR,NULL);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
static void pascal savedata(bool isDefault)
{
   static long   DefOffset;
   static int    Loop;
   static char   OK;

   if (Update == 1)
     Header.NumOfModems++;

   strcpy(Modemdata.Who, Config.Name);
   Modemdata.Date = Date;
   Header.Date = Date;
   strcpy(Header.Version, Config.Version);
   ecrypt(FALSE);  // Encrypt Modemdata;
   ecrypt(TRUE);   // Encrypt Manufdata;

   for (Loop=Header.NumOfModems+1; Loop<MaxModem; Loop++) {
     memset(Manufdata[Loop].Manufs, Loop/3, sizeof(Manufdata[Loop].Manufs));
     memset(Manufdata[Loop].Name, Loop/4, sizeof(Manufdata[Loop].Name));
   }

   if (Manufdata[CurrentModem].StdCfg == 0 && !isDefault)
     if (dosfseek(&DataFile, Manufdata[CurrentModem].Offset, SEEK_SET) == -1)
       quit(3,"savedata reg");
     else
       if (dosfwrite(&Modemdata, sizeof(modemdatatype), &DataFile) == -1)
         quit(11,"savedata");

   if (isDefault) {
     DefOffset = (long) sizeof(headertype) +
                 (long) sizeof(manufdatatype) * MaxModem +
                 (long) sizeof(modemdatatype) * (DefaultNum-1);
     if (dosfseek(&DataFile, DefOffset, SEEK_SET) == -1)
       quit(3,"savedata pre");
     else
       if (dosfwrite(&Modemdata, sizeof(modemdatatype), &DataFile) == -1)
         quit(11,"savedata");
   }

   Header.NextMdmOffset = dosfseek(&DataFile, 0, SEEK_END);

   if (dosfseek(&DataFile, 0, SEEK_SET) == -1)
     quit(4,"savedata");

   if (dosfwrite(&Header, sizeof(headertype), &DataFile) == -1)
     quit(9,"savedata");

   if (dosfwrite(Manufdata, sizeof(manufdatatype) * MaxModem, &DataFile) == -1)
     quit(10,"savedata");

   dcrypt(TRUE);          // Decrypt Manufdata
   createarray(FALSE);
   setcursor(CUR_BLANK);
   boxcls(3,11,76,13,4,SINGLE);
   fastcenter(12, "New information has been saved --- Press any key to continue.", 0x0e);
   inkey((char*)&OK, CLOCK);     //lint !e534)
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void pascal editdata(bool isDefault)
{
   static char  BlankLine [] = "                                                            ";
   static char  Str [ 80 ];

   FldType *Fields;

   window(2,3,78,24);
   clrscr();
   fastcenter(23, "--- press the page down key to save modem ---", 0x0f);

   if (isDefault)
   {
     if ((Fields = (FldType *) mallochk(NUMDEFCMDS * sizeof(FldType))) == NULL)
       return;

     initquest(Fields, NUMDEFCMDS-1);

     sprintf(Str,"%s : Default",Questions[0]);
     fastprint(2, 3, Str, 0x0f);
     sprintf(Str,"%s : Configuration #%d", Questions[1], DefaultNum);
     fastprint(39, 3, Str, 0x0f);
     sprintf(Str,"%s : 0", Questions[2]);
     fastprint(2, 4, Str, 0x0f);
     sprintf(Str,"%s : %d", Questions[4], DefaultNum);
     fastprint(39, 4, Str, 0x0f);
     initdeffields(Fields);
     readscrn(Fields,NUMDEFCMDS-1,0,"  New Modem Information  ",BlankLine,1,NOCLEARFLD);
     freescrn(Fields,NUMDEFCMDS-1);
   } else {
     if ((Fields = (FldType *) mallochk(NUMMDMCMDS * sizeof(FldType))) == NULL)
       return;

     initquest(Fields, NUMMDMCMDS-1);
     initmdmfields(Fields);
     readscrn(Fields,NUMMDMCMDS-1,0,"  New Modem Information  ",BlankLine,1,NOCLEARFLD);
     freescrn(Fields,NUMMDMCMDS-1);
   }

   if (KeyFlags == PGDN)
     savedata(isDefault);

   generalscreen(Head1,Head2);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
