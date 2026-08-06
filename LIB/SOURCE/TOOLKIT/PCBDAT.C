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


#include <string.h>
#include <stdlib.h>
#include "dosfunc.h"
#include "misc.h"
#include "newdata.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY errorexittodos(char *Str);

#define BUFLEN   2048

static DOSFILE pcbfile;

extern char *DatFile;

static int _NEAR_ LIBENTRY fgetstr(char *Str, int Len) {
  char T[BUFLEN+2];

  if (dosfgets(T,BUFLEN,&pcbfile) != -1) { /*allow lines up to BUFLEN characters long*/
    stripright(T,' ');
    maxstrcpy(Str,T,Len);                  /*but copy only the maximum of Len bytes  */
    return(0);
  } else {
    Str[0] = 0;                            /*if there was an error, return null string*/
    return(-1);
  }
}


static int _NEAR_ LIBENTRY fgethex(void) {
  char Str[20];
  fgetstr(Str,10);
  return(hextoint(Str));
}


static int _NEAR_ LIBENTRY fgetint(void) {
  char Str[20];
  fgetstr(Str,10);
  return(atoi(Str));
}


static long _NEAR_ LIBENTRY fgetlong(void) {
  char Str[20];
  fgetstr(Str,10);
  return(atol(Str));
}


static bool _NEAR_ LIBENTRY fgetlog(void) {
  char Str[20];
  fgetstr(Str,10);
  return(atoi(Str) == -1 ? TRUE : FALSE);
}


static void _NEAR_ LIBENTRY wasteit(void) {
  char T[BUFLEN+2];
  dosfgets(T,BUFLEN,&pcbfile);
}


static void _NEAR_ LIBENTRY read145file(void) {
  unsigned  x;

  fgetstr(PcbData.Sysop,sizeof(PcbData.Sysop));
  wasteit();
  PcbData.UseRealName = fgetlog();
  wasteit();

  for (x = SEC_READCOMMENTS; x <= SEC_14; x++)
    wasteit();

  for (x = 0; x < 3; x++)
    wasteit();

  fgetstr(PcbData.TxtLoc ,sizeof(PcbData.TxtLoc));

  wasteit();
  wasteit();

  fgetstr(PcbData.ClrFile,sizeof(PcbData.ClrFile));
  fgetstr(PcbData.CnfFile,sizeof(PcbData.CnfFile));

  for (x = 0; x < 20; x++)
    wasteit();

  fgetstr(PcbData.ModemPort,sizeof(PcbData.ModemPort));  PcbData.ModemPort[4] = 0;
  PcbData.ModemSpeed           = fgetlong();
  PcbData.LockSpeed            = fgetlog();

  wasteit();
  wasteit();

  PcbData.DisableCTS           = fgetlog();

  for (x = 0; x < 13; x++)
    wasteit();

  PcbData.DisableFilter        = fgetlog();

  for (x = 0; x < 8; x++)
    wasteit();

  PcbData.EliminateSnow        = fgetlog();

  for (x = 0; x < 8; x++)
    wasteit();

  PcbData.KbdTimeout           = (short) fgetint();
  PcbData.Network              = fgetlog();
  PcbData.NodeNum              = (unsigned short) fgetint();
  PcbData.NetTimeout           = (char) fgetint();
  PcbData.ChatDelay            = (char) fgetint();

  fgetstr(PcbData.BoardName,sizeof(PcbData.BoardName));

  for (x = 0; x < 14; x++)
    wasteit();

  PcbData.NumConf            = (unsigned short) fgetint();
  PcbData.NumAreas           = (unsigned short) (PcbData.NumConf + 1);

  wasteit();

  PcbData.DefaultColor       = (char) fgetint();
  PcbData.DefaultIntensity   = (char) fgetint();
  PcbData.EventActive        = fgetlog();
  fgetstr(PcbData.EventTime,sizeof(PcbData.EventTime));
  PcbData.EventSuspend       = (char) fgetint();
  PcbData.EventStopUplds     = fgetlog();
  PcbData.EventSlide         = fgetlog();

  wasteit();
  wasteit();

  PcbData.ParallelPortNum    = (char) fgetint();

  for (x = SEC_A; x <= SEC_BATCH+3; x++)
    wasteit();

  PcbData.ModemDelay            = (short) fgetint();
  wasteit();
  PcbData.IrqNum                = (char)  fgetint();
  PcbData.BaseAddress           = (short) fgethex();
  wasteit();

  if (memcmp(PcbData.ModemPort,"COM1",4) == 0) {
    PcbData.IrqNum = 4;
    PcbData.BaseAddress = 0x3F8;
  } else if (memcmp(PcbData.ModemPort,"COM2",4) == 0) {
    PcbData.IrqNum = 3;
    PcbData.BaseAddress = 0x2F8;
  } else if (memcmp(PcbData.ModemPort,"PORT",4) == 0) {
    PcbData.IrqNum = 0;
    PcbData.BaseAddress = 0;
  }

  dosfclose(&pcbfile);
}



void LIBENTRY readdatfile(void) {
  if (dosfopen(DatFile,OPEN_READ|OPEN_DENYNONE,&pcbfile) == -1)
    errorexittodos("Unable to open PCBOARD.DAT");

  fgetstr(PcbData.Version,sizeof(PcbData.Version));
  if (strstr(PcbData.Version,"14.5") == NULL)
    errorexittodos("Wrong version of PCBOARD.DAT file");

  read145file();
}
