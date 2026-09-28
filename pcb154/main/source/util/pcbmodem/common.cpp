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


// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
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
#include <dir.h>

// CDC includes
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <help.h>
#include <pcbtools.h>

// Local includes
#include "common.h"
#include "modems.h"

extern char *Errors[];

// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void  title(void)
{
  window(1,1,80,45);
  cls();
  generalscreen(Head1, Head2);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void  anykey(void)
{
  static char OK;

  setcursor(CUR_BLANK);
  fastcenter(22, Errors[0], 0x0f);

  if (!local)
    inkey((char*)&OK, CLOCK);   //lint !e534

  fastcenter(22, "                             ", 0x07);
  setcursor(CUR_NORMAL);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void  pascal  quit(int ErrNum, char *Module)
{
  window(1,1,80,25);
  clrscr();
  fastprint(0,0,"                               Have a great day!                                ",47);
  if (ErrNum) {
    sprintf(TempStr, "(%d) Error %s in (%s) -- Exiting to DOS.",ErrNum, Errors[ErrNum], Module);
    fastprint(0,1,TempStr,0x0c);
  }
  gotoxy(1,2);
  dosfclose(&DataFile);
  free(Manufdata);
  setcursor(CUR_NORMAL);
  exit(EXIT_FAILURE);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void  pascal ecrypt(bool Which)
{
  static word  Entries;

  if (Which)   // True = encrypt Manufdata
  {
    for (Entries=0; Entries<MaxModem; Entries++)
    {
      encrypt(Manufdata[Entries].Manufs, sizeof(Manufdata[Entries].Manufs));
      encrypt(Manufdata[Entries].Name, sizeof(Manufdata[Entries].Name));
    }
  } else {     // False = encrypt Modemdata
    encrypt(Modemdata.Hspd, sizeof(Modemdata.Hspd));
    encrypt(Modemdata.Lspd, sizeof(Modemdata.Lspd));
    encrypt(Modemdata.Send1, sizeof(Modemdata.Send1));
    encrypt(Modemdata.Send2, sizeof(Modemdata.Send2));
    encrypt(Modemdata.Send3, sizeof(Modemdata.Send3));
    encrypt(Modemdata.Cmnt1, sizeof(Modemdata.Cmnt1));
    encrypt(Modemdata.Cmnt2, sizeof(Modemdata.Cmnt2));
    encrypt(Modemdata.Cmnt3, sizeof(Modemdata.Cmnt3));
    encrypt(Modemdata.Init1, sizeof(Modemdata.Init1));
    encrypt(Modemdata.Init2, sizeof(Modemdata.Init2));
    encrypt(Modemdata.Offhook, sizeof(Modemdata.Offhook));
    encrypt(Modemdata.Who, sizeof(Modemdata.Who));
   }
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
void  pascal dcrypt(bool Which)
{
   static word  Entries;

   if (Which)   // True = Decrypt Manufdata
   {
      for (Entries=0; Entries<Header.NumOfModems; Entries++)
      {
         decrypt(Manufdata[Entries].Manufs, sizeof(Manufdata[Entries].Manufs));
         decrypt(Manufdata[Entries].Name, sizeof(Manufdata[Entries].Name));
      }
   } else {     // False = Decrypt Modemdata
      decrypt(Modemdata.Hspd, sizeof(Modemdata.Hspd));
      decrypt(Modemdata.Lspd, sizeof(Modemdata.Lspd));
      decrypt(Modemdata.Send1, sizeof(Modemdata.Send1));
      decrypt(Modemdata.Send2, sizeof(Modemdata.Send2));
      decrypt(Modemdata.Send3, sizeof(Modemdata.Send3));
      decrypt(Modemdata.Cmnt1, sizeof(Modemdata.Cmnt1));
      decrypt(Modemdata.Cmnt2, sizeof(Modemdata.Cmnt2));
      decrypt(Modemdata.Cmnt3, sizeof(Modemdata.Cmnt3));
      decrypt(Modemdata.Init1, sizeof(Modemdata.Init1));
      decrypt(Modemdata.Init2, sizeof(Modemdata.Init2));
      decrypt(Modemdata.Offhook, sizeof(Modemdata.Offhook));
      decrypt(Modemdata.Who, sizeof(Modemdata.Who));
   }
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
int  sortmanuf(const void *l, const void *r)
{
   manufdatatype *lp = (manufdatatype *) l;
   manufdatatype *rp = (manufdatatype *) r;
   return strcmp(lp -> Manufs, rp -> Manufs);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int  sortname(const void *l, const void *r)
{
   manufdatatype *lp = (manufdatatype *) l;
   manufdatatype *rp = (manufdatatype *) r;
   return strcmp(lp -> Name, rp -> Name);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int  sortmodemnum(const void *l, const void *r)
{
  manufdatatype *lp = (manufdatatype *) l;
  manufdatatype *rp = (manufdatatype *) r;

  if (lp->Number < rp->Number)
    return -1;
  else if (lp->Number > rp->Number)
    return  1;

  return 0;
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int  selectmanuf(void)
{
  static  int  Loop;

  static  char k,
               XValue,
               YValue,
               ZValue;

  static  char OK;

  static  char Str  [ 40 ],
               Temp [  5 ];

  boxcls(2,3,77,23,Colors[MENUBOX],SINGLE);
  fastcenter(3," M A N U F A C T U R E R S ",Colors[MENUTITLE]);
  fastcenter(23," Use arrow keys to move bar, press ENTER to select, ESC to exit ",Colors[MENUDESC]);
  strcpy(Mans[ManCount], "(not listed)");
  XValue = 3;
  YValue = 4;

  for (Loop = 1; Loop<ManCount+2; Loop++) {
    fastprint(XValue, YValue, Mans[Loop-1], Colors[MENUSELECT]);
    XValue = XValue + 15;
    if (Loop%5 == 0) {
      YValue++;
      XValue = 3;
    }
  }

  XValue = 3;
  YValue = 4;
  ZValue = 0;
  sprintf(Str, "%-14s", Mans[ZValue]);
  fastprint(XValue, YValue, Str, Colors[MENUBAR]);

   while (1) {
      k = inkey((char*)&OK, CLOCK);
      if (!OK)
      {
         if (k == 13 && ZValue+1 <= ManCount)
           return(ZValue+1);
         if (k == 13 && ZValue+1 > ManCount)
           return(0);
         if (k == 27)
           return(-1);
         sprintf(Temp, "%c", k);
         for (Loop=0; Loop<ManCount+1; Loop++)
         {
            if (strnicmp(Temp, Mans[Loop], 1) == 0)
            {
               ZValue = Loop;
               break;
            }
         }
      } else {
         if (k==75 || k==77 || k==72 || k==80)
         {
            if (k==75 && ZValue > 0)
              ZValue--;
            if (k==77 && ZValue < ManCount)
              ZValue++;
            if (k==72 && ZValue-4 > 0)
              ZValue = ZValue - 5;
            if (k==80 && ZValue+5 <= ManCount)
              ZValue = ZValue + 5;
         }
      }
      if (k==59 && OK)
        showhelp(PCBMODEMHLP);
      fastprint(XValue, YValue, Str, Colors[MENUSELECT]);
      XValue = ZValue % 5 * 15 + 3;
      YValue = ZValue / 5 + 4;
      sprintf(Str, "%-14s", Mans[ZValue]);
      fastprint(XValue, YValue, Str, Colors[MENUBAR]);
   }
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int pascal calcoffset(int DataOffset, char *Name)
{
  static unsigned Loop;

  for (Loop=0; Loop<Header.NumOfModems; Loop++)
  {
    if (strcmp(Manufdata[Loop].Manufs, Mans[DataOffset]) == 0)
      if (strcmp(Manufdata[Loop].Name, Name) == 0)
        return(Loop+1);
  }
  return(-1);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int pascal selectname(int DataOffset)
{
   static  char     NameCount,
                    k,
                    XValue,
                    YValue,
                    ZValue;
   static  char     OK;
   static  char     Str [ 40 ];
   static  unsigned Loop;

   boxcls(2, 3, 77, 23, Colors[MENUBOX], SINGLE);
   fastcenter(3, " M O D E M S ", Colors[MENUTITLE]);
   fastcenter(23,"Use arrow keys to move bar, press ENTER to select, ESC to exit",Colors[MENUDESC]);
   qsort(Manufdata, Header.NumOfModems, sizeof(manufdatatype), sortname);
   NameCount = 0;

   for (Loop = 0; Loop<Header.NumOfModems; Loop++)
   {
      if (strcmp(Manufdata[Loop].Manufs, Mans[DataOffset]) == 0)
      {
         strcpy(Names[NameCount], Manufdata[Loop].Name);
         NameCount++;
      }
   }

   strcpy(Names[NameCount], "(not listed)");
   XValue = 3;
   YValue = 4;

   for (Loop=1; Loop<NameCount+2; Loop++) {
      fastprint(XValue, YValue, Names[Loop-1], Colors[MENUSELECT]);
      XValue = XValue + 25;
      if (Loop%3 == 0) {
        YValue++;
        XValue = 3;
      }
   }

   XValue = 3;
   YValue = 4;
   ZValue = 0;
   sprintf(Str, "%-24s", Names[ZValue]);
   fastprint(XValue, YValue, Str, Colors[MENUBAR]);

   while (1) {
      k = inkey((char*)&OK, CLOCK);
      if (!OK) {
         if (k == 13 && ZValue+1 <= NameCount)
           return(calcoffset(DataOffset, Names[ZValue]));
         if (k == 13 && ZValue+1 > NameCount)
           return(0);
         if (k == 27)
           return(-1);
      } else {
         if (k==59)
           showhelp(PCBMODEMHLP+1);
         if (k==75 || k==77 || k==72 || k==80) {
            if (k==75 && ZValue > 0)
              ZValue--;
            if (k==77 && ZValue < NameCount)
              ZValue++;
            if (k==72 && ZValue-2 > 0)
              ZValue = ZValue - 3;
            if (k==80 && ZValue+3 <= NameCount)
              ZValue = ZValue + 3;
         }
      }
      fastprint(XValue, YValue, Str, Colors[MENUSELECT]);
      XValue = ZValue % 3 * 25 + 3;
      YValue = ZValue / 3 + 4;
      sprintf(Str, "%-24s", Names[ZValue]);
      fastprint(XValue, YValue, Str, Colors[MENUBAR]);
   }
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
int  selectmodem(void)
{
   static int RetCode;

   generalscreen(Head1,Head2);
   setcursor(CUR_BLANK);

   // Select the manufacturer
   if ((RetCode = selectmanuf()) < 1)
     return(RetCode);

   // Select the model of modem
   if ((RetCode = selectname(RetCode-1)) < 1)
     return(RetCode);

   CurrentModem = RetCode - 1;    // Correct Offset in Manufdata
   getmdmdata(FALSE);             // Read in the information for the modem
   return(1);
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void pascal getmdmdata(bool isDefault)
{
  if (!isDefault) {
    if (Manufdata[CurrentModem].StdCfg == 0)
      Offset = Manufdata[CurrentModem].Offset;
    else
      Offset = (long) sizeof(headertype) +
               (long) sizeof(manufdatatype) * MaxModem +
               (long) sizeof(modemdatatype) * (Manufdata[CurrentModem].StdCfg-1);
  } else {
    Offset = (long) sizeof(headertype) +
             (long) sizeof(manufdatatype) * MaxModem +
             (long) sizeof(modemdatatype) * (DefaultNum-1);
  }

  // Seek to the correct offset in MODEMS.DAT
  if (dosfseek(&DataFile, Offset, SEEK_SET) == -1)
    quit(2,"getmdmdata");

  // Read in the data for selected modem
  if (dosfread(&Modemdata, sizeof(modemdatatype), &DataFile) == -1)
    memset(&Modemdata,0,sizeof(modemdatatype));   // Clear the fields
  else
    dcrypt(FALSE);    // Decrypt the modem data.
}
// 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
void createarray(bool OpenFile)
{
  static unsigned Loop1;

  if (OpenFile) {
    if (dosfseek(&DataFile, 0, SEEK_SET) == -1)
      quit(4,"createarray");

    if (dosfread(&Header, sizeof(headertype), &DataFile) == -1)
      quit(5,"createarray");

    if (Header.Date < datetojulian("04/20/95"))
      quit(5,"createarray");

    if (dosfread(Manufdata, sizeof(manufdatatype) * MaxModem, &DataFile) == -1)
      quit(6,"createarray");

    dcrypt(TRUE);    // Decrypt Manufdata;
  }

  qsort(Manufdata, Header.NumOfModems, sizeof(manufdatatype), sortmanuf);
  ManCount = 0;

  if (Header.NumOfModems != 0) {
    strcpy(Mans[0], Manufdata[0].Manufs);
    for (Loop1=0; Loop1<Header.NumOfModems; Loop1++) {
      if (strcmp(Manufdata[Loop1].Manufs, Mans[ManCount]) != 0) {
        ManCount++;
        strcpy(Mans[ManCount], Manufdata[Loop1].Manufs);
      }
    }
    ManCount++;
  }
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
void  initprogram(void)
{
  static char *p;

  HelpVerifyStr  = VerifyStr;
  GeneralHelpNum = GENERAL;

  initscrnio();

  p = searchpath("PCBSM.HLP");
  if (p != NULL) strcpy(HelpName, p);
  else HelpName[0] = 0;

  p = searchpath("PCBSM.CLR");
  if (p != NULL) strcpy(ColorCnf, p);
  else ColorCnf[0] = 0;

  p = searchpath("MODEMS.DAT");
  if (p != NULL) strcpy(MFile, p);
  else quit(12,"initprog");

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
