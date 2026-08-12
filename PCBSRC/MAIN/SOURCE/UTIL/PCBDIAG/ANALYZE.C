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


#include <io.h>
#include <dir.h>
#include <dos.h>
#include <sys/stat.h>
#include <alloc.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <event.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMFIELDS  3

#define MUSTHAVE 0
#define WILLMAKE 1
#define YOUMAKE  2

int  HighestHandle;

char CurrentPath[40];
char RemoteFile[66];
char CompressFile[66];
char BoardBatFile[66];
char QwkCompressFile[66];
pcbconftype OldConf;
pcbconftype Conf;

bool PrintOuts[NUMFIELDS];

char Table[256];
int  NumSec;
bool BadPwrd;
bool BadIndex;

FldType Flds[NUMFIELDS] = {
  {vBOOL,YESNO, 0, 3, 8, 1,NOCLEARFLD,"Print ALL configuration files","",&PrintOuts[0],NULL },
  {vBOOL,YESNO, 0, 3, 9, 1,NOCLEARFLD,"Perform the Analysis Section ","",&PrintOuts[1],NULL },
  {vBOOL,YESNO, 0, 3,10, 1,NOCLEARFLD,"Analyze USERS and INDEX files","",&PrintOuts[2],NULL }
};


/********************************************************************
*
*  Function: checkpath()
*
*  Desc    : Checks a Path to see if it exists or if it needs to be created
*
*/

void  checkpath(char Msg[], char OrigPath[]) {
  char Output[200];
  char Str[80];
  char Temp[60];
  char Path[60];
  int  Len;
  char X;
  bool RemovedBackSlash;

  #ifdef DEBUG
    mc_register(Output,sizeof(Output));
    mc_register(Str,sizeof(Str));
    mc_register(Temp,sizeof(Temp));
    mc_register(Path,sizeof(Path));
  #endif

  strcpy(Path,OrigPath);

  if (Path[0] == 0) {
    strcpy(Temp,"No path defined");
    goto end;
  } else {
    strcpy(Temp,Path);
    addbackslash(Temp,sizeof(Temp));       /*  put the backslash at the end */
  }

  Len = strlen(Path);                      /*                                */
  if (Len >= 1 && Path[Len-1] == '\\') {   /*  remove the trailing backslash */
    Path[Len-1] = 0;                       /*                                */
    RemovedBackSlash = TRUE;
  } else RemovedBackSlash = FALSE;

  sprintf(Str,"%-30s(%-14s) ",Temp,Msg);

  if (Path[1] == ':' && ! driveok(Path[0])) {
    strcat(Str,"ERROR drive invalid");
  } else {
    X = fileexist(Path);

    if ((X & 16) != 0 && X != 255)
      strcat(Str,"exists");
    else {

      if (X == 255) {
        if ((Len == 1 || Len == 3) && RemovedBackSlash) {  /* was it the ROOT */
          strcat(Str,"exists");                            /* directory?      */
        } else {
          strcat(Str,"ERROR must be created!");
        }
      } else {
        strcat(Str,"ERROR found a file instead!");
      }
    }
  }
  sprintf(Output,"%s\r\n",Str);
  dosfputs(Output,&prn);

  if (strcmp(Msg,"Upload") == 0 && Path[0] != 0 && Path[1] != ':') {
    sprintf(Output,"NOTE: a DRIVE specifier is required for PCBoard to check the free space\r\n"
                   "on the upload drive.  Example:  C:\\PCB\\UPLOAD instead of \\PCB\\UPLOAD\r\n");
    dosfputs(Output,&prn);
  }

end: ;
  #ifdef DEBUG
    mc_unregister(Output);
    mc_unregister(Str);
    mc_unregister(Temp);
    mc_unregister(Path);
  #endif
}


/********************************************************************
*
*  Function: checkdownloadpaths()
*
*  Desc    : Checks the download paths to see if they exist
*
*  Calls   : open(), checkpath(), close()
*/

void  checkdownloadpaths(char *Srce) {
  char    Temp[60];
  DOSFILE File;

  if (dosfopen(Srce,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
    while (dosfgets(Temp,60,&File) != -1) {
      stripright(Temp,' ');
      if (Temp[0] == '%')
        checkfile(&Temp[1],MUSTHAVE,"Download");
      else
        checkpath("Download",Temp);
    }
    dosfclose(&File);
  }
}


/********************************************************************
*
*  Function: checkfile()
*
*  Desc    : Checks for the existence of a file.  If not found it displays a
*            message indicating if it needs to be created.  Also, it checks to
*            see if the file exists as a DIRECTORY instead of a file which
*            would be a setup error.
*/

void  checkfile(char *FileName, int Needed, char *Desc) {
  char Msg[80];
  char Temp[80];
  char Status;

  stripright(FileName,' ');
  if (FileName[0] == 0) {
    sprintf(Temp,"(%s is blank)",Desc);
    sprintf(Msg,"%-30s",Temp);
    switch (Needed) {
      case MUSTHAVE : strcat(Msg,"must create it or copy it from disk");  break;
      case WILLMAKE : strcat(Msg,"system will create it if/when needed"); break;
    /*case YOUMAKE  : strcat(Msg,"create it if desired");                 break;*/
      case YOUMAKE  : return;  /* don't say anything */
    }
  } else {
    sprintf(Msg,"%-30s",FileName);
    Status = fileexist(FileName);
    if (Status == 255) {
      strcat(Msg,"is missing, ");
      switch (Needed) {
        case MUSTHAVE : strcat(Msg,"must create it or copy it from disk");  break;
        case WILLMAKE : strcat(Msg,"system will create it if/when needed"); break;
      /*case YOUMAKE  : strcat(Msg,"create it if desired");                 break;*/
        case YOUMAKE  : return;  /* don't say anything */
      }
    } else if ((Status & 16) != 0)
      strcat(Msg,"is a DIRECTORY!  Expected to find a file instead!");
    else
      strcat(Msg,"exists");
  }
  dosfputs(Msg,&prn);
  dosfputs("\r\n",&prn);
}


/********************************************************************
*
*  Function: checkboth()
*
*  Desc    : Checks both normal and graphics file names by append a 'G' to the
*            name and calling the checkfile() function
*
*  Calls   : strcpy(), strlen(), checkfile()
*/

void  checkboth(char *Name, char *Desc) {
  char Temp[63];
  int  Len;

  checkfile(Name,YOUMAKE,Desc);

  if (Name[0] == 0) {
    strcpy(Temp,Desc);
    Len = strlen(Desc);
    Temp[Len] = 'G';
    Temp[Len+1] = 0;
    checkfile("",YOUMAKE,Temp);
  } else {
    strcpy(Temp,Name);
    Len = strlen(Name);
    Temp[Len] = 'G';
    Temp[Len+1] = 0;
    checkfile(Temp,YOUMAKE,Desc);
  }
}


/********************************************************************
*
*  Function: maxfilesopen()
*
*  Desc    : This is a RECURSIVE function which calls itself repeatedly opening
*            new files until an error occurs.  It then closes and erases all
*            opened files and returns the maximum number of open files per
*            this process.
*
*  Returns : Maximum number of files that can be opened in this process.
*/

int  maxfilesopen(int Current) {
  char Name[9];
  int  Handle;
  int  Max;

  sprintf(Name,"$$%d$$",Current);
  if ((Handle = doscreate(Name,OPEN_RDWR,OPEN_NORMAL)) == -1) {
    if (ExtendedError == 4)  /* too many files open */
      return(Current);
    else
      return(0);
  }

  HighestHandle = Handle;
  Max = maxfilesopen(Current+1);
  dosclose(Handle);
  unlink(Name);
  return(Max);
}


/********************************************************************
*
*  Function: checkbaudrate()
*
*  Desc    : Checks the number passed in Baud to see if it is valid
*
*  Returns : TRUE if value is valid, otherwise FALSE
*/

bool  checkbaudrate(long Baud) {
  long static Valid[9] = { 300, 1200, 2400, 4800, 9600, 19200, 38400L, 57600L, 115200L};
  int  X;

  for (X = 0; X < 9; X++) {
    if (Baud == Valid[X])
      return(TRUE);
  }
  return(FALSE);
}


void  checkcputype(void) {
  char Tmp[100];
  char Str[50];
  int  cpu;

  switch (cpu = cputype()) {
    case   86: strcpy(Str,"8086 / 8088");   break;
    case  186: strcpy(Str,"80186 / 80188"); break;
    case -286:
    case  286: sprintf(Str,"80286 (%s mode)",(cpu < 0 ? "protected" : "real")); break;
    case -386:
    case  386: sprintf(Str,"80386 (%s mode)",(cpu < 0 ? "protected" : "real")); break;
    case -486:
    case  486: sprintf(Str,"80486 (%s mode)",(cpu < 0 ? "protected" : "real")); break;
    default  : strcpy(Str,"unknown (error)");
  }
  sprintf(Tmp,"CPU Type: %s\r\n\r\n",Str);
  dosfputs(Tmp,&prn);
}



/********************************************************************
*
*  Function:  getsecurity()
*
*  Desc    :  Opens the PWRD file and reads all of the security levels in the
*             file.  It stores the security levels as SINGLE BYTE (char) ascii
*             characters in character array pointed to by *Table.
*
*  Returns :  The number of security levels found.
*/

int  getsecurity(char *Table) {
  char    Buf[256];
  DOSFILE TextFile;
  int     X;

  X = 0;
  if (dosfopen(PcbData.PwdFile,OPEN_READ|OPEN_DENYNONE,&TextFile) != -1) {
    while (dosfgets(Buf,256,&TextFile) != -1) {
      parse(Buf);
      *Table = atoi(parse(NULL));
      Table++;
      X++;
    }
    dosfclose(&TextFile);
  }
  return(X);
}

#pragma warn -par
int  checkusersrecord(long Counter, URead *p) {
  char Str[200];
  char Temp[26];

  movestr(Temp,p->Name,25);
  fastprint(33,20,Temp,Colors[STATUS]);
  if (strnchr(Table,p->SecurityLevel,NumSec) == NULL) {
    BadPwrd = TRUE;
    sprintf(Str,"%-25s sec level %3d has no matching entry in PWRD file\r\n",
                Temp,p->SecurityLevel);
    dosfputs(Str,&prn);
  }
  return(0);
}
#pragma warn +par


int  checkusersindexes(long Counter, URead *p) {
  char Tmp[200];
  char Temp[26];
  long IndexNum;

  movestr(Temp,p->Name,25);
  fastprint(52,20,Temp,Colors[STATUS]);
  if (Counter != (IndexNum = finduser(Temp))) {
    BadIndex = TRUE;
    sprintf(Tmp,"User record #%5ld / index record #%5ld (%-25s)\r\n",Counter,IndexNum,Temp);
    dosfputs(Tmp,&prn);
  }

  return(0);
}



void  checkusersfile(void) {
  char Tmp[200];

  BadPwrd  = FALSE;
  BadIndex = FALSE;

  memset(Table,0,256);
  NumSec = getsecurity(Table);

  dosfputs("\r\n\r\nScanning Users File:\r\n",&prn);
  fastprintmove(14,20,"  Scanning Users...",Colors[DISPLAY]);

  if (openusersfile() == -1)
    return;

  scanusersfile((void (*)(URead*,long))checkusersrecord);

  if (! BadPwrd) {
    sprintf(Tmp,"\r\nNo un-matched PWRD/Security Level entries found\r\n");
    dosfputs(Tmp,&prn);
  }

  dosfputs("\r\n\r\nChecking Indexes:\r\n",&prn);
  fastprintmove(31,20,"  Checking Indexes...",Colors[DISPLAY]);

  createsingleindex();
  scanusersfile((void (*)(URead*,long))checkusersindexes);
  closesingleindex();

  if (! BadIndex)
    sprintf(Tmp,"\r\nIndexes appear to be in working order\r\n\r\n");
  else {
    sprintf(Tmp,"\r\nIndexes are out-of-synch.  To correct, select Users File Maint/Make Index.\r\n"
                "Also, check the names listed above for duplicate names in your users file.\r\n\r\n");
  }
  dosfputs(Tmp,&prn);
  close(UsersFile);
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*/

void  checkfsec(char *Name) {
  char    Buf[256];
  char    Tmp[100];
  char    Path[100];
  DOSFILE TextFile;
  char    CurDrive;
  int     Exist;

  if (dosfopen(Name,OPEN_READ|OPEN_DENYNONE,&TextFile) != -1) {
    sprintf(Tmp,"\r\nChecking for existence of paths in %s:\r\n",Name);
    dosfputs(Tmp,&prn);

    while (dosfgets(Buf,256,&TextFile) != -1) {
      strcpy(Path,parse(Buf));
      if (Path[0] != 0) {
        strupr(Path);
        sprintf(Tmp,"Path: %-20s  ",Path);
        dosfputs(Tmp,&prn);
        if (strlen(Path) > 3)
          stripright(Path,'\\');
        Exist = fileexist(Path);
        if ((Exist & 16) != 0 && Exist != 255) {
          dosfputs("exists\r\n",&prn);
        } else {
          if (Exist == 255) {
            if (strlen(Path) <= 3) {
              CurDrive = getdisk();
              setdisk(Path[0]-'A');
              if (getdisk() == Path[0]-'A')
                dosfputs("exists\r\n",&prn);
              else
                dosfputs("does not exist!\r\n",&prn);
              setdisk(CurDrive);
            } else
              dosfputs("does not exist!\r\n",&prn);
          } else
            dosfputs("ERROR found a file instead of a path!\r\n",&prn);
        }
      }
    }
    dosfputs("\r\n",&prn);
    dosfclose(&TextFile);
  }
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*/

void  checkdoors(char *DoorsFile) {
  char    Buf[256];
  char    Tmp[100];
  char    FullName[100];
  char    Name[20];
  DOSFILE TextFile;
  int     Exist;

  if (dosfopen(DoorsFile,OPEN_READ|OPEN_DENYNONE,&TextFile) != -1) {
    while (dosfgets(Buf,256,&TextFile) != -1) {
      strcpy(Name,parse(Buf));
      if (Name[0] != 0) {
        sprintf(Tmp,"Door: %-13s  ",Name);
        parse(NULL);
        parse(NULL);
        parse(NULL);
        parse(NULL);
        strcpy(FullName,parse(NULL));
        strcat(FullName,Name);
        Exist = fileexist(FullName);
        if (Exist == 255)
          strcat(Tmp,"does not exist!\r\n");
        else if ((Exist & 16) != 0)
          strcat(Tmp,"ERROR found a directory instead of a file!)\r\n");
        else
          strcat(Tmp,"exists\r\n");
        dosfputs(Tmp,&prn);
      }
    }
    dosfputs("\r\n",&prn);
    dosfclose(&TextFile);
  }
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*/

void  checkprot(void) {
  char    static *Send = "PCBSx.BAT";
  char    static *Rcve = "PCBRx.BAT";
  char    Buf[256];
  char    Tmp[100];
  char    SName[66];
  char    RName[66];
  char    List[26];
  DOSFILE TextFile;
  char    *q;
  bool    Letter;
  char    Shell;
  int     X;

  memset(List,0,sizeof(List));
  SName[0] = RName[0] = 0;

  dosfputs("\r\nChecking the parameters inside PCBPROT.DAT:",&prn);

  if (dosfopen(PcbData.TrnFile,OPEN_READ|OPEN_DENYNONE,&TextFile) == -1) {
    dosfputs(" -- ERROR, file is missing!\r\n",&prn);
  } else {
    dosfputs("\r\n",&prn);
    while (dosfgets(Buf,256,&TextFile) != -1) {
      strupr(Buf);
      q = parse(Buf);   Letter = *q;
      q = parse(NULL);  Shell = *q;
      sprintf(Tmp,"Letter: %c,  Internal or Shell: %c - ",Letter,Shell);
      dosfputs(Tmp,&prn);
      switch (Shell) {
        case 'I' : if (strchr("AXCOFGYN",Letter) == NULL)
                     dosfputs("ERROR, valid internals are A,X,C,O,F,G,Y,N\r\n",&prn);
                   else
                     dosfputs("okay\r\n",&prn);
                   break;
        case 'B' :
        case 'D' :
        case 'S' : Send[4] = Rcve[4] = Letter;
                   strcpy(SName,Send);
                   strcpy(RName,Rcve);
                   if (srchpath(SName) == -1)
                     SName[0] = 0;
                   if (srchpath(RName) == -1)
                     RName[0] = 0;
                   sprintf(Tmp,"%s %-10s, %s %s\r\n",
                           Send,
                           (SName[0] != 0 ? "(exists)" : "(missing!)"),
                           Rcve,
                           (RName[0] != 0 ? "(exists)" : "(missing!)"));
                   dosfputs(Tmp,&prn);
                   if (SName[0] != 0 || RName[0] != 0)
                     List[Letter-'A'] = TRUE;
                   SName[0] = RName[0] = 0;
                   break;
        default  : dosfputs("ERROR, invalid type must be I, S, D or B!\r\n",&prn);
      }
    }
    dosfputs("\r\n",&prn);
    dosfclose(&TextFile);
  }

  for (X = 0; X < 26; X++)
    if (List[X]) {
      Send[4] = Rcve[4] = X + 'A';
      strcpy(SName,Send);
      strcpy(RName,Rcve);
      if (srchpath(SName) == 0)
        printtextfile(SName,"Send Batch File",0);
      if (srchpath(RName) == 0)
        printtextfile(RName,"Receive Batch File",0);
    }

}


static void near  printconfname(int ConfNum, char *ConfName, char *Str) {
 char Tmp[256];

 if (ConfNum == 0)
   sprintf(Tmp,"\r\nMain Board %s:\r\n",Str);
 else
   sprintf(Tmp,"\r\n%d) %s Conference %s:\r\n",ConfNum,ConfName,Str);

 dosfputs(Tmp,&prn);
}



static void near  checkevents(void) {
  char         Temp[100];
  char         FileName[66];
  DOSFILE      File;
  struct ffblk ffblk;
  EventType    Buffer;
  char         ActiveEventList[MAXACTIVEEVENTSIZE];
  char         Days[8];
  int          MaxEvents;
  int          EventCount;
  int          Num;
  int          CountMatching;
  char         *p;
  char         Hour[3];
  char         Min[3];

  dosfputs("\r\nChecking EVENT.DAT file\r\n",&prn);

  if (fileexist(PcbData.EventDatFile) == 255 || dosfopen(PcbData.EventDatFile,OPEN_READ|OPEN_DENYNONE,&File) == -1) {
    dosfputs("EVENT.DAT file not found!\r\n\r\n",&prn);
    return;
  }

  if (dosfread(&MaxEvents, MAXEVENTSIZE, &File) != MAXEVENTSIZE ||
      dosfread(ActiveEventList,ACTIVEEVENTSIZE, &File) != ACTIVEEVENTSIZE) {
    dosfputs("Could not read EVENT.DAT file\r\n",&prn);
    goto end;
  }

  for (EventCount = 0, Days[7] = 0; EventCount < MaxEvents; EventCount++) {
    if (dosfread(&Buffer, sizeof(Buffer), &File) != sizeof(Buffer)) {
      dosfputs("Could not read EVENT.DAT file\r\n",&prn);
      goto end;
    }

    if (isset(ActiveEventList,EventCount)) {
      if (Buffer.Mode != 'E' && Buffer.Mode != 'S' && Buffer.Mode != 'F' && Buffer.Mode != 'M') {
        sprintf(Temp,"Event #%d Mode is not properly specified (%c)\r\n",EventCount+1,Buffer.Mode);
        dosfputs(Temp,&prn);
      }

      if (Buffer.BatchFile[0] <= ' ' || strnchr(Buffer.BatchFile,'.',sizeof(Buffer.BatchFile)) != NULL) {
        sprintf(Temp,"Event #%d is active, but not named properly (%-8.8s)\r\n",EventCount+1,Buffer.BatchFile);
        dosfputs(Temp,&prn);
      }

      sprintf(FileName,"%s%-8.8s",PcbData.EventFiles,Buffer.BatchFile);
      stripright(FileName,' ');
      strcat(FileName,".*");

      CountMatching = 0;
      if (findfirst(FileName,&ffblk,0) != -1) {
        do {
          if ((p = strrchr(ffblk.ff_name,'.')) == NULL || atoi(p+1) > 0)
            CountMatching++;
        } while (findnext(&ffblk) != -1);
      }

      if (CountMatching == 0) {
        sprintf(Temp,"Event #%d No event files found for %-8.8s\r\n",Buffer.BatchFile);
        dosfputs(Temp,&prn);
      }

      Hour[2] = 0;
      Min[2]  = 0;
      Hour[0] = Buffer.BeginTime[0];
      Hour[1] = Buffer.BeginTime[1];
      Min[0]  = Buffer.BeginTime[3];
      Min[1]  = Buffer.BeginTime[4];

      if (Buffer.BeginTime[0] <= ' ' || Buffer.BeginTime[2] != ':' ||
          (Num = atoi(Hour)) < 0 || Num > 23 ||
          (Num = atoi(Min)) < 0 || Num > 59) {
        sprintf(Temp,"Event #%d Begin Time improperly specified (%-5.5s)\r\n",EventCount+1,Buffer.BeginTime);
        dosfputs(Temp,&prn);
        continue;
      }

      Hour[0] = Buffer.EndTime[0];
      Hour[1] = Buffer.EndTime[1];
      Min[0]  = Buffer.EndTime[3];
      Min[1]  = Buffer.EndTime[4];
      if (Buffer.EndTime[0] <= ' ' || Buffer.EndTime[2] != ':' ||
          (Num = atoi(Hour)) < 0 || Num > 23 ||
          (Num = atoi(Min)) < 0 || Num > 59) {
        sprintf(Temp,"Event #%d End Time improperly specified (%-5.5s)\r\n",EventCount+1,Buffer.EndTime);
        dosfputs(Temp,&prn);
        continue;
      }

      if (memcmp(Buffer.BeginTime,Buffer.EndTime,TIMELENGTH) > 0) {
        sprintf(Temp,"Event #%d: Begin Time / End Times should not cross midnight (%-5.5s / %-5.5s)\r\n",EventCount+1,Buffer.BeginTime,Buffer.EndTime);
        dosfputs(Temp,&prn);
      }

      if (memcmp(Buffer.Date,"        ",TIMELENGTH) == 0) {
        if (Buffer.Days == 0) {
          sprintf(Temp,"Event #%d: Neither a DATE nor the DAYS have been specified\r\n",EventCount+1);
          dosfputs(Temp,&prn);
        }
      } else {
        if (Buffer.Days[0] != 0) {
          sprintf(Temp,"Event #%d: When specifying dates, the DAYS field should be NNNNNNN\r\n",EventCount+1);
          dosfputs(Temp,&prn);
        }
      }
    }
  }


end: ;
  dosfclose(&File);
}


/********************************************************************
*
*  Function: analysis()
*
*  Desc    : Performs the overall system analysis
*
*/

void  analysis(void) {
  char    Tmp[2048];
  char    Caller[66];
  struct dfree disk;
  DOSFILE File;
  char    Str[5];
  long    TotalDisk;
  long    AvailDisk;
  int     Num;
  int     X;
  char    *q;
  char    *Path;
  char    *Comspec;
  char    *Temp;

  printtop("Analysis Section");

  getdfree(CurrentPath[0]-'@',&disk);
  TotalDisk = AvailDisk = disk.df_bsec * disk.df_sclus;
  TotalDisk *= disk.df_total;
  AvailDisk *= disk.df_avail;
  sprintf(Tmp,"Disk Drive Analysis:\r\n"
              "Current Path    = %s\r\n"
              "Bytes/Sector    = %u\r\n"
              "Sectors/Cluster = %u\r\n"
              "Total Clusters  = %6u  (Total Disk Space = %9lu)\r\n"
              "Avail. Clusters = %6u  (Free  Disk Space = %9lu)\r\n\r\n",
              CurrentPath,disk.df_bsec,disk.df_sclus,
              disk.df_total,TotalDisk,
              disk.df_avail,AvailDisk);
  dosfputs(Tmp,&prn);

  switch (Scrn_Adapter) {
    case VID_NONE: strcpy(Str,"NONE"); break;
    case VID_MDA : strcpy(Str,"MONO"); break;
    case VID_CGA : strcpy(Str,"CGA");  break;
    case VID_EGA : strcpy(Str,"EGA");  break;
    case VID_VGA : strcpy(Str,"VGA");  break;
    case VID_MCGA: strcpy(Str,"MCGA"); break;
  }

  sprintf(Tmp,"Screen Information:\r\n"
              "Screen Address       = %04Xh:%04Xh\r\n"
              "Retrace Checking     = O%s\r\n"
              "Video Adapter        = %s\r\n"
              "Mode (Color or B&W)  = %s\r\n\r\n",
              (int) (Scrn_Addr >> 16),
              (int) (Scrn_Addr & 0xFFFF),
              (Scrn_Rtrc      ? "N"     : "FF"),
              Str,
              (Scrn_Mode      ? "COLOR" : "B&W"));
  dosfputs(Tmp,&prn);

  q = getenv("COMSPEC"); Comspec = ((q == NULL) ? "NONE" : q);
  q = getenv("PATH");    Path    = ((q == NULL) ? "NONE" : q);
  q = getenv("PCB");     Temp    = ((q == NULL) ? "NONE" : q);
  sprintf(Tmp,"Environment:\r\n"
              "COMSPEC= %s\r\n"
              "PATH   = %s\r\n"
              "PCB    = %s\r\n",Comspec,Path,Temp);
  if ((q = getenv("BOX")) != NULL) {
    strcat(Tmp,"BOX    = ");
    strcat(Tmp,q);
    strcat(Tmp,"\r\n");
  }
  if ((q = getenv("CLOCK")) != NULL) {
    strcat(Tmp,"CLOCK  = ");
    strcat(Tmp,q);
    strcat(Tmp,"\r\n");
  }
  strcat(Tmp,"\r\n");
  dosfputs(Tmp,&prn);

  checkcputype();

  sprintf(Tmp,"Current FREE memory (with PCBDIAG loaded) is %ld\r\n\r\n",farcoreleft());
  dosfputs(Tmp,&prn);

  sprintf(Tmp,"DOS verify is O%s\r\n",(getverify() ? "N" : "FF"));
  dosfputs(Tmp,&prn);

  sprintf(Tmp,"DOS version number is %d.%d\r\n\r\n",_osmajor,_osminor);
  dosfputs(Tmp,&prn);

  checkshare();

  if (PcbData.Network && PcbData.NodeNum == 0)
    dosfputs("Error:  PCBoard is set up as a network but the Node Number is set to 0\r\n\r\n",&prn);

/*checkcommport();    removed due to lack of /M support */

  Num = maxfilesopen(0);
  if (Num != 0) {            /* if Num was 0 then there was an error */
    sprintf(Tmp,"Files:\r\n"
                "Current number of files open = %d\r\n"
                "Maximum number of open files (this process) = %d\r\n",HighestHandle-Num+1,HighestHandle+1);
    dosfputs(Tmp,&prn);
    if (HighestHandle < 19)
      dosfputs("It is recommended that you set FILES=20 for a stanalone system=\r\n"
               "or FILES=32 if you are running a network or multi-tasker\r\n",&prn);
    dosfputs("\r\n",&prn);
  } else {
    dosfputs("Error creating files for OPEN-FILE test\r\n\r\n",&prn);
  }


  stripright(PcbData.ViewBatch,' ');
  stripright(PcbData.ViewExt  ,' ');
  if (PcbData.ViewBatch[0] == 0)
    dosfputs("View Batch Filename is EMPTY!\r\n\r\n",&prn);
  if (PcbData.ViewExt[0] == 0)
    dosfputs("View Extension is EMPTY!\r\n\r\n",&prn);


  if (PcbData.Network)
    sprintf(Caller,"%s%d",PcbData.ClrFile,PcbData.NodeNum);
  else
    strcpy(Caller,PcbData.ClrFile);


  checkfile(PcbData.UsrFile  ,WILLMAKE,"USERS");
  checkfile(Caller           ,WILLMAKE,"CALLERS");
  checkfile(PcbData.CnfFile  ,MUSTHAVE,"CNAMES");
  checkfile(PcbData.PwdFile  ,MUSTHAVE,"PWRD");
  checkfile(PcbData.FscFile  ,YOUMAKE ,"FSEC");
  checkfile(PcbData.UscFile  ,YOUMAKE ,"USEC");
  checkfile(PcbData.TcnFile  ,YOUMAKE ,"TCAN");
  checkboth(PcbData.WlcFile           ,"WELCOME");
  checkboth(PcbData.NewFile           ,"NEWUSER");
  checkfile(PcbData.ClsFile  ,YOUMAKE ,"CLOSED");
  checkfile(PcbData.WrnFile  ,YOUMAKE ,"WARNING");
  checkfile(PcbData.ExpFile  ,YOUMAKE ,"EXPIRED");
  checkfile(PcbData.NetFile  ,WILLMAKE,"USERNET.DAT");
  checkboth(PcbData.CnfMenu           ,"CONF MENU");
  checkfile(PcbData.RegFile  ,YOUMAKE ,"NEWUSER SCRIPT");
  checkfile(PcbData.AnsFile  ,WILLMAKE,"NEWUSER ANSWER");
  checkfile(PcbData.TrnFile  ,MUSTHAVE,"PCBPROT.DAT");
  checkfile(PcbData.DldFile  ,WILLMAKE,"DOWNLOAD.TXT");
  checkfile(PcbData.LogOffScr,YOUMAKE ,"LOGOFF SCRIPT");
  checkfile(PcbData.LogOffAns,WILLMAKE,"LOGOFF ANSWER");
  checkfile(PcbData.MultiLang,YOUMAKE ,"PCBML.DAT");
  checkfile(PcbData.GroupChat,YOUMAKE ,"GROUP");
  checkfile(PcbData.ChatFile ,WILLMAKE,"CHAT.DAT");
  checkfile(PcbData.StatsFile,WILLMAKE,"PCBSTATS.DAT");
  checkfile(PcbData.ChatMenu ,YOUMAKE ,"CHATMENU");
  checkfile(PcbData.NoAnsi   ,YOUMAKE ,"NOANSI");

  if (PcbData.Swap)
    checkfile(PcbData.SwapPath ,WILLMAKE,"PCBSWAP.$$$");

  checkfile(PcbData.EventDatFile,YOUMAKE ,"EVENT.DAT");
  checkfile(PcbData.CmdLst      ,YOUMAKE ,"CMD.LST");
  checkfile(PcbData.AllFilesList,YOUMAKE ,"ALLFILES.LST");
  checkfile(PcbData.LogOnScr    ,YOUMAKE ,"LOGON SCRIPT");
  checkfile(PcbData.LogOnAns    ,WILLMAKE,"LOGON ANSWER");
  checkfile(PcbData.FileTcan    ,YOUMAKE ,"FILE TCAN");
  checkfile(PcbData.SlowDriveBat,YOUMAKE ,"SLOW DRIVE BAT");

  if (PcbData.QwkFile[0] == 0)
    dosfputs("QWK Packet Name is not defined!\r\n\r\n",&prn);

  checkfile(RemoteFile       ,MUSTHAVE,"REMOTE.SYS");
  checkfile(CompressFile     ,MUSTHAVE,"PCBCMPRS.BAT");
  checkfile(QwkCompressFile  ,MUSTHAVE,"PCBQWK.BAT");

  memset(&OldConf,0xFF,sizeof(pcbconftype));

  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&Conf);
    if (X == 0)
      strcpy(Tmp,"\r\nMain Board Files:\r\n");
    else
      sprintf(Tmp,"%d) %s Conference Files:\r\n",X,Conf.Name);
    dosfputs(Tmp,&prn);

    if (strcmp(Conf.MsgFile,OldConf.MsgFile) != 0)
      checkfile(Conf.MsgFile,WILLMAKE,"MSGS");

    if (strcmp(Conf.UserMenu,OldConf.UserMenu) != 0)
      checkboth(Conf.UserMenu,"USER MENU");

    if (strcmp(Conf.SysopMenu,OldConf.SysopMenu) != 0)
      checkboth(Conf.SysopMenu,"SYSOP MENU");

    if (strcmp(Conf.NewsFile,OldConf.NewsFile) != 0)
      checkboth(Conf.NewsFile,"NEWS");

    if (strcmp(Conf.UpldDir,OldConf.UpldDir) != 0)
      checkfile(Conf.UpldDir,YOUMAKE,"PUB UPLD DIR");

    if (strcmp(Conf.PrivDir,OldConf.PrivDir) != 0)
      checkfile(Conf.PrivDir,YOUMAKE,"PRIV UPLD DIR");

    if (strcmp(Conf.DrsMenu,OldConf.DrsMenu) != 0)
      checkboth(Conf.DrsMenu,"DOORS MENU");

    if (strcmp(Conf.DrsFile,OldConf.DrsFile) != 0)
      checkfile(Conf.DrsFile,YOUMAKE,"DOORS LIST");

    if (strcmp(Conf.BltMenu,OldConf.BltMenu) != 0)
      checkboth(Conf.BltMenu,"BLT MENU");

    if (strcmp(Conf.BltNameLoc,OldConf.BltNameLoc) != 0)
      checkfile(Conf.BltNameLoc,YOUMAKE,"BLT LIST");

    if (strcmp(Conf.ScrMenu,OldConf.ScrMenu) != 0)
      checkboth(Conf.ScrMenu,"SCRIPT MENU");

    if (strcmp(Conf.ScrNameLoc,OldConf.ScrNameLoc) != 0)
      checkfile(Conf.ScrNameLoc,YOUMAKE,"SCRIPT LIST");

    if (strcmp(Conf.DirMenu,OldConf.DirMenu) != 0)
      checkboth(Conf.DirMenu,"DIR MENU");

    if (strcmp(Conf.DirNameLoc,OldConf.DirNameLoc) != 0)
      checkfile(Conf.DirNameLoc,YOUMAKE,"DIR LIST");

    if (strcmp(Conf.PthNameLoc,OldConf.PthNameLoc) != 0)
      checkfile(Conf.PthNameLoc,YOUMAKE,"DNLD PATH LIST");

    if (strcmp(Conf.Intro,OldConf.Intro) != 0)
      checkfile(Conf.Intro,YOUMAKE,"CONF INTRO");

    if (strcmp(Conf.CmdLst,OldConf.CmdLst) != 0)
      checkfile(Conf.CmdLst,YOUMAKE,"CMD.LST");

    dosfputs("\r\n",&prn);
    OldConf = Conf;
  }

  dosfputs("\r\nPaths:\r\n",&prn);
  checkpath("Help"          ,PcbData.HlpLoc);
  checkpath("Security Files",PcbData.SecLoc);
  checkpath("Command Files" ,PcbData.CmdLoc);
  checkpath("Chat Files"    ,PcbData.ChtLoc);
  checkpath("PCBTEXT Files" ,PcbData.TxtLoc);
  checkpath("Index Files"   ,PcbData.NdxLoc);
  checkpath("CMD Disp Files",PcbData.CmdLoc);

  checkpath("EVENT FILES"   ,PcbData.EventFiles);
  checkevents();

  memset(&OldConf,0,sizeof(OldConf));

  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&Conf);
    if (fileexist(Conf.MsgFile) != 255) {
      if (strcmp(Conf.PubUpldLoc,OldConf.PubUpldLoc) != 0) {
        printconfname(X,Conf.Name,"Upload Path");
        checkpath("Upload",Conf.PubUpldLoc);
      }

      if (strcmp(Conf.PrvUpldLoc,OldConf.PrvUpldLoc) != 0) {
        printconfname(X,Conf.Name,"Private Upload Path");
        checkpath("Upload",Conf.PrvUpldLoc);
      }

      if (strcmp(Conf.AttachLoc,OldConf.AttachLoc) != 0) {
        printconfname(X,Conf.Name,"Attach Loc");
        checkpath("Attach",Conf.AttachLoc);
      }

    }
  }
  dosfputs("\r\n",&prn);

  memset(&OldConf,0xFF,sizeof(pcbconftype));
  resetfilenames();
  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&Conf);
    if (strcmp(Conf.DrsFile,OldConf.DrsFile) != 0) {
      if (fileexist(Conf.MsgFile) != 255) {
        printconfname(X,Conf.Name,"Doors");
        if (openfilename(Conf.DrsFile,"DOORS",X,&File) != -1) {
          dosfclose(&File);
          checkdoors(Conf.DrsFile);
        }
      }
    }
    OldConf = Conf;
  }
  dosfputs("\r\n",&prn);

  memset(&OldConf,0xFF,sizeof(pcbconftype));
  resetfilenames();
  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&Conf);
    if (strcmp(Conf.PthNameLoc,OldConf.PthNameLoc) != 0) {
      if (fileexist(Conf.MsgFile) != 255) {
        printconfname(X,Conf.Name,"Download Paths");
        dosfputs(Tmp,&prn);
        if (openfilename(Conf.PthNameLoc,"DL PATHS",X,&File) != -1) {
          dosfclose(&File);
          checkdownloadpaths(Conf.PthNameLoc);
        }
      }
    }
    OldConf = Conf;
  }
  dosfputs("\r\n",&prn);

  for (X = 0; X < PcbData.NumAreas; X++) {
    getconfrecord(X,&Conf);
    if (Conf.MsgBlocks < 1 || Conf.MsgBlocks > 32) {
      if (fileexist(Conf.MsgFile) != 255) {
        sprintf(Tmp,"ERROR: %s %smessage blocks = %d, must be from 1 to 32!\r\n\r\n",Conf.Name,(X == 0 ? "" : "conference "),Conf.MsgBlocks);
        dosfputs(Tmp,&prn);
      }
    }
  }

  checkfsec(PcbData.FscFile);
  checkfsec(PcbData.UscFile);
  checkprot();
  freefilelist();
}


/********************************************************************
*
*  Function: analyze()
*
*  Desc    : Requests from the user a list of printouts then prints those and
*            performs the analysis
*
*/

void  analyze(void) {
  char    Temp[80];

  memset(PrintOuts,TRUE,sizeof(PrintOuts));
  clscolor(Colors[OUTBOX]);
  fastcenter(18,"Please note:  To prepare an upload file answer 'Y' to all of the above",Colors[QUESTION]);
  fastcenter(19,"then press the PGDN key and enter 'DIAG.TXT' as the output filename.",Colors[QUESTION]);
  fastcenter(21," press PGDN to begin analysis or ESC to exit ",Colors[DESC]);

  PrintOuts[0] = PrintOuts[1] = PrintOuts[2] = TRUE;

  initquest(Flds,NUMFIELDS);
  readscrn(Flds,NUMFIELDS-1,0,MainHead1,MainHead2,1,NOCLEARFLD);
  freeanswers(Flds,NUMFIELDS);


  if (KeyFlags != ESC) {
    openprint();
    if (KeyFlags != ESC) {
      clsbox(3,16,78,21,Colors[OUTBOX]);
      box(1,18,78,22,Colors[OUTBOX],SINGLE);
      fastprintmove(5,20,"Printing...",Colors[DISPLAY]);

      CurrentPath[0] = getdisk() + 'A';
      CurrentPath[1] = ':';
      CurrentPath[2] = '\\';
      CurrentPath[3] = 0;
      getcurdir(0,Temp);
      strcat(CurrentPath,Temp);
      addbackslash(CurrentPath,sizeof(CurrentPath));

      strcpy(RemoteFile     ,"REMOTE.SYS");    srchpath(RemoteFile);
      strcpy(CompressFile   ,"PCBCMPRS.BAT");  srchpath(CompressFile);
      strcpy(QwkCompressFile,"PCBQWK.BAT");    srchpath(QwkCompressFile);
      strcpy(BoardBatFile   ,"BOARD.BAT");     srchpath(BoardBatFile);

      if (PrintOuts[0])
        printconfig();

      if (PrintOuts[1]) {
        clsbox(18,20,77,20,Colors[OUTBOX]);
        fastprint(18,20,"Analysis",Colors[STATUS]);
        analysis();
      }

      if (PrintOuts[2])
        checkusersfile();
    }
  }
}
