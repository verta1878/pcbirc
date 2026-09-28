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


#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <misc.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcboard.h"
#include "pcboard.ext"

#ifdef _MSC_VER
  #include <borland.h>
#endif

#ifdef __OS2__
  #define  INCL_DOSPROCESS
  #define  INCL_DOSMISC
  #include <os2.h>
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif

void LIBENTRY dosinit(void);

char         ComSpec[66];
char        *DatFile;
pcbdattype   PcbData;
char         PcbDir[66];
unsigned     ConfByteLen;
unsigned     ExtConfLen;
char         LineSeparator;
bool         OldIndex;
char         VerifyCDLoss = 1;
int          DebugLevel = 0;

#ifdef __OS2__
  char DosComSpec[66];
  char DosSettingsFile[128];
  int  ComHandle = 0;
#else
  char NO16550;
  char Allow16650;
  char FORCE16550A;
#endif


#ifdef __OS2__
static void LIBENTRY getdoscommand(void) {
  static  char *SysFile = "x:\\CONFIG.SYS";
  int     Drive;
  DOSFILE In;
  char    Str[512];

  DosQuerySysInfo(QSV_BOOT_DRIVE,QSV_BOOT_DRIVE,&Drive,sizeof(Drive));  /*lint !e534 */
  SysFile[0] = (char) (Drive + 'A' - 1);

  if (dosfopen(SysFile,OPEN_READ|OPEN_DENYNONE,&In) == -1)
    goto notfound;

  while (dosfgets(Str,sizeof(Str),&In) != -1) {
    if (memicmp(Str,"SHELL=",6) == 0) {
      tokenize(Str);  /*lint !e534 */
      maxstrcpy(DosComSpec,getnexttoken()+6,sizeof(DosComSpec));
      break;
    }
  }

  dosfclose(&In);
  if (fileexist(DosComSpec) != 255)
    return;

notfound:
  maxstrcpy(DosComSpec,"COMMAND.COM",sizeof(DosComSpec));
  srchpath(DosComSpec);  /*lint !e534 */
}

void LIBENTRY setdefaultpriority(void) {
/* set to default priority by setting regular class, priority 0 */
  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0,0);  /*lint !e534 */
}

void LIBENTRY adjustpriority(int PriorityDelta) {
  if (PriorityDelta != 0) {
/* first return to the default priority */
    setdefaultpriority();
/* now adjust the priority based on the delta that was passed */
    DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,PriorityDelta,0);  /*lint !e534 */
  }
}
#else
  #define setdefaultpriority()
  #define adjustpriority()
#endif


static void _NEAR_ LIBENTRY portoverride(char *Str) {
  char *p;
  int  Num;

  if ((p = strstr(Str,"/COM")) != NULL && (p[5] == ':' || p[6] == ':')) {
    Num = atoi(&p[4]);
    sprintf(PcbData.ModemPort,"COM%d:",Num);
    switch(Num) {
      case  1: PcbData.IrqNum = 4; PcbData.BaseAddress = 0x3F8; break;
      case  2: PcbData.IrqNum = 3; PcbData.BaseAddress = 0x2F8; break;
      default: if ((p = strstr(Str,"/IRQ:")) != NULL)
                 PcbData.IrqNum = (short) atoi(p+5);
               if ((p = strstr(Str,"/BASE:")) != NULL)
                 PcbData.BaseAddress = (short) hextoint(p+6);
    }
  } else  {
    #ifdef MULTIPORT
      if (strstr(Str,"/OS") != NULL)
        PcbData.OS2Driver = TRUE;

      if ((p = strstr(Str,"/PORT")) != NULL) {
        sprintf(PcbData.ModemPort,"PORT%d",atoi(p+5));
        if (p[strlen(PcbData.ModemPort)+1] == 'F')
          addchar(PcbData.ModemPort,'F');
        PcbData.IrqNum = 0;
        PcbData.BaseAddress = 0;
      }
    #endif
  }

  if ((p = strstr(Str,"/BAUD:")) != NULL)
    PcbData.ModemSpeed = atol(p+6);

  #ifdef __OS2__
    if ((p = strstr(Str,"/HANDLE:")) != NULL)
      ComHandle = atoi(p+8);
    else {
      if ((p = getenv("PCBHANDLE")) != NULL)
        ComHandle = atoi(p);
    }
  #endif
}


static void _NEAR_ LIBENTRY doswitches(char *Str) {
  char *p;

  strupr(Str);
  if (strstr(Str,"/NMT") != NULL)
    Novell = TRUE;
  if (strstr(Str,"/COLOR") != NULL)
    Scrn_Mode = VID_COLOR;
  if (strstr(Str,"/MONO") != NULL)
    Scrn_Mode = VID_MONO;
  if (strstr(Str,"/OLDINDEX") != NULL)
    OldIndex = TRUE;
  if ((p = strstr(Str,"/NODE:")) != NULL)
    PcbData.NodeNum = (unsigned short) atoi(p+6);
  if (strstr(Str,"/NOGIVEUP") != NULL)
    DisableGiveup = TRUE;

  #ifndef __OS2__
    if (strstr(Str,"/16650") != NULL)
      Allow16650 = TRUE;
  #endif
}


void LIBENTRY pcbinit(void) {
  char *p;
  char *PcbVar;

  dosinit();

  Scrn_24Hour         = TRUE;
  Scrn_DateSeparator  = '-';

  if ((p = getenv("COMSPEC")) != NULL)
    maxstrcpy(ComSpec,p,sizeof(ComSpec));
  else {
    #ifdef __OS2__
      strcpy(ComSpec,"CMD.EXE");
    #else
      strcpy(ComSpec,"COMMAND.COM");
    #endif
  }

  #ifdef __OS2__
    getdoscommand();

/* this is a "default" settings file for DOS windows under OS/2 */
/* inside shell.c it will search for a settings file that is specific to */
/* the program being executed and if not found, it will use this default */
/* one instead unless this one does not exist in which case no file is used */
    maxstrcpy(DosSettingsFile,"PCBDOS.OS2",sizeof(DosSettingsFile));
    if (srchpath(DosSettingsFile) == -1)
      DosSettingsFile[0] = 0;  /* blank out if the file does not exist */
  #endif

  if ((DatFile = getenv("PCBDAT")) == NULL) {
    DatFile = "PCBOARD.DAT";
    if (fileexist(DatFile) == 255) {
      cls();
      fastprint(0,0,"Can't find pcboard.dat",0x0F);
      #ifdef __OS2__
        updatelinesnow();
        mydelay(50);
      #endif
      exit(99);
    }
  }

  PcbDir[0] = 0;
  if ((p = getenv("PCBDRIVE")) != NULL) {
    strcpy(PcbDir,p);
    if ((p = getenv("PCBDIR")) != NULL) {
      strcat(PcbDir,p);
      addbackslash(PcbDir,sizeof(PcbDir));
    }
  }

  checkmultitaskers();
  readdatfile();

  #ifdef __OS2__
    ComHandle = 0;
    PcbData.OS2Driver = TRUE;
  #else
    Allow16650   = FALSE;
    NO16550      = PcbData.No16550;
    FORCE16550A  = PcbData.Force16550A;
  #endif

  VerifyCDLoss = PcbData.VerifyCDLoss;

  if ((PcbVar = getenv("PCB")) != NULL)
    doswitches(PcbVar);

  if ((p = getenv("PCBNODE")) != NULL)
    PcbData.NodeNum = (uint) atoi(p);

  ConfByteLen = (PcbData.NumAreas >> 3) + ((PcbData.NumAreas & 0x07) != 0 ? 1 : 0);
  if (ConfByteLen < 5)
    ConfByteLen = 5;
  ExtConfLen = ConfByteLen - 5;

  /* this has to occur AFTER it reads the pcboard.dat file */
  if (PcbVar != NULL)
    portoverride(PcbVar);

  LineSeparator = (char) (PcbData.Foreign ? 0x0D : 0xE3);
}
