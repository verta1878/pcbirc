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
*                                                         PCB FIDO Utility
*                                                               Module PCBFU
*
*                                                 Written by Stan Paulsen
*                                                        Copyright (C) 1995
*                                                        Clark Development
*
*
*****************************************************************************/

// Borland Header files
#include <iostream.h>
#include <stdio.h>
#include <dir.h>
#include <dos.h>
// #include <conio.h>

#include <stdlib.h>
#include <string.h>

// PCBoard headers

//#include <misc.h>
#include <bug.h>
#include <screen.h>
#include <scrnio.h>
//#include <scrnio.ext>
//#include <pcbtools.h>
#include <misc.h>
#include <pcb.h>
#include <help.h>
#include <vmdata.h>
#include <data.hpp>

extern char *HelpVerifyStr;
extern int GeneralHelpNum;
extern char HelpName[];
extern char ColorCnf[];
extern char MainHead1[];
extern char MainHead2[];
extern char MainHead3[];
extern int KeyFlags;

// Project headers
#include <convert.hpp>
#include <fidonet.hpp>
#include <ptsetup.hpp>
#include <report.hpp>
#include <analize.hpp>
#include <maint.hpp>

extern "C" int _Cdecl getch( void );

// Prototypes
void static near pascal processArgs(int numargs, char ** args);
void static near pascal menu(void);
int                      pascal srchpath(char * FileName);
static void near pascal printCommandHelp(void);

// Constants
const int CONVERT  = 1;
const int FIDONET  = 2;
const int PASSTHRU = 3;

extern unsigned int _stklen = 0x8000U;

const int MAIN_MENU    = 0;
const char * PCBVERSTR = "Version 15.23";
#define NumEKeys 27
#define MenuItemTotal  10
#define NumMenu 10
int NumExitKeys = NumEKeys;
int ExitKeyFlag[NumEKeys];
char ExitKeyNum[NumEKeys];
char MenuAvail[MenuItemTotal];
char MenuList[MenuItemTotal][36];
void pascal (*MenuFunc[MenuItemTotal])(void);
struct MenuType Menu[NumMenu];


pcbdattype PcbData;
char             PCBDat[MAXDIR];
char *DatFile=NULL;


int main(int argc, char **argv)
{

  VMDataStartUp("VMDATA.$$$",32,64,VM_FALSE);

  maxstrcpy(PCBDat,"PCBOARD.DAT",sizeof(PCBDat));
  if(fileexist(PCBDat) == 255) srchpath(PCBDat);
  if(fileexist(PCBDat) == 255)
  {
        cout << "Could not locate PCBOARD.DAT" << endl;
        cout << "Press any key..."<<endl;
        getch();
        return 1;
  }
  DatFile = PCBDat;
  //pcbinit();
  readdatfile();

  if(argc > 1) processArgs(argc,argv);
  else             menu();

  cls();
  return 0;
}

void static near pascal processArgs(int numargs, char ** args)
{

  unsigned int i = 0;

  for(i=0;i<numargs;i++)
  {
        if(strcmpi("/CONVERT",args[i]) == 0) convert();
  }

  for(i=0;i<numargs;i++)
  {
        if(strcmpi("/FIDONET",args[i]) == 0) fidonet();
        if(strcmpi("/AREASBBS",args[i])== 0) areasBBS();
        if(strcmpi("/REPORT",args[i])  == 0) generateLog();
        if(strcmpi("/MAINT",args[i])   == 0) maintain();
        if(strcmpi("/ANALIZE",args[i]) == 0) analize();
        if(strcmpi("/HELP",args[i])== 0 || strcmpi("?",args[i])== 0 || strcmpi("/?",args[i])==0) printCommandHelp();
  }

}
void static near pascal menu(void)
{
  HelpVerifyStr  = (char *)VERIFYSTR;
  GeneralHelpNum = GENERAL;
  strcpy(HelpName,"PCBFU.HLP");
  srchpath(HelpName);
  strcpy(ColorCnf,"PCBSM.CLR");
  srchpath(ColorCnf);
  initscrnio();


  setupscale(15,177,5);
  initmenu(MAIN_MENU,FALSE,"Select Option",PCBDat,(char *)PCBVERSTR);
  addmenu(MAIN_MENU,"Convert PCBFIDO.CFG",convert);
  addmenu(MAIN_MENU,"Process FIDONET.NA",fidonet);
  addmenu(MAIN_MENU,"Setup   Pass-Through areas",ptsetup);
  addmenu(MAIN_MENU,"Generate Fido log report",generateLog);
  //addmenu(MAIN_MENU,"Perform  Fido System Analysis",analize);
  addmenu(MAIN_MENU,"Perform  Fido System Maintenance",maintain);
  initmenu(1,FALSE,"Select Option","",(char *)PCBVERSTR);
  addmenu(1,"Setup Passthu Areas",setupAreas);
  addmenu(1,"Register Users in  Areas",regUsers);
  initmenu(2,FALSE,"Select Option","",(char *)PCBVERSTR);
  addmenu(2,"Use NA type file",setupAreasNA);
  addmenu(2,"Manually add areas",setupAreasMan);


  strcpy(MainHead1, "PCBoard Fido Utility");
  strcpy(MainHead2, "Copyright (C) 1996 Clark Development, Inc.");

   do
   {
          menusel(MAIN_MENU,0,SMALL,0,TRUE);
   }while(KeyFlags != ESC);

}

static void pascal near printCommandHelp(void)
{
  cout << endl;
  cout << "To use the command line interface, enter PCBFUTIL with no command line" << endl;
  cout << "arguments." << endl;
  cout << endl;
  cout << "  /REPORT      Compile Fido activity report from all caller logs." << endl;
  cout << "  /CONVERT     Convert Fido configuration files from version 2 to version 3." << endl;
  cout << endl;
  cout << "  /FIDONET     Process FIDONET.NA style file." << endl;
  cout << "    /START:xxx                  Conference number to start with." << endl;
  cout << "    /TEMPLATE:xxx               Template conference number." << endl;
  cout << "    /INDEX                      Build index files." << endl;
  cout << "    /MSGS:d:\\pcb\\main\\msgs      Path to message subdirectory." << endl;
  cout << "    /FIMPORT:d:\\pcb\\fidonet.na  Path to FIDONET.NA type file." << endl;
  cout << "    /SETNAMES                   Overwrite existing conferenfce names." << endl;
  cout << "    /USETAG                     Use fidonet areatag for conference name." << endl;
  cout << "    /USEDESC                    Use fidonet description for conference name." << endl;
  cout << endl;
  cout << "  /PASSTHRU    Set up passthru areas." << endl;
  cout << "    /FIMPORT:d:\\pcb\\passthru.na Path to passthru NA type file." << endl;
  cout << endl;
  cout << "<more>";
  getch();

  cout << endl<<"  /MAINT       Perform Fido System Maintenance" << endl;
  cout << "   /DUPES      Remove Duplicate Fido Areas." << endl;
  cout << "   /INACTIVE   Remove Inactive  Fido Areas." << endl;
  cout << "     /DAYS     Number of days to consider a conference inactive."<<endl;
  cout << "   /NOPROMPT   Supress Promting before removing conference."<<endl<<endl;
  cout << "   /PACKMSG:xx Delete SENT MSG files older than xx days."<<endl<<endl;
  cout << "  /AREASBBS    Setup existing areas using an AREAS.BBS type file" << endl;
  cout << "   /FILE:file  Name/Location of areas.bbs file"<<endl;

}




