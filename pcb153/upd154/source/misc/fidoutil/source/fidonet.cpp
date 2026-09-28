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
                                                                FIDONET.CPP
                                                  Written by: Stan Paulsen

                                 Copyright (C) 1995 Clark Development, inc.

  This module contains functions related to processing the FIDONET.NA file

 ******************************************************************************/


#include <iostream.h>
#include <dir.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>
#include <stdlib.h>
// #include <conio.h>

//#include <pcbtools.h>
#include <misc.h>
#include <pcb.h>
#include <data.hpp>
#include <scrnio.h>
//#include <scrnio.ext>
extern int KeyFlags;
#include <cnameidx.h>
#include <fidonet.hpp>
#include <ptsetup.hpp>
#include <defines.h>
#include <structs.h>
#include <dosclass.hpp>
#include <validate.h>
#include <dosclass.hpp>
#include <passthru.hpp>

extern "C" int _Cdecl getch( void );

static void near pascal fdNetMenu(void);
static bool near pascal fdNetCmdLn(FIELDS & F);
static void near pascal mangleFNameToPath(char * name, char * path);
static void near pascal addArea(const char * tag, const char * desc,FIELDS & f);
static bool near pascal confirm(char * msg);
static void near pascal doAreasBBS(char * filename);
static void near pascal processBBSline(const char * line);
static void near pascal addExistingArea(unsigned int confnum,const char * confname);
static void near pascal eatchars(char ** p);
static void near pascal eatwhite(char ** p);

char ALLFILE[]  = {10, 0,'#',')', '!', 0,'-',':', 0,'A','z' };
static unsigned int    curConf = 0;
static int allHit = FALSE;
char             mPath[MAXDIR];
pcbconftype  tlate;


#define FIDO_TYPE  5

void pascal fidonet(void)
{
bool cmd= FALSE;
FIELDS F;

  if(_argc >= 2) (cmd = fdNetCmdLn(F));
  if(cmd)
  {
        cls();
        sprintf(mPath,"%s\\",F.MSGLoc);
        processNa(F,FALSE);
  }
  else
        fdNetMenu();



}


static void near pascal fdNetMenu(void)
{
 char Q[7][60] = { "Name/Location of NA file   ",
                                   "Path to Message Base Files ",
                                   "Begin    Conference Number ",
                                   "Template Conference Number ",
                                   "Overwrite Existing  Conference Name ",
                                   "Use Areatag     for Conference Name ",
                                   "Use Description for Conference Name "};
  char YN[]                = {2,'Y','N'};
  char ALLNUM[]            = {3,0,'0','9'};
  char mask_alphanum[] = {3,0,' ','~'};

  FIELDS        F;
  FldType       * flds;

  flds=(FldType *)mallochk(NUMQS*sizeof(FldType));
  if(!flds) return;

  maxstrcpy(F.NALoc,"FIDONET.NA",sizeof(F.NALoc));
  F.MSGLoc[0] = '\x0';
  F.bConf                = 0;
  F.TemplateConf = 0;
  F.ovWr                 = FALSE;
  F.UseATag       = TRUE;
  F.UseDesc      = FALSE;


  initquest(flds,NUMQS-1);

  addquest(flds,0,vSTR,0,mask_alphanum,2,3,45,Q[0],F.NALoc,CLEAR,NULL);
  addquest(flds,1,vSTR,0,mask_alphanum,2,4,45,Q[1],F.MSGLoc,CLEAR,NULL);
  addquest(flds,2,vUNSIGNED,0,ALLNUM,2,6,5,Q[2],&F.bConf,CLEAR,NULL);
  addquest(flds,3,vUNSIGNED,0,ALLNUM,2,7,5,Q[3],&F.TemplateConf,CLEAR,NULL);
  addquest(flds,4,vBOOL,0,YN,2,9,1,Q[4],&F.ovWr,CLEAR,NULL);
  addquest(flds,5,vBOOL,0,YN,2,11,1,Q[5],&F.UseATag,CLEAR,NULL);
  addquest(flds,6,vBOOL,0,YN,2,12,1,Q[6],&F.UseDesc,CLEAR,NULL);

  cls();
  readscrn(flds,NUMQS-1,0,"  Fidonet style NA file processing  ","Press PGDN to begin, ESC to abort",1,NOCLEARFLD);
  freescrn(flds,NUMQS-1);


  free(flds);
  cls();
  maxstrcpy(mPath,F.MSGLoc,sizeof(mPath));
  if(KeyFlags == PGDN) processNa(F,FALSE);

}

static bool near pascal fdNetCmdLn(FIELDS & F)
{
  char             fidonetPath[MAXDIR];


  memset(&F,0,sizeof(F));
  F.ovWr        = FALSE;
  F.UseATag = TRUE;
  F.UseDesc = FALSE;

        memset(fidonetPath,0,sizeof(fidonetPath));
        memset(mPath,0,sizeof(mPath));

        for (int i = 0; i < _argc; ++i)
        {
                char * p = _argv[i];
                         if (memicmp(p,"/START:",    7) == 0) F.bConf = unsigned(atol(p+ 7));
                else if (memicmp(p,"/TEMPLATE:",10) == 0) F.TemplateConf = unsigned(atol(p+10));
                else if (memicmp(p,"/INDEX",     6) == 0) buildcnamesidx();
                else if (memicmp(p,"/MSGS:",     6) == 0) maxstrcpy(F.MSGLoc,p+6,sizeof(F.MSGLoc));
                else if (memicmp(p,"/FIMPORT:",9)   == 0) maxstrcpy(F.NALoc,p+9,sizeof(fidonetPath));
                else if (memicmp(p,"/SETNAMES",10)  == 0) F.ovWr    = TRUE;
                else if (memicmp(p,"/USETAG",8)     == 0) F.UseATag = TRUE;
                else if (memicmp(p,"/USEDESC",9)    == 0) F.UseDesc = TRUE;
        }
        if(F.bConf != 0 && F.MSGLoc[0] != '\x0' && F.NALoc[0] != '\x0') return TRUE;

        return FALSE;

}

/******************************************************************************/

void pascal processNa(FIELDS & f, bool passthru)
{
 cDOSFILE nafile;
 char     line[100];

  if(fileexist((char *)f.NALoc) == 255  || nafile.open((char *)f.NALoc,OPEN_READ | OPEN_DENYNONE) < 0) return;
  curConf = f.bConf;

  loadcnames(FALSE);
  getconfrecord(f.TemplateConf,&tlate);
  opencnamesidx(READ_IDX);
  while(nafile.getln(line,sizeof(line)) == 0  )
  {
        // Need to check for ESC
        processLine(line,passthru,f);
  }

  buildcnamesidx();
  closecnamesidx();
  {
        cAREAS areas;
        cout << endl << "                    Sorting AREAS.DAT..." << endl;
        areas.sortFile();
  }
  cout << endl <<       "                    Done.   Press any key..."<<endl;
  getch();
}

static void near pascal processLine(const char * line, bool passthru, FIELDS & f)
{
  char tag[100];
  char desc[100];
  char * walkPtr=NULL;
  unsigned int i=0;

        memset(tag,0,sizeof(tag));
        memset(desc,0,sizeof(desc));


        walkPtr = (char *)line;

        // Walk though the TAG field in the line
        while(*walkPtr != ' ' && i <= sizeof(tag) && *walkPtr != '\x0')
          tag[i++] = *walkPtr++;

        // Make sure we got to the end of the TAG field
        while(*walkPtr != ' ' && *walkPtr != '\x0') walkPtr++;

        // Now walk though the spaces
        while(*walkPtr == ' ' && *walkPtr != '\x0') walkPtr++;


        i=0;

        //Now walk through the description field
        while(*walkPtr != '\x0' && i < sizeof(desc))
          desc[i++] = *walkPtr++;

        stripright(tag,' ');
        stripright(desc,' ');
        if(passthru) addPArea(tag);
        else             addArea(tag,desc,f);
}





static bool near pascal confirm(char * msg)
{
        //allHit = FALSE;

        char a;

        cout << endl << msg << "?  ";
        cin >> a;

        if (toupper(a) == 'A')
        {
                allHit = TRUE;
                a = 'Y';
        }

        return (toupper(a) == 'Y');
}

static void near pascal mangleFNameToPath(char * name, char * path)
{
char buf[MAXDIR];
        strcpy(path,mPath);

        char tmp [ 128 ]; // Need this much space for later
        strcpy(tmp,name);

        char * p;

        do {

                p = strpbrk(tmp,"/. ");
                if (p) strcpy(p,p+1);

        } while (p);

        if (strlen(tmp) == 0)
                strcpy(tmp,"MSGS");
        else
                tmp[5] = NUL;

        char subdir [ 1 + 1 + 1 ];
        subdir[0] = tmp[0];
        subdir[1] = '\\';
        subdir[2] = NUL;

        strcat(path,subdir);
        validatepath(NULL,path,buf,1);
        strcat(path,tmp);

        int seq = 0;

        do {

                sprintf(tmp,"%s%03d",path,++seq);

        } while (fileexist(tmp) != 255);

        strupr(strcpy(path,tmp));


        int h = doscreatecheck(path,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);

        if (h > 0)
        {
                memset(tmp,0x20,sizeof(tmp));
                memset(tmp,0x00,12);
                writecheck(h,tmp,sizeof(tmp));
                dosclose(h);
        }
}


static void near pascal addArea(const char * tag, const char * desc,FIELDS & f)
{
  cAREAS           areas;
  NAREA_STRUCT tarea;
  pcbconftype  rec;
  char             name [ AREA_SIZE + 1 ];

        if(areas.findRec(tag) != 0)
        {
           cout << endl << "Areatag " << tag << " aleady exists. Skipping." << endl;
           return;
        }



        unsigned int conf = curConf++;

        static int overWriteAll = FALSE;

        maxstrcpy(name,f.UseATag ? (char *)tag : (char *) desc,sizeof(name));

        cout << endl << "Checking conference " << conf << " ...";

        if (getconfbyname(name) >= 0)
        {
                cout << endl << "Conference named " << name << " already exists!";
                if (confirm("Skip fido area (Y/N)")) return;
        }

        for (getconfrecord(conf,&rec);
        rec.Name[0] != '\x0';
        getconfrecord(conf,&rec))
        {
          if (overWriteAll)
          {
                 rec.Name[0] = '\x0';
                 break;
          }
          cout << endl << "Conference in use by (" << rec.Name << ")!";
          if      (!overWriteAll && confirm("Overwrite conference (Y/N/A) [Yes No All]")) // overwrite
          {
                 if (allHit) overWriteAll = TRUE;
                 rec.Name[0] = NUL;
                 break;
          }
          else if (confirm("Find next available (Y/N)"))  // next
                 ++conf;
          else                                                                          // skip
                 break;
        }

        if (rec.Name[0] == '\x0')
        {
           cout << endl << "Inserting (" << (f.UseATag ? tag : desc) <<") as conference " << conf << " ...";


           getconfrecord(conf,&rec);

           // $$$ remove old areatag before adding the new one
           tarea = areas.getAreaRec(conf);
           unsigned int recno = areas.findRec(tarea.AreaTag);
           if(recno != 0)  areas.removeRec(recno);

           memcpy(&rec,&tlate,sizeof(rec));
           memset(rec.Name,0,sizeof(rec.Name));
           strcpy(rec.Name,name);

           // rec.PublicConf       = TRUE;
           // rec.AutoRejoin       = FALSE;
           // rec.ViewMembers      = FALSE;
           // bool PrivUplds;
                  rec.PrivMsgs             = FALSE;
                  rec.EchoMail             = TRUE;
           // rec.ReqSecLevel      = 0;
           // rec.AddSec                   = 0;
           // rec.AddTime                  = 0;
           // rec.MsgBlocks        = 1;
           //   if (f.ovWr)  mangleFNameToPath((char *)tag,rec.MsgFile);
                  mangleFNameToPath((char *)tag,rec.MsgFile);
           // char UserMenu[32];
           // char SysopMenu[32];
           // char NewsFile[32];
           // char PubUpldSort;
           // char UpldDir[29];
           // char PubUpldLoc[26];
           // char PrvUpldSort;
           // char PrivDir[29];
           // char PrvUpldLoc[26];
           // char DrsMenu[29];
           // char DrsFile[33];
           // char BltMenu[29];
           // char BltNameLoc[33];
           // char ScrMenu[29];
           // char ScrNameLoc[33];
           // char DirMenu[29];
           // char DirNameLoc[33];
           // char PthNameLoc[33];
           // rec.ForceEcho        = TRUE;
           // rec.ReadOnly                 = FALSE;
           // rec.NoPrivateMsgs    = FALSE;
           // rec.RetReceiptLevel  = 255;
           // rec.RecordOrigin     = FALSE;
                  rec.PromptForRouting = FALSE;
           // rec.AllowAliases     = FALSE;
           // rec.ShowIntroOnRA    = FALSE;
           // rec.ReqLevelToEnter  = 0;
           // rec.Password[0]      = NUL;
           // char Intro[32];
           // char AttachLoc[32];
           // rec.RegFlags[0]      = NUL;
           // char AttachLevel;
           // char CarbonLimit;
           // char CmdLst[32];
           // rec.OldIndex                 = FALSE;
                  rec.LongToNames          = FALSE;
           // char CarbonLevel;
                  rec.ConfType             = FIDO_TYPE;
                  rec.ExportPtr            = 0;
                  putconfrecord(conf,&rec);

                  NAREA_STRUCT area;
                  memset(&area,0,sizeof(area));
                  area.ConfNum = conf;
                  maxstrcpy(area.AreaTag,(char *)tag,sizeof(area.AreaTag));
                  strupr(area.AreaTag);
                  area.AkaIndex = 0;
                  area.OriginIndex = 0;
                  area.HighAscii = 'N';
                  area.LastActivityDate = getjuliandate();
                  areas.addRec(area);
        }

//              cout << endl;
}

void areasBBS(void)
{
unsigned int i;
char filename[MAXFLEN];
  for(i=0;i<_argc;i++)
  {
         if (memicmp(_argv[i],"/FILE:",6) == 0)
         {
          maxstrcpy(filename,_argv[i]+6,sizeof(filename));
          doAreasBBS(filename);
          {
          cAREAS  areas;
                areas.sortFile();
          }
          return;
         }
  }
}

static void near pascal doAreasBBS(char * filename)
{
cDOSFILE   bbsfile;
char       line[100];

  if(bbsfile.open(filename,OPEN_RDWR | OPEN_DENYNONE) != 0) return;

  while( bbsfile.getln(line,sizeof(line)) == 0)
        processBBSline(line);
}

static void near pascal processBBSline(const char * line)
{
char *           ptr = (char *)line;
char *           end=NULL;
char             confname[100];
unsigned int confnum = 0;
bool             passthru=FALSE;


  if(*ptr == ';') return;
  if(isdigit(*ptr)) confnum = atoi(ptr);
  else if(*ptr == 'P') passthru = TRUE;
  eatchars(&ptr);
  if(*ptr == '\x0') return;
  eatwhite(&ptr);
  if(*ptr == '\x0') return;
  end=ptr;
  eatchars(&end);
  if(size_t(end-ptr) < sizeof(confname)) maxstrcpy(confname,ptr,size_t(end-ptr+1));
  else                                                                   maxstrcpy(confname,ptr,sizeof(confname));

  if(!passthru)  addExistingArea(confnum,confname);
  else
  {
        cPTINFO  PT;
        PT.addArea(confname);
        cout << endl << "Added passthru area:" << confname;
  }

}

static void near pascal addExistingArea(unsigned int confnum,const char * confname)
{

pcbconftype  rec;

           loadcnames(FALSE);

           getconfrecord(confnum,&rec);

           opencnamesidx(READ_IDX);

                  rec.PrivMsgs             = FALSE;
                  rec.EchoMail             = TRUE;
                  rec.PromptForRouting = FALSE;
                  rec.LongToNames          = FALSE;
                  rec.ConfType             = FIDO_TYPE;
                  rec.ExportPtr            = 0;
                  putconfrecord(confnum,&rec);

                  NAREA_STRUCT area;
                  cAREAS areas;
                  memset(&area,0,sizeof(area));
                  area.ConfNum = confnum;
                  maxstrcpy(area.AreaTag,(char *)confname,sizeof(area.AreaTag));
                  area.AkaIndex = 0;
                  area.OriginIndex = 0;
                  area.HighAscii = 'N';
                  area.LastActivityDate = getjuliandate();
                  areas.addRec(area);
                  cout << endl << "Conference " << confnum <<" converted to PCBoard Fido area: " << rec.Name;
}

static void near pascal eatchars(char ** p)
{
  while(*(*p) != ' ' && *(*p) != '\x0') ++(*p);
}

static void near pascal eatwhite(char ** p)
{
  while(*(*p) == ' ' && *(*p) != '\x0') ++(*p);
}






