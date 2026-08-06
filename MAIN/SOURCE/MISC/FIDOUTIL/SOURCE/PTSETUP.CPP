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


/******************************************************************************
                                PTSETUP.CPP
                          Written by: Stan Paulsen

                 Copyright (C) 1995 Clark Development, inc.

     This module contains functions related to setting up passthroughs

 ******************************************************************************/

#define PCBSETUP
#include <iostream.h>
#include <dir.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include <ctype.h>

#include <bug.h>
#include <pcb.h>
#include <misc.h>

#include <screen.h>
#include <scrnio.h>
#include <screen.h>
#include <scrnio.ext>
#include <help.h>
//#include <users.h>

//#include <data.hpp>
#include <defines.h>
#include <ptsetup.hpp>
#include <passthru.hpp>
#include <fidonet.hpp>

static bool         pressedESC = FALSE;
static char         Mans[75][25];
static unsigned int ManCount;

void pascal setupAreas(void);
void pascal regUsers(void);
int         retEsc(int Before);
int         selectuser(void);
static void near pascal fillMans(void);
void pascal regUser(unsigned int usernum);

void pascal ptsetup(void)
{
      menusel(1,0,SMALL,0,TRUE);
}

void pascal setupAreas(void)
{

   menusel(2,0,SMALL,0,TRUE);
}

void pascal regUsers(void)
{
  int user=1;

  {
    cPTINFO PT;
      PT.updateUsers();
      PT.delOldUsers();
  }

  fillMans();
  while(user > 0)
  {
    cls();
    user = selectuser();
    if(user > 0) regUser(user-1);
  }

}


void pascal regUser(unsigned int usernum)
{
cPTINFO PT;
  unsigned int row    = 1,low,hi;
  unsigned int screen = 0;
  char * area         =NULL;
  char * range        =NULL;
  bool   done         = FALSE,add=FALSE,found=FALSE;
  char   response[100],line[90];
  char mask_alphanum[] = {3,0,' ','~'};
  REGREC addU,removeU;

  memset(&addU,0xFF,sizeof(addU));
  memset(&removeU,0x00,sizeof(removeU));
  removeU.A=1;

  unsigned int numareas = PT.numAreas();
  while(!done)
  {
    low = hi = 0;
    cls();

    if(screen == UINT_MAX) screen = (numareas / 24) ;
    if(screen > (numareas / 24) )  screen = 0;
    for(row=1;row<22;row++)
    {
      area = PT.seekToAreaNum(row+(screen*21));
      if(area == NULL) break;
      sprintf(line,"%2d) %-60s [%c]",row+(screen*21),area == NULL ? " " : area,PT.userInArea(area,usernum) ?'X' : ' ');
      fastprint(2,row,line,0x0b);
    }

    memset(response,0,sizeof(response));
    fastprint(1,22,"(SR)[n-n]Sel Range (DR)[n-n]De-Sel Range (SA)Sel All (DA)De-Sel All ",0x0A);
    inputstr(1,23,10,"(DEL)[n] Delete Record n",response,response,mask_alphanum,INPUT_CAPS,0);
    strupr(response);
    found = FALSE;

    if( (range = strstr(response,"DEL")) != NULL )
    {
    uint anum;
      while(*range != ' ' && *range != '\x0') ++range;
      anum = atoi(range);
      PT.removeArea(PT.seekToAreaNum(anum));
      continue;
    }

    if( (range = strstr(response,"SR") ) != NULL )
    {
       while(!isdigit(*range) && *range != '\x0') range++;
       low = atoi(range);
       range = strchr(range,'-');
       if(range) hi = atoi(++range);
       if(low > hi) low = hi;
       if(hi > numareas) hi = numareas;
       add  = TRUE;
       found = TRUE;
    }
    if( (range = strstr(response,"DR"))!= NULL )
    {
       while(!isdigit(*range) && *range != '\x0') range++;
       low = atoi(range);
       range = strchr(range,'-');
       if(range)         hi = atoi(++range);
       if(low > hi)     low = hi;
       if(hi > numareas) hi = numareas;
       add  = FALSE;
       found = TRUE;
    }
    if(strstr(response,"DA") )
    {
       add  = FALSE;
       low  = 1;
       hi   = numareas;
       found = TRUE;
    }
    if(strstr(response,"SA"))
    {
      add  = TRUE;
      hi   = numareas;
      low  = 1;
       found = TRUE;
    }

    if(KeyFlags == PGUP) screen--;
    if(KeyFlags == PGDN || (KeyFlags == RET && response[0] == '\x0')) screen++;
    if(strchr(response,'Q') || KeyFlags == ESC)
    {
      low = hi = 0;
      done = add = found = TRUE;
    }

    if( !found && (range = strchr(response,'-') )!= NULL )
    {
       low  = atoi(response);
       hi   = atoi(++range);
       add  = found = TRUE;
    }
    else if (!done && !found)
    {
       low = hi = atoi(response);
       add = found = TRUE;
    }

    for(;low <= hi && low!= 0;low++)
    {
      area = PT.seekToAreaNum(low);
      if(!done && PT.userInArea(area,usernum)) add = FALSE;
      PT.seekToAreaNum(low);
      PT.seekToUser(usernum);
      if(add) PT.modifyRegRec(addU);
      else    PT.modifyRegRec(removeU);
    }
  }// while
}


void pascal setupAreasNA(void)
{
 char NALoc[MAXDIR];
 char Q[60] = "Name/Location of NA file   ";
 char mask_alphanum[] = {3,0,' ','~'};
 FldType   * flds;
 FIELDS      F;

  flds=(FldType *)mallochk(sizeof(FldType));
  if(!flds) return;

  maxstrcpy(NALoc,"PASSTHRU.NA",sizeof(NALoc));

  initquest(flds,0);

  addquest(flds,0,vUPSTR,0,mask_alphanum,2,3,45,Q,NALoc,CLEAR,retEsc);

  cls();
  readscrn(flds,0,0,"  Fidonet Style NA File Passthru Setup  ","Press PGDN to execute",1,NOCLEARFLD);
  freescrn(flds,0);

  fastprint(10,10,"Processing NA File, Please wait...",0x0b);
  memset(&F,0,sizeof(F));
  maxstrcpy(F.NALoc,NALoc,sizeof(F.NALoc));
  if(KeyFlags == PGDN) processNa(F,TRUE);
  free(flds);
}

int retEsc(int Before)
{
  if(Before) return 0;

  if(KeyFlags == ESC) pressedESC = TRUE;
  if(KeyFlags == RET)
     KeyFlags = ESC;

  return 2;
}



void pascal setupAreasMan(void)
{
 char name[AREA_SIZE];
 char oldname[AREA_SIZE];
 char Q[60] = "Area Tag to Add   ";
 char mask_alphanum[] = {3,0,' ','~'};
 FldType   * flds;
 cPTINFO PT;


  flds=(FldType *)mallochk(sizeof(FldType));
  if(!flds) return;
  initquest(flds,0);
  addquest(flds,0,vUPSTR,0,mask_alphanum,10,5,45,Q,name,CLEAR,retEsc);
  cls();
  fastprint(10,10,"Press ESC to quit adding areas",0x0b);
  while(pressedESC == FALSE)
  {
    memset(name,0,sizeof(name));
    readscrn(flds,0,0,"  Fidonet Style NA File Passthru Processing  ","Press PGDN to begin, ESC to abort.",1,NOCLEARFLD);
    if(name[0] != '\x0')
    {
       PT.addArea(name);
       fastprint(10,8,"Passthru Area Added :",0x0b);
       fastprint(37,8,"                                        ",0x0b);
       fastprint(37,8,name,0x0b);
    }

  }
  pressedESC = FALSE;
  freescrn(flds,0);
  free(flds);

}


void pascal addPArea(const char * tag)

{
  cPTINFO ptinfo;
  ptinfo.addArea(tag);
}

static void near pascal addrToNum(const char * addr, unsigned int & zone,
                                  unsigned int & net, unsigned int & node,
                                  unsigned int & point)
{
char   tBuf[25];
char * bPtr = NULL;

  // Initializations
  zone = net = node = point = 0;
  maxstrcpy(tBuf,(char *)addr,sizeof(tBuf));

  // grab ZONE
  bPtr = tBuf;
  zone = atoi(bPtr);

  // grab NET
  bPtr = strchr(tBuf,':');
  if(!bPtr++) return;
  net = atoi(bPtr);

  // Grab ZONE
  bPtr = strchr(tBuf,'/');
  if(!bPtr++) return;
  node = atoi(bPtr);

  // grab POINT
  bPtr = strchr(tBuf,'.');
  if(!bPtr++) return;
  point = atoi(bPtr);
}

static void near pascal fillMans(void)
{
  cPTINFO PT;
  char * addr;
  unsigned int i=0;

  while( (addr = PT.getUser(i) ) != NULL)
  {
     maxstrcpy(Mans[i],addr,sizeof(Mans[i]));
     i++;
  }
  ManCount = i+1;

}

int  selectuser(void)
{


  static  int  Loop;

  static  char k,
               XValue,
               YValue,
               ZValue;

  static  bool OK;

  static  char Str  [ 40 ],
               Temp [  5 ];


  boxcls(2,3,77,23,Colors[MENUBOX],SINGLE);
  fastcenter(3,"Select User to Register   ",Colors[MENUTITLE]);
  fastcenter(23," Use arrow keys to move bar, press ENTER to select, ESC to exit ",Colors[MENUDESC]);
  fastcenter(24," To add new users, please use PCBSM",Colors[MENUDESC]);
  //strcpy(Mans[ManCount], "(not listed)");
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
      { char ext; k = inkey(&ext, CLOCK); OK = ext; }
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
