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


/****************************************************************************
*
*                             PCB FIDO Utility
*                             Module MAINT.CPP
*
*                         Written by Stan Paulsen
*                            Copyright (C) 1995
*                            Clark Development
*
*****************************************************************************
*
*               Functions to perform Fido System maintenance
*
*****************************************************************************/

// Borland Header files
#include <iostream.h>
#include <stdio.h>
#include <dir.h>
#include <dos.h>
// #include <conio.h>
#include <ctype.h>

#include <stdlib.h>
#include <string.h>

// PCBoard headers

#include <pcb.h>
#include <bug.h>
#include <screen.h>
#include <scrnio.h>
//#include <scrnio.ext>
extern int KeyFlags;
#include <cnameidx.h>
//#include <pcbtools.h>
#include <misc.h>
#include <help.h>
#include <data.hpp>

extern "C" int _Cdecl getch( void );

// Local prototypes
static void _NEAR_ LIBENTRY  removeFidoConf       (NAREA_STRUCT & area, uint recno);
static void _NEAR_ LIBENTRY  removeDupeConfs      (void);
static void _NEAR_ LIBENTRY  removeInactiveConfs  (uint days);
static void _NEAR_ LIBENTRY  maintmenu            (void);
static void _NEAR_ LIBENTRY  packMSGs             (uint days);

// global data
bool  prompt = TRUE;

void LIBENTRY  maintain(void)
{
bool  dupe = FALSE;
bool  old  = FALSE;
bool  packmsg  = FALSE;
uint  days = 0,i=0,packdays=0;


  loadcnames(FALSE);
  if( _argc > 1)
  {
    for(i=0;i<_argc;i++)
    {
      strupr(_argv[i]);
      if(memcmp("/DUPES",_argv[i],6)      == 0) dupe = TRUE;
      if(memcmp("/INACTIVE",_argv[i],9)   == 0) old = TRUE;
      if(memcmp("/DAYS:",_argv[i],5)      == 0)
      {
        char * p = _argv[i]+6;
        days = atoi(p);
      }
      if(memcmp("/NOPROMPT",_argv[i],9)   == 0) prompt  = FALSE;
      if(memcmp("/PACKMSG",_argv[i],8)   == 0)
      {
          packmsg = TRUE;
          packdays = uint(atol(_argv[i]+9));
      }
    }
    if(dupe) removeDupeConfs();
    if(old)
    {
      if(days != 0) removeInactiveConfs(days);
      else          cout << "To remove inactive conferences, the /DAYS:xx switch must be used." << endl;
    }
    if(packmsg) packMSGs(packdays);
  }
  else
    maintmenu();

  opencnamesidx(FALSE);
  buildcnamesidx();
  closecnamesidx();
  closecnames();
}

static void _NEAR_ LIBENTRY  maintmenu(void)
{
bool     dupe = FALSE;
bool inactive = FALSE;
bool packmsg  = FALSE;
unsigned int days = 0,old=0;

const int qs = 6;

 char Q[6][40] = { "Remove Duplicate Fido Areas   ",
                   "Remove Inactive  Fido Areas   ",
                   "How many inactive Days?       ",
                   "Prompt Before Removal         ",
                   "Delete Sent MSG files         ",
                   "How Many Days Old?            "};
  char YN[]            = {2,'Y','N'};
  char ALLNUM[]        = {3,0,'0','9'};
//  char mask_alphanum[] = {3,0,' ','~'};

  FldType   * flds;

  flds=(FldType *)mallochk(qs*sizeof(FldType));
  if(!flds) return;

  initquest(flds,qs-1);

  addquest(flds,0,vBOOL,0,YN,2,3,1,Q[0],&dupe,CLEAR,NULL);
  addquest(flds,1,vBOOL,0,YN,2,5,1,Q[1],&inactive,CLEAR,NULL);
  addquest(flds,2,vUNSIGNED,0,ALLNUM,2,6,5,Q[2],&days,CLEAR,NULL);
  addquest(flds,3,vBOOL,0,YN,2,7,1,Q[3],&prompt,CLEAR,NULL);
  addquest(flds,4,vBOOL,0,YN,2,9,1,Q[4],&packmsg,CLEAR,NULL);
  addquest(flds,5,vUNSIGNED,0,ALLNUM,2,10,5,Q[5],&old,CLEAR,NULL);

  cls();
  readscrn(flds,qs-1,0,"  Fidonet style NA file processing  ","Press PGDN to begin. ",1,NOCLEARFLD);
  freescrn(flds,qs-1);
  free(flds);
  cls();
  if(KeyFlags == PGDN && dupe)        removeDupeConfs();
  if(KeyFlags == PGDN && inactive)    removeInactiveConfs(days);
  if(KeyFlags == PGDN && packmsg)     packMSGs(old);

}


static void _NEAR_ LIBENTRY  removeDupeConfs(void)
{
cAREAS areas;

NAREA_STRUCT     area1,area2;
unsigned    int  numareas = areas.totRecs();
unsigned    int  i,res=0;
char             info[2][20];

  cls();

  cout << endl << "Sorting AREAS.DAT" << endl;
  areas.sortFile();

  cout << endl << "             Removing duplicate AreaTags"<< endl;


  for(i = 1;i<=numareas;i++)
  {
    area1 = areas.getRec(i);
    area2 = areas.getRec(i+1);

    if(strcmp(area1.AreaTag,area2.AreaTag) == 0)
    {
      if(area1.ConfNum == area2.ConfNum)
      {
        areas.removeRec(i);
        cout << endl << "Removing "<<area1.AreaTag<<endl;
        continue;
      }
      cls();
      fastcenter(3,"Removing duplicate AreaTags",0x0C);

      sprintf(info[0],"1- Conference %u",area1.ConfNum);
      fastprint(1,5,info[0],0x0F);
      fastprint(20,5,area1.AreaTag,0x0B);

      sprintf(info[1],"2- Conference %u",area2.ConfNum);
      fastprint(1,10,info[1],0x0F);
      fastprint(20,10,area2.AreaTag,0x0B);

      fastprint(1,12,"KEEP which conference. (1/2)",0x0F);
      res = getch();
      if(res == '1') areas.removeRec(i);
      if(res == '2') areas.removeRec(i+1);
      if(res == 'q' || res == 'Q') break;

    }
  }
  cout <<"Press any key"<<endl;
  getch();



}

static void _NEAR_ LIBENTRY  removeInactiveConfs(uint days)
{
cAREAS         areas;
NAREA_STRUCT   area;
uint           totrecs=areas.totRecs();
uint           i;
uint           today = getjuliandate();

  for(i=1;i<=totrecs;i++)
  {
    area = areas.getRec(i);
    if(area.ConfNum == 0) continue;
    if( (today - area.LastActivityDate) >= days)
    {
      areas.close();
      removeFidoConf(area,i);
      areas.open();
    }
  }
}

void _NEAR_ LIBENTRY removeFidoConf(NAREA_STRUCT & area, uint recno)
{
cAREAS       areas;
pcbconftype  confrec;
char         res = ' ';

  if(recno !=  0)
  {
     getconfrecord(area.ConfNum,&confrec);
     cout << endl<<"Inactive Conference"<<endl;
     cout << endl << "Conference Name  : " << confrec.Name << endl;
     cout << "Conference Number: " << area.ConfNum << endl;
     cout << "Area Tag         : " << area.AreaTag << endl;
     if(prompt == TRUE)
     {
       cout << "Remove This conference? (Y/N/(A)bort) :";
       cin >> res;
     }
     else
     {
       cout << endl << "Removed" << endl << endl;
       res = 'Y';
     }

     if(res == 'y' || res == 'Y')
     {
       areas.removeRec(recno);
       memset(&confrec.Name,0,sizeof(confrec.Name));
       putconfrecord(area.ConfNum,&confrec);
       cout <<endl<< "Updating and sorting areas file..."<<endl;
     }
     if(res == 'A' || res == 'a') exit(0);
  }
}

static void _NEAR_ LIBENTRY packMSGs(uint days)
{
char            filespec[MAXFLEN];
char            file[MAXFLEN];
char            datestr[10];
char            base[MAXFLEN];
DIRECTORIES     dirs;
cDOSFILE        mfile;
STORED_FIDO_MSG msghdr;
int             done;
ffblk           fblk;
uint            curdate;
char            res;


  cls();
  if(prompt)
  {
    cout <<endl<<endl<<endl<<"About to delete all *.MSG files more than "<<days<<" days old."<<endl;
    cout << "continue? (Y/N)"<<endl;
    cin >> res;
  }
  else res = 'Y';

  res = toupper(res);

  if(res != 'Y') return;

  {
  cCONFIG cfg;
    cfg.getDirs(dirs);
  }

  maxstrcpy(base,dirs.outgoing_msg,sizeof(base));
  sprintf(filespec,"%-.*s*.MSG",MAXFLEN-15,base);

  curdate = getjuliandate();

  done = findfirst(filespec,&fblk,0);

  while(!done)
  {
    // if file is older than days, delete it
    if(curdate - datetojulian(dtoc(fblk.ff_fdate,datestr)) > days)
    {
       sprintf(file,"%-.*s%-.14s",MAXFLEN-15,base,fblk.ff_name);
       if(mfile.open(file,OPEN_READ | OPEN_DENYNONE) == 0)
       {
          mfile.read(&msghdr,sizeof(msghdr));
          if(!(msghdr.Attribute & MSG_SENT))
          {
             cout << endl <<file <<" has not been sent. Skipping.";
             done = findnext(&fblk);
             continue;
          }
          mfile.close();
       }
       unlink(file);
       cout << endl << "Deleting "<<file<<endl;
    }
    done = findnext(&fblk);
  }

  if(prompt)
  {
    cout << endl << "Press any key..." << endl;
    getch();
  }
}

